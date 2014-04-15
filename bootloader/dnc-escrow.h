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

#include "dnc-types.h"

#define ESCROW_REV   4

struct escrow_ent {
	unsigned numachip_rev : 4;
	unsigned size_x : 4;
	unsigned size_y : 4;
	unsigned size_z : 4;
	unsigned northbridges : 3;
	unsigned neigh_ht : 3;
	unsigned neigh_link : 2;
	unsigned symmetric : 1;
	unsigned renumbering : 1;
	unsigned remote_io : 1;
	unsigned observer : 1;
	unsigned cores : 8;
} __attribute__((packed));

checked int escrow_populate(void *data);

#endif
