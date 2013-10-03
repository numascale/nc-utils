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

extern "C" {
	#include <math.h>
}

#include "dnc-regs.h"
#include "dnc-defs.h"
#include "dnc-access.h"
#include "dnc-route.h"
#include "dnc-fabric.h"
#include "dnc-mmio.h"
#include "dnc-bootloader.h"
#include "dnc-commonlib.h"
#include "dnc-masterlib.h"
#include "dnc-maps.h"

#include "../interface/numachip-mseq.h"

#define SIG_DELAY 0x0043c00a
#define SIG_RET   0x001a0000
#define WASHDELAY_P 3.6
#define WASHDELAY_Q 117000000ULL
#define WASHDELAY_CALLS 12

static int zceil(const float num) {
    int inum = (int)num;
    if (num == (float)inum)
        return inum;
    return inum + 1;
}

void load_scc_microcode(void)
{
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
		fatal("No microcode for NumaChip version %d", dnc_chip_rev);

	/* Perform calculation twice to work around register corruption */
	zceil(pow(dnc_core_count, WASHDELAY_P) / WASHDELAY_Q / WASHDELAY_CALLS);
	const uint32_t delays = zceil(pow(dnc_core_count, WASHDELAY_P) / WASHDELAY_Q / WASHDELAY_CALLS);
	printf("Loading SCC microcode with washdelay %d for %d cores...", delays * WASHDELAY_CALLS, dnc_core_count);

	uint32_t used = 0;

	for (int i = 0; i < dnc_node_count; i++) {
		uint32_t counter = 0;
		int node = (i == 0) ? 0xfff0 : nodes[i].sci;
		dnc_write_csr(node, H2S_CSR_G0_SEQ_INDEX, 0x80000000);

		for (uint16_t j = 0; j < mseq_ucode_length; j++) {
			uint32_t val = mseq_ucode[j];

			if (val == SIG_DELAY) {
				counter++;
				if (counter > delays) {
					val = SIG_RET;
					if (!used)
						used = counter;
				}
			} else
				counter = 0;

			dnc_write_csr(node, H2S_CSR_G0_WCS_ENTRY, mseq_ucode[j]);
		}

		dnc_write_csr(node, H2S_CSR_G0_SEQ_INDEX, 0x80000000);

		for (uint16_t j = 0; j < mseq_table_length; j++)
			dnc_write_csr(node, H2S_CSR_G0_JUMP_ENTRY, mseq_table[j]);

		/* Start the microsequencer */
		uint32_t val = dnc_read_csr(node, H2S_CSR_G0_STATE_CLEAR);
		dnc_write_csr(node, H2S_CSR_G0_STATE_CLEAR, val);
	}

	printf("%d delays used\n", used * WASHDELAY_CALLS);
	assert(used > 0);
}

static void print_node_info(const node_info_t *node)
{
	for (ht_t ht = node->nb_ht_lo; ht <= node->nb_ht_hi; ht++)
		printf("- HT%d: base=%x size=%d pdom=%x cores=%d apic_base=%d scrub=%x\n",
			ht, node->ht[ht].base, node->ht[ht].size, node->ht[ht].pdom, node->ht[ht].cores, node->ht[ht].apic_base, node->ht[ht].scrub);
	printf("- node_mem=%d\n", node->node_mem);
	printf("- dram_base=%d/0x%x dram_limit=%d/0x%x\n", node->dram_base, node->dram_base, node->dram_limit, node->dram_limit);
	printf("- mmio32_base=0x%x mmio32_limit=0x%x\n", node->mmio32_base, node->mmio32_limit);
	printf("- mmio64_base=0x%llx mmio64_limit=0x%llx\n", node->mmio64_base, node->mmio64_limit);
	printf("- io_base=0x%x io_limit=0x%x\n", node->io_base, node->io_limit);
	printf("- apic_offset=%d\n", node->apic_offset);
	printf("- nb_ht_lo=%d nb_ht_hi=%d nc_ht=%d nc_neigh_ht=%d nc_neigh_link=%d\n",
		node->nb_ht_lo, node->nb_ht_hi, node->nc_ht, node->nc_neigh_ht, node->nc_neigh_link);
}

void tally_local_node(void)
{
	uint32_t val, base, limit, rest;
	uint16_t i, j, tot_cores;
	nodes[0].node_mem = 0;
	tot_cores = 0;
	nodes[0].sci = dnc_read_csr(0xfff0, H2S_CSR_G0_NODE_IDS) >> 16;
	nodes[0].bsp_ht = 0;
	nodes[0].nb_ht_lo = 0;
	nodes[0].nb_ht_hi = nodes[0].nc_ht - 1;
	nodes[0].dram_base = 0;
	val = cht_read_conf(0, FUNC0_HT, 0x60);
#ifdef __i386
	/* Save and restore EBX for the position-independent syslinux com32 binary */
	asm volatile("mov $0x80000008, %%eax; pushl %%ebx; cpuid; popl %%ebx" : "=c"(val) :: "eax", "edx");
#else
	asm volatile("mov $0x80000008, %%eax; cpuid" : "=c"(val) :: "eax", "ebx", "edx");
#endif
	apic_per_node = 1 << ((val >> 12) & 0xf);
	nodes[0].apic_offset = 0;

	uint32_t cpuid_bsp = cht_read_conf(0, FUNC3_MISC, 0xfc);

	for (i = nodes[0].nb_ht_lo; i <= nodes[0].nb_ht_hi; i++) {
		nodes[0].ht[i].cores = 0;
		nodes[0].ht[i].base  = 0;
		nodes[0].ht[i].size  = 0;

		uint32_t cpuid = cht_read_conf(i, FUNC3_MISC, 0xfc);
		if (cpuid == 0 || cpuid == 0xffffffff || cpuid != cpuid_bsp)
			fatal("Master server has mixed processor models with CPUIDs %08x and %08x", cpuid_bsp, cpuid);

		nodes[0].ht[i].pdom = ht_pdom_count++;

		/* Fam15h: Accesses to this register must first set F1x10C [DctCfgSel]=0;
		   Accesses to this register with F1x10C [DctCfgSel]=1 are undefined;
		   See erratum 505 */
		if (family >= 0x15)
			cht_write_conf(i, FUNC1_MAPS, 0x10c, 0);

		/* Disable DRAM scrubbers */
		nodes[0].ht[i].scrub = cht_read_conf(i, FUNC3_MISC, 0x58);
		if (nodes[0].ht[i].scrub & 0x1f) {
			cht_write_conf(i, FUNC3_MISC, 0x58, nodes[0].ht[i].scrub & ~0x1f);
			/* Allow outstanding scrub requests to finish */
			udelay(40);
		}

		base = cht_read_conf(i, FUNC1_MAPS, 0x120);
		limit = cht_read_conf(i, FUNC1_MAPS, 0x124);

		if (limit & 0x1fffff) {
			nodes[0].ht[i].base = (base & 0x1fffff) << (27 - DRAM_MAP_SHIFT);
			nodes[0].ht[i].size = ((limit & 0x1fffff) - (base & 0x1fffff) + 1) << (27 - DRAM_MAP_SHIFT);
			nodes[0].node_mem += nodes[0].ht[i].size;

			/* Subtract 16MB from each local memory range for CC6 save area */
			if (pf_cstate6) {
				nodes[0].ht[i].size -= 1;

				/* Remove 16MB from ranges to this Northbridge */
				for (j = nodes[0].nb_ht_lo; j <= nodes[0].nb_ht_hi; j++) {
					uint64_t base2, limit2;
					int dst;

					/* Assuming only one local range */
					for (int range = 0; range < 8; range++) {
						/* Skip inactivate ranges and ranges to other Northbridges */
						if (!dram_range_read(0xfff0, i, j, &base2, &limit2, &dst) || dst != j)
							continue;

						limit2 -= 16 << 20;
						dram_range(0xfff0, i, j, base2, limit2, dst);
						break;
					}
				}
			}

			if (nodes[0].node_mem > max_mem_per_node) {
				printf("Node exceeds cachable memory range, clamping...\n");
				nodes[0].ht[i].size -= nodes[0].node_mem - max_mem_per_node;
				nodes[0].node_mem = max_mem_per_node;
				
				/* Account for Cstate6 save area */
				limit = nodes[0].ht[i].base + nodes[0].ht[i].size - 1 + pf_cstate6;
				asm volatile("wbinvd" ::: "memory");

				for (j = nodes[0].nb_ht_lo; j <= nodes[0].nb_ht_hi; j++)
					cht_write_conf(j, FUNC1_MAPS, 0x44 + i * 8, (limit << 16) |
					               (cht_read_conf(j, FUNC1_MAPS, 0x44 + i * 8) & 0xffff));

				cht_write_conf(i, FUNC1_MAPS, 0x124, limit >> (27 - DRAM_MAP_SHIFT));
				asm volatile("wbinvd" ::: "memory");
			}
		}

		/* Assume at least one core */
		nodes[0].ht[i].cores = 1;

		if (family < 0x15) {
			val = cht_read_conf(i, FUNC0_HT, 0x68);
			if (val & 0x20) nodes[0].ht[i].cores++; /* Cpu1En */

			val = cht_read_conf(i, FUNC0_HT, 0x168);
			if (val & 0x01) nodes[0].ht[i].cores++; /* Cpu2En */
			if (val & 0x02) nodes[0].ht[i].cores++; /* Cpu3En */
			if (val & 0x04) nodes[0].ht[i].cores++; /* Cpu4En */
			if (val & 0x08) nodes[0].ht[i].cores++; /* Cpu5En */
		} else {
			val = cht_read_conf(i, FUNC5_EXTD, 0x84);
			nodes[0].ht[i].cores += val & 0xff;
			val = cht_read_conf(i, FUNC3_MISC, 0x190);

			while (val > 0) {
				if (val & 1)
					nodes[0].ht[i].cores--;

				val >>= 1;
			}
		}

		nodes[0].ht[i].apic_base = post_apic_mapping[tot_cores];
		ht_next_apic = nodes[0].ht[i].apic_base + apic_per_node;
		tot_cores += nodes[0].ht[i].cores;
	}

	printf("SCI000 has %d cores and %dMB of memory and I/O maps\n", tot_cores, nodes[0].node_mem << 4);

	dnc_top_of_mem = nodes[0].ht[nodes[0].nb_ht_hi].base + nodes[0].ht[nodes[0].nb_ht_hi].size;
	rest = dnc_top_of_mem & (SCC_ATT_GRAN - 1);
	if (rest) {
		rest = SCC_ATT_GRAN - rest;
		printf("Adding %dMB to SCI%03x to accommodate granularity requirements\n",
		       rest << (DRAM_MAP_SHIFT - 20), nodes[0].sci);
		mtrr_range((uint64_t)dnc_top_of_mem << DRAM_MAP_SHIFT, (uint64_t)(dnc_top_of_mem + rest) << DRAM_MAP_SHIFT, MTRR_UC);
		dnc_top_of_mem += rest;
	}
	nodes[0].dram_limit = dnc_top_of_mem;

	/* Set PCI I/O map */
	dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00);

	for (i = 0; i < 256; i++)
		dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4, nodes[0].sci);

	/* Set IntRecCtrl */
	dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x30);

	for (i = 0; i < 256; i++)
		dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4, 0);

	dnc_node_count++;
	dnc_core_count += tot_cores;
	if (verbose > 1)
		print_node_info(&nodes[0]);
}

static bool tally_remote_node(const uint16_t sci)
{
	uint32_t val, base, limit;
	uint16_t i, tot_cores;
	uint16_t apic_used[16];
	uint16_t last = 0;
	uint16_t cur_apic;
	node_info_t *node;
	uint32_t mem_limit = max_mem_per_node;

	if (dnc_raw_read_csr(sci, H2S_CSR_G3_FAB_CONTROL, &val) != 0) {
		warning("Unable to contact SCI%03x", sci);
		return 0;
	}

	/* Set MMCFG base register so remote NC will accept our forwarded requests */
	val = dnc_read_csr(sci, H2S_CSR_G3_MMCFG_BASE);

	if (val != (DNC_MCFG_BASE >> 24)) {
		printf("Setting SCI%03x MCFG_BASE to %08llx\n", sci, DNC_MCFG_BASE >> 24);
		dnc_write_csr(sci, H2S_CSR_G3_MMCFG_BASE, DNC_MCFG_BASE >> 24);
	}

	node = &nodes[dnc_node_count];
	node->node_mem = 0;
	tot_cores = 0;
	node->sci = sci;
	node->nc_ht = dnc_read_csr(sci, H2S_CSR_G3_HT_NODEID) & 0xf;
	node->nb_ht_lo = 0;
	node->nb_ht_hi = node->nc_ht - 1;

	/* FIXME: Read from remote somehow, instead of assuming the same as ours */
	node->nc_neigh_ht = nodes[0].nc_neigh_ht;
	node->nc_neigh_link = nodes[0].nc_neigh_link;

	/* Ensure that all nodes start out on 1G boundaries */
	dnc_top_of_mem = (dnc_top_of_mem + (0x3fffffff >> DRAM_MAP_SHIFT)) & ~(0x3fffffff >> DRAM_MAP_SHIFT);
	node->dram_base = dnc_top_of_mem;
	val = dnc_read_conf(sci, 0, 24, FUNC0_HT, 0x60);
	assertf(val != 0xffffffff, "Failed to access config space on SCI%03x", sci);

	dnc_write_csr(sci, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00000020); /* Select APIC ATT */
	for (i = 0; i < 16; i++)
		apic_used[i] = dnc_read_csr(sci, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4);

	ht_next_apic = (ht_next_apic + 0xf) & ~0xf;
	node->apic_offset = ht_next_apic;
	cur_apic = 0;

	uint32_t cpuid_bsp = cht_read_conf(0, FUNC3_MISC, 0xfc);

	for (i = node->nb_ht_lo; i <= node->nb_ht_hi; i++) {
		node->ht[i].cores = 0;
		node->ht[i].base  = 0;
		node->ht[i].size  = 0;

		uint32_t cpuid = dnc_read_conf(sci, 0, 24 + i, FUNC3_MISC, 0xfc);
		if (cpuid == 0 || cpuid == 0xffffffff || cpuid != cpuid_bsp)
			fatal("SCI%03x has CPUID %08x differing from master server CPUID %08x", sci, cpuid_bsp, cpuid);

		node->ht[i].pdom = ht_pdom_count++;

		/* Fam15h: Accesses to this register must first set F1x10C [DctCfgSel]=0;
		   Accesses to this register with F1x10C [DctCfgSel]=1 are undefined;
		   See erratum 505 */
		if (family >= 0x15)
			dnc_write_conf(sci, 0, 24 + i, FUNC1_MAPS, 0x10c, 0);

		node->ht[i].scrub = dnc_read_conf(sci, 0, 24 + i, FUNC3_MISC, 0x58);
		if (node->ht[i].scrub & 0x1f)
			dnc_write_conf(sci, 0, 24 + i, FUNC3_MISC, 0x58, node->ht[i].scrub & ~0x1f);

		base  = dnc_read_conf(sci, 0, 24 + i, FUNC1_MAPS, 0x120);
		limit = dnc_read_conf(sci, 0, 24 + i, FUNC1_MAPS, 0x124);

		if (limit & 0x1fffff) {
			base = (base & 0x1fffff) << (27 - DRAM_MAP_SHIFT);
			limit = (limit & 0x1fffff) << (27 - DRAM_MAP_SHIFT);
			limit |= (0xffffffff >> (32 - 27 + DRAM_MAP_SHIFT));

			val = dnc_read_conf(sci, 0, 24 + i, FUNC1_MAPS, 0xf0);
			if ((val & 3) == 3)
				limit -= ((val >> 8) & 0xff) - base;

			node->ht[i].base = dnc_top_of_mem;
			node->ht[i].size = limit - base + 1;
			dnc_top_of_mem += node->ht[i].size;
			if (pf_cstate6)
				node->ht[i].size -= 1;

			node->node_mem += node->ht[i].size;
			if (node->node_mem > mem_limit) {
				printf("SCI%03x exceeds cachable memory range; clamping...\n", sci);
				node->ht[i].size -= node->node_mem - mem_limit;
				node->node_mem = mem_limit;
			}
		}

		while ((cur_apic < 256) && !(apic_used[cur_apic >> 4] & (1 << (cur_apic & 0xf))))
			cur_apic++;

		/* Assume at least one core */
		node->ht[i].cores = 1;

		if (family < 0x15) {
			val = dnc_read_conf(sci, 0, 24 + i, FUNC0_HT, 0x68);
			if (val & 0x20) node->ht[i].cores++; /* Cpu1En */

			val = dnc_read_conf(sci, 0, 24 + i, FUNC0_HT, 0x168);
			if (val & 0x01) node->ht[i].cores++; /* Cpu2En */
			if (val & 0x02) node->ht[i].cores++; /* Cpu3En */
			if (val & 0x04) node->ht[i].cores++; /* Cpu4En */
			if (val & 0x08) node->ht[i].cores++; /* Cpu5En */
		} else {
			val = dnc_read_conf(sci, 0, 24 + i, FUNC5_EXTD, 0x84);
			node->ht[i].cores += val & 0xff;
			val = dnc_read_conf(sci, 0, 24 + i, FUNC3_MISC, 0x190);

			while (val > 0) {
				if (val & 1)
					node->ht[i].cores--;

				val = val >> 1;
			}
		}

		node->ht[i].apic_base = cur_apic;
		tot_cores += node->ht[i].cores;
		cur_apic += node->ht[i].cores;
		last = i;
	}

	/* Check if any DRAM ranges overlap the HyperTransport address space */
	if ((node->dram_base < (HT_LIMIT >> DRAM_MAP_SHIFT)) &&
		((node->dram_base + node->node_mem) > (HT_BASE >> DRAM_MAP_SHIFT))) {
		/* Move whole server up to HT decode limit */
		uint64_t shift = (HT_LIMIT >> DRAM_MAP_SHIFT) - node->dram_base;

		printf("Moving SCI%03x past HyperTransport decode range by %lldGB\n", sci, shift >> (30 - DRAM_MAP_SHIFT));

		for (i = node->nb_ht_lo; i <= node->nb_ht_hi; i++)
			node->ht[i].base += shift;

		ht_base = (uint64_t)node->dram_base << DRAM_MAP_SHIFT; /* End of last node limit */
		node->dram_base += shift;
		dnc_top_of_mem += shift;
	}

	/* If rebased apicid[7:0] of last core is above a given threshold,
	   bump base for entire SCI node to next 8-bit interval */
	if ((ht_next_apic & 0xff) + node->ht[last].apic_base + node->ht[last].cores > 0xf0)
		ht_next_apic = (ht_next_apic & ~0xff) + 0x100 + node->ht[0].apic_base;

	node->apic_offset = ht_next_apic - node->ht[0].apic_base;
	ht_next_apic = node->apic_offset + node->ht[last].apic_base + apic_per_node;
	node->dram_limit = dnc_top_of_mem;

	printf("SCI%03x has %d cores and %dMB of memory\n", sci, tot_cores, node->node_mem << (DRAM_MAP_SHIFT - 20));

	/* Set PCI I/O map */
	dnc_write_csr(sci, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00);

	for (i = 0; i < 256; i++)
		dnc_write_csr(sci, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4, nodes[0].sci);

	/* Set IntRecCtrl */
	dnc_write_csr(sci, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x30);

	for (i = 0; i < 256; i++)
		dnc_write_csr(sci, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4, 0);

	dnc_node_count++;
	dnc_core_count += tot_cores;
	if (verbose > 1)
		print_node_info(node);
	return 1;
}

void tally_all_remote_nodes(void)
{
	bool ret = 1;

	for (uint16_t num = 1; num < 4096; num++)
		if ((nodedata[num] & 0xc0) == 0x80)
			ret &= tally_remote_node(num);

	assertf(ret == 1, "Unable to communicate with all servers");
}
