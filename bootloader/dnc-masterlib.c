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

#include "dnc-regs.h"
#include "dnc-defs.h"
#include "dnc-access.h"
#include "dnc-route.h"
#include "dnc-fabric.h"
#include "dnc-mmio.h"
#include "dnc-bootloader.h"
#include "dnc-commonlib.h"
#include "dnc-masterlib.h"

#include "../interface/numachip-mseq.h"

void load_scc_microcode(uint16_t node)
{
	uint32_t val;
	uint16_t i;
	const uint32_t *mseq_ucode;
	const uint16_t *mseq_table;
	int mseq_ucode_length, mseq_table_length;

	if (!dnc_asic_mode || (dnc_chip_rev == 2)) {
		/* Use this microcode for FPGA and RevC asic */
		mseq_ucode = numachip_mseq_ucode_revc;
		mseq_table = numachip_mseq_table_revc;
		mseq_ucode_length = sizeof(numachip_mseq_ucode_revc) / sizeof(numachip_mseq_ucode_revc[0]);
		mseq_table_length = sizeof(numachip_mseq_table_revc) / sizeof(numachip_mseq_table_revc[0]);
	} else if (dnc_asic_mode && dnc_chip_rev < 2) {
		/* Use this microcode for RevA and RevB asic */
		mseq_ucode = numachip_mseq_ucode_revb;
		mseq_table = numachip_mseq_table_revb;
		mseq_ucode_length = sizeof(numachip_mseq_ucode_revb) / sizeof(numachip_mseq_ucode_revb[0]);
		mseq_table_length = sizeof(numachip_mseq_table_revb) / sizeof(numachip_mseq_table_revb[0]);
	} else
		fatal("No microcode for NumaChip version %d\n", dnc_chip_rev);

	dnc_write_csr(node, H2S_CSR_G0_SEQ_INDEX, 0x80000000);

	for (i = 0; i < mseq_ucode_length; i++)
		dnc_write_csr(node, H2S_CSR_G0_WCS_ENTRY, mseq_ucode[i]);

	dnc_write_csr(node, H2S_CSR_G0_SEQ_INDEX, 0x80000000);

	for (i = 0; i < mseq_table_length; i++)
		dnc_write_csr(node, H2S_CSR_G0_JUMP_ENTRY, mseq_table[i]);

	/* Start the microsequencer */
	val = dnc_read_csr(node, H2S_CSR_G0_STATE_CLEAR);
	dnc_write_csr(node, H2S_CSR_G0_STATE_CLEAR, val);
}

void tally_local_node(void)
{
	uint32_t val, base, limit, rest;
	uint16_t i, j, max_ht_node, tot_cores;
	uint16_t last = 0;
	nc_node[0].node_mem = 0;
	tot_cores = 0;
	nc_node[0].sci_id = dnc_read_csr(0xfff0, H2S_CSR_G0_NODE_IDS) >> 16;
	nc_node[0].nc_ht_id = dnc_master_ht_id;
	nc_node[0].nc_neigh = nc_neigh;
	nc_node[0].dram_base = 0;
	val = cht_read_conf(0, FUNC0_HT, 0x60);
	max_ht_node = (val >> 4) & 7;
#ifdef __i386
	/* Save and restore EBX for the position-independent syslinux com32 binary */
	asm volatile("mov $0x80000008, %%eax; pushl %%ebx; cpuid; popl %%ebx" : "=c"(val) :: "eax", "edx");
#else
	asm volatile("mov $0x80000008, %%eax; cpuid" : "=c"(val) :: "eax", "ebx", "edx");
#endif
	apic_per_node = 1 << ((val >> 12) & 0xf);
	nc_node[0].apic_offset = 0;
	printf("Examining SCI%03x...", nc_node[0].sci_id);

	for (i = 0; i <= max_ht_node; i++) {
		if (i == dnc_master_ht_id)
			continue;

		nc_node[0].ht[i].cores = 0;
		nc_node[0].ht[i].base  = 0;
		nc_node[0].ht[i].size  = 0;
		nc_node[0].ht[i].cpuid = cht_read_conf(0, FUNC3_MISC, 0xfc);

		if ((nc_node[0].ht[i].cpuid == 0) ||
		    (nc_node[0].ht[i].cpuid == 0xffffffff) ||
		    (nc_node[0].ht[i].cpuid != nc_node[0].ht[0].cpuid)) {
			fatal("Master server has mixed processor models with CPUIDs %08x and %08x", nc_node[0].ht[0].cpuid, nc_node[0].ht[i].cpuid);
			nc_node[0].ht[i].cpuid = 0;
			nc_node[0].ht[i].pdom = 0;
			continue;
		}

		nc_node[0].ht[i].pdom = ht_pdom_count++;

		/* Fam15h: Accesses to this register must first set F1x10C [DctCfgSel]=0;
		   Accesses to this register with F1x10C [DctCfgSel]=1 are undefined;
		   See erratum 505 */
		if (family >= 0x15)
			cht_write_conf(i, FUNC1_MAPS, 0x10c, 0);

		/* Disable DRAM scrubbers */
		nc_node[0].ht[i].scrub = cht_read_conf(i, FUNC3_MISC, 0x58);
		if (nc_node[0].ht[i].scrub & 0x1f) {
			cht_write_conf(i, FUNC3_MISC, 0x58, nc_node[0].ht[i].scrub & ~0x1f);
			/* Allow outstanding scrub requests to finish */
			udelay(40);
		}

		base = cht_read_conf(i, FUNC1_MAPS, 0x120);
		limit = cht_read_conf(i, FUNC1_MAPS, 0x124);

		if (limit & 0x1fffff) {
			nc_node[0].ht[i].base = (base & 0x1fffff) << (27 - DRAM_MAP_SHIFT);
			nc_node[0].ht[i].size = ((limit & 0x1fffff) - (base & 0x1fffff) + 1) << (27 - DRAM_MAP_SHIFT);
			nc_node[0].node_mem += nc_node[0].ht[i].size;
			last = i;

			if (nc_node[0].node_mem > max_mem_per_node) {
				printf("Node exceeds cachable memory range, clamping...\n");
				nc_node[0].ht[i].size -= nc_node[0].node_mem - max_mem_per_node;
				nc_node[0].node_mem = max_mem_per_node;
				limit = nc_node[0].ht[i].base + nc_node[0].ht[i].size - 1;
				asm volatile("wbinvd" ::: "memory");

				for (j = 0; j <= max_ht_node; j++) {
					if (j == dnc_master_ht_id)
						continue;

					if (!nc_node[0].ht[j].cpuid)
						continue;

					cht_write_conf(j, FUNC1_MAPS, 0x44 + i * 8, (limit << 16) |
					               (cht_read_conf(j, FUNC1_MAPS, 0x44 + i * 8) & 0xffff));
				}

				cht_write_conf(i, FUNC1_MAPS, 0x124, limit >> (27 - DRAM_MAP_SHIFT));
				asm volatile("wbinvd" ::: "memory");
			}
		}

		/* Assume at least one core */
		nc_node[0].ht[i].cores = 1;

		if (family < 0x15) {
			val = cht_read_conf(i, FUNC0_HT, 0x68);
			if (val & 0x20) nc_node[0].ht[i].cores++; /* Cpu1En */

			val = cht_read_conf(i, FUNC0_HT, 0x168);
			if (val & 0x01) nc_node[0].ht[i].cores++; /* Cpu2En */
			if (val & 0x02) nc_node[0].ht[i].cores++; /* Cpu3En */
			if (val & 0x04) nc_node[0].ht[i].cores++; /* Cpu4En */
			if (val & 0x08) nc_node[0].ht[i].cores++; /* Cpu5En */
		} else {
			val = cht_read_conf(i, FUNC5_EXTD, 0x84);
			nc_node[0].ht[i].cores += val & 0xff;
			val = cht_read_conf(i, FUNC3_MISC, 0x190);

			while (val > 0) {
				if (val & 1)
					nc_node[0].ht[i].cores--;

				val >>= 1;
			}
		}

		nc_node[0].ht[i].apic_base = post_apic_mapping[tot_cores];
		ht_next_apic = nc_node[0].ht[i].apic_base + apic_per_node;
		tot_cores += nc_node[0].ht[i].cores;
	}

	printf("%d cores and %dMB of memory and I/O maps\n", tot_cores, nc_node[0].node_mem << 4);
	dnc_top_of_mem = nc_node[0].ht[last].base + nc_node[0].ht[last].size;
	rest = dnc_top_of_mem & (SCC_ATT_GRAN - 1);
	if (rest) {
		rest = SCC_ATT_GRAN - rest;
		printf("Adding %dMB to SCI%03x to accommodate granularity requirements\n",
		       rest << (DRAM_MAP_SHIFT - 20), nc_node[0].sci_id);
		mtrr_range((uint64_t)dnc_top_of_mem << DRAM_MAP_SHIFT, (uint64_t)(dnc_top_of_mem + rest) << DRAM_MAP_SHIFT, MTRR_UC);
		dnc_top_of_mem += rest;
	}
	nc_node[0].dram_limit = dnc_top_of_mem;

	/* Set PCI I/O map */
	dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00);

	for (i = 0; i < 256; i++)
		dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4, nc_node[0].sci_id);

	/* Set IntRecCtrl */
	dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x30);

	for (i = 0; i < 256; i++)
		dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4, 0);

	dnc_node_count++;
	assert(dnc_node_count < 128);
}

static bool tally_remote_node(uint16_t node)
{
	uint32_t val, base, limit;
	uint16_t i, max_ht_node, tot_cores;
	uint16_t apic_used[16];
	uint16_t last = 0;
	uint16_t cur_apic;
	nc_node_info_t *cur_node;
	uint32_t mem_limit = max_mem_per_node;

	if (pf_maxmem) {
		printf("Limiting per-server memory to %dGB\n", pf_maxmem);
		mem_limit = min(max_mem_per_node, pf_maxmem << (30 - DRAM_MAP_SHIFT));
	}

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
	cur_node->dram_base = dnc_top_of_mem;
	val = dnc_read_conf(node, 0, 24, FUNC0_HT, 0x60);

	assertf(val != 0xffffffff, "Failed to access config space on SCI%03x", node);

	max_ht_node = (val >> 4) & 7;
	dnc_write_csr(node, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00000020); /* Select APIC ATT */

	for (i = 0; i < 16; i++)
		apic_used[i] = dnc_read_csr(node, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4);

	ht_next_apic = (ht_next_apic + 0xf) & ~0xf;
	cur_node->apic_offset = ht_next_apic;
	cur_apic = 0;
	printf("Examining SCI%03x...", node);

	for (i = 0; i <= max_ht_node; i++) {
		if (i == cur_node->nc_ht_id) {
			cur_node->ht[i].cpuid = 0;
			continue;
		}

		cur_node->ht[i].cores = 0;
		cur_node->ht[i].base  = 0;
		cur_node->ht[i].size  = 0;
		cur_node->ht[i].cpuid = dnc_read_conf(node, 0, 24 + i, FUNC3_MISC, 0xfc);

		if ((cur_node->ht[i].cpuid == 0) ||
		    (cur_node->ht[i].cpuid == 0xffffffff) ||
		    (cur_node->ht[i].cpuid != nc_node[0].ht[0].cpuid))
			fatal("SCI%03x has CPUID %08x differing from master server CPUID %08x", node, cur_node->ht[i].cpuid, nc_node[0].ht[0].cpuid);

		cur_node->ht[i].pdom = ht_pdom_count++;

		/* Fam15h: Accesses to this register must first set F1x10C [DctCfgSel]=0;
		   Accesses to this register with F1x10C [DctCfgSel]=1 are undefined;
		   See erratum 505 */
		if (family >= 0x15)
			dnc_write_conf(node, 0, 24 + i, FUNC1_MAPS, 0x10c, 0);

		cur_node->ht[i].scrub = dnc_read_conf(node, 0, 24 + i, FUNC3_MISC, 0x58);
		if (cur_node->ht[i].scrub & 0x1f)
			dnc_write_conf(node, 0, 24 + i, FUNC3_MISC, 0x58, cur_node->ht[i].scrub & ~0x1f);

		base  = dnc_read_conf(node, 0, 24 + i, FUNC1_MAPS, 0x120);
		limit = dnc_read_conf(node, 0, 24 + i, FUNC1_MAPS, 0x124);

		if (limit & 0x1fffff) {
			base = (base & 0x1fffff) << (27 - DRAM_MAP_SHIFT);
			limit = (limit & 0x1fffff) << (27 - DRAM_MAP_SHIFT);
			limit = limit | (0xffffffff >> (32 - 27 + DRAM_MAP_SHIFT));
			val = dnc_read_conf(node, 0, 24 + i, FUNC1_MAPS, 0xf0);

			if ((val & 3) == 3)
				limit -= ((val >> 8) & 0xff) - base;

			cur_node->ht[i].base = dnc_top_of_mem;
			cur_node->ht[i].size = limit - base + 1;
			cur_node->node_mem += cur_node->ht[i].size;

			if (cur_node->node_mem > mem_limit) {
				printf("Node exceeds cachable memory range; clamping...\n");
				cur_node->ht[i].size -= cur_node->node_mem - mem_limit;
				cur_node->node_mem = mem_limit;
			}

			dnc_top_of_mem += cur_node->ht[i].size;
		}

		while ((cur_apic < 256) && !(apic_used[cur_apic >> 4] & (1 << (cur_apic & 0xf))))
			cur_apic++;

		/* Assume at least one core */
		cur_node->ht[i].cores = 1;

		if (family < 0x15) {
			val = dnc_read_conf(node, 0, 24 + i, FUNC0_HT, 0x68);
			if (val & 0x20) cur_node->ht[i].cores++; /* Cpu1En */

			val = dnc_read_conf(node, 0, 24 + i, FUNC0_HT, 0x168);
			if (val & 0x01) cur_node->ht[i].cores++; /* Cpu2En */
			if (val & 0x02) cur_node->ht[i].cores++; /* Cpu3En */
			if (val & 0x04) cur_node->ht[i].cores++; /* Cpu4En */
			if (val & 0x08) cur_node->ht[i].cores++; /* Cpu5En */
		} else {
			val = dnc_read_conf(node, 0, 24 + i, FUNC5_EXTD, 0x84);
			cur_node->ht[i].cores += val & 0xff;
			val = dnc_read_conf(node, 0, 24 + i, FUNC3_MISC, 0x190);

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

	/* Check if any DRAM ranges overlap the HyperTransport address space */
	if ((cur_node->dram_base < (HT_LIMIT >> DRAM_MAP_SHIFT)) &&
		((cur_node->dram_base + cur_node->node_mem) > (HT_BASE >> DRAM_MAP_SHIFT))) {
		/* Move whole server up to HT decode limit */
		uint64_t shift = (HT_LIMIT >> DRAM_MAP_SHIFT) - cur_node->dram_base;

		printf("Moving SCI%3x past HyperTransport decode range by %lldGB\n", node, shift >> (30 - DRAM_MAP_SHIFT));

		for (i = 0; i <= max_ht_node; i++) {
			if (cur_node->ht[i].cpuid == 0)
				continue;

			cur_node->ht[i].base += shift;
		}

		ht_base = (uint64_t)cur_node->dram_base << DRAM_MAP_SHIFT; /* End of last node limit */
		cur_node->dram_base += shift;
		dnc_top_of_mem += shift;
	}

	/* If rebased apicid[7:0] of last core is above a given threshold,
	   bump base for entire SCI node to next 8-bit interval */
	if ((ht_next_apic & 0xff) + cur_node->ht[last].apic_base + cur_node->ht[last].cores > 0xf0)
		ht_next_apic = (ht_next_apic & ~0xff) + 0x100 + cur_node->ht[0].apic_base;

	cur_node->apic_offset = ht_next_apic - cur_node->ht[0].apic_base;
	ht_next_apic = cur_node->apic_offset + cur_node->ht[last].apic_base + apic_per_node;
	cur_node->dram_limit = dnc_top_of_mem;
	printf("%d cores and %dGB of memory\n", tot_cores, cur_node->node_mem >> 6);

	/* Set PCI I/O map */
	dnc_write_csr(node, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00);

	for (i = 0; i < 256; i++)
		dnc_write_csr(node, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4, nc_node[0].sci_id);

	/* Set IntRecCtrl */
	dnc_write_csr(node, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x30);

	for (i = 0; i < 256; i++)
		dnc_write_csr(node, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4, 0);

	dnc_node_count++;
	return 1;
}

bool tally_all_remote_nodes(void)
{
	bool ret = 1;
	uint16_t node;

	for (node = 1; node < 4096; node++) {
		if ((nodedata[node] & 0xc0) != 0x80)
			continue;

		ret &= tally_remote_node(node);
	}

	/* MMIO is added after DRAM */
	dnc_top_of_dram = dnc_top_of_mem;

	if (remote_io < 2)
		return ret;

	for (node = 0; node < dnc_node_count; node++)
		tally_remote_node_mmio(node);

	for (node = 0; node < dnc_node_count; node++)
		ret &= setup_remote_node_mmio(node);

	printf("DRAM top is 0x%012" PRIx64 "; MMIO top is 0x%012" PRIx64 "\n",
	       (uint64_t)dnc_top_of_dram << DRAM_MAP_SHIFT, (uint64_t)dnc_top_of_mem << DRAM_MAP_SHIFT);
	return ret;
}
