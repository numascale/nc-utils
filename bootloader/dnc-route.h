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

#ifndef __DNC_ROUTE
#define __DNC_ROUTE 1

#include "dnc-types.h"

extern void wait_key(void);

void add_chunk_route(u16 dest, u16 node, u8 link);
void del_chunk_route(u16 dest, u16 node);
void set_route(u16 dest, u16 node, u16 width, u8 link);
void add_route(u16 dest, u16 node, u16 width, u8 link);
void del_route(u16 dest, u16 node, u16 width);
void set_route_geo(u16 dest, u16 node, u8 bid, u16 width, u8 link);
void add_route_geo(u16 dest, u16 node, u8 bid, u16 width, u8 link);
void del_route_geo(u16 dest, u16 node, u8 bid, u16 width);

#endif
