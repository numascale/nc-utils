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

#define NC2_BTCE_COMPLETION 0x10000
#define NC2_BTCE_HEADTAIL   0x10400

struct ncbte_context {
	int      devfd;
	void    *bte_io;
};

struct ncbte_region {
	struct ncbte_mem      mem;
	int                   allocated;
	int                   is_continous;
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
		__u64 unused:17;
		__u64 move:1;
		__u64 direction:1;
		__u64 valid:1;
	} __attribute__ ((packed)) s;
} __attribute__ ((packed, aligned(16))) btce_t;

static int check_continous(struct ncbte_mem *mem)
{
	__u64 start_phys_addr = mem->phys_addr[0];
	int page;
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

static inline __u64 get_completion_status(void *bte_io)
{
	return *(__u64 *)((__u64)bte_io + NC2_BTCE_COMPLETION);
}

static inline __u64 get_head_tail_status(void *bte_io)
{
	return *(__u64 *)((__u64)bte_io + NC2_BTCE_HEADTAIL);
}

static inline void commit_btce(void *bte_io, int btce_num, const btce_t *btcep)
{
	_mm_store_si128((__m128i *)((__u64)bte_io+(btce_num&3)*sizeof(btce_t)), btcep->m128);
}

#define WORKAROUND_ALIGN_BUG 1

static int ncbte_transfer_region(struct ncbte_context *context,
				 struct ncbte_region *local_region, off_t local_offset,
				 struct ncbte_region *remote_region, off_t remote_offset,
				 size_t length, btce_direction_t direction,
				 struct ncbte_completion **completionp)
{
	const __u32 max_btce_dwords = (1<<16);
	__u64 local_addr, remote_addr;
	__u32 ndwords;
#ifdef WORKAROUND_ALIGN_BUG
	__u32 rest;
#endif
	int i, num_btce;

	if (!context || !local_region || !remote_region)
		return -1;

	if (((local_offset+length) > local_region->mem.size) ||
	    ((remote_offset+length) > remote_region->mem.size) ||
	    BTCE_ALIGN_CHECK(local_offset) ||
	    BTCE_ALIGN_CHECK(local_offset) ||
	    BTCE_ALIGN_CHECK(length))
		return -1;

	if (!local_region->is_continous) {
		fprintf(stderr, "local region is not physically continous, not yet supported!\n");
		return -1;
	}

	if (!remote_region->is_continous) {
		fprintf(stderr, "to region is not physically continous, not yet supported!\n");
		return -1;
	}

	ndwords = length>>2;
#ifdef WORKAROUND_ALIGN_BUG
	// Align bulk transfer to 64 bytes (16 dwords) because of known BTE issue
	rest = ndwords & 0xf;
	ndwords -= rest;
#endif
	local_addr = local_region->mem.phys_addr[0] + local_offset;
	remote_addr = remote_region->mem.phys_addr[0] + remote_offset;
	num_btce = DIV_ROUND_UP(ndwords, max_btce_dwords);

	for (i=0; i<num_btce && ndwords > 0; i++) {
		btce_t btce;
		__u32 curr_dwords = (ndwords > max_btce_dwords) ? max_btce_dwords : ndwords;
		btce.m128 = _mm_setzero_si128();
		btce.s.valid = 1;
		btce.s.direction = direction;
		btce.s.move = 0;
		btce.s.dw_cnt = curr_dwords - 1;
		btce.s.remote_addr = remote_addr >> 2; // dword-aligned
		btce.s.local_addr = local_addr >> 2; // dword-aligned

		DEBUG_STATEMENT(printf("full btce: remote %016"PRIx64", local %016"PRIx64", dw_cnt %x\n",
				       (uint64_t)remote_addr, (uint64_t)local_addr, curr_dwords));

		remote_addr += curr_dwords<<2;
		local_addr += curr_dwords<<2;
		ndwords -= curr_dwords;

		commit_btce(context->bte_io, i, &btce);
	}
#ifdef WORKAROUND_ALIGN_BUG
	if (rest) {
		/* Transfer the rest using up to 16-byte blocks */
		ndwords = rest;
		num_btce = DIV_ROUND_UP(ndwords, 4);
		DEBUG_STATEMENT(printf("rest = %x, num_btce = %x\n", rest, num_btce));
		for (i=0; i<num_btce && ndwords > 0; i++) {
			btce_t btce;
			__u32 curr_dwords = (ndwords > 4) ? 4 : ndwords;
			btce.m128 = _mm_setzero_si128();
			btce.s.valid = 1;
			btce.s.direction = direction;
			btce.s.move = 0;
			btce.s.dw_cnt = curr_dwords - 1;
			btce.s.remote_addr = remote_addr >> 2; // dword-aligned
			btce.s.local_addr = local_addr >> 2; // dword-aligned

			DEBUG_STATEMENT(printf("partial btce: remote %016"PRIx64", local %016"PRIx64", dw_cnt %x\n",
					       (uint64_t)remote_addr, (uint64_t)local_addr, curr_dwords));

			remote_addr += curr_dwords<<2;
			local_addr += curr_dwords<<2;
			ndwords -= curr_dwords;

			commit_btce(context->bte_io, i, &btce);
		}
	}
#endif
	_mm_sfence();

	/* TODO: add proper per-transfer completion mechanisms */
	if (completionp)
		*completionp = NULL;

	return 0;
}

int ncbte_write_region(struct ncbte_context *context,
		       struct ncbte_region *local_region, off_t local_offset,
		       struct ncbte_region *remote_region, off_t remote_offset,
		       size_t length,
		       struct ncbte_completion **completionp)
{
	return ncbte_transfer_region(context, local_region, local_offset,
				     remote_region, remote_offset, length, REM_WRITE, completionp);
}

int ncbte_read_region(struct ncbte_context *context,
		      struct ncbte_region *local_region, off_t local_offset,
		      struct ncbte_region *remote_region, off_t remote_offset,
		      size_t length,
		      struct ncbte_completion **completionp)
{
	return ncbte_transfer_region(context, local_region, local_offset,
				     remote_region, remote_offset, length, REM_READ, completionp);
}

int ncbte_check_completion(struct ncbte_context *context, struct ncbte_completion *UNUSED(completion))
{
	__u64 comp = get_completion_status(context->bte_io);
	if (comp == 0xffffffffffffffffULL)
		return 1;
	return 0;
}

void ncbte_wait_completion(struct ncbte_context *context, struct ncbte_completion *UNUSED(completion))
{
	for (;;) {
		__u64 comp = get_completion_status(context->bte_io);
		if (comp == 0xffffffffffffffffULL) break;
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
