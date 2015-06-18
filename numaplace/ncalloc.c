/*
 * Id: $Id$
 *
 * Copyright © 2010 Numascale AS, Oslo, Norway
 * Author:
 *
 * Distribution restricted.
 *
 */

#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <dlfcn.h>
#include <limits.h>
#include <stdarg.h>
#include <malloc.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/syscall.h>

#ifndef __MALLOC_HOOK_VOLATILE
    #define __MALLOC_HOOK_VOLATILE
#endif

#ifndef MAP_HUGETLB
	#define MAP_HUGETLB	0x80000
#endif


#define HUGE_PAGE_SIZE (2 * 1024 * 1024)
#define MAX_THREADS 4096
#define BLKSIZE HUGE_PAGE_SIZE
#define BINCNT 48

// leave some space for hdr_t and hdr_t* and keep 16 byte aligned
#define SMALL_MAX ((BLKSIZE - 64) / 2 - 64)
#define BIG_BLK_MAP_TRESH  16  // number of unused big blocks to keep mapped
#define PAGE_BLK_MAP_TRESH 64  // total number of unused blocks to keep mapped

static void* (*gnu_malloc) (size_t, const void *);
static void  (*gnu_free) (void *, const void *);
static void* (*gnu_realloc)(void *ptr, size_t size, const void *caller);
static void* (*gnu_memalign)(size_t alignment, size_t size, const void *caller);

// memory block header, every block starts with this
typedef struct blk {
	int32_t small;		// flag indicating small block / big block
    int32_t tid;        // thread index
    int32_t binndx;     // owning bin index
    int32_t restcnt;    // rest chunks count
    int32_t freecnt;    // free chunks count
    int32_t cnt;        // total chunk count
    void*   rest;       // pointer to rest of free space
    void**  free;       // list of pointers to free chunks
    size_t  size;       // binsize including header hdr_t
    struct blk* prev;   // pointer to previous block descripter
    struct blk* next;   // pointer to next block descripter
} blk_t;


// header prepended to all buffers returned be malloc()
// must be returned be free()
typedef struct {
    blk_t* blk;         // pointer to owning block
    size_t alignment;   // buffer alignment set by memalign()
} hdr_t;


// statistics record, one instance per heap used
typedef struct {
	size_t   mapped;		// total mapped by os
	size_t   allocated;		// total allocated (incl. header and adjustments to bigger size)
	size_t   mcalls;		// malloc calls
	size_t   fcalls;		// free calls
	size_t   rcalls;		// realloc calls
	size_t   acalls;		// memalign calls
	uint64_t mticks;	    // malloc() tsc ticks
	uint64_t fticks;	    // free() tsc ticks
	size_t   interthrfree;	// inter-thread-free
	size_t   mmapcalls;     // number of mmap() calls
	size_t   unmapcalls;    // number of munmap() calls
    uint64_t dist[64];      // buffer size ditribution used for statistics output
} stat_t;


typedef struct {
	blk_t*   blkfree;		 // list of free blocks
	blk_t*	 biglruhead;   	 // head of recently used big blocks list
	blk_t*	 biglrutail;   	 // tail of recently used big blocks list
	blk_t*   binhead[BINCNT];//list bin heads, one for each bin size
	blk_t*   bintail[BINCNT];//list bin tails, one for each bin size
	stat_t*  stat;		     // statistics record
	int32_t  blkfreecnt;	 // count of mapped free blocks
	int32_t  biglrucnt;    	 // count least recently used free big blocks
	volatile int32_t lock;   // lock used in malloc and free calls
} heap_t;


static __thread int32_t __tid = -1;		// thread id (index within current process)
static __thread heap_t* __thrheap = 0;		// threads own heap

static stat_t* __stat = 0;			// global statistics record
static int32_t __goballock = 0;		// global lock used in heap construction/destruction
static int32_t __thrcount = 0;		// thread index counter
static heap_t* __allheaps[MAX_THREADS];  // pointer to all heaps
static size_t  __heapcreate = 0;	// heap create calls
static size_t  __heapexit = 0;  	// heap exit calls

static void null_free(void* ptr, const void* caller)
{
}

static void ncprintf(const char* function, int line, const char* form, ...)
{
	__malloc_hook = gnu_malloc;
	__free_hook   = null_free;

	va_list argptr;
	char cline[1024];

	va_start(argptr, form);
	vsprintf(cline, form, argptr);
	va_end(argptr);

	fprintf(stderr, "%s %d : %s", function, line, cline);
	fflush(stderr);
	exit(-1);
}

static uint64_t rdtscp(uint32_t* id) {
	uint32_t lo, hi;
	__asm__ __volatile__ ("rdtscp" : "=a" (lo), "=d" (hi), "=c" (*id));
	return (uint64_t)hi << 32 | lo;
}

static void semlock(volatile int32_t* v)
{
	while( !__sync_bool_compare_and_swap(v, 0, 1) ) {
		__asm__ __volatile__ ("pause");
	}
}


static void semunlock(volatile int32_t* v)
{
	*v = 0;
}


// integer 2-logarithm
static int ilog2(uint32_t size)
{
	int z = 0;
	__asm__ __volatile__ (
		"bsr %1, %%ebx\n"
		"movl %%ebx, %0\n"
		: "=r" (z) : "r" (size) : "ebx");
	return z;
}


static void setstatistics(const size_t size)
{
	if( size ) {
		// find bit position of msb in size
		uint64_t i = 0;
		__asm__ __volatile__ (
			"bsr %1, %%rbx\n"
			"movq %%rbx, %0\n"
			: "=r" (i) : "r" (size) : "rbx");

		if( size & (size - 1) ) {
			// size is not power of 2
			++i;
		}

		++__thrheap->stat->dist[i];
	}
}


static void* map(size_t size, bool huge)
{
	int fd = -1;
	int flags = MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE;
	if( huge ) {
		flags |= MAP_HUGETLB;
	}

	void* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, flags, fd, 0);
	if( huge && (ptr == MAP_FAILED) ) {
		flags &= ~MAP_HUGETLB;
		ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, flags, fd, 0);
	}
	if( !ptr || (ptr == MAP_FAILED) ) {
		ncprintf(__FUNCTION__, __LINE__, "mmap() allocation failed. terminating !\n");
	}

	return ptr;
}


static void init_heap()
{
	int size = sizeof(heap_t);
	size = ((size + 4095) & ~4095);
	__thrheap = (heap_t*)map(sizeof(heap_t), false);
	memset(__thrheap, 0, sizeof(heap_t));

	if( __stat ) {
		__thrheap->stat = (stat_t*)map(sizeof(stat_t), false);
		memset(__thrheap->stat, 0, sizeof(stat_t));
		__thrheap->stat->mapped = size;
	}

	uint32_t cpuid;
	rdtscp(&cpuid);

	semlock(&__goballock);

	if( __thrcount >= MAX_THREADS ) {
		fprintf(stderr, "thread count > MAX_THREADS, exiting !\n");
		exit(-1);
	}

	if( __thrcount == 0 ) {
		memset(__allheaps, 0, sizeof(__allheaps));
	}

	for( int i = 0; i < MAX_THREADS; i++ ) {
		if( !__allheaps[i] ) {
			__tid = i;
			__allheaps[i] = __thrheap;
			break;
		}
	}

	++__thrcount;
	++__heapcreate;

	semunlock(&__goballock);
}


static void nc_free_small(heap_t* thrheap, hdr_t* hdr, int size)
{
	blk_t* blk = hdr->blk;

	if( ++blk->freecnt < blk->cnt ) {

		--blk->free;
		*(hdr_t**)(blk->free) = hdr;

		// place block at head
		blk_t* prev = blk->prev;

		if( prev ) {
			blk_t* next = blk->next;
			prev->next = next;
			int binndx = blk->binndx;
			if( next ) {
				next->prev = prev;
			}
			else {
				thrheap->bintail[binndx] = prev;
			}
			blk_t* head = thrheap->binhead[binndx];
			thrheap->binhead[binndx] = blk;
			head->prev = blk;
			blk->prev = 0;
			blk->next = head;
		}
	}
	else {
		// block is completely free
		// take block out of bins linked list of used blocks
		blk_t* prev = blk->prev;
		blk_t* next = blk->next;

		if( prev ) {
			prev->next = next;
		}
		else {
			thrheap->binhead[blk->binndx] = next;
		}

		if( next ) {
			next->prev = prev;
		}
		else {
			thrheap->bintail[blk->binndx] = prev;
		}

		if( thrheap->blkfreecnt < PAGE_BLK_MAP_TRESH ) {
			// keep last PAGE_BLK_MAP_TRESH blocks mapped
			++thrheap->blkfreecnt;
			blk->next = thrheap->blkfree;
			thrheap->blkfree = blk;
		}
		else {
			// unmap block
			munmap(blk, BLKSIZE);

			if( thrheap->stat ) {
				thrheap->stat->mapped -= BLKSIZE;
				++thrheap->stat->unmapcalls;
			}
		}
	}
}


// map os memory
// size is multiple of huge page size
static blk_t* bklalloc(size_t size)
{
	blk_t* blk = (blk_t*)map(size + HUGE_PAGE_SIZE, true);

	// align block on huge page boundary
	uint64_t head = ((uint64_t)blk & (HUGE_PAGE_SIZE - 1));
	if( head ) {
		munmap(blk, head);
	}
	blk = (blk_t*)((void*)blk + head);
	munmap((void*)blk + size, HUGE_PAGE_SIZE - head);

	if( __thrheap->stat ) {
		++__thrheap->stat->mmapcalls;
	}

	blk->tid = __tid;

	if( __thrheap->stat ) {
		__thrheap->stat->mapped += size;
	}
	return blk;
}


static void initblk(blk_t* blk, int binsize)
{
	int cnt = (BLKSIZE - sizeof(blk_t)) / (binsize + sizeof(hdr_t*));

	blk->small = 1;
	blk->size = binsize;
	blk->tid = __tid;
	blk->free = (void**)((uint8_t*)blk + BLKSIZE);
	blk->restcnt = cnt;
	blk->freecnt = 0;
	blk->cnt = cnt;
	blk->rest = (uint8_t*)(blk + 1);
}


static void* nc_malloc_small(int size)
{
	int size0 = size + sizeof(hdr_t);

	int binndx = ilog2(size0);
	int binsize = (1 << binndx);
	binndx <<= 1;
	binndx -= 8;

	// bin sizes grow by half of power 2
	if( size0 > binsize ) {
		int bz0 = binsize;
		binsize += (bz0 >> 1);
		++binndx;

		// take care of size 24 for having buffer 16 byte aligned
		if( (size0 > binsize) || (binsize == 24) ) {
			binsize += (bz0 >> 1);
			++binndx;
		}
	}

	// adjust bin sizes of buffers > 64KB to make use of left over space
	if( size0 >= 64 * 1024 ) {
		if( size0 <= BLKSIZE / 4 ) {
			int maxcnt = BLKSIZE / (binsize + sizeof(hdr_t*));
			binsize = (BLKSIZE / maxcnt) - sizeof(hdr_t*);
			if( binsize & 0xf ) {
				binsize = (binsize + 0xf) & ~0xf;
			}
		}
		else {
			// leave some space for hdr_t and hdr_t* and keep 16 byte aligned
			binsize = SMALL_MAX + sizeof(hdr_t);
		}
	}

	semlock(&__thrheap->lock);

	hdr_t* hdr = 0;
	blk_t* blk = __thrheap->binhead[binndx];

	if( blk ) {
		assert( (blk->freecnt >= 0) && (blk->freecnt < blk->cnt) );
		assert( (blk->restcnt >= 0) && (blk->restcnt < blk->cnt) );

		if( blk->restcnt ) {
			--blk->restcnt;
			hdr = (hdr_t*)blk->rest;
			blk->rest += binsize;
			hdr->blk = blk;
			hdr->alignment = 0;
		}
		else
		if( blk->freecnt ) {
			--blk->freecnt;
			hdr = *(hdr_t**)(blk->free);
			++blk->free;
		}

		if( blk->restcnt + blk->freecnt == 0 ) {
			// place full block at tail
			blk_t* next = blk->next;
			if( next ) {
				blk_t* tail = __thrheap->bintail[binndx];
				__thrheap->bintail[binndx] = blk;
				__thrheap->binhead[binndx] = next;
				tail->next = blk;
				next->prev = 0;
				blk->prev = tail;
				blk->next = 0;
			}
		}
	}

	if( !hdr ) {
		blk = __thrheap->blkfree;

		if( blk ) {
			// reuse already mapped block
			__thrheap->blkfree = blk->next;
			--__thrheap->blkfreecnt;
		}
		else {
			// map new block
			blk = bklalloc(BLKSIZE);
		}

		initblk(blk, binsize);

		blk->binndx = binndx;

		--blk->restcnt;
		hdr = (hdr_t*)blk->rest;
		blk->rest += binsize;
		hdr->blk = blk;
		hdr->alignment = 0;

		// place block at head
		blk_t* head = __thrheap->binhead[binndx];
		blk->prev = 0;
		blk->next = head;
		if( head ) {
			head->prev = blk;
		}
		__thrheap->binhead[binndx] = blk;
		if( !__thrheap->bintail[binndx] ) {
			__thrheap->bintail[binndx] = blk;
		}
	}

	semunlock(&__thrheap->lock);

	return hdr + 1;
}


static void* nc_malloc_big(size_t size)
{
	blk_t* blk;

	size_t size0 = size + sizeof(blk_t) + sizeof(hdr_t);

	semlock(&__thrheap->lock);

	// try to reuse one of the recently used mapped blocks
	// find a block big enough
	blk = __thrheap->biglruhead;
	while( blk ) {
		if( blk->size >= size0 ) {
			break;
		}
		blk = blk->next;
	}

	if( blk ) {
		// reuse recently used mapped block
		--__thrheap->biglrucnt;

		blk_t* prev = blk->prev;
		blk_t* next = blk->next;

		if( prev ) {
			prev->next = next;
		}
		else {
			__thrheap->biglruhead = next;
		}

		if( next ) {
			next->prev = prev;
		}
		else {
			__thrheap->biglrutail = prev;
		}
	}
	else {
		size0 = ((size0 + (BLKSIZE - 1)) & ~(BLKSIZE - 1));
		blk = bklalloc(size0);

		blk->small = 0;
		blk->size = size0;
	}

	hdr_t* hdr = (hdr_t*)(blk + 1);
	hdr->blk = blk;
	hdr->alignment = 0;

	semunlock(&__thrheap->lock);

	return hdr + 1;
}


static void* nc_malloc(size_t size, const void* caller)
{
	uint32_t cpuid;
	uint64_t t0 = rdtscp(&cpuid);

	if( !__thrheap ) {
		init_heap();
	}

	void* buf;

	if( size <= SMALL_MAX ) {
		buf = nc_malloc_small((int)size);
	}
	else {
		buf = nc_malloc_big(size);
	}

	if( __thrheap->stat ) {
		__thrheap->stat->allocated += size;
		setstatistics(size);
		++__thrheap->stat->mcalls;
		uint64_t t1 = rdtscp(&cpuid);
		__thrheap->stat->mticks += (t1 - t0);
	}

	assert( (((uint64_t)buf) & 0xf) == 0 );

	return buf;
}


static void nc_free_big(heap_t* thrheap, hdr_t* hdr, size_t size)
{
	blk_t* blk = hdr->blk;

	// prepend block to recently used block list
	blk_t* head = thrheap->biglruhead;
	if( head ) {
		head->prev = blk;
	}
	blk->prev = 0;
	blk->next = head;
	thrheap->biglruhead = blk;

	if( thrheap->biglrucnt < BIG_BLK_MAP_TRESH ) {
		if( ++thrheap->biglrucnt == 1 ) {
			thrheap->biglrutail = blk;
		}
	}
	else {
		// unmap least recently used block from tail
		blk_t* tail = thrheap->biglrutail;

		if( thrheap->stat ) {
			thrheap->stat->mapped -= tail->size;
		}

		// remove tail from lru list
		thrheap->biglrutail = tail->prev;
		thrheap->biglrutail->next = 0;

		munmap(tail, tail->size);

		if( thrheap->stat ) {
			++thrheap->stat->unmapcalls;
		}
	}
}


static void nc_free(void* ptr, const void* caller)
{
	uint32_t cpuid;
	uint64_t t0 = rdtscp(&cpuid);

	if( !ptr ) {
		return;
	}

	if( !__thrheap ) {
		init_heap();
	}

	hdr_t* hdr = (hdr_t*)ptr - 1;
	blk_t* blk = hdr->blk;

	size_t size = blk->size;

	if( hdr->alignment ) {
		hdr = (hdr_t*)((size_t)hdr - hdr->alignment);
		hdr->alignment = 0;
	}

	if( blk->tid == __tid ) {

		semlock(&__thrheap->lock);

		if( blk->small ) {
			nc_free_small(__thrheap, hdr, (int)size);
		}
		else {
			nc_free_big(__thrheap, hdr, size);
		}

		if( __thrheap->stat ) {
	 		++__thrheap->stat->fcalls;
			uint64_t t1 = rdtscp(&cpuid);
			__thrheap->stat->fticks += (t1 - t0);
		}

		semunlock(&__thrheap->lock);
	}
	else {
		heap_t* thrheap = __allheaps[blk->tid];

		semlock(&thrheap->lock);

		if( blk->small ) {
			nc_free_small(thrheap, hdr, (int)size);
		}
		else {
			nc_free_big(thrheap, hdr, size);
		}

		if( thrheap->stat ) {
			++thrheap->stat->interthrfree;
			++thrheap->stat->fcalls;
			uint64_t t1 = rdtscp(&cpuid);
			thrheap->stat->fticks += (t1 - t0);
		}

		semunlock(&thrheap->lock);
	}
}


static void* nc_realloc(void* ptr, size_t newsize, const void* caller)
{
	if( !__thrheap ) {
		init_heap();
	}

	uint8_t* buf = ptr;

	if( ptr ) {
		hdr_t* hdr = (hdr_t*)ptr - 1;
		blk_t* blk = hdr->blk;

		size_t oldsize = blk->size - sizeof(hdr_t);  // size incl. hdr, maybe adjusted to new higher bin size
		if( !blk->small ) {
			oldsize -= sizeof(blk_t);
		}
		if( newsize > oldsize ) {
			buf = nc_malloc(newsize, caller);

			size_t sz = (oldsize <= newsize) ? oldsize : newsize;

			memcpy(buf, ptr, sz);

			nc_free(ptr, caller);
		}
	}
	else {
		buf = nc_malloc(newsize, caller);
	}

	return buf;
}


static void* nc_memalign(size_t alignment, size_t size, const void* caller)
{
	if( !__thrheap ) {
		init_heap();
	}

	// return if not power of 2 alignment
	if( (alignment == 0) || (alignment & (alignment - 1)) ) {
		return 0;
	}

	void* buf;

	if( alignment <= 16 ) {
		buf = nc_malloc(size, caller);
	}
	else {
		// the offset needs to be either 0 or at least sizeof(hdr_t)
		uint64_t offs = (uint64_t)((alignment >= sizeof(hdr_t)) ? alignment : sizeof(hdr_t));

		uint64_t p0 = (uint64_t)nc_malloc(offs + size, caller);

		uint64_t p1 = ((p0 + (offs - 1)) & ~(offs - 1));

		buf = (void*)p1;

		uint64_t shift = p1 - p0;

		if( shift > 0 ) {
			hdr_t* hdr = (hdr_t*)p0 - 1;
			hdr->alignment = (size_t)shift;

			*((hdr_t*)p1 - 1) = *hdr;
		}
	}

	return buf;
}


static void init_malloc_hooks(void)
{
	// save original
	gnu_malloc	 = __malloc_hook;
	gnu_free	 = __free_hook;
	gnu_realloc  = __realloc_hook;
	gnu_memalign = __memalign_hook;

	char* ev = getenv("NCALLOC_STATISTICS");
	if( ev && (!strcasecmp(ev, "yes") || !strcasecmp(ev, "1")) ) {
		__stat = (stat_t*)map(sizeof(stat_t), false);
	}

	// hook to the replacement functions
	__malloc_hook   = nc_malloc;
	__free_hook     = nc_free;
	__realloc_hook  = nc_realloc;
	__memalign_hook = nc_memalign;
}


static void listfree(blk_t* blk)
{
	while( blk ) {
		blk_t* next = blk->next;
		size_t sz = (blk->size < BLKSIZE) ? BLKSIZE : blk->size;
		munmap(blk, sz);
		blk = next;
	}
}


void exit_heap()
{
	if( __thrheap ) {

		semlock(&__thrheap->lock);
		semlock(&__goballock);

		__allheaps[__tid] = 0;
		__tid = -1;

		--__thrcount;
		++__heapexit;

		if( __thrheap->stat ) {
			stat_t* stats = __thrheap->stat;
			for( int i = 0; i < 64; i++ ) {
				__stat->dist[i] += stats->dist[i];
			}
			__stat->interthrfree += stats->interthrfree;
			__stat->mcalls += stats->mcalls;
			__stat->fcalls += stats->fcalls;
			__stat->rcalls += stats->rcalls;
			__stat->acalls += stats->acalls;

			__stat->mticks += stats->mticks;
			__stat->fticks += stats->fticks;

			munmap(__thrheap->stat, sizeof(stat_t));
			__thrheap->stat = 0;
		}

		semunlock(&__goballock);

		// unmap all mapped blocks
		for( int i = 0; i < BINCNT; i++ ) {
			listfree(__thrheap->binhead[i]);
		}

		listfree(__thrheap->blkfree);
		listfree(__thrheap->biglruhead);

		// unmap heap structure
		munmap(__thrheap, sizeof(heap_t));

		__thrheap = 0;
	}
}

static void printdist()
{
	// find largest buffer allocated
	int n0 = -1;
	int n1 = -1;
	for( int j = 0; j < 64; j++ ) {
		if( __stat->dist[j] > 0 ) {
			if( n0 == -1 ) {
				n0 = j;
			}
			if( j > n1 ) n1 = j;
		}
	}

	if( n0 < 0 || n1 < 0 ) {
		return;
	}

	// create strings for all size values distribution
	char val[64][32];

	for( int j = n0; j <= n1; j++ ) {
		sprintf(val[j], "%lld", (long long int)__stat->dist[j]);
	}

	static const char* hdr[64] = {
		"<=1", "<=2", "<=4", "<=8",
		"<=16", "<=32", "<=64", "<=128", "<=256", "<=512", "<=1K", "<=2K", "<=4K",
		"<=8K", "<=16K", "<=32K", "<=64K", "<=128K", "<=256K", "<=512k", "<=1M", "<=2M",
		"<=4M", "<=8M", "<=16M", "<=32M", "<=64M", "<=128M", "<=256M", "<=512M",
		"<=1G", "<=2G", "<=4G", "<=8G", "<=16G", "<=32G", "<=64G", "<=128G", "<=256G",
		"<=512G", "<=1T", "<=2T", "<=4T", "<=8T", "<=16T", "<=32T", "<=64T", "<=128T", "<=256T"};

	char unit[2048];
	char cnt[2048];
	unit[0] = '\0';
	cnt[0] = '\0';

	for( int j = n0; j <= n1; j++ ) {
		int w1 = strlen(hdr[j]);
		int w2 = strlen(val[j]);
		int w = (w1 >= w2) ? w1 : w2;
		++w;
		for( int i = 0; i < w - w1; i++ ) {
			strcat(unit, " ");
		}
		strcat(unit, hdr[j]);

		for( int i = 0; i < w - w2; i++ ) {
			strcat(cnt, " ");
		}
		strcat(cnt, val[j]);
	}

	printf("%s\n", unit);
	printf("%s\n", cnt);
	printf("\n");
}


static char* timadj(uint64_t* dt) {
	char* unit = "us";
	*dt /= 2600;
	if( *dt > 10000 ) {
		*dt /= 1000;
		unit = "ms";
		if( *dt > 10000 ) {
			*dt /= 1000;
			unit = "sec";
		}
	}
	return unit;
}


static void printstat()
{
	semlock(&__goballock);

	uint64_t itf = __stat->interthrfree;
	uint64_t mcalls = __stat->mcalls;
	uint64_t fcalls = __stat->fcalls;
	uint64_t rcalls = __stat->rcalls;
	uint64_t acalls = __stat->acalls;

	uint64_t mticks = __stat->mticks;
	uint64_t fticks = __stat->fticks;

	uint64_t mmapcalls = __stat->mmapcalls;
	uint64_t unmapcalls = __stat->unmapcalls;

	printf("\n");

	for( int i = 0; i < MAX_THREADS; i++ ) {
		if( __allheaps[i] ) {
			heap_t* heap = __allheaps[i];
			stat_t* stats = heap->stat;

			int64_t mapped = stats->mapped / (1024 * 1024);
			int64_t allocated = stats->allocated / (1024 * 1024);
			itf += stats->interthrfree;
			mcalls += stats->mcalls;
			fcalls += stats->fcalls;
			rcalls += stats->rcalls;
			acalls += stats->acalls;

			mmapcalls += stats->mmapcalls;
			unmapcalls += stats->unmapcalls;

			mticks += stats->mticks;
			fticks += stats->fticks;

			uint64_t dtm = stats->mticks;
			uint64_t dtf = stats->fticks;

			char* munit = timadj(&dtm);
			char* funit = timadj(&dtf);

			printf("heap %3d, mapped %4"PRId64" MB, allocated %4"PRId64" MB, malloc time %4d %s, free time %4d %s\n",
				i, mapped, allocated, (int)dtm, munit, (int)dtf, funit);

			for( int j = 0; j < 64; j++ ) {
				__stat->dist[j] += stats->dist[j];
			}
		}
	}

	printf("\ntotal combined values for all threads :\n\n");
	printf("malloc calls            %"PRId64"\n", mcalls);
	printf("free calls              %"PRId64"\n", fcalls);
	printf("realloc calls           %"PRId64"\n", rcalls);
	printf("memalign calls          %"PRId64"\n", acalls);
	printf("create heap calls       %"PRId64"\n", __heapcreate);
	printf("create exit calls       %"PRId64"\n", __heapexit);
	printf("inter thread free calls %"PRId64"\n", itf);

	printf("mmap() calls            %"PRId64"\n", mmapcalls);
	printf("unmap() calls           %"PRId64"\n", unmapcalls);

	char* munit = timadj(&mticks);
	char* funit = timadj(&fticks);

	printf("\n");
	printf("malloc time             %4d %s\n", (int)mticks, munit);
	printf("free time               %4d %s\n", (int)fticks, funit);
	printf("\n");

	semunlock(&__goballock);

	printdist();
}


// library unload
__attribute__((destructor)) static void unload(void)
{
	if( __stat ) {
		printstat();
	}
}

void (*__MALLOC_HOOK_VOLATILE __malloc_initialize_hook)(void) = init_malloc_hooks;
