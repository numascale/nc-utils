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
	struct escrow_ent *start = (escrow_ent *)data;
	struct escrow_ent *cur = start;

	cur->numachip_rev = 2;
	cur->size_x       = cfg_fabric.size[0];
	cur->size_y       = cfg_fabric.size[1];
	cur->size_z       = cfg_fabric.size[2];
	cur->northbridges = nodes[0].nc_ht;
	cur->neigh_ht     = nodes[0].nc_neigh_ht;
	cur->neigh_link   = nodes[0].nc_neigh_link;
	cur->symmetric    = 1;
	cur->renumbering  = renumber_bsp == 1;
	cur->remote_io    = !!remote_io;
	cur->observer     = local_info->sync_only;
	cur->cores        = 0;

	for (int i = 0; i < nodes[0].nc_ht; i++)
		cur->cores += nodes[0].ht[i].cores;

	if (verbose)
		printf("Escrow: numachip_rev=%u size=%ux%ux%u northbridges=%u neigh=%u.%u symmetric=%u renumbering=%u remote-io=%u observer=%u cores=%u\n",
			cur->numachip_rev, cur->size_x, cur->size_y, cur->size_z,
			cur->northbridges, cur->neigh_ht, cur->neigh_link,
			cur->symmetric, cur->renumbering, cur->remote_io,
			cur->observer, cur->cores);
	cur++;

	return (cur - start) * sizeof(cur[0]);
}

