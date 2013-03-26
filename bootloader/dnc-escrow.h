/*
 * Copyright (C) 2008-2012 Numascale AS, support@numascale.com
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

#ifndef __DNC_ESCROW
#define __DNC_ESCROW 1

#include <stdint.h>
#include <stdbool.h>

#define ESCROW_VER   0

#define ID_CONFIG    1
#define ID_GEOM      2
#define ID_BOOTLOG   3

struct ent_global {
	unsigned int numachip_major : 4;
	unsigned int numachip_minor : 4;
	uint8_t escrow_ver;
	unsigned int size_x : 4;
	unsigned int size_y : 4;
	unsigned int size_z : 4;
	bool symmetric;
	bool renumbering;
	unsigned int limit_ht : 3;
	unsigned int numachip_ht : 3;
	unsigned int neigh_ht : 3;
	unsigned int neigh_link : 2;
};

struct ent_external {
	uint8_t id;
	uint8_t flags;
	uint64_t base : 48;
	uint16_t reserved;
	uint64_t limit : 48;	
};

union escrow_ent {
	uint8_t id;
	struct ent_global global;
	struct ent_external external;
};

extern int escrow_populate(void *data);

#endif

