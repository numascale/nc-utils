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
#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>

#include "numachip_lib.h"

static void _block_xmm_copy(void *to, const void *from, size_t n)
{
    int32_t i;
    
    /* Align to 16 bytes with movnti*/
    i = ((unsigned long)to & 0xf) >> 2;
    n = n - (i<<2);
    for (; i>0; i--) {
	__asm__ __volatile__ (
	    "1: movl (%0), %%eax\n"
	    "   movnti %%eax, (%1)\n"
	    : : "r" (from), "r" (to) : "eax", "memory");
	from+=4;
	to+=4;
    }

    i = n >> 6;
    n &= 63;
    for (; i>0; i--) {
	__asm__ __volatile__ (
	    "1: prefetchnta 320(%0)\n"
	    "2: movdqu   (%0), %%xmm0\n"
	    "   movdqu 16(%0), %%xmm1\n"
	    "   movntdq %%xmm0,   (%1)\n"
	    "   movntdq %%xmm1, 16(%1)\n"
	    "   movdqu 32(%0), %%xmm0\n"
	    "   movdqu 48(%0), %%xmm1\n"
	    "   movntdq %%xmm0, 32(%1)\n"
	    "   movntdq %%xmm1, 48(%1)\n"
	    : : "r" (from), "r" (to) : "memory");
	from+=64;
	to+=64;
    }

    i = n >> 4;
    n &= 15;
    for (; i>0; i--) {
	__asm__ __volatile__ (
	    "1: movdqu (%0), %%xmm0\n"
	    "   movntdq %%xmm0, (%1)\n"
	    : : "r" (from), "r" (to) : "memory");
	from+=16;
	to+=16;
    }

    i = n >> 3;
    n &= 7;
    for (; i>0; i--) {
	__asm__ __volatile__ (
	    "1: movq (%0), %%rax\n"
	    "   movnti %%rax, (%1)\n"
	    : : "r" (from), "r" (to) : "rax", "memory");
	from+=8;
	to+=8;
    }

    i = n >> 2;
    n &= 3;
    if (n) i++;
    for (; i>0; i--) {
	__asm__ __volatile__ (
	    "1: movl (%0), %%eax\n"
	    "   movnti %%eax, (%1)\n"
	    : : "r" (from), "r" (to) : "eax", "memory");
	from+=4;
	to+=4;
    }
}

typedef struct { unsigned long a,b; } __attribute__((aligned(16))) xmm_store_t;

void numachip_sge_copy(struct numachip_sge *sg_list, int32_t num_sge)
{
    xmm_store_t xmm_save[2];
    int32_t i;

    /* Save XMM registers */
    __asm__ __volatile__ (
	" movdqa %%xmm0,(%0)\n"
	" movdqa %%xmm1,16(%0)\n"
	: : "r" (xmm_save) : "memory" );

    for (i=0; i<num_sge; i++) {
	struct numachip_sge *sge = &sg_list[i];
	void *from = (void *)sge->from;
	void *to = (void *)sge->to;
	size_t n = sge->length;
	
	__asm__ __volatile__ (
	    "1: prefetchnta (%0)\n"
	    "   prefetchnta 64(%0)\n"
	    "   prefetchnta 128(%0)\n"
	    "   prefetchnta 192(%0)\n"
	    "   prefetchnta 256(%0)\n"
	    : : "r" (from) );

	assert(!(((unsigned long)to) & 0x3));

	_block_xmm_copy(to, from, n);
    }

    /* Fence & Restore XMM registers */
    __asm__ __volatile__ (
	" sfence \n "
	" movdqa (%0),%%xmm0\n"
	" movdqa 16(%0),%%xmm1\n"
	:: "r" (xmm_save) );
}

