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
#include "dnc-defs.h"
#include "dnc-access.h"
#include "dnc-commonlib.h"
#include "dnc-masterlib.h"
#include "dnc-bootloader.h"
#include "dnc-mmio.h"

#define _base(range)  ((range < 8 ? 0x080 : (0x1a0 - 0x40)) + range * 8)
#define _limit(range) ((range < 8 ? 0x084 : (0x1a4 - 0x40)) + range * 8)
#define _high(range)  ((range < 8 ? 0x180 : (0x1c0 - 0x20)) + range * 4)

static void mmio_range_read_fam10(u16 sci, int range, u64 *base, u64 *limit, int *flags)
{
    u32 mask;
    u32 val = (dnc_read_conf(sci, 0, 24, FUNC0_HT, 0x168) >> 8) & 3;
    u32 shift = 19 + val * 4;

    dnc_write_conf(sci, 0, 24, FUNC1_MAPS, 0x110, (3 << 28) | (range - 8));
    mask = (dnc_read_conf(sci, 0, 24, FUNC1_MAPS, 0x114) >> 8) & 0x1fffff;
    *limit = 0; /* FIXME */
    dnc_write_conf(sci, 0, 24, FUNC1_MAPS, 0x110, (2 << 28) | (range - 8));
    val = dnc_read_conf(sci, 0, 24, FUNC1_MAPS, 0x114);
    *base = (u64)val << shift;

    if (verbose)
	printf("SCI%03x: MMIO range %d base=0x%016llx mask=0x%08x\n", sci, range, *base, mask);
}

static void mmio_range_read(u16 sci, int range, u64 *base, u64 *limit, int *flags)
{
    assert(family < 0x15 ? (range < (8 + 16)) : (range < 12));

    if (family < 0x15 && range > 7) {
	mmio_range_read_fam10(sci, range, base, limit, flags);
	return;
    }

    u32 base_low = dnc_read_conf(sci, 0, 24, FUNC1_MAPS, _base(range));
    u32 limit_low = dnc_read_conf(sci, 0, 24, FUNC1_MAPS, _limit(range));

    *base = ((u64)base_low & (~0xff)) << (16 - 8);
    *limit = (((u64)limit_low & (~0xff)) << (16 - 8)) | 0xffff;
    *flags = (base_low & 0xff) | ((limit_low & 0xff) << 8); /* Lock, WE, RE etc flags */
    if (family >= 0x15) {
	u32 high = dnc_read_conf(sci, 0, 24, FUNC1_MAPS, _high(range));
	*base |= ((u64)high & 0xff0000) << (40 - 16);
	*limit |= ((u64)high & 0xff) << 40;
    }

    if (verbose)
	printf("SCI%03x: MMIO range %d @ 0x%016llx-0x%016llx, flags 0x%x\n",
	    sci, range, *base, *limit, *flags);
}

void mmio_range_write(u16 sci, int range, u64 base, u64 limit, int ht, int link, int sublink)
{
    assert(family < 0x15 ? (range < (8 + 16)) : (range < 12));

    if (verbose)
	printf("SCI%03x: adding MMIO range %d @ 0x%016llx-0x%016llx -> HT %d:%d.%d\n",
	    sci, range, base << DRAM_MAP_SHIFT, limit << DRAM_MAP_SHIFT, ht, link, sublink);

    u32 base_low = (((base << DRAM_MAP_SHIFT) & 0xffffff0000ULL) >> 8);
    /* Use limit to indicate if range is enabled, ie set read, write bits */
    if (limit)
	base_low |= 3;

    u32 limit_low = ((((limit << DRAM_MAP_SHIFT) - 1) & 0xffffff0000ULL) >> 8) |
	ht | (link << 4) | (sublink << 6);

    if (family >= 0x15) {
	u32 high = (((base << DRAM_MAP_SHIFT) & 0xff0000000000ULL) >> 40) |
	    ((((limit << DRAM_MAP_SHIFT) - 1) & 0xff0000000000ULL) >> (40 - 16));
	dnc_write_conf(sci, 0, 24, FUNC1_MAPS, _high(range), high);
    }

    dnc_write_conf(sci, 0, 24, FUNC1_MAPS, _limit(range), limit_low);
    dnc_write_conf(sci, 0, 24, FUNC1_MAPS, _base(range), base_low);
}

void mmio_show(u16 sci)
{
    int i;
    int regs = family < 0x15 ? (8 + 2) : 12; /* Should be 8+12, but it hangs */

    for (i = 0; i < regs; i++) {
	u64 base, limit;
	int flags;
	mmio_range_read(sci, i, &base, &limit, &flags);
    }
}

void dram_show(u16 sci)
{
    int i;

    printf("SCI%03x: F1xF0=%08x F2x110=%08x F2x114=%08x F2x118=%08x F2x11C=%08x\n", sci,
	dnc_read_conf(sci, 0, 24, FUNC1_MAPS, 0xf0),
	dnc_read_conf(sci, 0, 24, FUNC2_DRAM, 0x110),
	dnc_read_conf(sci, 0, 24, FUNC2_DRAM, 0x114),
	dnc_read_conf(sci, 0, 24, FUNC2_DRAM, 0x118),
	dnc_read_conf(sci, 0, 24, FUNC2_DRAM, 0x11c)
    );

    for (i = 0; i < 8; i++) {
	u32 base_low = dnc_read_conf(sci, 0, 24, FUNC1_MAPS, 0x40 + i * 8);
	u32 limit_low = dnc_read_conf(sci, 0, 24, FUNC1_MAPS, 0x44 + i * 8);
	u32 base_high = dnc_read_conf(sci, 0, 24, FUNC1_MAPS, 0x140 + i * 8);
	u32 limit_high = dnc_read_conf(sci, 0, 24, FUNC1_MAPS, 0x144 + i * 8);
	u64 base = (((u64)base_low & 0xffff0000) << 8) | (((u64)base_high & 0xff) << 40);
	u64 limit = (((u64)limit_low & 0xffff0000) << 8) | (((u64)limit_high & 0xff) << 40) | 0xffffff;
	printf("SCI%03x: DRAM range=%d base=0x%016llx limit=0x%016llx r=%d w=%d node=%d intl=%d\n", sci, i, base, limit,
		base_low & 1, !!(base_low & 2), limit_low & 3, (limit_low >> 8) & 7);
    }
}

void io_show(u16 sci)
{
    int i;

    for (i = 0; i < 4; i++)
	printf("SCI%03x: I/O maps base=0x%08x limit=%08x\n", sci,
	    dnc_read_conf(sci, 0, 24, FUNC1_MAPS, 0xc0 + i * 8),
	    dnc_read_conf(sci, 0, 24, FUNC1_MAPS, 0xc4 + i * 8));

    for (i = 0; i < 4; i++)
	printf("SCI%03x: config maps base=0x%08x\n", sci,
	    dnc_read_conf(sci, 0, 24, FUNC1_MAPS, 0xe0 + i * 4));
}

static int mmio_range_add(u16 sci, u64 base_new, u64 limit_new, int ht, int link, int sublink)
{
    int i;
    int regs = family >= 0x15 ? 12 : 8;
    
    for (i = 0; i < regs; i++) {
	u64 base, limit;
	int flags;
	mmio_range_read(sci, i, &base, &limit, &flags);
	if (base || limit) /* Skip non-free entries */
	    continue;

	mmio_range_write(sci, i, base_new, limit_new, ht, link, sublink);
	break;
    }

    if (i == 12)
    	return 0;
    return 1;
}

/* Add a bridge entry if not present, returning 1 if unique */
static int bridge_add_unique(int *bridges, int flags)
{
    int i;

    /* Leave only HT, link, sublink bits */
    flags &= (7 << 8) | (3 << 12) | (1 << 14);

    for (i = 0; i < MAX_BRIDGES; i++) {
	if (bridges[i] == flags)
	    return 0; /* Not new */

	if (bridges[i] == 0) {
	    bridges[i] = flags;
	    if ((i + 1) < MAX_BRIDGES)
	    	bridges[i + 1] = 0;
	    return 1; /* New */
	}
    }

    /* Safely ignore additional bridges */
    return 0;
}

static void setup_device(int node, int bus, int dev, int fun)
{
    u64 global = nc_node[node].mmio_base >> (32 - DRAM_MAP_SHIFT);
    u16 sci = nc_node[node].sci_id;
    int offset = 0;
    u32 val, val2;
    u64 addr;

    printf("node-global MMIO space at 0x%016llx\n", global);

    while (offset < 6) {
	/* Detect and disable any 32-bit I/O base */
	val = dnc_read_conf(sci, bus, dev, fun, 16 + offset * 4);
	if (val & 1) {
/*	    dnc_write_conf(sci, bus, dev, fun, 16 + offset * 4, 0); */
	    printf("[%d device %d:%d.%d] skipping incompatible PIO region 0x%08x\n", node, bus, dev, fun, val);
	    offset++;
	    continue;
	}

	/* Set all bits; read back to detect size and if 64-bit is valid */
	dnc_write_conf(sci, bus, dev, fun, 16 + offset * 4, 0xffffffff);
	val2 = dnc_read_conf(sci, bus, dev, fun, 16 + offset * 4);
	if ((val2 & 6) != 4) {
/*	    dnc_write_conf(sci, bus, dev, fun, 16 + offset * 4, 0); */
	    printf("[%d device %d:%d.%d] skipping incompatible MMIO region 0x%08x\n", node, bus, dev, fun, val);
	    offset++;
	    continue;
	}

	/* Current BAR is 64-bit capable */
	addr = dnc_read_conf(sci, bus, dev, fun, 16 + offset * 4);
	offset++;
	dnc_write_conf(sci, bus, dev, fun, 16 + offset * 4, global);	
	addr |= (u64)dnc_read_conf(sci, bus, dev, fun, 16 + offset * 4) << 32;
	printf("[%03x device %d:%d.%d] setup BAR at offset %d to 0x%016llx\n", sci, bus, dev, fun, offset, addr);
	offset++;
    }

    /* Disable 32-bit expansion ROM base address */
    dnc_write_conf(sci, bus, dev, fun, 48, 0);

    /* Disable 32-bit I/O address space decoding in command register */
    val = dnc_read_conf(sci, bus, dev, fun, 4);
    dnc_write_conf(sci, bus, dev, fun, 4, val & ~1);
}

static void setup_bridge(int node, int bus, int dev, int fun)
{
    u64 global = nc_node[node].mmio_base >> (32 - DRAM_MAP_SHIFT);
    u16 sci = nc_node[node].sci_id;
    int offset = 0;
    u32 val, val2;
    u64 addr;

    printf("Node-global MMIO space at 0x%016llx\n", global);

    while (offset < 2) {
	/* Detect and disable any 32-bit I/O base */
	val = dnc_read_conf(sci, bus, dev, fun, 16 + offset * 4);
	if (val & 1) {
/*	    dnc_write_conf(sci, bus, dev, fun, 16 + offset * 4, 0); */
	    printf("[%d bridge %d:%d.%d] Skipping incompatible PIO region 0x%08x\n", node, bus, dev, fun, val);
	    offset++;
	    continue;
	}

	/* Set all bits; read back to detect size and if 64-bit is valid */
	dnc_write_conf(sci, bus, dev, fun, 16 + offset * 4, 0xffffffff);
	val2 = dnc_read_conf(sci, bus, dev, fun, 16 + offset * 4);
	if ((val2 & 6) != 4) {
/*	    dnc_write_conf(sci, bus, dev, fun, 16 + offset * 4, 0); */
	    printf("[%d bridge %d:%d.%d] Skipping incompatible MMIO region 0x%08x\n", node, bus, dev, fun, val);
	    offset++;
	    continue;
	}

	/* Current BAR is 64-bit capable */
	addr = dnc_read_conf(sci, bus, dev, fun, 16 + offset * 4);
	offset++;
	dnc_write_conf(sci, bus, dev, fun, 16 + offset * 4, global);	
	addr |= (u64)dnc_read_conf(sci, bus, dev, fun, 16 + offset * 4) << 32;
	printf("[%03x bridge %d:%d.%d] Setup BAR at offset %d to 0x%016llx\n", sci, bus, dev, fun, offset, addr);
	offset++;
    }

    /* Disable 32-bit expansion ROM base address */
    dnc_write_conf(sci, bus, dev, fun, 14*4, 0);

    /* Disable 32-bit I/O address space decoding in command register */
    val = dnc_read_conf(sci, bus, dev, fun, 4);
    dnc_write_conf(sci, bus, dev, fun, 4, val & ~1);
}

static void mmio_setup_bridge(int node, int ht, int link, int sublink, u32 base, u32 limit) {
    u16 sci = nc_node[node].sci_id;
    int bus, dev, fun, type;

    for (bus = 0; bus < 256; bus++)
	for (dev = 0; dev < 32; dev++)
	    for (fun = 0; fun < 8; fun++) {
		type = (dnc_read_conf(sci, bus, dev, fun, 0xc) >> 16) & 0xff;
		switch (type) {
		case 0xff: /* Master Abort, skip */
/*		    printf("No device at node %d %02x:%02x.%x\n", node, bus, dev, fun); */
		    continue;
		case 0:
		    printf("Device at node %d %02x:%02x.%x\n", node, bus, dev, fun);
		    setup_device(node, bus, dev, fun);
		    break;
		case 1:
		    printf("Bridge at node %d %02x:%02x.%x\n", node, bus, dev, fun);
		    setup_bridge(node, bus, dev, fun);
		    break;
		case 2:
		    printf("Cardbus bridge at node %d %02x:%02x.%x - skipping\n", node, bus, dev, fun);
		    break;
		default:
		    printf("Unknown device type %d at node %d %02x:%02x.%x - skipping\n", type, node, bus, dev, fun);
		}
	    }
}

/* Calculate contiguous MMIO regions */
void tally_remote_node_mmio(u16 node)
{
    int bridges[MAX_BRIDGES] = {0, };
    int i, flags;
    u16 sci = nc_node[node].sci_id;
    u64 base, limit, local_base;
    
    /* First node has MMIO setup below 4GB already */
    if (node == 0)
	return;

    local_base = dnc_top_of_mem;
    nc_node[node].mmio_base = dnc_top_of_mem;
    nc_node[node].mmio_end = nc_node[node].mmio_base;

    /* Check number of configured I/O bridges */
    for (i = 0; i < 12; i++) {
	mmio_range_read(sci, i, &base, &limit, &flags);
	if (!base && !limit) /* Skip empty entries */
	    continue;

	if (verbose)
	    printf("Node %d: existing MMIO range %d from 0x%016llx-0x%016llx %s %s %s %s HT=%d link=%d sublink=%d flags=0x%08x\n",
	    node, i, base << DRAM_MAP_SHIFT, limit << DRAM_MAP_SHIFT,
	    (flags & 1) ? "R" : "", (flags & 2) ? "W" : "", (flags & 8) ? "L" : "",
	    (flags & 0x8000) ? "NP" : "P", (flags >> 8) & 7, (flags >> 12) & 3, (flags >> 14) & 1, flags);

	/* Check if bridge wasn't previously seen */
	bridge_add_unique(bridges, flags);
    }
    
    /* Setup each found bridge */
    for (i = 0; bridges[i]; i++) {
	int ht = (bridges[i] >> 8) & 7;
	int link = (bridges[i] >> 12) & 3;
	int sublink = (bridges[i] >> 14) & 1;

	/* Renumbering swaps proc @ HT0 with maxnode */
	if (renumber_bsp && ht == 0)
	    ht = nc_node[node].nc_ht_id;

	nc_node[node].mmio_end += SCC_ATT_GRAN;

	/* Add local bridge high MMIO entry */
	mmio_range_add(sci, local_base, nc_node[node].mmio_end, ht, link, sublink);
	mmio_setup_bridge(node, ht, link, sublink, local_base, nc_node[node].mmio_end);
	local_base += SCC_ATT_GRAN;
    }

    if (nc_node[node].mmio_end == nc_node[node].mmio_base) {
	printf("No bridges detected on node %d\n", node);
	return;
    }

    dnc_top_of_mem = nc_node[node].mmio_end;
}

/* Leave existing entries pointing to the root server, adding remote I/O regions */
int setup_remote_node_mmio(u16 node)
{
    int ret = 1;
    int ht = nc_node[node].nc_ht_id;
    u16 sci = nc_node[node].sci_id;
    u32 top;

    /* Renumbering swaps proc @ HT0 with maxnode */
    if (renumber_bsp)
	ht = 0;

    /* Remote MMIO below this server's local MMIO */
    if (nc_node[node].mmio_base > dnc_top_of_dram)
        ret &= mmio_range_add(sci, dnc_top_of_dram, nc_node[node].mmio_base, ht, nc_neigh_link, 0);

    /* For root node, use top of DRAM */
    if (nc_node[node].mmio_end)
	top = nc_node[node].mmio_end;
    else
	top = dnc_top_of_dram;

    /* Remote MMIO above this server's local MMIO */
    if (top < dnc_top_of_mem)
	ret &= mmio_range_add(sci, top, dnc_top_of_mem, ht, nc_neigh_link, 0);

    return ret;
}

