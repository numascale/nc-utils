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
