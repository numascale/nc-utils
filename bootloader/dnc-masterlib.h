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

#ifndef __DNC_MASTERLIB_H
#define __DNC_MASTERLIB_H 1

#include "dnc-types.h"

#define MAX_MEM_PER_NODE	(32*1024*1024*1024ULL)
#define DRAM_MAP_SHIFT 24
#define SCC_ATT_INDEX_RANGE 2   /* 3 = 47:36, 2 = 43:32, 1 = 39:28, 0 = 35:24 */
#define SCC_ATT_GRAN            ((0x1000000ULL << (SCC_ATT_INDEX_RANGE * 4)) >> DRAM_MAP_SHIFT)

extern int dnc_master_ht_id;     /* HT id of NC on master node, equivalent to dnc_node_ht_id[0] */

extern u16 dnc_node_count;
extern nc_node_info_t nc_node[128];
extern u16 ht_pdom_count;
extern u16 apic_per_node;
extern u16 ht_next_apic;
extern u32 dnc_top_of_mem;
extern u8 post_apic_mapping[256];

extern u32 *mseq_ucode;
extern u16 *mseq_table;
extern int mseq_ucode_length;
extern int mseq_table_length;

/* Traversal info per node.  Bit 7: seen, bits 5:0 rings walked. */
extern u8 nodedata[4096];

void load_scc_microcode(u16 node);

void tally_local_node(int enforce_alignment);
int tally_remote_node(u16 node);
int tally_all_remote_nodes(void);

#endif
