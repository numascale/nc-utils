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

#include <stdio.h>

#include "dnc-regs.h"
#include "dnc-route.h"
#include "dnc-access.h"

#define OFFS_RTBLL (LC3_CSR_ROUT_BXTBLL00 - LC3_CSR_ROUT_LCTBL00)
#define OFFS_RTBLM (LC3_CSR_ROUT_BLTBL00  - LC3_CSR_ROUT_LCTBL00)
#define OFFS_RTBLH (LC3_CSR_ROUT_BXTBLH00 - LC3_CSR_ROUT_LCTBL00)
#define OFFS_LTBL  0

#define DEBUG_ROUTE(fmt, ...)
//#define DEBUG_ROUTE(fmt, ...) printf(fmt, __VA_ARGS__)

#define SCI_ID_CHUNK(id) (((id) >> 8) & 0xf) // Note: Routing is deep enough to cover bit 12:8, but we use only 12bit NodeIDs
#define SCI_ID_REGNR(id) (((id) >> 4) & 0xf)
#define SCI_ID_BITNR(id) (((id) >> 0) & 0xf)

/* Route all 256 bxbar entries for chunk corresponding to "dest" over "link"
 * on "node". */
void add_chunk_route(u16 dest, u16 node, u8 link)
{
    int scc = ((node == 0xfff0) || ((node & 0xf000) == 0));
    u16 base = scc ? H2S_CSR_G0_ROUT_LCTBL00 : LC3_CSR_ROUT_LCTBL00;
    u16 reg;
    
    dnc_write_csr(node,
		  scc 
		  ? H2S_CSR_G0_ROUT_TABLE_CHUNK
		  : LC3_CSR_SW_INFO3,
		  SCI_ID_CHUNK(dest));

    for (reg = 0; reg < 16; reg++) {
	dnc_write_csr(node, base + OFFS_RTBLL + reg * 4,
		      (link & 1) ? 0xffff : 0);
	dnc_write_csr(node, base + OFFS_RTBLM + reg * 4,
		      (link & 2) ? 0xffff : 0);
	dnc_write_csr(node, base + OFFS_RTBLH + reg * 4,
		      (link & 4) ? 0xffff : 0);
	if (!scc) dnc_write_csr(node, base + OFFS_LTBL + reg * 4, 0xffff);
    }

    DEBUG_ROUTE("add_chunk_route: on %04x to %04x over %d\n",
		node, dest, link);
}

/* Unroute all 256 entries for chunk corresponding to "dest" on "node". */
void del_chunk_route(u16 dest, u16 node)
{
    int scc = ((node == 0xfff0) || ((node & 0xf000) == 0));
    u16 reg;

    if (!scc) {
        dnc_write_csr(node, LC3_CSR_SW_INFO3, SCI_ID_CHUNK(dest));

        for (reg = 0; reg < 16; reg++) {
            dnc_write_csr(node, LC3_CSR_ROUT_LCTBL00 + OFFS_LTBL + reg * 4, 0x0000);
        }
        DEBUG_ROUTE("del_chunk_route: on %04x from %04x\n",
                    node, dest);
    }
}

/* Set route on "node" towards "dest" over "link".  Bitmask "width"
   signifies width of route. */
void set_route(u16 dest, u16 node, u16 width, u8 link)
{
    int scc = ((node == 0xfff0) || ((node & 0xf000) == 0));
    u16 reg = SCI_ID_REGNR(dest) * 4;
    u16 bit = SCI_ID_BITNR(dest);
    u16 base = scc ? H2S_CSR_G0_ROUT_LCTBL00 : LC3_CSR_ROUT_LCTBL00;

    dnc_write_csr(node,
		  scc 
		  ? H2S_CSR_G0_ROUT_TABLE_CHUNK
		  : LC3_CSR_SW_INFO3,
		  SCI_ID_CHUNK(dest));

    dnc_write_csr(node, base + OFFS_RTBLL + reg, ((link & 1) ? width : 0) << bit);
    dnc_write_csr(node, base + OFFS_RTBLM + reg, ((link & 2) ? width : 0) << bit);
    dnc_write_csr(node, base + OFFS_RTBLH + reg, ((link & 4) ? width : 0) << bit);
    if (!scc) dnc_write_csr(node, base + OFFS_LTBL + reg, width << bit);

    DEBUG_ROUTE("sdd_route: on %04x to %04x/%x over %d\n",
		node, dest, width, link);
}

/* Add route on "node" towards "dest" over "link".  Bitmask "width"
   signifies width of route. */
void add_route(u16 dest, u16 node, u16 width, u8 link)
{
    int scc = ((node == 0xfff0) || ((node & 0xf000) == 0));
    u16 reg = SCI_ID_REGNR(dest) * 4;
    u16 bit = SCI_ID_BITNR(dest);
    u16 base = scc ? H2S_CSR_G0_ROUT_LCTBL00 : LC3_CSR_ROUT_LCTBL00;
    u32 csr;

    dnc_write_csr(node,
		  scc 
		  ? H2S_CSR_G0_ROUT_TABLE_CHUNK
		  : LC3_CSR_SW_INFO3,
		  SCI_ID_CHUNK(dest));

    if (!scc) {
        csr = dnc_read_csr(node, LC3_CSR_CONFIG4);
        csr |= (1 << 6); // CONFIG4.tblrd is needed to read routing tables through CSR
        dnc_write_csr(node, LC3_CSR_CONFIG4, csr);
    }
        
    csr = dnc_read_csr(node, base + OFFS_RTBLL + reg);
    csr = (csr & ~(width << bit)) | (((link & 1) ? width : 0) << bit);
    dnc_write_csr(node, base + OFFS_RTBLL + reg, csr);

    csr = dnc_read_csr(node, base + OFFS_RTBLM + reg);
    csr = (csr & ~(width << bit)) | (((link & 2) ? width : 0) << bit);
    dnc_write_csr(node, base + OFFS_RTBLM + reg, csr);

    csr = dnc_read_csr(node, base + OFFS_RTBLH + reg);
    csr = (csr & ~(width << bit)) | (((link & 4) ? width : 0) << bit);
    dnc_write_csr(node, base + OFFS_RTBLH + reg, csr);

    if (!scc) {
        csr = dnc_read_csr(node, base + OFFS_LTBL + reg);
        csr |= (width << bit);
        dnc_write_csr(node, base + OFFS_LTBL + reg, csr);

        csr = dnc_read_csr(node, LC3_CSR_CONFIG4);
        csr &= ~(1 << 6); // Disbale CONFIG4.tblrd to enable normal routing operation
        dnc_write_csr(node, LC3_CSR_CONFIG4, csr);
    }

    DEBUG_ROUTE("add_route: on %04x to %04x/%x over %d\n",
		node, dest, width, link);
}

/* Remove route on "node" towards "dest".  Bitmask "width" signifies
   width of route. */
void del_route(u16 dest, u16 node, u16 width)
{
    int scc = ((node == 0xfff0) || ((node & 0xf000) == 0));
    u16 reg = SCI_ID_REGNR(dest) * 4;
    u16 bit = SCI_ID_BITNR(dest);
    u32 csr;

    if (!scc) {
        dnc_write_csr(node, LC3_CSR_SW_INFO3, SCI_ID_CHUNK(dest));

        csr = dnc_read_csr(node, LC3_CSR_CONFIG4);
        csr |= (1 << 6); // CONFIG4.tblrd is needed to read routing tables through CSR
        dnc_write_csr(node, LC3_CSR_CONFIG4, csr);

        csr = dnc_read_csr(node, LC3_CSR_ROUT_LCTBL00 + OFFS_LTBL + reg);
        csr &= ~(width << bit);
        dnc_write_csr(node, LC3_CSR_ROUT_LCTBL00 + OFFS_LTBL + reg, csr);

        csr = dnc_read_csr(node, LC3_CSR_CONFIG4);
        csr &= ~(1 << 6); // Disbale CONFIG4.tblrd to enable normal routing operation
        dnc_write_csr(node, LC3_CSR_CONFIG4, csr);
        
        DEBUG_ROUTE("del_route: on %04x to %04x/%x\n",
                    node, dest, width);
    }
}

/* Set route towards "dest" over "link" on blink id "bid" via "node".
   Bitmask "width" signifies width of route. */
void set_route_geo(u16 dest, u16 node, u8 bid, u16 width, u8 link)
{
    int scc = (bid == 0);
    u16 reg = SCI_ID_REGNR(dest) * 4;
    u16 bit = SCI_ID_BITNR(dest);
    u16 base = scc ? H2S_CSR_G0_ROUT_LCTBL00 : LC3_CSR_ROUT_LCTBL00;

    dnc_write_csr_geo(node, bid,
		      scc 
		      ? H2S_CSR_G0_ROUT_TABLE_CHUNK
		      : LC3_CSR_SW_INFO3,
		      SCI_ID_CHUNK(dest));

    dnc_write_csr_geo(node, bid, base + OFFS_RTBLL + reg, ((link & 1) ? width : 0) << bit);
    dnc_write_csr_geo(node, bid, base + OFFS_RTBLM + reg, ((link & 2) ? width : 0) << bit);
    dnc_write_csr_geo(node, bid, base + OFFS_RTBLH + reg, ((link & 4) ? width : 0) << bit);
    if (!scc) dnc_write_csr_geo(node, bid, base + OFFS_LTBL + reg, width << bit);

    DEBUG_ROUTE("set_route_geo: on %04x bid %d to %04x/%x over %d\n",
		node, bid, dest, width, link);
}

/* Add route towards "dest" over "link" on blink id "bid" via "node".
   Bitmask "width" signifies width of route. */
void add_route_geo(u16 dest, u16 node, u8 bid, u16 width, u8 link)
{
    int scc = (bid == 0);
    u16 reg = SCI_ID_REGNR(dest) * 4;
    u16 bit = SCI_ID_BITNR(dest);
    u16 base = scc ? H2S_CSR_G0_ROUT_LCTBL00 : LC3_CSR_ROUT_LCTBL00;
    u32 csr;
    
    dnc_write_csr_geo(node, bid,
		      scc 
		      ? H2S_CSR_G0_ROUT_TABLE_CHUNK
		      : LC3_CSR_SW_INFO3,
		      SCI_ID_CHUNK(dest));

    if (!scc) {
        csr = dnc_read_csr_geo(node, bid, LC3_CSR_CONFIG4);
        csr |= (1 << 6); // CONFIG4.tblrd is needed to read routing tables through CSR
        dnc_write_csr_geo(node, bid, LC3_CSR_CONFIG4, csr);
    }
    
    csr = dnc_read_csr_geo(node, bid, base + OFFS_RTBLL + reg);
    csr = (csr& ~(width << bit)) | (((link & 1) ? width : 0) << bit);
    dnc_write_csr_geo(node, bid, base + OFFS_RTBLL + reg, csr);

    csr = dnc_read_csr_geo(node, bid, base + OFFS_RTBLM + reg);
    csr = (csr & ~(width << bit)) | (((link & 2) ? width : 0) << bit);
    dnc_write_csr_geo(node, bid, base + OFFS_RTBLM + reg, csr);

    csr = dnc_read_csr_geo(node, bid, base + OFFS_RTBLH + reg);
    csr = (csr & ~(width << bit)) | (((link & 4) ? width : 0) << bit);
    dnc_write_csr_geo(node, bid, base + OFFS_RTBLH + reg, csr);

    if (!scc) {
        csr = dnc_read_csr_geo(node, bid, base + OFFS_LTBL + reg);
        csr |= (width << bit);
        dnc_write_csr_geo(node, bid, base + OFFS_LTBL + reg, csr);

        csr = dnc_read_csr_geo(node, bid, LC3_CSR_CONFIG4);
        csr &= ~(1 << 6); // Disbale CONFIG4.tblrd to enable normal routing operation
        dnc_write_csr_geo(node, bid, LC3_CSR_CONFIG4, csr);
    }

    DEBUG_ROUTE("add_route_geo: on %04x bid %d to %04x/%x over %d\n",
		node, bid, dest, width, link);
}

/* Remove route towards "dest" on blink id "bid" via "node".  Bitmask
   "width" signifies width of route. */
void del_route_geo(u16 dest, u16 node, u8 bid, u16 width)
{
    int scc = (bid == 0);
    u16 reg = SCI_ID_REGNR(dest) * 4;
    u16 bit = SCI_ID_BITNR(dest);
    u32 csr;

    if (!scc) {
        dnc_write_csr_geo(node, bid, LC3_CSR_SW_INFO3, SCI_ID_CHUNK(dest));
        
        csr = dnc_read_csr_geo(node, bid, LC3_CSR_CONFIG4);
        csr |= (1 << 6); // CONFIG4.tblrd is needed to read routing tables through CSR
        dnc_write_csr_geo(node, bid, LC3_CSR_CONFIG4, csr);
    
        csr = dnc_read_csr_geo(node, bid, LC3_CSR_ROUT_LCTBL00 + OFFS_LTBL + reg);
        csr &= ~(width << bit);
        dnc_write_csr_geo(node, bid, LC3_CSR_ROUT_LCTBL00 + OFFS_LTBL + reg, csr);

        csr = dnc_read_csr_geo(node, bid, LC3_CSR_CONFIG4);
        csr &= ~(1 << 6); // Disbale CONFIG4.tblrd to enable normal routing operation
        dnc_write_csr_geo(node, bid, LC3_CSR_CONFIG4, csr);
        
        DEBUG_ROUTE("del_route_geo: on %04x bid %d to %04x/%x\n",
                    node, bid, dest, width);
    }
}

