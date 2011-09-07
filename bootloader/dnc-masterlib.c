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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "dnc-regs.h"
#include "dnc-types.h"
#include "dnc-access.h"
#include "dnc-route.h"
#include "dnc-fabric.h"

#include "dnc-commonlib.h"
#include "dnc-masterlib.h"

#include "hw-config.h"

// -------------------------------------------------------------------------

#include "../interface/numachip-mseq-ucode.h"
#include "../interface/numachip-mseq-table.h"

u32 *mseq_ucode = numachip_mseq_ucode;
u16 *mseq_table = numachip_mseq_table;
int mseq_ucode_length = (sizeof(numachip_mseq_ucode)/sizeof(numachip_mseq_ucode[0]));
int mseq_table_length = (sizeof(numachip_mseq_table)/sizeof(numachip_mseq_table[0]));

#define NUMACHIP_MSEQ_UCODE_LEN (mseq_ucode_length)
#define NUMACHIP_MSEQ_TABLE_LEN (mseq_table_length)

void load_scc_microcode(u16 node) {
    u32 val;
    u16 i;

    dnc_write_csr(node, H2S_CSR_G0_SEQ_INDEX, 0x80000000);
    for (i = 0; i < NUMACHIP_MSEQ_UCODE_LEN; i++)
        dnc_write_csr(node, H2S_CSR_G0_WCS_ENTRY, mseq_ucode[i]);

    dnc_write_csr(node, H2S_CSR_G0_SEQ_INDEX, 0x80000000);
    for (i = 0; i < NUMACHIP_MSEQ_TABLE_LEN; i++)
        dnc_write_csr(node, H2S_CSR_G0_JUMP_ENTRY, mseq_table[i]);

    /* Start the microsequencer */
    val = dnc_read_csr(node, H2S_CSR_G0_STATE_CLEAR);
    dnc_write_csr(node, H2S_CSR_G0_STATE_CLEAR, val);
}

extern int nc_neigh, nc_neigh_link;

void tally_local_node(int enforce_alignment)
{
    u32 val, base, limit, rest;
    u16 i, max_ht_node, tot_cores;
    u16 last = 0;

    nc_node[0].node_mem = 0;
    tot_cores = 0;
    nc_node[0].sci_id = dnc_read_csr(0xfff0, H2S_CSR_G0_NODE_IDS) >> 16;
    nc_node[0].nc_ht_id = dnc_master_ht_id;
    nc_node[0].nc_neigh = nc_neigh;
    nc_node[0].addr_base = 0;

    val = cht_read_config(0, NB_FUNC_HT, 0x60);
    max_ht_node = (val >> 4) & 7;

#ifdef __i386
    /* save and restore EBX for the position-independent syslinux com32 binary */
    asm volatile("mov $0x80000008, %%eax; pushl %%ebx; cpuid; popl %%ebx" : "=c"(val) :: "eax", "edx");
#else
    asm volatile("mov $0x80000008, %%eax; cpuid" : "=c"(val) :: "eax", "ebx", "edx");
#endif
    apic_per_node = 1 << ((val >> 12) & 0xf);
    
    nc_node[0].apic_offset = 0;

    for (i = 0; i <= max_ht_node; i++) {
        if (i == dnc_master_ht_id)
            continue;

        printf("Examining SCI node %03x HT node %d...\n", nc_node[0].sci_id, i);
        
	nc_node[0].ht[i].cores = 0;
	nc_node[0].ht[i].base  = 0;
	nc_node[0].ht[i].size  = 0;

	nc_node[0].ht[i].cpuid = cht_read_config(0, NB_FUNC_MISC, 0xfc);
	if ((nc_node[0].ht[i].cpuid == 0) ||
	    (nc_node[0].ht[i].cpuid == 0xffffffff) ||
	    (nc_node[0].ht[i].cpuid != nc_node[0].ht[0].cpuid))
	{
	    printf("** Node has unknown/incompatible CPUID: %08x, skipping...\n", nc_node[0].ht[i].cpuid);
	    nc_node[0].ht[i].cpuid = 0;
	    nc_node[0].ht[i].pdom = 0;
	    continue;
	}
	    
	nc_node[0].ht[i].pdom = ht_pdom_count++;

        base = cht_read_config(i, NB_FUNC_MAPS, 0x120);
        limit = cht_read_config(i, NB_FUNC_MAPS, 0x124);

	if (limit & 0x1fffff) {
	    nc_node[0].ht[i].base = (base & 0x1fffff) << (27 - DRAM_MAP_SHIFT);
	    nc_node[0].ht[i].size = ((limit & 0x1fffff) - (base & 0x1fffff) + 1) << (27 - DRAM_MAP_SHIFT);
	    nc_node[0].node_mem += nc_node[0].ht[i].size;
	    last = i;
	}

        /* Assume at least one core */
        nc_node[0].ht[i].cores = 1;
        val = cht_read_config(i, NB_FUNC_HT, 0x68);
        if (val & 0x20) nc_node[0].ht[i].cores++; /* Cpu1En */

        val = cht_read_config(i, NB_FUNC_HT, 0x168);
        if (val & 0x01) nc_node[0].ht[i].cores++; /* Cpu2En */
        if (val & 0x02) nc_node[0].ht[i].cores++; /* Cpu3En */
        if (val & 0x04) nc_node[0].ht[i].cores++; /* Cpu4En */
        if (val & 0x08) nc_node[0].ht[i].cores++; /* Cpu5En */
	nc_node[0].ht[i].apic_base = post_apic_mapping[tot_cores];
	ht_next_apic = nc_node[0].ht[i].apic_base + apic_per_node;
 
	tot_cores += nc_node[0].ht[i].cores;
    }

    printf("ht_next_apic: %d\n", ht_next_apic);

    printf("%2d CPU cores and %2d GBytes of memory found in SCI node %03x\n",
           tot_cores, nc_node[0].node_mem >> 6, nc_node[0].sci_id);

    dnc_top_of_mem = nc_node[0].ht[last].base + nc_node[0].ht[last].size;
    nc_node[0].addr_end = dnc_top_of_mem;

    rest = dnc_top_of_mem & (SCC_ATT_GRAN-1);
    if (rest && enforce_alignment) {
	printf("Deducting %x from node %d to accommodate granularity requirements.\n",
	       rest, last);
	asm volatile("wbinvd" ::: "memory");
	nc_node[0].ht[last].size -= rest;
	dnc_top_of_mem -= rest;
	for (i = 0; i <= max_ht_node; i++) {
	    if (i == dnc_master_ht_id)
		continue;
	    if (!nc_node[0].ht[i].cpuid)
		continue;
	    limit = cht_read_config(i, NB_FUNC_MAPS, 0x44 + last*8);
	    cht_write_config(i, NB_FUNC_MAPS, 0x44 + last*8, limit - (rest << 16));
	}
        limit = cht_read_config(last, NB_FUNC_MAPS, 0x124);
        cht_write_config(last, NB_FUNC_MAPS, 0x124, limit - (rest >> (27 - DRAM_MAP_SHIFT)));
	asm volatile("wbinvd" ::: "memory");
    }
    
    printf("Initializing sci node %03x PCI I/O and IntRecCtrl tables...\n", nc_node[0].sci_id);
    /* Set PCI I/O map */
    dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00);
    for (i = 0; i < 256; i++)
       dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i*4, nc_node[0].sci_id);
    
    /* Set IntRecCtrl */
    dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x30);
    for (i = 0; i < 256; i++)
        dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i*4, 0);

    /* Set MMCFG base register so local NC will forward correctly */
    val = dnc_read_csr(0xfff0, H2S_CSR_G3_MMCFG_BASE);
    if (val != (DNC_MCFG_BASE >> 24)) {
        printf("Setting local MCFG_BASE to %08llx\n", DNC_MCFG_BASE >> 24);
        dnc_write_csr(0xfff0, H2S_CSR_G3_MMCFG_BASE, DNC_MCFG_BASE >> 24);
    }
    
    dnc_node_count++;
}

int tally_remote_node(u16 node)
{
    u32 val, base, limit;
    u16 i, max_ht_node, tot_cores;
    u16 apic_used[16];
    u16 last = 0;
    u16 cur_apic;
    nc_node_info_t *cur_node;

    if (dnc_raw_read_csr(node, H2S_CSR_G3_FAB_CONTROL, &val) != 0) {
        printf("Can't find node %04x!\n", node);
        return 0;
    }

    /* Set MMCFG base register so remote NC will accept our forwarded requests */
    val = dnc_read_csr(node, H2S_CSR_G3_MMCFG_BASE);
    if (val != (DNC_MCFG_BASE >> 24)) {
        printf("Setting SCI node %03x MCFG_BASE to %08llx\n", node, DNC_MCFG_BASE >> 24);
        dnc_write_csr(node, H2S_CSR_G3_MMCFG_BASE, DNC_MCFG_BASE >> 24);
    }
    
    cur_node = &nc_node[dnc_node_count];
    cur_node->node_mem = 0;
    tot_cores = 0;
    cur_node->sci_id = node;
    cur_node->nc_ht_id = dnc_read_csr(node, H2S_CSR_G3_HT_NODEID) & 0xf;
    cur_node->nc_neigh = nc_neigh; // FIXME: read from remote somehow, instead of assuming the same as ours

    /* Ensure that all nodes start out on 1G boundaries.
       FIXME: Add IO holes to cover address space discontinuity? */ 
    dnc_top_of_mem = (dnc_top_of_mem + (0x3fffffff >> DRAM_MAP_SHIFT)) & ~(0x3fffffff >> DRAM_MAP_SHIFT);
    cur_node->addr_base = dnc_top_of_mem;

    val = dnc_read_conf(node, 0, 24, NB_FUNC_HT, 0x60);
    if (val == 0xffffffff) {
        printf("Can't access config space on SCI node %03x!\n", node);
        return 1; // ignore node
    }
    max_ht_node = (val >> 4) & 7;
    
    dnc_write_csr(node, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00000020); // Select APIC ATT
    for (i = 0; i < 16; i++) {
	apic_used[i] = dnc_read_csr(node, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i*4);
	if (apic_used[i] == 0) /* skip trailing empty entries */
	    break;
	printf("apic_used[%d]: %08x\n", i, apic_used[i]);
    }

    ht_next_apic = (ht_next_apic + 0xf) & ~0xf;
    cur_node->apic_offset = ht_next_apic;
    cur_apic = 0;

    for (i = 0; i <= max_ht_node; i++) {
        if (i == cur_node->nc_ht_id) {
	    cur_node->ht[i].cpuid = 0;
            continue;
	}

        printf("Examining SCI node %03x HT node %d...\n", node, i);
       
	cur_node->ht[i].cores = 0;
	cur_node->ht[i].base  = 0;
	cur_node->ht[i].size  = 0;

	cur_node->ht[i].cpuid = dnc_read_conf(node, 0, 24+i, NB_FUNC_MISC, 0xfc);
	if ((cur_node->ht[i].cpuid == 0) ||
	    (cur_node->ht[i].cpuid == 0xffffffff) ||
	    (cur_node->ht[i].cpuid != nc_node[0].ht[0].cpuid))
	{
	    printf("** Node has unknown/incompatible CPUID: %08x, skipping...\n", cur_node->ht[i].cpuid);
	    cur_node->ht[i].pdom = 0;
	    cur_node->ht[i].cpuid = 0;
	    continue;
	}
	cur_node->ht[i].pdom = ht_pdom_count++;

        base  = dnc_read_conf(node, 0, 24+i, NB_FUNC_MAPS, 0x120);
        limit = dnc_read_conf(node, 0, 24+i, NB_FUNC_MAPS, 0x124);

	if (limit & 0x1fffff) {
	    base = (base & 0x1fffff) << (27 - DRAM_MAP_SHIFT);
	    limit = (limit & 0x1fffff) << (27 - DRAM_MAP_SHIFT);
	    limit = limit | (0xffffffff >> (32 - 27 + DRAM_MAP_SHIFT));

	    val = dnc_read_conf(node, 0, 24+i, NB_FUNC_MAPS, 0xf0);
	    if ((val & 3) == 3)
		limit -= (val >> 8) & 0xff;
	    cur_node->ht[i].base = dnc_top_of_mem;
	    cur_node->ht[i].size = limit - base + 1;
            cur_node->node_mem += cur_node->ht[i].size;
	    if (cur_node->node_mem > (MAX_MEM_PER_NODE >> DRAM_MAP_SHIFT)) {
		printf("** Node exceeds cachable memory range, clamping...\n");
		cur_node->ht[i].size -=
		    cur_node->node_mem - (MAX_MEM_PER_NODE >> DRAM_MAP_SHIFT);
		cur_node->node_mem = MAX_MEM_PER_NODE >> DRAM_MAP_SHIFT;
	    }

	    dnc_top_of_mem += cur_node->ht[i].size;
	}

	while ((cur_apic < 256) && !(apic_used[cur_apic >> 4] & (1 << (cur_apic & 0xf))))
	    cur_apic++;
	
	/* Assume at least one core */
        cur_node->ht[i].cores = 1;

        val = dnc_read_conf(node, 0, 24+i, NB_FUNC_HT, 0x68);
        if (val & 0x20) cur_node->ht[i].cores++; /* Cpu1En */

        val = dnc_read_conf(node, 0, 24+i, NB_FUNC_HT, 0x168);
        if (val & 0x01) cur_node->ht[i].cores++; /* Cpu2En */
        if (val & 0x02) cur_node->ht[i].cores++; /* Cpu3En */
        if (val & 0x04) cur_node->ht[i].cores++; /* Cpu4En */
        if (val & 0x08) cur_node->ht[i].cores++; /* Cpu5En */
	cur_node->ht[i].apic_base = cur_apic;

        tot_cores += cur_node->ht[i].cores;
	cur_apic += cur_node->ht[i].cores;
	last = i;
	printf("SCI%03x HT#%d: apic_base: %d, cores: %d, cur_apic: %d\n",
	       cur_node->sci_id, i,
	       cur_node->ht[i].apic_base,
	       cur_node->ht[i].cores,
	       cur_apic);
    }

    /* If rebased apicid[7:0] of last core is above a given threshold,
       bump base for entire SCI node to next 8-bit interval. */
    if ((ht_next_apic & 0xff) + cur_node->ht[last].apic_base + cur_node->ht[last].cores > 0xf0)
	ht_next_apic = (ht_next_apic & ~0xff) + 0x100 + cur_node->ht[0].apic_base;

    cur_node->apic_offset = ht_next_apic - cur_node->ht[0].apic_base;
    ht_next_apic = cur_node->apic_offset + cur_node->ht[last].apic_base + apic_per_node;

    printf("SCI%03x: apic_offset: %d\n",
	   cur_node->sci_id,
	   cur_node->apic_offset);

    printf("ht_next_apic: %d\n", ht_next_apic);

    cur_node->addr_end = dnc_top_of_mem;

    printf("%2d CPU cores and %2d GBytes of memory found in SCI node %03x\n",
           tot_cores, cur_node->node_mem >> 6, node);

    printf("Initializing sci node %03x PCI I/O and IntRecCtrl tables...\n", node);
    /* Set PCI I/O map */
    dnc_write_csr(node, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00);
    for (i = 0; i < 256; i++)
        dnc_write_csr(node, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i*4, nc_node[0].sci_id);
    
    /* Set IntRecCtrl */
    dnc_write_csr(node, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x30);
    for (i = 0; i < 256; i++)
        dnc_write_csr(node, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i*4, 0);

    dnc_node_count++;
    return 1;
}

int tally_all_remote_nodes(void)
{
    int ret = 1;
    u16 node;
    for (node = 1; node < 4096; node++) {
        if ((nodedata[node] & 0xc0) != 0x80)
            continue;
        ret = tally_remote_node(node) && ret;
    }
    return ret;
}

int walk_ring_lc3ccr(u8 dim)
{
    u16 linkida = 2*dim + 1;
    u16 linkidb = 2*dim + 2;
    u16 targetid, destid, lastid, saveida, saveidb;
    u16 nextid = (1<<(dim*4));
    uint32_t val1, val2, val3;
    uint32_t sccnodeid, lc3nodeid;
    int i, done;

    // Counter rotating LC3 rings
    if (!_get_linkname(linkida) || !_get_linkname(linkidb)) {
        printf("Invalid ring dimension %d!\n", dim);
        return -1;
    }

    printf("Dimension = %d, nextid = %03x\n", dim, nextid);
    sccnodeid = dnc_read_csr(0xfff0, H2S_CSR_G0_NODE_IDS);
    sccnodeid = (sccnodeid >> 16) & 0xfff; // 12 bit NodeID

check_link:
    done = 0;
    while (!done) {
        done = (dnc_check_phy(linkida) == 0);
        done &= (dnc_check_phy(linkidb) == 0);

        if (done) {
            done &= (dnc_check_lc3(linkida) == 0);
            done &= (dnc_check_lc3(linkidb) == 0);
        }

        if (!done) {
            printf("PHY errors on PHY%s and PHY%s, issuing PHY reset!\n",
                   _get_linkname(linkida), _get_linkname(linkidb));
            // Counter rotating rings, reset both PHYs
            dnc_reset_phy(linkida); dnc_reset_phy(linkidb);
        }
    }

    saveida = dnc_read_csr(0xfff0 + linkida, LC3_CSR_NODE_IDS) & 0x0000ffff;
    saveidb = dnc_read_csr(0xfff0 + linkidb, LC3_CSR_NODE_IDS) & 0x0000ffff;

    printf("LC3%s NODE_IDS = %04x, LC3%s NODE_IDS = %04x\n",
           _get_linkname(linkida), saveida, _get_linkname(linkidb), saveidb);
           
    // Set Local LC3 NODEIDs
    lc3nodeid = sccnodeid | (linkida << 13);
    dnc_write_csr(0xfff0 + linkida, LC3_CSR_NODE_IDS, lc3nodeid << 16);
    lc3nodeid = sccnodeid | (linkidb << 13);
    dnc_write_csr(0xfff0 + linkidb, LC3_CSR_NODE_IDS, lc3nodeid << 16);

    dnc_write_csr(0xfff0 + linkida, LC3_CSR_ROUT_CTRL, 1 << 14); // ROUT_CTRL.rtype = 2'b01 (table routing)
    dnc_write_csr(0xfff0 + linkidb, LC3_CSR_ROUT_CTRL, 1 << 14); // ROUT_CTRL.rtype = 2'b01 (table routing)
    
    set_route(sccnodeid, 0xfff0 + linkida, 0x1, 0); // Make sure responses get back to SCC
    set_route(sccnodeid, 0xfff0 + linkidb, 0x1, 0); // Make sure responses get back to SCC
    
    //
    // First we check that the scrubber is scrubber in both directions.
    //

    targetid = 0xffef;
    if (saveida != targetid) {
        // We are not scrubber ourselves
        // Check A Scrubber
        set_route(0xffe0, 0xfff0, 0xffff, linkida); // Route all hwinit nodes over correct link
        if (dnc_raw_read_csr_geo(targetid, linkidb, LC3_CSR_NODE_IDS, &val1) != 0) {
            printf("Can't find scrubber on %s ring, resetting link!\n",
                   _get_linkname(linkida));
            // Counter rotating rings, reset both PHYs
            dnc_reset_phy(linkida); dnc_reset_phy(linkidb);
            goto check_link;
        }

        // Check B Scrubber
        set_route(0xffe0, 0xfff0, 0xffff, linkidb); // Route all hwinit nodes over correct link
        if (dnc_raw_read_csr_geo(targetid, linkida, LC3_CSR_NODE_IDS, &val2) != 0) {
            printf("Can't find scrubber on %s ring, resetting link!\n",
                   _get_linkname(linkidb));
            // Counter rotating rings, reset both PHYs
            dnc_reset_phy(linkida); dnc_reset_phy(linkidb);
            goto check_link;
        }
        
        if ((val1 != val2) || ((val1 & 0x0000ffff) != targetid) || ((val2 & 0x0000ffff) != targetid)) {
            printf("Scrubber on %s ring is not scrubber on %s ring, check cable setup!\n",
                   _get_linkname(linkida), _get_linkname(linkidb));
            return -1;
        }
    } else {
        if (saveida != saveidb) {
            printf("Scrubber on %s ring is not scrubber on %s ring, check cable setup!\n",
                   _get_linkname(linkida), _get_linkname(linkidb));
            return -1;
        }
    }

    //
    // Now we can proceed to read the scrubber neighbour node to find the lowest nodeid.
    //

    targetid = 0xffee;
    if (saveida != targetid) {
        set_route(0xffe0, 0xfff0, 0xffff, linkida); // Route all hwinit nodes over correct link
        if (dnc_raw_read_csr_geo(targetid, linkidb, LC3_CSR_NODE_IDS, &val1) != 0) {
            printf("Can't find hwid %04x on %s ring, resetting link!\n",
                   targetid, _get_linkname(linkida));
            // Counter rotating rings, reset both PHYs
            dnc_reset_phy(linkida); dnc_reset_phy(linkidb);
            goto check_link;
        }
        lastid = val1 & 0x0000ffff;
    } else {
        lastid = saveidb;
    }

    // Max 16 nodes per ring, 0xffe0-0xffef
    if (lastid < 0xffe0 || lastid > 0xffef) {
        printf("Invalid hwid found (%04x), check cable setup!\n", lastid);
        return -1;
    }

    printf("Found %d nodes on the %s and %s rings\n",
           0xffef - lastid, _get_linkname(linkida), _get_linkname(linkidb));

    // Use A ring to check all nodes and collect their UID.
    // Also reset the appropriate routing register and enumerate with proper nodeIDs.
    set_route(0xffe0, 0xfff0, 0xffff, linkida); // Route all hwinit nodes over correct link
    set_route(nextid, 0xfff0, 0xffff, linkida); // Route the correct routing segment over the correct link

    destid = nextid;
    for (i = 0; i < (0xfff0 - lastid); i++) {
        u32 uid;
        int tries;

        targetid = 0xffef - i;
        if (targetid == saveida)
            continue;

        if (dnc_raw_read_csr(targetid, LC3_CSR_UID1, &val1) != 0) {
            printf("Can't find hwid %04x on %s ring, resetting link!\n",
                   targetid, _get_linkname(linkida));
            // Counter rotating rings, reset both PHYs
            dnc_reset_phy(linkida); dnc_reset_phy(linkidb);
            goto check_link;
        }
        if (dnc_raw_read_csr(targetid, LC3_CSR_UID2, &val2) != 0) {
            printf("Can't find hwid %04x on %s ring, resetting link!\n",
                   targetid, _get_linkname(linkida));
            // Counter rotating rings, reset both PHYs
            dnc_reset_phy(linkida); dnc_reset_phy(linkidb);
            goto check_link;
        }
        if (dnc_raw_read_csr(targetid, LC3_CSR_SAVE_ID, &val3) != 0) {
            printf("Can't find hwid %04x on %s ring, resetting link!\n",
                   targetid, _get_linkname(linkida));
            // Counter rotating rings, reset both PHYs
            dnc_reset_phy(linkida); dnc_reset_phy(linkidb);
            goto check_link;
        }

        uid = ((val3 & 7) << 29) | (val2 << 13) | (val1 >> 3);

        if ((val1 & 7) != linkida) {
            printf("Ring inconsistency detected, hwid %04x (UID:%08x) on the %s ring does not have the correct BXBAR ID (%d != %d)\n",
                   targetid, uid, _get_linkname(linkida), val1 & 7, linkida);
            return -1;
        }

        printf("Initializing fabric on sci node %03x (hwid:%04x, UID:%08x)\n", destid, targetid, uid);

        // Add route on both target LC3s to route requests/responses to SCC
        set_route(destid, targetid, 0x1, 0);
        set_route_geo(destid, targetid, linkidb, 0x1, 0);

        // Communicate with the slave loader via LC3 SW_INFO register as a work-around for the SCC geo-access bug.
        dnc_write_csr(targetid, LC3_CSR_SW_INFO2, 0xffff); // The routing mask to use
        dnc_write_csr(targetid, LC3_CSR_SW_INFO, sccnodeid | 0x8000); 
        val1 = 0;
#define MAX_TRIES 10
        for (tries = 0; !(val1 & 0x4000) && (tries < MAX_TRIES); tries++) {
            if (dnc_raw_read_csr(targetid, LC3_CSR_SW_INFO, &val1) != 0) {
                printf("Can't read SW_INFO on hwid %04x on %s ring, resetting link!\n",
                       targetid, _get_linkname(linkida));
                // Counter rotating rings, reset both PHYs
                dnc_reset_phy(linkida); dnc_reset_phy(linkidb);
                goto check_link;
            }
            tsc_wait(200);
        }
        
        if (tries >= MAX_TRIES) {
            printf("Slave node with hwid %04x (UID:%08x) on the %s ring did not set routing properly in time, resetting link!\n",
                   targetid, uid, _get_linkname(linkida));
            // Counter rotating rings, reset both PHYs
            dnc_reset_phy(linkida); dnc_reset_phy(linkidb);
            goto check_link;
        }

        // Set both LC3s to use table routing
        dnc_write_csr(targetid, LC3_CSR_ROUT_CTRL, 1 << 14); // ROUT_CTRL.rtype = 2'b01 (table routing)
        dnc_write_csr_geo(targetid, linkidb, LC3_CSR_ROUT_CTRL, 1 << 14); // ROUT_CTRL.rtype = 2'b01 (table routing)
        
        dnc_write_csr_geo(targetid, 0, H2S_CSR_G0_NODE_IDS, destid << 16);
        
        lc3nodeid = (destid | (linkidb << 13));
        dnc_write_csr_geo(targetid, linkidb, LC3_CSR_NODE_IDS, lc3nodeid << 16);

        lc3nodeid = (destid | (linkida << 13));
        dnc_write_csr(targetid, LC3_CSR_NODE_IDS, lc3nodeid << 16);

        if (dnc_raw_read_csr(destid, H2S_CSR_G0_NODE_IDS, &val1) != 0) {
            printf("Failed verifying sci node %03x SCC NODE_IDS...\n", destid);
            return -1;
        }
    
        nodedata[destid] = 0x80;
        destid += nextid;
    }

    return (int)destid;
}
