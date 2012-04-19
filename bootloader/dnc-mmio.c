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

#include <stdio.h>
#include "dnc-defs.h"
#include "dnc-access.h"
#include "dnc-commonlib.h"
#include "dnc-masterlib.h"
#include "dnc-bootloader.h"
#include "dnc-mmio.h"

static void mmio_pair_read(int node, int range, u32 *base, u32 *limit, int *flags)
{
    int sci = nc_node[node].sci_id;
    u32 base_low = dnc_read_conf(sci, 0, 0x18, NB_FUNC_MAPS, 0x80 + range * 8);
    u32 limit_low = dnc_read_conf(sci, 0, 0x18, NB_FUNC_MAPS, 0x84 + range * 8);
    u32 high = dnc_read_conf(sci, 0, 0x18, NB_FUNC_MAPS, 0x180 + range * 8);

    *base = ((base_low & 0xffffff00) >> (DRAM_MAP_SHIFT - 8)) | ((high & 0xff0000) << (40 - 16 - DRAM_MAP_SHIFT));
    *limit = (((limit_low & 0xffffff00) + 0x100) >> (DRAM_MAP_SHIFT - 8)) | ((high & 0xff) << (40 - DRAM_MAP_SHIFT));
    *flags = (base_low & 0xff) | ((limit_low & 0xff) << 8); /* Lock, WE, RE etc flags */
}

static void mmio_pair_write(int node, int range, u32 base, u32 limit, int ht, int link, int sublink)
{
    u32 base_low = (((u64)base << DRAM_MAP_SHIFT) & 0xffffff0000ULL) >> 8;
    u32 limit_low = (((((u64)limit << DRAM_MAP_SHIFT) - 1) & 0xffffff0000ULL) >> 8) |
	ht | (link << 4) | (sublink << 6);
    u32 high = ((((u64)base << DRAM_MAP_SHIFT) & 0xff0000000000ULL) >> 40) |
	(((((u64)limit << DRAM_MAP_SHIFT) - 1) & 0xff0000000000ULL) >> (40 - 16));
    int sci = nc_node[node].sci_id;

    dnc_write_conf(sci, 0, 0x18, NB_FUNC_MAPS, 0x80 + range * 8, base_low);
    dnc_write_conf(sci, 0, 0x18, NB_FUNC_MAPS, 0x84 + range * 8, limit_low);
    dnc_write_conf(sci, 0, 0x18, NB_FUNC_MAPS, 0x180 + range * 8, high);

    printf("node %d: adding MMIO range %d @ 0x%016llx-0x%016llx -> HT %d:%d.%d\n",
	node, range, (u64)base << DRAM_MAP_SHIFT, (u64)limit << DRAM_MAP_SHIFT, ht, link, sublink);

    base_low |= 11; /* Read, write and lock bits */
    dnc_write_conf(sci, 0, 0x18, NB_FUNC_MAPS, 0x80 + range * 8, base_low);
}

static int mmio_pair_add(int node, u32 base_new, u32 limit_new, int ht, int link, int sublink)
{
    int i;
    
    for (i = 0; i < 12; i++) {
	u32 base, limit;
	int flags;
	mmio_pair_read(node, i, &base, &limit, &flags);
	if (base || limit) /* Skip non-free entries */
	    continue;

	mmio_pair_write(node, i, base_new, limit_new, ht, link, sublink);
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
    int sci = nc_node[node].sci_id;
    int offset = 0;
    u32 val, val2;
    u64 addr;

    printf("node-global MMIO space at 0x%016llx\n", global);

    while (offset < 6) {
	/* Detect and disable any 32-bit I/O base */
	val = dnc_read_conf(sci, bus, dev, fun, 16 + offset * 4);
	if (val & 1) {
//	    dnc_write_conf(sci, bus, dev, fun, 16 + offset * 4, 0);
	    printf("[%d device %d:%d.%d] skipping incompatible PIO region 0x%08x\n", node, bus, dev, fun, val);
	    offset++;
	    continue;
	}

	/* Set all bits; read back to detect size and if 64-bit is valid */
	dnc_write_conf(sci, bus, dev, fun, 16 + offset * 4, 0xffffffff);
	val2 = dnc_read_conf(sci, bus, dev, fun, 16 + offset * 4);
	if ((val2 & 6) != 4) {
//	    dnc_write_conf(sci, bus, dev, fun, 16 + offset * 4, 0);
	    printf("[%d device %d:%d.%d] skipping incompatible MMIO region 0x%08x\n", node, bus, dev, fun, val);
	    offset++;
	    continue;
	}

	// current BAR is 64-bit capable
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
    int sci = nc_node[node].sci_id;
    int offset = 0;
    u32 val, val2;
    u64 addr;

    printf("node-global MMIO space at 0x%016llx\n", global);

    while (offset < 2) {
	/* Detect and disable any 32-bit I/O base */
	val = dnc_read_conf(sci, bus, dev, fun, 16 + offset * 4);
	if (val & 1) {
//	    dnc_write_conf(sci, bus, dev, fun, 16 + offset * 4, 0);
	    printf("[%d bridge %d:%d.%d] skipping incompatible PIO region 0x%08x\n", node, bus, dev, fun, val);
	    offset++;
	    continue;
	}

	/* Set all bits; read back to detect size and if 64-bit is valid */
	dnc_write_conf(sci, bus, dev, fun, 16 + offset * 4, 0xffffffff);
	val2 = dnc_read_conf(sci, bus, dev, fun, 16 + offset * 4);
	if ((val2 & 6) != 4) {
//	    dnc_write_conf(sci, bus, dev, fun, 16 + offset * 4, 0);
	    printf("[%d bridge %d:%d.%d] skipping incompatible MMIO region 0x%08x\n", node, bus, dev, fun, val);
	    offset++;
	    continue;
	}

	// current BAR is 64-bit capable
	addr = dnc_read_conf(sci, bus, dev, fun, 16 + offset * 4);
	offset++;
	dnc_write_conf(sci, bus, dev, fun, 16 + offset * 4, global);	
	addr |= (u64)dnc_read_conf(sci, bus, dev, fun, 16 + offset * 4) << 32;
	printf("[%03x bridge %d:%d.%d] setup BAR at offset %d to 0x%016llx\n", sci, bus, dev, fun, offset, addr);
	offset++;
    }

    /* Disable 32-bit expansion ROM base address */
    dnc_write_conf(sci, bus, dev, fun, 14*4, 0);

    /* Disable 32-bit I/O address space decoding in command register */
    val = dnc_read_conf(sci, bus, dev, fun, 4);
    dnc_write_conf(sci, bus, dev, fun, 4, val & ~1);
}

static void mmio_setup_bridge(int node, int ht, int link, int sublink, u32 base, u32 limit) {
    int sci = nc_node[node].sci_id;
    int bus, dev, fun, type;

    for (bus = 0; bus < 256; bus++)
	for (dev = 0; dev < 32; dev++)
	    for (fun = 0; fun < 8; fun++) {
		type = (dnc_read_conf(sci, bus, dev, fun, 0xc) >> 16) & 0xff;
		switch (type) {
		case 0xff: /* Master Abort, skip */
//		    printf("no device at node %d %02x:%02x.%x\n", node, bus, dev, fun);
		    continue;
		case 0:
		    printf("device at node %d %02x:%02x.%x\n", node, bus, dev, fun);
		    setup_device(node, bus, dev, fun);
		    break;
		case 1:
		    printf("bridge at node %d %02x:%02x.%x\n", node, bus, dev, fun);
		    setup_bridge(node, bus, dev, fun);
		    break;
		case 2:
		    printf("cardbus bridge at node %d %02x:%02x.%x - skipping\n", node, bus, dev, fun);
		    break;
		default:
		    printf("unknown device type %d at node %d %02x:%02x.%x - skipping\n", type, node, bus, dev, fun);
		}
	    }
}

/* Calculate contiguous MMIO regions */
void tally_remote_node_mmio(u16 node)
{
    int bridges[MAX_BRIDGES] = {0, };
    int i, flags;
    u32 base, limit, local_base;
    
    /* First node has MMIO setup below 4GB already */
    if (node == 0)
	return;

    local_base = dnc_top_of_mem;
    nc_node[node].mmio_base = dnc_top_of_mem;
    nc_node[node].mmio_end = nc_node[node].mmio_base;

    /* Check number of configured I/O bridges */
    for (i = 0; i < 12; i++) {
	mmio_pair_read(node, i, &base, &limit, &flags);
	if (!base && !limit) /* Skip empty entries */
	    continue;

	if (verbose)
	    printf("node %d: existing MMIO range %d from 0x%016llx-0x%016llx %s %s %s %s HT=%d link=%d sublink=%d flags=0x%08x\n",
	    node, i, (u64)base << DRAM_MAP_SHIFT, (u64)limit << DRAM_MAP_SHIFT,
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
	mmio_pair_add(node, local_base, nc_node[node].mmio_end, ht, link, sublink);
	mmio_setup_bridge(node, ht, link, sublink, local_base, nc_node[node].mmio_end);
	local_base += SCC_ATT_GRAN;
    }

    if (nc_node[node].mmio_end == nc_node[node].mmio_base) {
	printf("no bridges detected on node %d\n", node);
	return;
    }

    dnc_top_of_mem = nc_node[node].mmio_end;
}

/* Leave existing entries pointing to the root server, adding remote I/O regions */
int setup_remote_node_mmio(u16 node)
{
    int ret = 1, ht = nc_node[node].nc_ht_id;
    u32 top;

    /* Renumbering swaps proc @ HT0 with maxnode */
    if (renumber_bsp)
	ht = 0;

    /* Remote MMIO below this server's local MMIO */
    if (nc_node[node].mmio_base > dnc_top_of_dram)
        ret &= mmio_pair_add(node, dnc_top_of_dram, nc_node[node].mmio_base, ht, nc_neigh_link, 0);

    /* For root node, use top of DRAM */
    if (nc_node[node].mmio_end)
	top = nc_node[node].mmio_end;
    else
	top = dnc_top_of_dram;

    /* Remote MMIO above this server's local MMIO */
    if (top < dnc_top_of_mem)
	ret &= mmio_pair_add(node, top, dnc_top_of_mem, ht, nc_neigh_link, 0);

    return ret;
}

