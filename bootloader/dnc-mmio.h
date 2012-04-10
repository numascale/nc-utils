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

#ifndef __DNC_MMIO_H
#define __DNC_MMIO_H 1

#define MAX_BRIDGES 4

extern void tally_remote_node_mmio(u16 node);
extern int setup_remote_node_mmio(u16 node);

#endif

