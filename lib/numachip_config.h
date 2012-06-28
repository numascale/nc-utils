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

#ifndef __NC_CONFIG
#define __NC_CONFIG 1


typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

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
    uint32_t sync_only;
};

struct part_info {
    uint32_t master;
    uint32_t builder;
};

int parse_config_file(const char *filename,
		      struct node_info **cfg_nodelist,
		      int *num_nodes);
#endif
