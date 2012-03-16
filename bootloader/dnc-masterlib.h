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

#define SCC_ATT_INDEX_RANGE 2   /* 3 = 47:36, 2 = 43:32, 1 = 39:28, 0 = 35:24 */
#define SCC_ATT_GRAN            ((0x1000000ULL << (SCC_ATT_INDEX_RANGE * 4)) >> DRAM_MAP_SHIFT)

extern u32 *mseq_ucode;
extern u16 *mseq_table;
extern int mseq_ucode_length;
extern int mseq_table_length;

void load_scc_microcode(u16 node);

void tally_local_node(int enforce_alignment);
int tally_all_remote_nodes(void);

#endif
