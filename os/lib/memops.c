// $Id:$
// This source code including any derived information including but
// not limited by net-lists, fpga bit-streams, and object files is the
// confidential and proprietary property of
//
// Numascale AS
// Enebakkveien 302A
// NO-1188 Oslo
// Norway
//
// Any unauthorized use, reproduction or transfer of the information
// provided herein is strictly prohibited.
//
// Copyright © 2008-2012
// Numascale AS Oslo, Norway.
// All Rights Reserved.
//

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "numachip_memops.h"

/* Branch prediction macros */
#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)

static void _memops_lib_constructor(void) __attribute__((constructor));

#define ALIGN_DEST(align)					\
    do {							\
	i = ((~(unsigned long)to+1) & ((align)-1)) >> 2;	\
	for (; i>0 && n>0; i--) {				\
	    asm volatile (					\
		"movl (%0), %%eax\n\t"				\
		"movnti %%eax, (%1)\n\t"			\
		: : "r" (from), "r" (to) : "eax", "memory");	\
	    from+=4;						\
	    to+=4;						\
	    n-=4;						\
	}							\
    } while (0)

#define COPY_TAIL()						\
    do {							\
	i = n >> 3;						\
	n &= 7;							\
	for (; i>0; i--) {					\
	    asm volatile (					\
		"movq (%0), %%rax\n\t"				\
		"movnti %%rax, (%1)\n\t"			\
		: : "r" (from), "r" (to) : "rax", "memory");	\
	    from+=8;						\
	    to+=8;						\
	}							\
								\
	i = n >> 2;						\
	n &= 3;							\
	if (n) i++;						\
	for (; i>0; i--) {					\
	    asm volatile (					\
		"movl (%0), %%eax\n\t"				\
		"movnti %%eax, (%1)\n\t"			\
		: : "r" (from), "r" (to) : "eax", "memory");	\
	    from+=4;						\
	    to+=4;						\
	}							\
    } while(0)

/**
 * _block_nt_copy - Block copy function using SSE2 (64bit) non-temporal stores.
 * @to: mandatory. Provides the destination pointer.
 * @from: mandatory. Provides the source pointer.
 * @n: mandatory. Provides the number of bytes to copy.
 */
static void _block_nt_copy(void *to, const void *from, size_t n)
{
    int32_t i;

    /* Align destination to 8 bytes */
    ALIGN_DEST(8);

    /* Copy blocks, 64 bytes per loop with prefetching using 64bit instructions */
    i = n >> 6;
    n &= 63;
    for (; i>0; i--) {
	asm volatile (
	    "prefetchnta 320(%0)\n\t"
	    "movq   (%0), %%rax\n\t"
	    "movq  8(%0), %%r14\n\t"
	    "movnti %%rax,   (%1)\n\t"
	    "movnti %%r14,  8(%1)\n\t"
	    "movq 16(%0), %%rax\n\t"
	    "movq 24(%0), %%r14\n\t"
	    "movnti %%rax, 16(%1)\n\t"
	    "movnti %%r14, 24(%1)\n\t"
	    "movq 32(%0), %%rax\n\t"
	    "movq 40(%0), %%r14\n\t"
	    "movnti %%rax, 32(%1)\n\t"
	    "movnti %%r14, 40(%1)\n\t"
	    "movq 48(%0), %%rax\n\t"
	    "movq 56(%0), %%r14\n\t"
	    "movnti %%rax, 48(%1)\n\t"
	    "movnti %%r14, 56(%1)\n\t"
	    : : "r" (from), "r" (to) : "rax", "r14", "memory");	\
	from+=64;
	to+=64;
    }

    COPY_TAIL();
}

/**
 * _block_xmm_copy - Block copy function using SSE2 (128bit) non-temporal stores.
 * @to: mandatory. Provides the destination pointer.
 * @from: mandatory. Provides the source pointer.
 * @n: mandatory. Provides the number of bytes to copy.
 */
static void _block_xmm_copy(void *to, const void *from, size_t n)
{
    int32_t i;

    /* Align destination to 16 bytes */
    ALIGN_DEST(16);

    /* Copy blocks, 64 bytes per loop with prefetching using 128bit XMM instructions */
    i = n >> 6;
    n &= 63;
    if (likely(!((unsigned long)from & 15))) {
	/* Source aligned */
	for (; i>0; i--) {
	    asm volatile (
		"prefetchnta 320(%0)\n\t"
		"movdqa   (%0), %%xmm0\n\t"
		"movdqa 16(%0), %%xmm1\n\t"
		"movntdq %%xmm0,   (%1)\n\t"
		"movntdq %%xmm1, 16(%1)\n\t"
		"movdqa 32(%0), %%xmm0\n\t"
		"movdqa 48(%0), %%xmm1\n\t"
		"movntdq %%xmm0, 32(%1)\n\t"
		"movntdq %%xmm1, 48(%1)\n\t"
		: : "r" (from), "r" (to) : "memory");
	    from+=64;
	    to+=64;
	}

	i = n >> 4;
	n &= 15;
	for (; i>0; i--) {
	    asm volatile (
		"movdqa (%0), %%xmm0\n\t"
		"movntdq %%xmm0, (%1)\n\t"
		: : "r" (from), "r" (to) : "memory");
	    from+=16;
	    to+=16;
	}
    } else {
	/* Source unaligned */
	for (; i>0; i--) {
	    asm volatile (
		"prefetchnta 320(%0)\n\t"
		"movdqu   (%0), %%xmm0\n\t"
		"movdqu 16(%0), %%xmm1\n\t"
		"movntdq %%xmm0,   (%1)\n\t"
		"movntdq %%xmm1, 16(%1)\n\t"
		"movdqu 32(%0), %%xmm0\n\t"
		"movdqu 48(%0), %%xmm1\n\t"
		"movntdq %%xmm0, 32(%1)\n\t"
		"movntdq %%xmm1, 48(%1)\n\t"
		: : "r" (from), "r" (to) : "memory");
	    from+=64;
	    to+=64;
	}

	i = n >> 4;
	n &= 15;
	for (; i>0; i--) {
	    asm volatile (
		"movdqu (%0), %%xmm0\n\t"
		"movntdq %%xmm0, (%1)\n\t"
		: : "r" (from), "r" (to) : "memory");
	    from+=16;
	    to+=16;
	}
    }

    COPY_TAIL();
}

/**
 * _block_avx_copy - Block copy function using AVX (256bit) non-temporal stores.
 * @to: mandatory. Provides the destination pointer.
 * @from: mandatory. Provides the source pointer.
 * @n: mandatory. Provides the number of bytes to copy.
 */
static void _block_avx_copy(void *to, const void *from, size_t n)
{
    int32_t i;

    /* Align destination to 32 bytes with movnti */
    ALIGN_DEST(32);

    /* Copy blocks, 128 bytes per loop with prefetching using 256bit AVX instructions */
    i = n >> 7;
    n &= 127;
    if (likely(!((unsigned long)from & 32))) {
	/* Source aligned */
	for (; i>0; i--) {
	    asm volatile (
		"prefetchnta 320(%0)\n\t"
		"prefetchnta 384(%0)\n\t"
		"vmovdqa   (%0), %%ymm0\n\t"
		"vmovdqa 32(%0), %%ymm1\n\t"
		"vmovntdq %%ymm0,   (%1)\n\t"
		"vmovntdq %%ymm1, 32(%1)\n\t"
		"vmovdqa 64(%0), %%ymm0\n\t"
		"vmovdqa 96(%0), %%ymm1\n\t"
		"vmovntdq %%ymm0, 64(%1)\n\t"
		"vmovntdq %%ymm1, 96(%1)\n\t"
		: : "r" (from), "r" (to) : "memory");
	    from+=128;
	    to+=128;
	}

	i = n >> 5;
	n &= 31;
	for (; i>0; i--) {
	    asm volatile (
		"vmovdqa (%0), %%ymm0\n\t"
		"vmovntdq %%ymm0, (%1)\n\t"
		: : "r" (from), "r" (to) : "memory");
	    from+=32;
	    to+=32;
	}
    } else {
	/* Source unaligned */
	for (; i>0; i--) {
	    asm volatile (
		"prefetchnta 320(%0)\n\t"
		"prefetchnta 384(%0)\n\t"
		"vmovdqu   (%0), %%ymm0\n\t"
		"vmovdqu 32(%0), %%ymm1\n\t"
		"vmovntdq %%ymm0,   (%1)\n\t"
		"vmovntdq %%ymm1, 32(%1)\n\t"
		"vmovdqu 64(%0), %%ymm0\n\t"
		"vmovdqu 96(%0), %%ymm1\n\t"
		"vmovntdq %%ymm0, 64(%1)\n\t"
		"vmovntdq %%ymm1, 96(%1)\n\t"
		: : "r" (from), "r" (to) : "memory");
	    from+=128;
	    to+=128;
	}

	i = n >> 5;
	n &= 31;
	for (; i>0; i--) {
	    asm volatile (
		"vmovdqu (%0), %%ymm0\n\t"
		"vmovntdq %%ymm0, (%1)\n\t"
		: : "r" (from), "r" (to) : "memory");
	    from+=32;
	    to+=32;
	}
    }

    COPY_TAIL();
}

/**
 * _numachip_sge_copy_nt - Platform optimized ScatterGather copy function using SSE2
 * @sg_list: mandatory. Provides a pointer to the scatter-gather list.
 * @num_sge: mandatory. Provides the number of scatter-gather entries in the list.
 */
static void _numachip_sge_copy_nt(struct numachip_sge *sg_list, size_t num_sge)
{
    int32_t i = 0;

    for (i=0; i<num_sge; i++) {
	asm volatile (
	    "prefetchnta (%0)\n\t"
	    "prefetchnta 64(%0)\n\t"
	    "prefetchnta 128(%0)\n\t"
	    "prefetchnta 192(%0)\n\t"
	    "prefetchnta 256(%0)\n\t"
	    : : "r" ((void*)sg_list[i].from) );
	assert(!(((unsigned long)sg_list[i].to) & 0x3));

	_block_nt_copy((void*)sg_list[i].to, (void*)sg_list[i].from, (size_t)sg_list[i].length);
    }

    /* Fence */
    asm volatile (
	"mfence\n\t"
	::: "memory");
}

/**
 * _numachip_sge_copy_xmm - Platform optimized ScatterGather copy function using SSE2 XMM instructions.
 * @sg_list: mandatory. Provides a pointer to the scatter-gather list.
 * @num_sge: mandatory. Provides the number of scatter-gather entries in the list.
 */
static void _numachip_sge_copy_xmm(struct numachip_sge *sg_list, size_t num_sge)
{
    struct { unsigned long a,b; } xmm_save[2] __attribute__((aligned(16)));
    int32_t i = 0;

    /* Save XMM registers */
    asm volatile (
	"movdqa %%xmm0,(%0)\n\t"
	"movdqa %%xmm1,16(%0)\n\t"
	: : "r" (xmm_save) : "memory");

    for (i=0; i<num_sge; i++) {
	asm volatile (
	    "prefetchnta (%0)\n\t"
	    "prefetchnta 64(%0)\n\t"
	    "prefetchnta 128(%0)\n\t"
	    "prefetchnta 192(%0)\n\t"
	    "prefetchnta 256(%0)\n\t"
	    : : "r" ((void*)sg_list[i].from) );

	assert(!(((unsigned long)sg_list[i].to) & 0x3));

	_block_xmm_copy((void*)sg_list[i].to, (void*)sg_list[i].from, (size_t)sg_list[i].length);
    }

    /* Fence & Restore XMM registers */
    asm volatile (
	"mfence\n\t"
	"movdqa (%0),%%xmm0\n\t"
	"movdqa 16(%0),%%xmm1\n\t"
	: : "r" (xmm_save) : "memory");
}

/**
 * _numachip_sge_copy_avx - Platform optimized ScatterGather copy function using AVX instructions.
 * @sg_list: mandatory. Provides a pointer to the scatter-gather list.
 * @num_sge: mandatory. Provides the number of scatter-gather entries in the list.
 */
static void _numachip_sge_copy_avx(struct numachip_sge *sg_list, size_t num_sge)
{
    struct { unsigned long a,b,c,d; } avx_save[2] __attribute__((aligned(32)));
    int32_t i = 0;

    /* Save AVX registers */
    asm volatile (""
	"vmovdqa %%ymm0,(%0)\n\t"
	"vmovdqa %%ymm1,32(%0)\n\t"
	: : "r" (avx_save) : "memory");

    for (i=0; i<num_sge; i++) {
	asm volatile (
	    "prefetchnta (%0)\n\t"
	    "prefetchnta 64(%0)\n\t"
	    "prefetchnta 128(%0)\n\t"
	    "prefetchnta 192(%0)\n\t"
	    "prefetchnta 256(%0)\n\t"
	    : : "r" ((void*)sg_list[i].from) );

	assert(!(((unsigned long)sg_list[i].to) & 0x3));

	_block_avx_copy((void*)sg_list[i].to, (void*)sg_list[i].from, (size_t)sg_list[i].length);
    }

    /* Fence & Restore AVX registers */
    asm volatile (
	"mfence\n\t"
	"vmovdqa (%0),%%ymm0\n\t"
	"vmovdqa 32(%0),%%ymm1\n\t"
	: : "r" (avx_save) : "memory");
}

/**
 * cpuid - Generic CPUID function
 */
static inline void cpuid(uint32_t op,
			 uint32_t *eax, uint32_t *ebx,
			 uint32_t *ecx, uint32_t *edx)
{
    *eax = op;
    *ecx = 0;
    /* ecx is often an input as well as an output. */
    asm volatile(
	"cpuid\n\t"
	: "=a" (*eax),
	  "=b" (*ebx),
	  "=c" (*ecx),
	  "=d" (*edx)
	: "0" (*eax), "2" (*ecx)
	: "memory");
}

/**
 * CPUID functions returning a single datum
 */
static inline uint32_t cpuid_eax(uint32_t op)
{
    uint32_t eax, ebx, ecx, edx;
    cpuid(op, &eax, &ebx, &ecx, &edx);
    return eax;
}

static inline uint32_t cpuid_ebx(uint32_t op)
{
    uint32_t eax, ebx, ecx, edx;
    cpuid(op, &eax, &ebx, &ecx, &edx);
    return ebx;
}

static inline uint32_t cpuid_ecx(uint32_t op)
{
    uint32_t eax, ebx, ecx, edx;
    cpuid(op, &eax, &ebx, &ecx, &edx);
    return ecx;
}

static inline uint32_t cpuid_edx(uint32_t op)
{
    uint32_t eax, ebx, ecx, edx;
    cpuid(op, &eax, &ebx, &ecx, &edx);
    return edx;
}

static inline uint64_t xgetbv(uint32_t index)
{
    uint32_t eax, edx;

    asm volatile(
	"xgetbv\n\t"
	: "=a" (eax), "=d" (edx)
	: "c" (index));
    return eax + ((uint64_t)edx << 32);
}

#define SSE2_FLAGS                (1<<26)
#define AVX_FLAGS                 ((1<<27)|(1<<28))
#define XCR_XFEATURE_ENABLED_MASK 0x00000000

static inline int have_sse2(void)
{
    if ((cpuid_edx(0x00000001) & SSE2_FLAGS) != SSE2_FLAGS)
	return 0;

    return 1;
}

static inline int have_avx(void)
{
    if ((cpuid_ecx(0x00000001) & AVX_FLAGS) != AVX_FLAGS)
	return 0;

    if ((xgetbv(XCR_XFEATURE_ENABLED_MASK) & 6) != 6)
	return 0;

    return 1;
}

static void _memops_lib_constructor(void)
{
    numachip_sge_copy = _numachip_sge_copy_nt;

    /* We need a minimum of SSE2 support */
    assert(have_sse2());

    /* XMM and AVX is optional */
    if (have_avx() && getenv("MEMOPS_USE_AVX"))
	numachip_sge_copy = _numachip_sge_copy_avx;
    else if (getenv("MEMOPS_USE_XMM"))
	numachip_sge_copy = _numachip_sge_copy_xmm;
}

