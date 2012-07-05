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

#ifndef __DNC_FABRIC_H
#define __DNC_FABRIC_H 1

#include "dnc-types.h"

/* These functions can be used to do remote CSR accesses without the risk of locking the CPU
   typically used when performing fabric requests */

int dnc_raw_read_csr(u32 node, u16 csr, u32 *val);
int dnc_raw_read_csr_geo(u32 node, u8 bid, u16 csr, u32 *val);

void dnc_reset_phy(int phy);
void dnc_reset_lc3(int lc);
int dnc_check_phy(int phy);
int dnc_check_lc3(int lc);
int dnc_init_lc3(u16 nodeid, int lc, u16 maxchunk,
                 u16 rtbll[], u16 rtblm[], u16 rtblh[], u16 ltbl[]);

static inline const char *_get_linkname(int linkno)
{
    switch (linkno) {
        case 0: return "XA";
        case 1: return "XB";
        case 2: return "YA";
        case 3: return "YB";
        case 4: return "ZA";
        case 5: return "ZB";
        default: return NULL;
    }
}

#endif
