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

static int _have_avx = 0;

static void _memops_lib_constructor(void) __attribute__((constructor));

/**
 * _block_xmm_copy - Block copy function using SSE4.1 (128bit) non-temporal stores.
 * @to: mandatory. Provides the destination pointer.
 * @from: mandatory. Provides the source pointer.
 * @n: mandatory. Provides the number of bytes to copy.
 */
static void _block_xmm_copy(void *to, const void *from, size_t n)
{
    int32_t i;
    
    /* Align to 64 bytes with movnti */
    i = ((~(unsigned long)to+1) & 0x3f) >> 2;
    for (; i>0 && n>0; i--) {
	asm volatile (
	    "movl (%0), %%eax\n"
	    "movnti %%eax, (%1)\n"
	    : : "r" (from), "r" (to) : "eax", "memory");
	from+=4;
	to+=4;
	n-=4;
    }

    i = n >> 6;
    n &= 63;
    for (; i>0; i--) {
	asm volatile (
	    "prefetchnta 320(%0)\n"
	    "movdqu   (%0), %%xmm0\n"
	    "movdqu 16(%0), %%xmm1\n"
	    "movntdq %%xmm0,   (%1)\n"
	    "movntdq %%xmm1, 16(%1)\n"
	    "movdqu 32(%0), %%xmm0\n"
	    "movdqu 48(%0), %%xmm1\n"
	    "movntdq %%xmm0, 32(%1)\n"
	    "movntdq %%xmm1, 48(%1)\n"
	    : : "r" (from), "r" (to) : "memory");
	from+=64;
	to+=64;
    }

    i = n >> 4;
    n &= 15;
    for (; i>0; i--) {
	asm volatile (
	    "movdqu (%0), %%xmm0\n"
	    "movntdq %%xmm0, (%1)\n"
	    : : "r" (from), "r" (to) : "memory");
	from+=16;
	to+=16;
    }

    i = n >> 3;
    n &= 7;
    for (; i>0; i--) {
	asm volatile (
	    "movq (%0), %%rax\n"
	    "movnti %%rax, (%1)\n"
	    : : "r" (from), "r" (to) : "rax", "memory");
	from+=8;
	to+=8;
    }

    i = n >> 2;
    n &= 3;
    if (n) i++;
    for (; i>0; i--) {
	asm volatile (
	    "movl (%0), %%eax\n"
	    "movnti %%eax, (%1)\n"
	    : : "r" (from), "r" (to) : "eax", "memory");
	from+=4;
	to+=4;
    }
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
    
    /* Align to 64 bytes with movnti */
    i = ((~(unsigned long)to+1) & 0x3f) >> 2;
    for (; i>0 && n>0; i--) {
	asm volatile (
	    "movl (%0), %%eax\n"
	    "movnti %%eax, (%1)\n"
	    : : "r" (from), "r" (to) : "eax", "memory");
	from+=4;
	to+=4;
	n-=4;
    }

    i = n >> 7;
    n &= 127;
    for (; i>0; i--) {
	asm volatile (
	    "prefetchnta 320(%0)\n"
	    "prefetchnta 384(%0)\n"
	    "vmovdqu   (%0), %%ymm0\n"
	    "vmovdqu 32(%0), %%ymm1\n"
	    "vmovntdq %%ymm0,   (%1)\n"
	    "vmovntdq %%ymm1, 32(%1)\n"
	    "vmovdqu 64(%0), %%ymm0\n"
	    "vmovdqu 96(%0), %%ymm1\n"
	    "vmovntdq %%ymm0, 64(%1)\n"
	    "vmovntdq %%ymm1, 96(%1)\n"
	    : : "r" (from), "r" (to) : "memory");
	from+=128;
	to+=128;
    }

    i = n >> 5;
    n &= 31;
    for (; i>0; i--) {
	asm volatile (
	    "vmovdqu (%0), %%ymm0\n"
	    "vmovntdq %%ymm0, (%1)\n"
	    : : "r" (from), "r" (to) : "memory");
	from+=32;
	to+=32;
    }

    i = n >> 4;
    n &= 15;
    for (; i>0; i--) {
	asm volatile (
	    "movdqu (%0), %%xmm0\n"
	    "movntdq %%xmm0, (%1)\n"
	    : : "r" (from), "r" (to) : "memory");
	from+=16;
	to+=16;
    }

    i = n >> 3;
    n &= 7;
    for (; i>0; i--) {
	asm volatile (
	    "movq (%0), %%rax\n"
	    "movnti %%rax, (%1)\n"
	    : : "r" (from), "r" (to) : "rax", "memory");
	from+=8;
	to+=8;
    }

    i = n >> 2;
    n &= 3;
    if (n) i++;
    for (; i>0; i--) {
	asm volatile (
	    "movl (%0), %%eax\n"
	    "movnti %%eax, (%1)\n"
	    : : "r" (from), "r" (to) : "eax", "memory");
	from+=4;
	to+=4;
    }
}

/**
 * _numachip_sge_copy_xmm - Platform optimized ScatterGather copy function using SSE4.1
 * @sg_list: mandatory. Provides a pointer to the scatter-gather list.
 * @num_sge: mandatory. Provides the number of scatter-gather entries in the list.
 */
static void _numachip_sge_copy_xmm(struct numachip_sge *sg_list, size_t num_sge)
{
    struct { unsigned long a,b; } xmm_save[2] __attribute__((aligned(16)));
    int32_t i = 0;

    /* Save XMM registers */
    asm volatile (
	"movdqa %%xmm0,(%0)\n"
	"movdqa %%xmm1,16(%0)\n"
	: : "r" (xmm_save) : "memory");

    for (i=0; i<num_sge; i++) {
	asm volatile (
	    "1: prefetchnta (%0)\n"
	    "   prefetchnta 64(%0)\n"
	    "   prefetchnta 128(%0)\n"
	    "   prefetchnta 192(%0)\n"
	    "   prefetchnta 256(%0)\n"
	    : : "r" ((void*)sg_list[i].from) );

	assert(!(((unsigned long)sg_list[i].to) & 0x3));
	
	_block_xmm_copy((void*)sg_list[i].to, (void*)sg_list[i].from, (size_t)sg_list[i].length);
    }

    /* Fence & Restore XMM registers */
    asm volatile (
	"mfence \n"
	"movdqa (%0),%%xmm0\n"
	"movdqa 16(%0),%%xmm1\n"
	: : "r" (xmm_save) : "memory");
}

/**
 * _numachip_sge_copy_avx - Platform optimized ScatterGather copy function using AVX
 * @sg_list: mandatory. Provides a pointer to the scatter-gather list.
 * @num_sge: mandatory. Provides the number of scatter-gather entries in the list.
 */
static void _numachip_sge_copy_avx(struct numachip_sge *sg_list, size_t num_sge)
{
    struct { unsigned long a,b,c,d; } avx_save[3] __attribute__((aligned(16)));
    unsigned long save_ptr = ((unsigned long)avx_save & 0x1f) ? (unsigned long)(&avx_save[0].c) : (unsigned long)avx_save;
    int32_t i = 0;

    /* Save AVX registers */
    asm volatile (
	"vmovdqa %%ymm0,(%0)\n"
	"vmovdqa %%ymm1,32(%0)\n"
	: : "r" (save_ptr) : "memory");

    for (i=0; i<num_sge; i++) {
	asm volatile (
	    "1: prefetchnta (%0)\n"
	    "   prefetchnta 64(%0)\n"
	    "   prefetchnta 128(%0)\n"
	    "   prefetchnta 192(%0)\n"
	    "   prefetchnta 256(%0)\n"
	    : : "r" ((void*)sg_list[i].from) );

	assert(!(((unsigned long)sg_list[i].to) & 0x3));
	
	_block_avx_copy((void*)sg_list[i].to, (void*)sg_list[i].from, (size_t)sg_list[i].length);
    }

    /* Fence & Restore AVX registers */
    asm volatile (
	"mfence \n"
	"vmovdqa (%0),%%ymm0\n"
	"vmovdqa 32(%0),%%ymm1\n"
	: : "r" (save_ptr) : "memory");
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
    asm volatile("cpuid"
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

    asm volatile(".byte 0x0f,0x01,0xd0" /* xgetbv */
		 : "=a" (eax), "=d" (edx)
		 : "c" (index));
    return eax + ((uint64_t)edx << 32);
}

#define AVX_FLAGS                 ((1<<27)|(1<<28))
#define XCR_XFEATURE_ENABLED_MASK 0x00000000

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
    numachip_sge_copy = _numachip_sge_copy_xmm;
    _have_avx = have_avx();
    if (_have_avx && !getenv("MEMOPS_NO_AVX")) {
	printf("We have AVX!!\n");
	numachip_sge_copy = _numachip_sge_copy_avx;
    }
}

