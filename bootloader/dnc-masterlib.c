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

#define SIG_CALL         0x0018029b
#define SIG_CALLRET      0x00140184
#define SIG_DELAY        0x0043c00a
#define SIG_DELAYRET     0x001a0000
#define WASHDELAY_P      3.6
#define WASHDELAY_Q      9750000ULL
#define WASHDELAY_CALLS  248
#define WASHDELAY_DELAYS 248
#define WASHDELAY_LIMIT  100

static int zceil(const float num) {
    int inum = (int)num;
    if (num == (float)inum)
        return inum;
    return inum + 1;
}

/* Find the ratio that is nearest goal */
static void ratio(const unsigned goal, unsigned *calls, unsigned *delays)
{
	unsigned error = ~0;

	for (unsigned i = 1; i <= WASHDELAY_CALLS; i++) {
		for (unsigned j = 1; j <= WASHDELAY_DELAYS; j++) {
			unsigned new_err = abs((int)goal - i * j);
			if (new_err >= error)
				continue;

			*calls = i;
			*delays = j;
			error = new_err;
		}
	}
}

void load_scc_microcode(void)
{
	uint32_t *mseq_ucode;
	const uint16_t *mseq_table;
	int mseq_ucode_length, mseq_table_length;

	if (!dnc_asic_mode || (dnc_chip_rev == 2)) {
		/* Use this microcode for FPGA and RevC asic */
		mseq_ucode = numachip_mseq_ucode_revc;
		mseq_table = numachip_mseq_table_revc;
		mseq_ucode_length = sizeof(numachip_mseq_ucode_revc) / sizeof(numachip_mseq_ucode_revc[0]);
		mseq_table_length = sizeof(numachip_mseq_table_revc) / sizeof(numachip_mseq_table_revc[0]);
	} else
		fatal("No microcode for NumaChip version %d", dnc_chip_rev);

	/* Call pow() a second time to prevent result corruption */
	pow(dnc_core_count, WASHDELAY_P);
	const unsigned goal = min(zceil(pow(dnc_core_count, WASHDELAY_P) / WASHDELAY_Q), WASHDELAY_LIMIT);
	printf("Loading SCC microcode with washdelay %u for %u cores...", goal, dnc_core_count);

	unsigned calls = 0, delays = 0;
	ratio(goal, &calls, &delays);
	uint32_t counter = 0;

	/* Tune number of calls */
	for (uint16_t i = 0; i < mseq_ucode_length; i++) {
		if (mseq_ucode[i] != SIG_CALL)
			continue;

		counter++;
		if (counter > calls) {
			mseq_ucode[i] = SIG_CALLRET;
			counter--;
			break;
		}
	}
	printf("%u calls", counter);
	assert(counter);
	counter = 0;

	/* Tune number of delays */
	for (uint16_t i = 0; i < mseq_ucode_length; i++) {
		if (mseq_ucode[i] != SIG_DELAY)
			continue;

		counter++;
		if (counter > delays) {
			mseq_ucode[i] = SIG_DELAYRET;
			counter--;
			break;
		}
	}
	assert(counter);
	printf(", %u delays...", counter);

	/* Write microcode */
	for (unsigned i = 0; i < dnc_node_count; i++) {
		uint16_t sci = (i == 0) ? 0xfff0 : nodes[i].sci;

		dnc_write_csr(sci, H2S_CSR_G0_SEQ_INDEX, 0x80000000);
		for (uint16_t j = 0; j < mseq_ucode_length; j++)
			dnc_write_csr(sci, H2S_CSR_G0_WCS_ENTRY, mseq_ucode[j]);

		dnc_write_csr(sci, H2S_CSR_G0_SEQ_INDEX, 0x80000000);
		for (uint16_t j = 0; j < mseq_table_length; j++)
			dnc_write_csr(sci, H2S_CSR_G0_JUMP_ENTRY, mseq_table[j]);

		/* Start the microsequencer */
		uint32_t val = dnc_read_csr(sci, H2S_CSR_G0_STATE_CLEAR);
		dnc_write_csr(sci, H2S_CSR_G0_STATE_CLEAR, val);
	}
	printf("done\n");
}

static void print_node_info(const node_info_t *node)
{
	for (ht_t ht = node->nb_ht_lo; ht <= node->nb_ht_hi; ht++)
		printf("- HT%d: base=0x%x size=%d pdom=%d cores=%d apic_base=%d scrub=%x\n",
			ht, node->ht[ht].base, node->ht[ht].size, node->ht[ht].pdom, node->ht[ht].cores, node->ht[ht].apic_base, node->ht[ht].scrub);
	printf("- node_mem=%d\n", node->node_mem);
	printf("- dram_base=0x%x dram_limit=0x%x\n", node->dram_base, node->dram_limit);
	printf("- mmio32_base=0x%x mmio32_limit=0x%x\n", node->mmio32_base, node->mmio32_limit);
	printf("- mmio64_base=0x%llx mmio64_limit=0x%llx\n", node->mmio64_base, node->mmio64_limit);
	printf("- io_base=0x%x io_limit=0x%x\n", node->io_base, node->io_limit);
	printf("- apic_offset=%d\n", node->apic_offset);
	printf("- nb_ht_lo=%d nb_ht_hi=%d nc_ht=%d nc_neigh_ht=%d nc_neigh_link=%d\n",
		node->nb_ht_lo, node->nb_ht_hi, node->nc_ht, node->nc_neigh_ht, node->nc_neigh_link);
}

static void adjust_dram_maps(node_info_t *const node)
{
	if (memlimit) {
		for (int i = node->nb_ht_lo; i <= node->nb_ht_hi; i++) {
			uint64_t limit = memlimit;

			/* first northbridges must have a minimum of 4GB */
			if (node == &nodes[0] && i == node->nb_ht_lo)
				limit = 4ULL << 30;

			node->node_mem -= node->ht[i].size - (limit >> DRAM_MAP_SHIFT);
			node->ht[i].size = (limit >> DRAM_MAP_SHIFT);
		}

		return;
	}

	/* trim nodes if over supported memory config */
	int over = min(node->node_mem - max_mem_per_server, node->node_mem & (SCC_ATT_GRAN - 1));
	if (over > 0) {
		printf("Trimming %03x maps by %uMB\n", node->sci, over << (DRAM_MAP_SHIFT - 20));

		while (over > 0) {
			unsigned max = 0;

			/* find largest HT node */
			for (int i = node->nb_ht_lo; i <= node->nb_ht_hi; i++)
				if (node->ht[i].size > max)
					max = node->ht[i].size;

			/* reduce largest HT node by 16MB */
			for (int i = node->nb_ht_lo; i <= node->nb_ht_hi; i++) {
				if (node->ht[i].size == max) {
					node->ht[i].size -= 1;
					node->node_mem -= 1;
					over -= 1;
				}
			}
		}
	}
}

static void adjust_dram_window(node_info_t *const node)
{
	printf("Updating DRAM maps...");
	for (int i = node->nb_ht_lo; i <= node->nb_ht_hi; i++) {
		node->ht[i].base = dnc_top_of_mem;
		dnc_top_of_mem += node->ht[i].size;
		node->ht[i].size -= pf_cstate6;

		const uint64_t base = (uint64_t)node->ht[i].base << DRAM_MAP_SHIFT;
		const uint64_t limit = (((uint64_t)node->ht[i].base + node->ht[i].size) << DRAM_MAP_SHIFT) - 1;

		for (int j = node->nb_ht_lo; j <= node->nb_ht_hi; j++) {
			for (int range = 0; range < 8; range++) {
				uint8_t dst;

				if (dram_range_read(node->sci, j, range, NULL, NULL, &dst) && dst == i) {
					dram_range(node->sci, j, range, base, limit, i);
					break;
				}
			}
		}

		dnc_write_conf(node->sci, 0, 24 + i, FUNC1_MAPS, 0x120, node->ht[i].base >> (27 - DRAM_MAP_SHIFT));
		/* Account for Cstate6 save area */
		dnc_write_conf(node->sci, 0, 24 + i, FUNC1_MAPS, 0x124, (node->ht[i].base + node->ht[i].size - 1 + pf_cstate6) >> (27 - DRAM_MAP_SHIFT));
	}
	printf("\n");
}

void tally_local_node(void)
{
	uint32_t val, base, limit;
	uint16_t i, tot_cores;

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

	/* Start APICs at 0x100 when remote IO is enabled to keep device interrupts local */
	nodes[0].apic_offset = remote_io ? 0x100 : 0;

	uint32_t cpuid_bsp = cht_read_conf(0, FUNC3_MISC, 0xfc);

	/* Size HT nodes */
	for (i = nodes[0].nb_ht_lo; i <= nodes[0].nb_ht_hi; i++) {
		nodes[0].ht[i].size  = 0;

		base = cht_read_conf(i, FUNC1_MAPS, 0x120);
		limit = cht_read_conf(i, FUNC1_MAPS, 0x124);

		if (limit & 0x1fffff) {
			nodes[0].ht[i].size = ((limit & 0x1fffff) - (base & 0x1fffff) + 1) << (27 - DRAM_MAP_SHIFT);
			nodes[0].node_mem += nodes[0].ht[i].size;

			/* Subtract 16MB from each local memory range for CC6 save area */
			if (pf_cstate6) {
				nodes[0].ht[i].size -= 1;

				/* Remove 16MB from ranges to this Northbridge */
				for (int j = nodes[0].nb_ht_lo; j <= nodes[0].nb_ht_hi; j++) {
					uint64_t base2, limit2;
					uint8_t dst;

					/* Assuming only one local range */
					for (int range = 0; range < 8; range++) {
						/* Skip inactivate ranges and ranges to other Northbridges */
						if (!dram_range_read(nodes[0].sci, i, j, &base2, &limit2, &dst) || dst != j)
							continue;

						limit2 -= 16 << 20;
						dram_range(nodes[0].sci, i, j, base2, limit2, dst);
						break;
					}
				}
			}
		}

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
	}

	adjust_dram_maps(&nodes[0]);
	adjust_dram_window(&nodes[0]);
	dnc_top_of_mem = 0;

	for (i = nodes[0].nb_ht_lo; i <= nodes[0].nb_ht_hi; i++) {
		nodes[0].ht[i].base = dnc_top_of_mem;
		dnc_top_of_mem += nodes[0].ht[i].size;

		uint32_t cpuid = cht_read_conf(i, FUNC3_MISC, 0xfc);
		if (cpuid == 0 || cpuid == 0xffffffff || cpuid != cpuid_bsp)
			fatal("Master server has mixed processor models with CPUIDs %08x and %08x", cpuid_bsp, cpuid);

		nodes[0].ht[i].pdom = ht_pdom_count++;

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
		ht_next_apic = nodes[0].apic_offset + nodes[0].ht[i].apic_base + apic_per_node;
		tot_cores += nodes[0].ht[i].cores;
	}

	printf("SCI%03x has %u cores and %uMB of memory and I/O maps\n", nodes[0].sci, tot_cores, nodes[0].node_mem << 4);
	nodes[0].dram_limit = dnc_top_of_mem;

	dnc_write_csr(nodes[0].sci, H2S_CSR_G3_NC_ATT_MAP_SELECT, NC_ATT_IO);
	for (i = 0; i < 256; i++)
		dnc_write_csr(nodes[0].sci, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4, nodes[0].sci);

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

	dnc_write_csr(sci, H2S_CSR_G3_NC_ATT_MAP_SELECT, NC_ATT_APIC);
	for (i = 0; i < 16; i++)
		apic_used[i] = dnc_read_csr(sci, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4);

	ht_next_apic = (ht_next_apic + 0xf) & ~0xf;
	node->apic_offset = ht_next_apic;
	cur_apic = 0;

	uint32_t cpuid_bsp = cht_read_conf(0, FUNC3_MISC, 0xfc);

	/* Size HT nodes */
	for (i = node->nb_ht_lo; i <= node->nb_ht_hi; i++) {
		base  = dnc_read_conf(sci, 0, 24 + i, FUNC1_MAPS, 0x120);
		limit = dnc_read_conf(sci, 0, 24 + i, FUNC1_MAPS, 0x124);

		if (limit & 0x1fffff) {
			base = (base & 0x1fffff) << (27 - DRAM_MAP_SHIFT);
			limit = (limit & 0x1fffff) << (27 - DRAM_MAP_SHIFT);
			limit |= (0xffffffff >> (32 - 27 + DRAM_MAP_SHIFT));

			val = dnc_read_conf(sci, 0, 24 + i, FUNC1_MAPS, 0xf0);
			if ((val & 3) == 3)
				limit -= ((val >> 8) & 0xff) - base;

			node->ht[i].size = limit - base + 1;
			if (pf_cstate6)
				node->ht[i].size -= 1;

			node->node_mem += node->ht[i].size;
		}

		/* Fam15h: Accesses to this register must first set F1x10C [DctCfgSel]=0;
		   Accesses to this register with F1x10C [DctCfgSel]=1 are undefined;
		   See erratum 505 */
		if (family >= 0x15)
			dnc_write_conf(sci, 0, 24 + i, FUNC1_MAPS, 0x10c, 0);

		node->ht[i].scrub = dnc_read_conf(sci, 0, 24 + i, FUNC3_MISC, 0x58);
		if (node->ht[i].scrub & 0x1f)
			dnc_write_conf(sci, 0, 24 + i, FUNC3_MISC, 0x58, node->ht[i].scrub & ~0x1f);
	}

	adjust_dram_maps(node);

	for (i = node->nb_ht_lo; i <= node->nb_ht_hi; i++) {
		node->ht[i].base = dnc_top_of_mem;
		dnc_top_of_mem += node->ht[i].size;

		uint32_t cpuid = dnc_read_conf(sci, 0, 24 + i, FUNC3_MISC, 0xfc);
		if (cpuid == 0 || cpuid == 0xffffffff || cpuid != cpuid_bsp)
			fatal("SCI%03x has CPUID %08x differing from master server CPUID %08x", sci, cpuid_bsp, cpuid);

		node->ht[i].pdom = ht_pdom_count++;

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

				val >>= 1;
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

		printf("Moving SCI%03x past HyperTransport decode range by %lluGB\n", sci, shift >> (30 - DRAM_MAP_SHIFT));
		under_ht_base = ((uint64_t)(node-1)->dram_base + (node-1)->node_mem) << DRAM_MAP_SHIFT;

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

	printf("SCI%03x has %u cores and %uMB of memory\n", sci, tot_cores, node->node_mem << (DRAM_MAP_SHIFT - 20));

	dnc_write_csr(sci, H2S_CSR_G3_NC_ATT_MAP_SELECT, NC_ATT_IO);
	for (i = 0; i < 256; i++)
		dnc_write_csr(sci, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4, nodes[0].sci);

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
