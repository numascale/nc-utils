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
// Copyright © 2008-2011
// Numascale AS Oslo, Norway. 
// All Rights Reserved.
//

#ifndef __DNC_CONFIG_H
#define __DNC_CONFIG_H 1

#include "dnc-types.h"

struct fabric_info {
    u32 x_size;
    u32 y_size;
    u32 z_size;
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

int parse_config_file(char *data);
struct node_info* get_node_config(u32 uuid);
struct part_info* get_partition_config(int idx);

//#define DEBUG_CONFIG 1

#endif
