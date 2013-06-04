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

#include <stdbool.h>
#include <inttypes.h>
#include <string.h>

#include "dnc-bootloader.h"

/* #define DEBUG_CONFIG 1 */

struct fabric_info {
	uint32_t x_size;
	uint32_t y_size;
	uint32_t z_size;
	uint32_t strict;
};

struct node_info {
	uint32_t uuid;
	uint32_t sciid;
	uint32_t partition;
	uint32_t osc;
	char desc[32];
	bool sync_only;
};

struct part_info {
	uint32_t master;
	uint32_t builder;
};

extern struct fabric_info cfg_fabric;
extern struct node_info *cfg_nodelist;
extern struct part_info *cfg_partlist;
extern int cfg_nodes, cfg_partitions;
extern bool name_matching;

int parse_config_file(char *data);
void make_singleton_config(void);
void get_node_config(void);
struct part_info *get_partition_config(int idx);
char *get_master_name(uint32_t sciid);
extern char *hostname;

static inline int config_local(struct node_info *info, uint32_t uuid)
{
	if (name_matching && hostname) {
		return strcmp(info->desc, hostname) == 0;
	} else
		return info->uuid == uuid;
}

#endif
