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

#include "dnc-escrow.h"
#include "dnc-config.h"
#include "dnc-commonlib.h"
#include "dnc-bootloader.h"

#include <stdio.h>

int escrow_populate(void *data)
{
	union escrow_ent *start = (escrow_ent *)data;
	union escrow_ent *cur = start;

	cur->global.numachip_rev = dnc_chip_rev;
	cur->global.size_x       = cfg_fabric.x_size;
	cur->global.size_y       = cfg_fabric.y_size;
	cur->global.size_z       = cfg_fabric.z_size;
	cur->global.northbridges = nc_node[0].nc_ht;
	cur->global.neigh_ht     = nc_node[0].nc_neigh_ht;
	cur->global.neigh_link   = nc_node[0].nc_neigh_link;
	cur->global.symmetric    = 1;
	cur->global.renumbering  = renumber_bsp == 1;
	cur->global.remote_io    = !!remote_io;

	if (verbose > 1)
		printf("Escrow: numachip_rev=%d size=%d,%d,%d northbridges=%d neigh=%d,%d symmetric=%d renumbering=%d\n",
			cur->global.numachip_rev, cur->global.size_x, cur->global.size_y, cur->global.size_z,
			cur->global.northbridges, cur->global.neigh_ht, cur->global.neigh_link,
			cur->global.symmetric, cur->global.renumbering);
	cur++;

	return (cur - start) * sizeof(cur[0]);
}

