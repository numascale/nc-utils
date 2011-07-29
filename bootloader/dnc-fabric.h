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

#ifndef __DNC_FABRIC_H
#define __DNC_FABRIC_H 1

#include "dnc-types.h"

// These functions can be used to do remote CSR accesses without the risk of locking the CPU
// typically used when performing fabric requests

int dnc_raw_read_csr(u32 node, u16 csr, u32 *val);
int dnc_raw_read_csr_geo(u32 node, u8 bid, u16 csr, u32 *val);

void dnc_reset_phy(int linkno);
void dnc_reset_lc3(int linkno);
int dnc_check_phy(int linkno);
int dnc_check_lc3(int linkno);
int dnc_init_lc3(int linkno, u16 maxchunk,
                 u16 rtbll[], u16 rtblm[], u16 rtblh[], u16 ltbl[]);

static inline const char *_get_linkname(int linkno)
{
    switch (linkno) {
        case 1: return "XA";
        case 2: return "XB";
        case 3: return "YA";
        case 4: return "YB";
        case 5: return "ZA";
        case 6: return "ZB";
        default: return NULL;
    }
}

#endif
