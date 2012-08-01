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
#include <console.h>
#include <com32.h>
#include <inttypes.h>
#include <sys/io.h>

#include "dnc-regs.h"
#include "dnc-types.h"
#include "dnc-access.h"
#include "dnc-route.h"
#include "dnc-acpi.h"

#define NB_FUNC_HT    0
#define NB_FUNC_MAPS  1
#define NB_FUNC_DRAM  2
#define NB_FUNC_MISC  3
#define NB_FUNC_LINK  4

com32sys_t inreg, outreg;
u32 dnc_ht_node = 2;

unsigned char msleep(unsigned int msec)
{
    unsigned long micro = 1000*msec;

    inreg.eax.b[1] = 0x86;
    inreg.ecx.w[0] = (micro >> 16);
    inreg.edx.w[0] = (micro & 0xFFFF);
    __intcall(0x15, &inreg, &outreg);
    return outreg.eax.b[1];
}

void add_extd_mmio_maps(int node, int idx, u64 start, u64 end, int dest)
{
    u32 val;
    u64 mask;

    mask = 0;
    start = start >> 27;
    end   = end >> 27;
    while ((start | mask) != (end | mask))
        mask = (mask << 1) | 1;

    /* CHtExtAddrEn */
    val	= cht_read_conf(node, NB_FUNC_HT, 0x68);
    cht_write_conf(node, NB_FUNC_HT, 0x68, val | (1<<25));

    /* Set ExtMmioMapAddSel granularity to 128M */
    val	= cht_read_conf(node,  NB_FUNC_HT, 0x168);
    cht_write_conf(node, NB_FUNC_HT, 0x168, (val & ~0x300) | 0x200);

    /* Direct FF00_0000_0000 - FFFF_FFFF_FFFF towards DNC node */
    cht_write_conf(node, NB_FUNC_MAPS, 0x110, (2 << 28) | idx);
    cht_write_conf(node, NB_FUNC_MAPS, 0x114, (start << 8) | dest);
    cht_write_conf(node, NB_FUNC_MAPS, 0x110, (3 << 28) | idx);
    cht_write_conf(node, NB_FUNC_MAPS, 0x114, (mask << 8) | 1);
}

void reset_remote(u32 node)
{
    u8 portcf9;

    /* Direct 08fd_fc00_0000 to node 0x008 00fd_fc00_0000 for remote PCI I/O */
    add_extd_mmio_maps(0, 1, 0x08fdfc000000ULL, 0x08fdfc000000ULL,
		       dnc_ht_node);
    add_extd_mmio_maps(1, 1, 0x08fdfc000000ULL, 0x08fdfc000000ULL,
		       dnc_ht_node);

    dnc_write_csr(0xfff0, H2S_CSR_G0_ATT_INDEX, 0xa00008fd);
    dnc_write_csr(0xfff0, H2S_CSR_G0_ATT_ENTRY, 0x00000008);

    dnc_write_conf(node, 0, 24+2, 1, H2S_CSR_F1_RESOURCE_MAPPING_CAPABILITY_HEADER,
		   0x4c020008);
    dnc_write_conf(node, 0, 24+2, 1, H2S_CSR_F1_RESOURCE_MAPPING_ENTRY_INDEX,
		   7);
    dnc_write_conf(node, 0, 24+2, 1, H2S_CSR_F1_EXT_D_MMIO_ADDRESS_BASE_REGISTERS,
		   (0x08fdfc000000 >> 27 << 8) | 0xc0000000);
    dnc_write_conf(node, 0, 24+2, 1, H2S_CSR_F1_EXT_D_MMIO_ADDRESS_MASK_REGISTERS,
		   (0x000000ffffff >> 27 << 8) | 1);     

    printf("Resetting node %x in 5 seconds...\n", node);
    msleep(5000);

    portcf9 = mem64_read8(0x08fdfc000cf9);
    printf("Node %x port cf9: %x\n", node, portcf9);
    mem64_write8(0x08fdfc000cf9, 0xa);
    mem64_write8(0x08fdfc000cf9, 0xe);
}

int main(void)
{
    u32 val;
    openconsole(&dev_rawcon_r, &dev_stdcon_w);

    val = dnc_read_csr(0xfff0, H2S_CSR_G0_NODE_IDS);
    reset_remote((val >> 16) ^ 0x000c);

    return -1;
}
