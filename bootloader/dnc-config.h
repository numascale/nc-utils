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

#ifndef __DNC_CONFIG_H
#define __DNC_CONFIG_H 1

#include "dnc-types.h"
#include "string.h"

//#define DEBUG_CONFIG 1

struct fabric_info {
    u32 x_size;
    u32 y_size;
    u32 z_size;
    u32 strict;
};

struct node_info {
    u32 uuid;
    u32 sciid;
    u32 partition;
    u32 osc;
    char desc[32];
    u32 sync_only;
};

struct part_info {
    u32 master;
    u32 builder;
};

extern struct fabric_info cfg_fabric;
extern struct node_info *cfg_nodelist;
extern struct part_info *cfg_partlist;
extern int cfg_nodes, cfg_partitions;
extern int name_matching;

int parse_config_file(char *data);
void make_singleton_config(u32 uuid);
struct node_info* get_node_config(u32 uuid);
struct part_info* get_partition_config(int idx);
extern char *hostname;

static inline int config_local(struct node_info *info, u32 uuid)
{
    if (name_matching && hostname) {
	return strcmp(info->desc, hostname) == 0;
    } else
	return info->uuid == uuid;
}

#endif
