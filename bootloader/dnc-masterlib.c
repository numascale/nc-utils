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
#include "dnc-defs.h"
#include "dnc-types.h"
#include "dnc-access.h"
#include "dnc-route.h"
#include "dnc-fabric.h"
#include "dnc-mmio.h"

#include "dnc-bootloader.h"
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

void load_scc_microcode(u16 node)
{
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

void tally_local_node(int enforce_alignment)
{
    u32 val, base, limit, rest;
    u16 i, j, max_ht_node, tot_cores;
    u16 last = 0;

    nc_node[0].node_mem = 0;
    tot_cores = 0;
    nc_node[0].sci_id = dnc_read_csr(0xfff0, H2S_CSR_G0_NODE_IDS) >> 16;
    nc_node[0].nc_ht_id = dnc_master_ht_id;
    nc_node[0].nc_neigh = nc_neigh;
    nc_node[0].addr_base = 0;

    val = cht_read_conf(0, NB_FUNC_HT, 0x60);
    max_ht_node = (val >> 4) & 7;

#ifdef __i386
    /* Save and restore EBX for the position-independent syslinux com32 binary */
    asm volatile("mov $0x80000008, %%eax; pushl %%ebx; cpuid; popl %%ebx" : "=c"(val) :: "eax", "edx");
#else
    asm volatile("mov $0x80000008, %%eax; cpuid" : "=c"(val) :: "eax", "ebx", "edx");
#endif
    apic_per_node = 1 << ((val >> 12) & 0xf);
    
    nc_node[0].apic_offset = 0;

    for (i = 0; i <= max_ht_node; i++) {
        if (i == dnc_master_ht_id)
            continue;

        printf("Examining SCI%03x HT node %d...\n", nc_node[0].sci_id, i);
        
	nc_node[0].ht[i].cores = 0;
	nc_node[0].ht[i].base  = 0;
	nc_node[0].ht[i].size  = 0;

	nc_node[0].ht[i].cpuid = cht_read_conf(0, NB_FUNC_MISC, 0xfc);
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

        base = cht_read_conf(i, NB_FUNC_MAPS, 0x120);
        limit = cht_read_conf(i, NB_FUNC_MAPS, 0x124);

	if (limit & 0x1fffff) {
	    nc_node[0].ht[i].base = (base & 0x1fffff) << (27 - DRAM_MAP_SHIFT);
	    nc_node[0].ht[i].size = ((limit & 0x1fffff) - (base & 0x1fffff) + 1) << (27 - DRAM_MAP_SHIFT);
	    nc_node[0].node_mem += nc_node[0].ht[i].size;
	    last = i;
	    if (nc_node[0].node_mem > max_mem_per_node) {
		printf("** Node exceeds cachable memory range, clamping...\n");
		nc_node[0].ht[i].size -= nc_node[0].node_mem - max_mem_per_node;
		nc_node[0].node_mem = max_mem_per_node;
		limit = nc_node[0].ht[i].base + nc_node[0].ht[i].size - 1;

		asm volatile("wbinvd" ::: "memory");
		for (j = 0; j <= max_ht_node; j++) {
		    if (j == dnc_master_ht_id)
			continue;
		    if (!nc_node[0].ht[j].cpuid)
			continue;
		    cht_write_conf(j, NB_FUNC_MAPS, 0x44 + i*8, (limit << 16) |
				     (cht_read_conf(j, NB_FUNC_MAPS, 0x44 + i*8) & 0xffff));
		}
		cht_write_conf(i, NB_FUNC_MAPS, 0x124, limit >> (27 - DRAM_MAP_SHIFT));
		asm volatile("wbinvd" ::: "memory");
	    }
	}

	/* Assume at least one core */
	nc_node[0].ht[i].cores = 1;

	if (family < 0x15) {
	    val = cht_read_conf(i, NB_FUNC_HT, 0x68);
	    if (val & 0x20) nc_node[0].ht[i].cores++; /* Cpu1En */

	    val = cht_read_conf(i, NB_FUNC_HT, 0x168);
	    if (val & 0x01) nc_node[0].ht[i].cores++; /* Cpu2En */
	    if (val & 0x02) nc_node[0].ht[i].cores++; /* Cpu3En */
	    if (val & 0x04) nc_node[0].ht[i].cores++; /* Cpu4En */
	    if (val & 0x08) nc_node[0].ht[i].cores++; /* Cpu5En */
	}
	else {
	    val = cht_read_conf(i, 5, 0x84);
	    nc_node[0].ht[i].cores += val & 0xff;
	    val = cht_read_conf(i, 3, 0x190);
	    while (val > 0) {
		if (val & 1)
		    nc_node[0].ht[i].cores--;
		val = val >> 1;
	    }
	}

	nc_node[0].ht[i].apic_base = post_apic_mapping[tot_cores];
	ht_next_apic = nc_node[0].ht[i].apic_base + apic_per_node;
 
	tot_cores += nc_node[0].ht[i].cores;
    }

    printf("ht_next_apic: %d\n", ht_next_apic);

    printf("%2d CPU cores and %2d GBytes of memory and I/O maps found in SCI%03x\n",
           tot_cores, nc_node[0].node_mem >> 6, nc_node[0].sci_id);

    dnc_top_of_mem = nc_node[0].ht[last].base + nc_node[0].ht[last].size;
    nc_node[0].addr_end = dnc_top_of_mem;

    rest = dnc_top_of_mem & (SCC_ATT_GRAN-1);
    if (rest && enforce_alignment) {
	printf("Deducting 0x%x from node %d to accommodate granularity requirements\n",
	       rest, last);
	asm volatile("wbinvd" ::: "memory");
	nc_node[0].ht[last].size -= rest;
	dnc_top_of_mem -= rest;
	for (i = 0; i <= max_ht_node; i++) {
	    if (i == dnc_master_ht_id)
		continue;
	    if (!nc_node[0].ht[i].cpuid)
		continue;
	    limit = cht_read_conf(i, NB_FUNC_MAPS, 0x44 + last*8);
	    cht_write_conf(i, NB_FUNC_MAPS, 0x44 + last*8, limit - (rest << 16));
	}
        limit = cht_read_conf(last, NB_FUNC_MAPS, 0x124);
        cht_write_conf(last, NB_FUNC_MAPS, 0x124, limit - (rest >> (27 - DRAM_MAP_SHIFT)));
	asm volatile("wbinvd" ::: "memory");
    }
    
    printf("Initializing SCI%03x PCI I/O and IntRecCtrl tables...\n", nc_node[0].sci_id);
    /* Set PCI I/O map */
    dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00);
    for (i = 0; i < 256; i++)
       dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i*4, nc_node[0].sci_id);
    
    /* Set IntRecCtrl */
    dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x30);
    for (i = 0; i < 256; i++)
        dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i*4, 0);

    dnc_node_count++;
}

static int tally_remote_node(u16 node)
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
        printf("Setting SCI%03x MCFG_BASE to %08llx\n", node, DNC_MCFG_BASE >> 24);
        dnc_write_csr(node, H2S_CSR_G3_MMCFG_BASE, DNC_MCFG_BASE >> 24);
    }
    
    cur_node = &nc_node[dnc_node_count];
    cur_node->node_mem = 0;
    tot_cores = 0;
    cur_node->sci_id = node;
    cur_node->nc_ht_id = dnc_read_csr(node, H2S_CSR_G3_HT_NODEID) & 0xf;
    cur_node->nc_neigh = nc_neigh; /* FIXME: Read from remote somehow, instead of assuming the same as ours */

    /* Ensure that all nodes start out on 1G boundaries
       FIXME: Add IO holes to cover address space discontinuity? */ 
    dnc_top_of_mem = (dnc_top_of_mem + (0x3fffffff >> DRAM_MAP_SHIFT)) & ~(0x3fffffff >> DRAM_MAP_SHIFT);
    cur_node->addr_base = dnc_top_of_mem;

    val = dnc_read_conf(node, 0, 24, NB_FUNC_HT, 0x60);
    if (val == 0xffffffff) {
        printf("Error: Can't access config space on SCI%03x\n", node);
        return 1; /* Ignore node */
    }
    max_ht_node = (val >> 4) & 7;
    
    dnc_write_csr(node, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00000020); /* Select APIC ATT */
    for (i = 0; i < 16; i++) {
	apic_used[i] = dnc_read_csr(node, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i*4);
	if (apic_used[i] != 0) /* May be sparse */
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

        printf("Examining SCI%03x HT node %d...\n", node, i);
       
	cur_node->ht[i].cores = 0;
	cur_node->ht[i].base  = 0;
	cur_node->ht[i].size  = 0;

	cur_node->ht[i].cpuid = dnc_read_conf(node, 0, 24+i, NB_FUNC_MISC, 0xfc);
	if ((cur_node->ht[i].cpuid == 0) ||
	    (cur_node->ht[i].cpuid == 0xffffffff) ||
	    (cur_node->ht[i].cpuid != nc_node[0].ht[0].cpuid))
	{
	    printf("Error: Node has unknown/incompatible CPUID: %08x; skipping...\n", cur_node->ht[i].cpuid);
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
		limit -= ((val >> 8) & 0xff) - base;
	    cur_node->ht[i].base = dnc_top_of_mem;
	    cur_node->ht[i].size = limit - base + 1;
            cur_node->node_mem += cur_node->ht[i].size;
	    if (cur_node->node_mem > max_mem_per_node) {
		printf("Error: Node exceeds cachable memory range; clamping...\n");
		cur_node->ht[i].size -= cur_node->node_mem - max_mem_per_node;
		cur_node->node_mem = max_mem_per_node;
	    }

	    dnc_top_of_mem += cur_node->ht[i].size;
	}

	while ((cur_apic < 256) && !(apic_used[cur_apic >> 4] & (1 << (cur_apic & 0xf))))
	    cur_apic++;
	
	/* Assume at least one core */
	cur_node->ht[i].cores = 1;

	if (family < 0x15) {
	    val = dnc_read_conf(node, 0, 24+i, NB_FUNC_HT, 0x68);
	    if (val & 0x20) cur_node->ht[i].cores++; /* Cpu1En */

	    val = dnc_read_conf(node, 0, 24+i, NB_FUNC_HT, 0x168);
	    if (val & 0x01) cur_node->ht[i].cores++; /* Cpu2En */
	    if (val & 0x02) cur_node->ht[i].cores++; /* Cpu3En */
	    if (val & 0x04) cur_node->ht[i].cores++; /* Cpu4En */
	    if (val & 0x08) cur_node->ht[i].cores++; /* Cpu5En */
	}
	else {
	    val = dnc_read_conf(node, 0, 24+i, NB_FUNC_EXTD, 0x84);
	    cur_node->ht[i].cores += val & 0xff;
	    val = dnc_read_conf(node, 0, 24+i, NB_FUNC_MISC, 0x190);
	    while (val > 0) {
		if (val & 1)
		    cur_node->ht[i].cores--;
		val = val >> 1;
	    }
	}
	cur_node->ht[i].apic_base = cur_apic;

        tot_cores += cur_node->ht[i].cores;
	cur_apic += cur_node->ht[i].cores;
	last = i;
    }

    /* If rebased apicid[7:0] of last core is above a given threshold,
       bump base for entire SCI node to next 8-bit interval */
    if ((ht_next_apic & 0xff) + cur_node->ht[last].apic_base + cur_node->ht[last].cores > 0xf0)
	ht_next_apic = (ht_next_apic & ~0xff) + 0x100 + cur_node->ht[0].apic_base;

    cur_node->apic_offset = ht_next_apic - cur_node->ht[0].apic_base;
    ht_next_apic = cur_node->apic_offset + cur_node->ht[last].apic_base + apic_per_node;

    printf("ht_next_apic: %d\n", ht_next_apic);

    cur_node->addr_end = dnc_top_of_mem;

    printf("%2d CPU cores and %2d GBytes of memory found in SCI%03x\n",
           tot_cores, cur_node->node_mem >> 6, node);

    printf("Initializing SCI%03x PCI I/O and IntRecCtrl tables...\n", node);
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

    /* MMIO is added after DRAM */
    dnc_top_of_dram = dnc_top_of_mem;

    if (!remote_io)
	return ret;

    for (node = 0; node < dnc_node_count; node++)
	tally_remote_node_mmio(node);

    for (node = 0; node < dnc_node_count; node++)
	ret &= setup_remote_node_mmio(node);

    printf("DRAM top is 0x%016llx; MMIO top is 0x%016llx\n",
	(u64)dnc_top_of_dram << DRAM_MAP_SHIFT, (u64)dnc_top_of_mem << DRAM_MAP_SHIFT);

    return ret;
}
