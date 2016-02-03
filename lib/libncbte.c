/* -*- Mode: C; c-basic-offset:8 ; indent-tabs-mode:t ; -*- */
/*
 * Copyright (C) 2008-2015 Numascale AS, support@numascale.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <emmintrin.h>

#include "libncbte.h"
#include "ncbte_io.h"

#ifdef DEBUG
#define DEBUG_STATEMENT(x) x
#else
#define DEBUG_STATEMENT(x)
#endif

#ifndef MIN
#define MIN(a,b) ((a < b) ? a : b)
#endif

#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif

#ifndef HUGE_PAGE_SIZE
#define HUGE_PAGE_SIZE (2 * 1024 * 1024)
#endif

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

#ifndef DIV_ROUND_UP
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#endif

#define BTCE_ALIGN_CHECK(x) ((x) & 3)

struct ncbte_context {
	int   devfd;
	void *bte_io;
};

struct ncbte_region {
	struct ncbte_mem mem;
	int              allocated;
	int              is_continous;
};

struct ncbte_completion {
	__u64  queue_num;
	__u64  fence_paddr;
	__u32 *fence;
};

typedef enum {
	REM_WRITE = 0,
	REM_READ  = 1
} btce_direction_t;

typedef union {
	__m128i m128;
	struct {
		__u64 lo;
		__u64 hi;
	} __attribute__ ((packed)) m64;
	struct {
		__u64 local_addr:46;
		__u64 remote_addr:46;
		__u64 dw_cnt:16;
		__u64 unused:16;
		__u64 fence:1;
		__u64 move:1;
		__u64 direction:1;
		__u64 valid:1;
	} __attribute__ ((packed)) s;
} __attribute__ ((packed, aligned(16))) btce_t;

static int check_continous(struct ncbte_mem *mem)
{
	__u64 start_phys_addr = mem->phys_addr[0];
	unsigned page;
	for (page = 0; page < mem->nr_pages; page++) {
		__u64 expected_phys_addr = start_phys_addr + page*PAGE_SIZE;
		__u64 current_phys_addr = mem->phys_addr[page];
		if (current_phys_addr != expected_phys_addr)
			return 0;
	}
	return 1;
}

/**
 * ncbte_alloc_region() - Allocate and pin a user region of variable size
 * @context:	Current BTE context
 * @ptr:	Pointer to user virtual address if buffer is pre-allocated, else NULL
 * @length:	Size of buffer
 * @flags:	Flags, use NCBTE_ALLOCATE_HUGEPAGE if huge pages are desired
 * @regionp:	Pointer to region structure, allocated and initialized by function
 *
 * This function will allocate and pin a memory buffer of desired size if @vaddrp is not already
 * initialized with a value. If @vaddrp already points to a valid (and committed) buffer
 * address, it will only try to pin the memory.
 *
 * Return: Virtual address of the buffer allocated (or as given by the @ptr argument) if
 * successful, or NULL if error occured.
 */
void *ncbte_alloc_region(struct ncbte_context *context, void *ptr, size_t length, int flags, struct ncbte_region **regionp)
{
	struct ncbte_region *region;
	void *vaddr = ptr;

	if (!context || !regionp)
		return NULL;

	region = (struct ncbte_region *)malloc(sizeof(*region));
	if (!region) {
		fprintf(stderr,"malloc failed: %s\n", strerror(errno));
		return NULL;
	}
	memset(region, 0, sizeof(*region));

	/* No region was given, allocate one */
	if (!ptr) {
		const size_t alignment = (flags & NCBTE_ALLOCATE_HUGEPAGE) ? HUGE_PAGE_SIZE : PAGE_SIZE;
		if (posix_memalign(&vaddr, alignment, length) != 0) {
			fprintf(stderr, "posix_memalign failed: %s\n", strerror(errno));
			goto err;
		}

		region->allocated = 1;

		if (flags & NCBTE_ALLOCATE_HUGEPAGE)
			(void)madvise(vaddr, length, MADV_HUGEPAGE);

		memset(vaddr, 0, length);
	} else if ((__u64)ptr & (PAGE_SIZE-1)) {
		fprintf(stderr, "virtual address not page aligned %p\n", ptr);
		goto err;
	}

	region->mem.addr = (__u64)vaddr;
	region->mem.size = (__u64)length;
	region->mem.nr_pages = DIV_ROUND_UP(length, PAGE_SIZE);
	region->mem.phys_addr = (__u64 *)malloc(region->mem.nr_pages*sizeof(__u64));
	if (!region->mem.phys_addr) {
		fprintf(stderr,"malloc failed: %s\n", strerror(errno));
		goto err;
	}

	if (ioctl(context->devfd, NCBTE_PIN_MEM, &region->mem) < 0) {
		fprintf(stderr, "ioctl(NCBTE_PIN_MEM) failed: %s\n", strerror(errno));
		goto err;
	}

	region->is_continous = check_continous(&region->mem);

	*regionp = region;

	return vaddr;

err:
	if (region->mem.phys_addr != 0)
		free((void *)region->mem.phys_addr);
	if (region->allocated && (region->mem.addr != 0))
		free((void *)region->mem.addr);
	free(region);

	return NULL;
}

/**
 * ncbte_free_region() - Unpin and free a user region
 * @context:	Current BTE context
 * @region:	Pointer to region structure as returned by ncbte_alloc_region()
 *
 * This function will unpin and free a user buffer if it was allocated
 * by a previous call to ncbte_alloc_region(). If the user buffer was pre-allocated
 * it is the callers responsibility to free that memory, it will still be unpinned
 * by this function however and the resources tied to the region structure freed.
 *
 * Return: 0 if successful, or -1 of error occured.
 */
int ncbte_free_region(struct ncbte_context *context, struct ncbte_region *region)
{
	if (!context || !region)
		return -1;

	if (ioctl(context->devfd, NCBTE_UNPIN_MEM, &region->mem) < 0) {
		fprintf(stderr, "ioctl(NCBTE_UNPIN_MEM) failed: %s\n", strerror(errno));
		return -1;
	}

	if (region->mem.phys_addr != 0)
		free((void *)region->mem.phys_addr);
	if (region->allocated && (region->mem.addr != 0))
		free((void *)region->mem.addr);
	free(region);

	return 0;
}

static inline __u64 _get_region_phys_addr(struct ncbte_region *region, off_t offset, size_t *lengthp)
{
	unsigned page;

	if (region->is_continous) {
		*lengthp = region->mem.size - offset;
		return region->mem.phys_addr[0] + offset;
	}

	*lengthp = 0;

	for (page = 0; page < region->mem.nr_pages && offset > 0; page++) {
		if (offset <= PAGE_SIZE-4)
			break;
		offset -= PAGE_SIZE;
	}

	/* offset out of range */
	if (page == region->mem.nr_pages)
		return 0;

	*lengthp = PAGE_SIZE - offset;
	return region->mem.phys_addr[page] + offset;
}

#define SETUP_BTCE(iter, curr, lim)					\
	do {								\
		btce_t btce;						\
		__u32 ndwords = ((curr) > (lim)) ? (lim) : (curr);	\
		__u64 btce_addr = (__u64)context->bte_io+comp_offset+((iter)*sizeof(btce_t)); \
		btce.m128 = _mm_setzero_si128();			\
		btce.s.valid = 1;					\
		btce.s.direction = direction;				\
		btce.s.dw_cnt = ndwords - 1;				\
		btce.s.remote_addr = remote_addr >> 2;			\
		btce.s.local_addr = local_addr >> 2;			\
		DEBUG_STATEMENT(printf("btce "#curr": remote %016"PRIx64", local %016"PRIx64", dw_cnt %x\n", \
				       (uint64_t)remote_addr, (uint64_t)local_addr, ndwords)); \
		remote_addr += ndwords<<2;				\
		local_addr += ndwords<<2;				\
		(curr) -= ndwords;					\
		_mm_store_si128((__m128i *)btce_addr, btce.m128);       \
	} while(0)

#define UPDATE_LENGTH_OFFSET(x)			\
	do {					\
		length -= (x)<<2;		\
		local_offset += (x)<<2;		\
		remote_offset += (x)<<2;	\
	} while(0)

static int ncbte_transfer_region(struct ncbte_context *context,
				 struct ncbte_region *local_region, off_t local_offset,
				 struct ncbte_region *remote_region, off_t remote_offset,
				 size_t length, btce_direction_t direction,
				 struct ncbte_completion *completion)
{
	const __u32 max_btce_dwords = (1<<16);
	const __u64 comp_offset = (completion) ? (completion->queue_num << 12) : 0;

	if (!context || !local_region || !remote_region)
		return -1;

	if (((local_offset+length) > local_region->mem.size) ||
	    ((remote_offset+length) > remote_region->mem.size) ||
	    ((local_offset & 0x3f) != (remote_offset & 0x3f)) ||
	    BTCE_ALIGN_CHECK(local_offset) ||
	    BTCE_ALIGN_CHECK(length))
		return -1;

	do {
		__u64 local_addr, remote_addr;
		size_t local_length, remote_length;
		__u32 head, bulk, tail;
		unsigned i, num_btce;

		local_addr = _get_region_phys_addr(local_region, local_offset, &local_length);
		remote_addr = _get_region_phys_addr(remote_region, remote_offset, &remote_length);
		if ((local_length == 0) || (remote_length == 0)) {
			fprintf(stderr, "local or remote region segment size is not supported!\n");
			return -1;
		}

		bulk = MIN(MIN(local_length, remote_length), length) >>2;

		DEBUG_STATEMENT(printf("bulk %d, loffset %"PRIx64", roffset %"PRIx64", llength %"PRIx64", rlength %"PRIx64", length %"PRIx64"\n",
				       bulk, (uint64_t)local_offset, (uint64_t)remote_offset,
				       (uint64_t)local_length, (uint64_t)remote_length, length));

		/* Head alignment */
		head = (local_addr & 0x3f) >> 2;
		if (head) {
			head = 0x10 - head; /* Align up to next 64b block */
			bulk -= head;
			UPDATE_LENGTH_OFFSET(head);
			num_btce = DIV_ROUND_UP(head, 4);
			DEBUG_STATEMENT(printf("head = %x, num_btce = %x\n", head, num_btce));
			for (i=0; i<num_btce && head > 0; i++)
				SETUP_BTCE(i, head, 4);
			_mm_sfence();
		}

		/* Shave off the tail before we do bulk */
		tail = bulk & 0xf;
		bulk -= tail;

		/* Do bulk transfer in as long blocks as we can (64b aligned) */
		if (bulk) {
			UPDATE_LENGTH_OFFSET(bulk);
			num_btce = DIV_ROUND_UP(bulk, max_btce_dwords);
			for (i=0; i<num_btce && bulk > 0; i++)
				SETUP_BTCE(i, bulk, max_btce_dwords);
			_mm_sfence();
		}

		/* Tail alignment */
		if (tail) {
			UPDATE_LENGTH_OFFSET(tail);
			num_btce = DIV_ROUND_UP(tail, 4);
			DEBUG_STATEMENT(printf("tail = %x, num_btce = %x\n", tail, num_btce));
			for (i=0; i<num_btce && tail > 0; i++)
				SETUP_BTCE(i, tail, 4);
			_mm_sfence();
		}
	} while (length > 0);

	if (completion && completion->fence) {
		btce_t btce;
		__u64 btce_addr = (__u64)context->bte_io+comp_offset;
		*completion->fence = -1;
		btce.m128 = _mm_setzero_si128();
		btce.s.valid = 1;
		btce.s.fence = 1;
		btce.s.local_addr = completion->fence_paddr >> 2;
		DEBUG_STATEMENT(printf("btce fence: addr %016"PRIx64"\n",
				       (uint64_t)local_addr, ndwords));
		_mm_store_si128((__m128i *)btce_addr, btce.m128);
		_mm_sfence();
	}

	return 0;
}

/**
 * ncbte_write_region() - Read from local region and write to remote region
 * @context:		Current BTE context
 * @local_region:	Pointer to local region structure as returned by ncbte_alloc_region()
 * @local_offset:	Offset into the local region where transfer should start
 * @remote_region:	Pointer to remote region structure as returned by ncbte_alloc_region()
 * @remote_offset:	Offset into the remote region where transfer should start
 * @length:		Length of the transfer
 * @completion:		Optional pointer to completion structure as returned by ncbte_alloc_completion()
 *
 * This function will start a BTE transfer that reads from the local_region and writes to
 * the remote_region. If a @completion pointer is provided, this object will be signalled
 * upon succesful transfer.
 *
 * Return: 0 if successful, or -1 if error occured.
 */
int ncbte_write_region(struct ncbte_context *context,
		       struct ncbte_region *local_region, off_t local_offset,
		       struct ncbte_region *remote_region, off_t remote_offset,
		       size_t length,
		       struct ncbte_completion *completion)
{
	return ncbte_transfer_region(context, local_region, local_offset,
				     remote_region, remote_offset, length, REM_WRITE, completion);
}

/**
 * ncbte_read_region() - Read from remote region and write to local region
 * @context:		Current BTE context
 * @local_region:	Pointer to local region structure as returned by ncbte_alloc_region()
 * @local_offset:	Offset into the local region where transfer should start
 * @remote_region:	Pointer to remote region structure as returned by ncbte_alloc_region()
 * @remote_offset:	Offset into the remote region where transfer should start
 * @length:		Length of the transfer
 * @completion:		Optional pointer to completion structure as returned by ncbte_alloc_completion()
 *
 * This function will start a BTE transfer that reads from the remote_region and writes to
 * the local_region. If a @completion pointer is provided, this object will be signalled
 * upon succesful transfer.
 *
 * Return: 0 if successful, or -1 if error occured.
 */
int ncbte_read_region(struct ncbte_context *context,
		      struct ncbte_region *local_region, off_t local_offset,
		      struct ncbte_region *remote_region, off_t remote_offset,
		      size_t length,
		      struct ncbte_completion *completion)
{
	return ncbte_transfer_region(context, local_region, local_offset,
				     remote_region, remote_offset, length, REM_READ, completion);
}

/**
 * ncbte_alloc_completion() - Allocate a completion object
 * @context:	Current BTE context
 * @flags:	Length of the transfer
 *
 * This function will allocate a completion object that can be used in read/write
 * transfers and later in ncbte_check_completion() and ncbte_wait_completion().
 *
 * Return: Pointer to the completion object if successful, or NULL if error occured.
 */
struct ncbte_completion *ncbte_alloc_completion(struct ncbte_context *context, int UNUSED(flags))
{
	struct ncbte_completion *completion;

	if (!context)
		return NULL;

	completion = (struct ncbte_completion *)malloc(sizeof(*completion));
	if (!completion) {
		fprintf(stderr,"malloc failed: %s\n", strerror(errno));
		return NULL;
	}
	memset(completion, 0, sizeof(*completion));


	return completion;
}

/**
 * ncbte_free_completion() - Free a completion object
 * @context:	Current BTE context
 * @completion:	Pointer to completion structure as returned by ncbte_alloc_completion()
 *
 * This function will free a completion structure allocated by a previous call to
 * ncbte_alloc_region(). If the completion object has pending transactions, this call
 * will block until they complete or are interrupted.
 *
 * Return: 0 if successful, or -1 if error occured.
 */
int ncbte_free_completion(struct ncbte_context *context, struct ncbte_completion *completion)
{
	if (!context || !completion)
		return -1;

	free(completion);

	return 0;
}

static inline __u64 _get_completion_status(void *bte_io, __u64 qword)
{
	return *(__u64 *)((__u64)bte_io + (qword<<3));
}

/**
 * ncbte_check_completion() - Check a completion object for completion event
 * @context:	Current BTE context
 * @completion:	Pointer to completion structure as returned by ncbte_alloc_completion()
 *
 * This function will check completion structure for pending events.
 *
 * Return: 1 if no pending events, 0 if there still are pending events, or -1 if errors.
 */
int ncbte_check_completion(struct ncbte_context *context, struct ncbte_completion *completion)
{
	if (!context || !completion)
		return -1;

	if (!completion->fence) {
		__u64 comp = _get_completion_status(context->bte_io, completion->queue_num >> 6);
		if (comp & (1ULL << (completion->queue_num & 63)))
			return 1;
		return 0;
	}
	else
		return *completion->fence == 0;
}

/**
 * ncbte_wait_completion() - Wait for completion evetns on a completion object
 * @context:	Current BTE context
 * @completion:	Pointer to completion structure as returned by ncbte_alloc_completion()
 *
 * This function will check a completion structure for pending events and wait until
 * all pending events complete.
 */
void ncbte_wait_completion(struct ncbte_context *context, struct ncbte_completion *completion)
{
	for (;;) {
		if (ncbte_check_completion(context, completion)) break;
		asm volatile("pause" ::: "memory");
	}
}

/**
 * ncbte_open - Initialize BTE for use
 */
struct ncbte_context *ncbte_open(int UNUSED(flags))
{
	const char *devname = "/dev/"NCBTE_DEVNAME;
	struct ncbte_context *context;

	context = (struct ncbte_context *)malloc(sizeof(*context));
	if (!context) {
		fprintf(stderr,"malloc failed: %s\n", strerror(errno));
		return NULL;
	}

	context->devfd = open(devname, O_RDWR);
	if (context->devfd < 0) {
		fprintf(stderr, "Unable to open %s: %s\n", devname, strerror(errno));
		goto err;
	}

	context->bte_io = (char *)mmap(NULL, NCBTE_MAX_MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, context->devfd, 0);
	if (context->bte_io == MAP_FAILED) {
		fprintf(stderr, "Mmap failed: %s\n", strerror(errno));
		goto err;
	}

	return context;

err:
	if (context->devfd >= 0)
		close(context->devfd);

	free(context);

	return NULL;
}

/**
 * ncbte_close - Release BTE
 */
int ncbte_close(struct ncbte_context *context)
{
	munmap(context->bte_io, NCBTE_MAX_MMAP_SIZE);
	close(context->devfd);

	free(context);

	return 0;
}