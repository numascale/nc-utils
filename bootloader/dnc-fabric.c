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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "dnc-regs.h"
#include "dnc-access.h"
#include "dnc-fabric.h"
#include "dnc-commonlib.h"
#include "dnc-bootloader.h"

/* RAW CSR accesses, use the RAW engine in SCC to make remote CSR accesses */

static inline void _setrawentry(uint32_t index, uint64_t entry)
{
    dnc_write_csr(0xfff0, H2S_CSR_G0_RAW_INDEX, index);
    dnc_write_csr(0xfff0, H2S_CSR_G0_RAW_ENTRY_LO, (uint32_t)(entry >> 32));
    dnc_write_csr(0xfff0, H2S_CSR_G0_RAW_ENTRY_HI, (uint32_t)(entry & 0xffffffff));
}

static inline uint64_t _getrawentry(uint32_t index)
{
    uint32_t lo, hi;
    
    dnc_write_csr(0xfff0, H2S_CSR_G0_RAW_INDEX, index);
    lo = dnc_read_csr(0xfff0, H2S_CSR_G0_RAW_ENTRY_LO);
    hi = dnc_read_csr(0xfff0, H2S_CSR_G0_RAW_ENTRY_HI);
    return (uint64_t)lo << 32 | hi;
}

static int _raw_read(uint32_t dest, int geo, uint32_t addr, uint32_t *val)
{
    uint16_t ownnodeid;
    uint32_t ctrl;
    uint32_t cmd;

    cmd = (addr & 0xc) | 0x3; /* readsb */

    ownnodeid = dnc_read_csr(0xfff0, H2S_CSR_G0_NODE_IDS) >> 16;

    dnc_write_csr(0xfff0, H2S_CSR_G0_RAW_CONTROL, 0x1000); /* Reset RAW engine */

    _setrawentry(0, (0x1fULL << 48) | (((uint64_t)dest & 0xffffULL) << 32) | ((uint64_t)cmd << 16) | ownnodeid);
    if (geo) {
        _setrawentry(1, (0x1fULL << 48) | (0xffffeULL << 28) | (addr & 0x7ffc)); /* Bits 14:11 contains bxbarid */
    }
    else {
        _setrawentry(1, (0x1fULL << 48) | (0xfffffULL << 28) | (addr & 0x0ffc));
    }

    dnc_write_csr(0xfff0, H2S_CSR_G0_RAW_CONTROL, 0x2); /* Start RAW access */

    udelay(100);
    
    ctrl = dnc_read_csr(0xfff0, H2S_CSR_G0_RAW_CONTROL);
    if ((ctrl & 0xc00) != 0) {
        printf("RAW packet timeout : %08x\n", ctrl);
        *val = 0xffffffff;
        return -1;
    }
    else {
        if (((ctrl >> 5) & 0xf) != 4) {
            printf("Wrong response packet size : %08x\n", ctrl);
            printf("Entry 0: %016" PRIx64 "\n", _getrawentry(0));
            printf("Entry 1: %016" PRIx64 "\n", _getrawentry(1));
            printf("Entry 2: %016" PRIx64 "\n", _getrawentry(2));
            printf("Entry 3: %016" PRIx64 "\n", _getrawentry(3));
            *val = 0xffffffff;
            return -1;
        } else {
            switch (addr & 0xc) {
                case 0x0: *val = _getrawentry(2) >> 32; break;
                case 0x4: *val = _getrawentry(2) & 0xffffffff; break;
                case 0x8: *val = _getrawentry(3) >> 32; break;
                case 0xc: *val = _getrawentry(3) & 0xffffffff; break;
            }
        }
    }

    dnc_write_csr(0xfff0, H2S_CSR_G0_RAW_CONTROL, 0x1000); /* Reset RAW engine */
    
    return 0;
}

int dnc_raw_read_csr(uint32_t node, uint16_t csr, uint32_t *val)
{
    return _raw_read(node, 0, csr, val);
}

int dnc_raw_read_csr_geo(uint32_t node, uint8_t bid, uint16_t csr, uint32_t *val)
{
    if (csr >= 0x800) {
	printf("*** dnc_write_csr_geo: read from unsupported range: "
	       "%04x#%d @%x\n",
	       node, bid, csr);
        *val = 0xffffffff;
        return -1;
    }

    return _raw_read(node, 1, (bid << 11) | csr, val);
}

/* Fabric routines */

void dnc_reset_phy(int phy)
{
    uint32_t val;
    val = dnc_read_csr(0xfff0, H2S_CSR_G0_PHYXA_LINK_CTR + 0x40 * phy);
    dnc_write_csr(0xfff0, H2S_CSR_G0_PHYXA_LINK_CTR + 0x40 * phy, val | (1<<7) | (1<<6) | (1<<13) | (1<<12));
    udelay(1000);
}

void dnc_reset_lc3(int lc)
{
    uint32_t val;
    val = dnc_read_csr(0xfff0, H2S_CSR_G0_PHYXA_LINK_CTR + 0x40 * lc);
    dnc_write_csr(0xfff0, H2S_CSR_G0_PHYXA_LINK_CTR + 0x40 * lc, val | (1<<6) | (1<<13) | (1<<12));
    udelay(1000);
}

int dnc_check_phy(int phy)
{
    const char *phyname = _get_linkname(phy);
    uint32_t stat, elog, ctrl, tries;

    tries = 0;
again:    
    /* Enable the dynamic frequency drift control (only FPGA has this so far) */
    ctrl = dnc_read_csr(0xfff0, H2S_CSR_G0_PHYXA_LINK_CTR + 0x40 * phy);
    dnc_write_csr(0xfff0, H2S_CSR_G0_PHYXA_LINK_CTR + 0x40 * (phy-1), ctrl | (1<<13) | (1<<12));

    stat = dnc_read_csr(0xfff0, H2S_CSR_G0_HSSXA_STAT_1 + 0x40 * phy);
    if (!(stat & (1<<8)) || (stat & 0xff)) {
	if (dnc_asic_mode && dnc_chip_rev < 2) {
	    if (tries++ > 4)
		return -1;
	    printf("HSS%s STAT_1 is 0x%x; issuing HSS reset\n", phyname, stat);
	    /* Trigger a HSS PLL reset */
	    dnc_write_csr(0xfff0, H2S_CSR_G1_PIC_RESET_CTRL, 1);
	    udelay(500);
	    (void)dnc_read_csr(0xfff0, H2S_CSR_G1_PIC_INDIRECT_READ); /* Use a read operation to terminate the current I2C transaction, to avoid a bug in the uC */
	    udelay(2000);
	    goto again;
	} else {
	    printf("HSS%s STAT_1 is 0x%x; NEED HSS RESET!\n", phyname, stat);
	    return -1;
	}
    }
    
    stat = dnc_read_csr(0xfff0, H2S_CSR_G0_PHYXA_LINK_STAT + 0x40 * phy);
    elog = dnc_read_csr(0xfff0, H2S_CSR_G0_PHYXA_ELOG + 0x40 * phy);
    if (((stat & 1) == 0) || ((elog & 0xff) != 0)) {
        printf("PHY%s LINK_STAT 0x%x, ELOG 0x%x\n", phyname, stat, elog);
        return -1;
    }

    return 0;
}

int dnc_check_lc3(int lc)
{
    const char *linkname = _get_linkname(lc);
    uint32_t initst, error_count;
    const uint32_t fatal_mask0 = 0x0;
    const uint32_t fatal_mask1 = 0x0;

    if (dnc_raw_read_csr(0xfff1 + lc, LC3_CSR_INIT_STATE, &initst) != 0) {
        return -1;
    }
    if (dnc_raw_read_csr(0xfff1 + lc, LC3_CSR_ERROR_COUNT, &error_count) != 0) {
        return -1;
    }
    if ((((initst & 7) != 2) && ((initst & 7) != 4))) {
        printf("LC3%s INIT_STATE 0x%x\n", linkname, initst);
        return -1;
    }
    if (error_count != 0) {
        uint32_t elog0, elog1;
        if (dnc_raw_read_csr(0xfff1 + lc, LC3_CSR_ELOG0, &elog0) != 0) {
            return -1;
        }
        if (dnc_raw_read_csr(0xfff1 + lc, LC3_CSR_ELOG1, &elog1) != 0) {
            return -1;
        }
        dnc_write_csr(0xfff1 + lc, LC3_CSR_ERROR_COUNT, 0);
        dnc_write_csr(0xfff1 + lc, LC3_CSR_ELOG0, 0);
        dnc_write_csr(0xfff1 + lc, LC3_CSR_ELOG0, 0);
        printf("LC3%s ERROR_COUNT %d, ELOG0 0x%04x, ELOG1 0x%04x\n", linkname, error_count, elog0, elog1);
        if ((elog0 & fatal_mask0) || (elog1 & fatal_mask1))
            return -1;
    }
    
    return 0;
}

int dnc_init_lc3(uint16_t nodeid, int lc, uint16_t maxchunk,
                 uint16_t rtbll[], uint16_t rtblm[], uint16_t rtblh[], uint16_t ltbl[])
{
    const char *linkname = _get_linkname(lc);
    uint16_t expected_id = (nodeid | ((lc+1) << 13));
    uint32_t error_count1, error_count2;
    uint16_t chunk, offs;

    printf("Initializing LC3%s...\n", linkname);

    if (dnc_raw_read_csr(0xfff1 + lc, LC3_CSR_ERROR_COUNT, &error_count1) != 0)
        return -1;

    dnc_write_csr(0xfff1 + lc, LC3_CSR_NODE_IDS, expected_id << 16);
    dnc_write_csr(0xfff1 + lc, LC3_CSR_SAVE_ID, expected_id);
/*    dnc_write_csr(0xfff1 + lc, LC3_CSR_CONFIG1,
                  dnc_read_csr(0xfff0 + lc, LC3_CSR_CONFIG1)); */
    dnc_write_csr(0xfff1 + lc, LC3_CSR_CONFIG2,
                  (dnc_read_csr(0xfff1 + lc, LC3_CSR_CONFIG2) & ~(0xf0)) | (forwarding_mode << 6) | (forwarding_mode << 4));
/*    dnc_write_csr(0xfff1 + lc, LC3_CSR_CONFIG3,
                  dnc_read_csr(0xfff1 + lc, LC3_CSR_CONFIG3) | (1<<12));*/ /* Set fatal2dead */

    /* 1. Disable table routing
       2. Initialize routing table
       3. Enable table routing */

    /* 1. Disable table routing */
    dnc_write_csr(0xfff1 + lc, LC3_CSR_ROUT_CTRL, 3 << 14); /* ROUT_CTRL.rtype = 2'b11 (all) */

    /* 2. Initialize routing table */
    for (chunk = 0; chunk < maxchunk; chunk++) {
        dnc_write_csr(0xfff1 + lc, LC3_CSR_SW_INFO3, chunk);
        for (offs = 0; offs < 16; offs++) {
            dnc_write_csr(0xfff1 + lc, LC3_CSR_ROUT_LCTBL00  + (offs<<2), ltbl[(chunk<<4)+offs]);
            dnc_write_csr(0xfff1 + lc, LC3_CSR_ROUT_BXTBLL00 + (offs<<2), rtbll[(chunk<<4)+offs]);
            dnc_write_csr(0xfff1 + lc, LC3_CSR_ROUT_BLTBL00  + (offs<<2), rtblm[(chunk<<4)+offs]);
            dnc_write_csr(0xfff1 + lc, LC3_CSR_ROUT_BXTBLH00 + (offs<<2), rtblh[(chunk<<4)+offs]);
        }
    }
    printf("done\n");

    /* 3. Enable table routing */
    dnc_write_csr(0xfff1 + lc, LC3_CSR_ROUT_CTRL, 1 << 14); /* ROUT_CTRL.rtype = 2'b01 (table routing) */

    if (dnc_raw_read_csr(0xfff1 + lc, LC3_CSR_ERROR_COUNT, &error_count2) != 0)
        return -1;

    if (error_count1 != error_count2) {
        printf("Errors while initializing LC3%s (%d != %d)\n",
               linkname, error_count1, error_count2);
        return -1;
    }
    
    return 0;
}
