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

#ifndef __NUMACHIP_LIB_H
#define __NUMACHIP_LIB_H

#include "numachip_user.h"
#include "../../interface/numachip-defines.h"

#define NUMACHIP_CSR_BASE 0x3fff00000000ULL

#define HIDDEN	__attribute__((visibility ("hidden")))

struct numachip_device {
	uint32_t nodeid;
};

struct numachip_context {
	struct numachip_device    *device;
	int32_t                    memfd;
	uint32_t                  *csr_space;
};

static inline uint32_t u32bswap(uint32_t val)
{
	asm volatile("bswap %0" : "+r"(val));
	return val;
}

HIDDEN int32_t numachip_init(struct numachip_device ***list);

#endif


