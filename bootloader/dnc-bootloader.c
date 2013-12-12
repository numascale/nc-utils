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
#include <inttypes.h>
#include <sys/io.h>

extern "C" {
	#include <com32.h>
	#include <syslinux/pxe.h>
}

#include "dnc-regs.h"
#include "dnc-defs.h"
#include "dnc-access.h"
#include "dnc-route.h"
#include "dnc-acpi.h"
#include "dnc-aml.h"
#include "dnc-fabric.h"
#include "dnc-config.h"
#include "dnc-bootloader.h"
#include "dnc-commonlib.h"
#include "dnc-masterlib.h"
#include "dnc-devices.h"
#include "dnc-mmio.h"
#include "dnc-maps.h"
#include "dnc-version.h"

#define PIC_MASTER_CMD          0x20
#define PIC_MASTER_IMR          0x21
#define PIC_SLAVE_CMD           0xa0
#define PIC_SLAVE_IMR           0xa1

#define MTRR_TYPE(x) (x) == 0 ? "uncacheable" : (x) == 1 ? "write-combining" : (x) == 4 ? "write-through" : (x) == 5 ? "write-protect" : (x) == 6 ? "write-back" : "unknown"

#define TABLE_AREA_SIZE		256*1024
#define MIN_NODE_MEM		256*1024*1024

bool dnc_asic_mode;
uint32_t dnc_chip_rev;
char dnc_card_type[16];
uint16_t dnc_node_count = 0, dnc_core_count = 0;
node_info_t *nodes = NULL;
struct node_info *local_info;
uint16_t ht_pdom_count = 0;
uint16_t apic_per_node;
uint16_t ht_next_apic;
uint32_t dnc_top_of_mem;       /* Top of MMIO, in 16MB chunks */
uint8_t post_apic_mapping[256]; /* POST APIC assigments */
static bool scc_started = 0;
static struct in_addr myip = {0xffffffff};
uint64_t ht_base = HT_BASE;
uint64_t old_mcfg = 0;

/* Traversal info per node.  Bit 7: seen, bits 5:0 rings walked */
uint8_t nodedata[4096];

char *asm_relocated;
static char *tables_relocated;
char *tables_next;
static acpi_sdt_p rsdt = NULL, xsdt = NULL;

IMPORT_RELOCATED(new_e820_handler);
IMPORT_RELOCATED(old_int15_vec);
IMPORT_RELOCATED(init_dispatch);
IMPORT_RELOCATED(cpu_status);
IMPORT_RELOCATED(cpu_apic_renumber);
IMPORT_RELOCATED(cpu_apic_hi);
IMPORT_RELOCATED(new_mcfg_msr);
IMPORT_RELOCATED(new_topmem_msr);
IMPORT_RELOCATED(new_topmem2_msr);
IMPORT_RELOCATED(new_e820_len);
IMPORT_RELOCATED(new_e820_map);
IMPORT_RELOCATED(new_mpfp);
IMPORT_RELOCATED(new_mptable);
IMPORT_RELOCATED(new_mtrr_default);
IMPORT_RELOCATED(fixed_mtrr_regs);
IMPORT_RELOCATED(new_mtrr_fixed);
IMPORT_RELOCATED(new_mtrr_var_base);
IMPORT_RELOCATED(new_mtrr_var_mask);
IMPORT_RELOCATED(new_syscfg_msr);
IMPORT_RELOCATED(rem_topmem_msr);
IMPORT_RELOCATED(rem_smm_base_msr);
#ifdef BROKEN
IMPORT_RELOCATED(new_osvw_id_len_msr);
IMPORT_RELOCATED(new_osvw_status_msr);
IMPORT_RELOCATED(new_int_halt_msr);
#endif
IMPORT_RELOCATED(new_lscfg_msr);
IMPORT_RELOCATED(new_cucfg2_msr);
IMPORT_RELOCATED(new_cpuwdt_msr);
IMPORT_RELOCATED(new_hwcr_msr);
IMPORT_RELOCATED(new_mc4_misc0_msr);
IMPORT_RELOCATED(new_mc4_misc1_msr);
IMPORT_RELOCATED(new_mc4_misc2_msr);
IMPORT_RELOCATED(msr_readback);

extern uint8_t smm_handler_start;
extern uint8_t smm_handler_end;

struct e820entry *orig_e820_map = NULL;
int orig_e820_len = 0;

void set_cf8extcfg_enable(const int ht)
{
	uint32_t val = cht_read_conf(ht, FUNC3_MISC, 0x8c);
	cht_write_conf(ht, FUNC3_MISC, 0x8c, val | (1 << (46 - 32)));
}

static void set_wrap32_disable(void)
{
	uint64_t val = rdmsr(MSR_HWCR);
	wrmsr(MSR_HWCR, val | (1ULL << 17));
}

static void set_wrap32_enable(void)
{
	uint64_t val = rdmsr(MSR_HWCR);
	wrmsr(MSR_HWCR, val & ~(1ULL << 17));
}

static void clear_bsp_flag(void)
{
	uint64_t val = rdmsr(MSR_APIC_BAR);
	wrmsr(MSR_APIC_BAR, val & ~(1ULL << 8));
}

static void disable_xtpic(void)
{
	inb(PIC_MASTER_IMR);
	outb(0xff, PIC_MASTER_IMR);
	inb(PIC_SLAVE_IMR);
	outb(0xff, PIC_SLAVE_IMR);
}

#define E820_MAX_LEN 4096

static void load_orig_e820_map(void)
{
	orig_e820_map = (e820entry *)lzalloc(E820_MAX_LEN);
	assert(orig_e820_map);

	static com32sys_t rm;
	rm.eax.l = 0x0000e820;
	rm.edx.l = STR_DW_N("SMAP");
	rm.ebx.l = 0;
	rm.ecx.l = sizeof(struct e820entry);
	rm.edi.w[0] = OFFS(orig_e820_map);
	rm.es = SEG(orig_e820_map);
	__intcall(0x15, &rm, &rm);

	assert(rm.eax.l == STR_DW_N("SMAP"));
	printf("E820 memory map:\n");
	orig_e820_len = rm.ecx.l;

	while (rm.ebx.l > 0) {
		rm.eax.l = 0x0000e820;
		rm.edx.l = STR_DW_N("SMAP");
		rm.ecx.l = sizeof(struct e820entry);
		rm.edi.w[0] = OFFS(orig_e820_map) + orig_e820_len;
		rm.es = SEG(orig_e820_map);
		__intcall(0x15, &rm, &rm);
		orig_e820_len += rm.ecx.l ? rm.ecx.l : sizeof(struct e820entry);
		assert(orig_e820_len < E820_MAX_LEN);
	}

	struct e820entry elem;
	int i, j, len = orig_e820_len / sizeof(elem);

	/* Sort e820 map entries */
	for (j = 1; j < len; j++) {
		memcpy(&elem, &orig_e820_map[j], sizeof(elem));

		for (i = j - 1; (i >= 0) && (orig_e820_map[i].base > elem.base); i--)
			memcpy(&orig_e820_map[i + 1], &orig_e820_map[i], sizeof(elem));

		memcpy(&orig_e820_map[i + 1], &elem, sizeof(elem));
	}

	for (i = 0; i < len; i++) {
		printf("- 0x%011llx:0x%011llx type %x\n",
		       orig_e820_map[i].base, orig_e820_map[i].base + orig_e820_map[i].length,
		       orig_e820_map[i].type);
	}
}

extern void *__mem_end;
extern size_t __stack_size;

static inline size_t sp(void)
{
	size_t sp;
	asm volatile ("movl %%esp,%0":"=rm" (sp));
	return sp;
}

static int install_e820_handler(void)
{
	uint32_t *int_vecs = 0x0;
	struct e820entry *e820;
	volatile uint16_t *bda_tom_lower = (uint16_t *)0x413;
	uint32_t tom_lower = *bda_tom_lower << 10;
	uint32_t relocate_size;
	int last_32b = -1;
	relocate_size = (&asm_relocate_end - &asm_relocate_start + 1023) / 1024;
	relocate_size *= 1024;
	asm_relocated = (char *)((tom_lower - relocate_size) & ~0xfff);
	/* http://groups.google.com/group/comp.lang.asm.x86/msg/9b848f2359f78cdf
	 * *bda_tom_lower = ((uint32_t)asm_relocated) >> 10; */
	memcpy(asm_relocated, &asm_relocate_start, relocate_size);
	e820 = (e820entry *)REL32(new_e820_map);
	unsigned int i, j = 0;

	for (i = 0; i < orig_e820_len / sizeof(struct e820entry); i++) {
		uint64_t orig_end = orig_e820_map[i].base + orig_e820_map[i].length;

		if ((orig_e820_map[i].base >> DRAM_MAP_SHIFT) > max_mem_per_node) {
			/* Skip entry altogether */
			continue;
		}

		if ((orig_end >> DRAM_MAP_SHIFT) > max_mem_per_node) {
			/* Adjust length to fit */
			printf("Master node exceeds cachable memory range, clamping...\n");
			orig_end = (uint64_t)max_mem_per_node << DRAM_MAP_SHIFT;
			orig_e820_map[i].length = orig_end - orig_e820_map[i].base;
		}

		/* Reserve space for relocated code */
		if ((orig_end > (uint32_t)asm_relocated) && (orig_end <= tom_lower)) {
			/* Current entry ends within relocate space */
			if (orig_e820_map[i].base > (uint32_t)asm_relocated)
				continue;

			e820[j].base = orig_e820_map[i].base;
			e820[j].length =
			    (uint32_t)asm_relocated - orig_e820_map[i].base;
			e820[j].type = orig_e820_map[i].type;
			j++;
		} else if ((orig_e820_map[i].base > (uint32_t)asm_relocated) &&
		           (orig_e820_map[i].base < tom_lower)) {
			/* Current entry starts within relocate space */
			e820[j].base = tom_lower;
			e820[j].length = orig_end - tom_lower;
			e820[j].type = orig_e820_map[i].type;
			j++;
		} else if ((orig_e820_map[i].base < (uint32_t)asm_relocated) &&
		           (orig_end > tom_lower)) {
			/* Current entry covers relocate space */
			e820[j].base = orig_e820_map[i].base;
			e820[j].length =
			    (uint32_t)asm_relocated - orig_e820_map[i].base;
			e820[j].type = orig_e820_map[i].type;
			j++;
			e820[j].base = tom_lower;
			e820[j].length = orig_end - tom_lower;
			e820[j].type = orig_e820_map[i].type;
			j++;
		} else {
			e820[j].base = orig_e820_map[i].base;
			e820[j].length = orig_e820_map[i].length;
			e820[j].type = orig_e820_map[i].type;
			j++;
		}

		if ((orig_end < 0x100000000ULL) &&
		    (orig_e820_map[i].length > (TABLE_AREA_SIZE + __stack_size)) &&
		    (orig_e820_map[i].type == 1))
			last_32b = j - 1;
	}

	*REL16(new_e820_len)  = j;
	*REL32(old_int15_vec) = int_vecs[0x15];

	if (last_32b < 0) {
		error("Unable to allocate room for ACPI tables");
		return 0;
	}

	e820[last_32b].length -= (TABLE_AREA_SIZE + __stack_size);
	tables_relocated = (char *)(long)e820[last_32b].base + (long)e820[last_32b].length;
	tables_next = tables_relocated;
	int_vecs[0x15] = (((uint32_t)asm_relocated) << 12) |
	                 ((uint32_t)(&new_e820_handler_relocate - &asm_relocate_start));
	printf("Persistent code relocated to %p\n", asm_relocated);
	printf("Allocating ACPI tables at %p - %p\n", tables_relocated, tables_relocated + TABLE_AREA_SIZE);
	if (verbose > 0)
		printf("__mem_end = %p, __stack_size = 0x%x, sp() = 0x%x\n", __mem_end, __stack_size, sp());

	return 1;
}

static void update_e820_map(void)
{
	uint64_t prev_end;
	uint64_t trace_buf = 0;
	unsigned int i, j, max;
	struct e820entry *e820;
	uint16_t *len;
	e820 = (e820entry *)REL32(new_e820_map);
	len  = REL16(new_e820_len);
	prev_end = 0;
	max = 0;

	for (i = 0; i < *len; i++) {
		if (prev_end < e820[i].base + e820[i].length) {
			max = i;
			prev_end = e820[max].base + e820[max].length;
		}
	}

	/* Truncate to SCI000/HT 0 end; rest added below */
	e820[max].length = ((uint64_t)nodes[0].ht[0].size << DRAM_MAP_SHIFT) - e820[max].base;

	if ((trace_buf_size > 0) && (e820[max].length > trace_buf_size)) {
		e820[max].length -= trace_buf_size;
		trace_buf = e820[max].base + e820[max].length;
		printf("SCI%03x#%x tracebuffer reserved @ 0x%llx:0x%llx\n",
		       nodes[0].sci, 0, trace_buf, trace_buf + trace_buf_size - 1);
	}

	/* Add remote nodes */
	for (i = 0; i < dnc_node_count; i++) {
		for (j = nodes[i].nb_ht_lo; j <= nodes[i].nb_ht_hi; j++) {
			if ((i == 0) && (j == 0))
				continue; /* Skip BSP */

			uint64_t base   = ((uint64_t)nodes[i].ht[j].base << DRAM_MAP_SHIFT);
			uint64_t length = ((uint64_t)nodes[i].ht[j].size << DRAM_MAP_SHIFT);

			if (mem_offline && (i > 0)) {
				if (length > MIN_NODE_MEM)
					length = MIN_NODE_MEM;
			} else {
				if ((trace_buf_size > 0) && (length > trace_buf_size)) {
					length -= trace_buf_size;
					printf("SCI%03x#%x tracebuffer reserved @ 0x%llx:0x%llx\n",
					       nodes[i].sci, j, base + length, base + length + trace_buf_size - 1);
				}
			}

			/* Extend any previous adjacent segment */
			if ((e820[(*len) - 1].base + e820[(*len) - 1].length) == base)
				e820[(*len) - 1].length += length;
			else {
				e820[*len].base = base;
				e820[*len].length = length;
				e820[*len].type = 1;
				(*len)++;
			}
		}
	}

	/* Reserve IO window */
	e820[*len].base   = IO_BASE;
	e820[*len].length = IO_LIMIT - IO_BASE + 1;
	e820[*len].type   = 2;
	(*len)++;

	/* Reserve MCFG address range so Linux accepts it */
	e820[*len].base   = DNC_MCFG_BASE;
	e820[*len].length = DNC_MCFG_LIM - DNC_MCFG_BASE + 1;
	e820[*len].type   = 2;
	(*len)++;

	assert((len - REL16(new_e820_len)) < E820_MAX_LEN);
	printf("Updated E820 map:\n");

	for (i = 0; i < *len; i++) {
		printf(" %011llx:%011llx (%011llx) [%x]\n",
		       e820[i].base, e820[i].base + e820[i].length,
		       e820[i].length, e820[i].type);
	}
}

static void load_existing_apic_map(void)
{
	acpi_sdt_p srat = find_sdt("SRAT");
	uint16_t apic_used[16];
	int i, c;
	memset(post_apic_mapping, ~0, sizeof(post_apic_mapping));
	memset(apic_used, 0, sizeof(apic_used));
	i = 12;
	c = 0;

	while (i + sizeof(*srat) < srat->len) {
		if (srat->data[i] == 0) {
			struct acpi_core_affinity *af =
			    (struct acpi_core_affinity *) &(srat->data[i]);

			if (af->enabled) {
				assert(af->apic_id != 0xff); /* Ensure ID is valid */
				post_apic_mapping[c] = af->apic_id;
				apic_used[af->apic_id >> 4] |= 1 << (af->apic_id & 0xf);
				c++;
			}

			i += af->len;
		} else if (srat->data[i] == 1) {
			struct acpi_mem_affinity *af =
			    (struct acpi_mem_affinity *) &(srat->data[i]);
			i += af->len;
		} else
			break;
	}

	/* Use APIC ATT map as scratch area to communicate APIC maps to master */
	dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, NC_ATT_APIC);
	for (i = 0; i < 16; i++)
		dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4, apic_used[i]);
}

static void tables_add(const size_t len)
{
	tables_next += roundup(len, TABLE_ALIGNMENT);
	assert((char *)tables_next < (tables_relocated + TABLE_AREA_SIZE));
}

static void update_acpi_tables_early(void)
{
	acpi_sdt_p rsdt = find_root("RSDT");
	assert(rsdt);
	acpi_sdt_p xsdt = find_root("XSDT");
	assert(xsdt);

	acpi_sdt_p apic = find_child("APIC", rsdt, 4);
	assert(apic);

	/* Find e820 entry with ACPI table */
	struct e820entry *e820 = orig_e820_map;
	while (((uint32_t)apic < e820->base) || ((uint32_t)apic >= (e820->base + e820->length)))
		e820++;

	printf("Existing ACPI tables in e820 range 0x%011llx:0x%010llx\n", e820->base, e820->base + e820->length - 1);

	acpi_sdt_p oemn = acpi_build_oemn();
	acpi_sdt_p gap = acpi_gap(e820, oemn->len);
	if (!gap) {
		warning("No space for OEMN table");
		free(oemn);
		return;
	}

	printf("Adding OEMN at 0x%x\n", (uint32_t)gap);
	memcpy((char *)gap, oemn, oemn->len);
	free(oemn);

	add_child(gap, xsdt, 8);
	add_child(gap, rsdt, 4);
}

static uint8_t dist_fn(const sci_t src, const sci_t dst)
{
	int changes = 0;
	const sci_t mask = src ^ dst;

	for (int shift = 0; shift < 12; shift += 4)
		if (mask & (0xf << shift))
			changes++;

	return 70 + changes * 30;
}

static void update_acpi_tables(void)
{
	acpi_sdt_p oroot;
	unsigned int i, j, pnum;
	unsigned int node, ht;

	/* replace_root may fail if rptr is r/o, so we read the pointers
	 * back. In case of failure, we'll assume the existing rsdt/xsdt
	 * tables can be extended where they are */
	rsdt = (acpi_sdt_p)tables_next;
	tables_add(RSDT_MAX);
	xsdt = (acpi_sdt_p)tables_next;
	tables_add(RSDT_MAX);

	oroot = find_root("RSDT");
	if (oroot) {
		memcpy(rsdt, oroot, oroot->len);
		assert(replace_root("RSDT", rsdt));
		rsdt = find_root("RSDT");
	} else
		rsdt = NULL;

	oroot = find_root("XSDT");
	if (oroot) {
		memcpy(xsdt, oroot, oroot->len);
		assert(replace_root("XSDT", xsdt));
		xsdt = find_root("XSDT");
	} else
		xsdt = NULL;

	assert(rsdt || xsdt);

	acpi_sdt_p oapic = find_sdt("APIC");
	if (!oapic) {
		printf("Default ACPI APIC table not found\n");
		return;
	}

	/* With APIC we reuse the old info and add our new entries */
	acpi_sdt_p apic = (acpi_sdt_p)tables_next;
	memcpy(apic, oapic, oapic->len);
	memcpy(apic->oemid, "NUMASC", 6);
	apic->len = offsetof(struct acpi_sdt, data) + 8; /* Count 'Local Interrupt Controller' and 'Flags' fields */

	/* Apply enable mask to existing APICs, find first unused ACPI ProcessorId */
	pnum = 0;
	j = 0;

	for (i = 44; i < oapic->len;) {
		struct acpi_local_apic *lapic = (acpi_local_apic *)&oapic->data[i - sizeof(*oapic)];

		if (lapic->type != 0) {
			memcpy(&apic->data[apic->len - sizeof(*apic)], lapic, lapic->len);
			apic->len += lapic->len;
		}

		if (lapic->len == 0) {
			printf("APIC entry at %p (offset %u) reports len 0, aborting!\n",
			       lapic, i);
			break;
		}

		i += lapic->len;
	}

	for (node = 0; node < dnc_node_count; node++) {
		for (ht = nodes[node].nb_ht_lo; ht <= nodes[node].nb_ht_hi; ht++) {
			for (j = 0; j < nodes[node].ht[ht].cores; j++) {
				unsigned int apicid = j + nodes[node].ht[ht].apic_base + nodes[node].apic_offset;
				assert(apicid != 0xff);

				if (ht_next_apic < 0x100) {
					struct acpi_local_apic *lapic = (acpi_local_apic *)&apic->data[apic->len - sizeof(*apic)];
					lapic->type = 0;
					lapic->len = 8;
					lapic->proc_id = pnum; /* ACPI Processor ID */
					lapic->apic_id = apicid; /* APIC ID */
					lapic->flags = 1;
					apic->len += lapic->len;
				} else {
					struct acpi_local_x2apic *x2apic = (acpi_local_x2apic *)&apic->data[apic->len - sizeof(*apic)];
					x2apic->type = 9;
					x2apic->len = 16;
					x2apic->reserved1[0] = 0;
					x2apic->reserved1[1] = 0;
					x2apic->x2apic_id = apicid;
					x2apic->proc_uid = 0;
					x2apic->flags = 1;
					apic->len += x2apic->len;
				}

				pnum++;
			}
		}
	}

	apic->checksum += checksum(apic, apic->len);
	tables_add(apic->len);

	if (rsdt) assert(replace_child("APIC", apic, rsdt, 4));
	if (xsdt) assert(replace_child("APIC", apic, xsdt, 8));

	acpi_sdt_p oslit = find_sdt("SLIT");

	/* Make SLIT info from scratch (ie replace existing table if any) */
	acpi_sdt_p slit = (acpi_sdt_p)tables_next;

	memcpy(slit->sig.s, "SLIT", 4);
	slit->revision = ACPI_REV;
	memcpy(slit->oemid, "NUMASC", 6);
	memcpy(slit->oemtableid, "N313NUMA", 8);
	slit->oemrev = 1;
	memcpy(slit->creatorid, "1B47", 4);
	slit->creatorrev = 1;
	slit->checksum = 0;
	memset(slit->data, 0, 8); /* Number of System Localities */
	int index = 0;
	const int nbs = nodes->nb_ht_hi - nodes->nb_ht_lo + 1;
	uint8_t *odist, *dist = (uint8_t *)&(slit->data[8]);

	if (oslit) {
		odist = (uint8_t *)&(oslit->data[8]);
		assert(odist[0] <= 13);
	} else
		odist = NULL;

	if (verbose > 1) {
		printf("Topology distances:\n   ");
		for (int snode = 0; snode < dnc_node_count; snode++)
			for (ht_t snb = 0; snb < nbs; snb++)
				printf(" %3d", snode * nbs + snb);
		printf("\n");
	}

	for (sci_t snode = 0; snode < dnc_node_count; snode++) {
		for (ht_t snb = 0; snb < nbs; snb++) {
			if (verbose > 1)
				printf("%2d:", snode * nbs + snb);

			for (sci_t dnode = 0; dnode < dnc_node_count; dnode++) {
				for (ht_t dnb = 0; dnb < nbs; dnb++) {
					if (snode == dnode) {
						if (snb == dnb)
							/* Linux requires distance to be 10 to same node */
							dist[index] = 10;
						else
							dist[index] = oslit ? odist[snb + dnb * nbs] : 16;
					} else
						dist[index] = dist_fn(nodes[snode].sci, nodes[dnode].sci);
					if (verbose > 1)
						printf(" %3d", dist[index]);
					index++;
				}
			}

			if (verbose > 1)
				printf("\n");
		}
	}

	memcpy(slit->data, &ht_pdom_count, sizeof(ht_pdom_count));
	slit->len = 44 + ht_pdom_count * ht_pdom_count;
	slit->checksum = checksum(slit, slit->len);
	tables_add(slit->len);

	if (rsdt) assert(replace_child("SLIT", slit, rsdt, 4));
	if (xsdt) assert(replace_child("SLIT", slit, xsdt, 8));

	/* With SRAT we reuse the old info and add our new entries */
	acpi_sdt_p osrat = find_sdt("SRAT");
	if (!osrat) {
		printf("Default ACPI SRAT table not found\n");
		return;
	}

	acpi_sdt_p srat = (acpi_sdt_p)tables_next;

	memcpy(srat, osrat, osrat->len);
	memcpy(srat->oemid, "NUMASC", 6);
	srat->len = 48;

	for (node = 0; node < dnc_node_count; node++) {
		for (ht = nodes[node].nb_ht_lo; ht <= nodes[node].nb_ht_hi; ht++) {
			struct acpi_mem_affinity mem;
			memset(&mem, 0, sizeof(mem));
			mem.type     = 1;
			mem.len      = sizeof(mem);
			mem.prox_dom = nodes[node].ht[ht].pdom;
			mem.mem_base = (uint64_t)nodes[node].ht[ht].base << DRAM_MAP_SHIFT;
			mem.mem_size = (uint64_t)nodes[node].ht[ht].size << DRAM_MAP_SHIFT;
			mem.enabled  = 1;
			mem.hotplug  = 0;
			mem.nonvol   = 0;
			memcpy((unsigned char *)srat + srat->len, &mem, mem.len);
			srat->len += mem.len;
		}
	}

	for (node = 0; node < dnc_node_count; node++) {
		for (ht = nodes[node].nb_ht_lo; ht <= nodes[node].nb_ht_hi; ht++) {
			for (j = 0; j < nodes[node].ht[ht].cores; j++) {
				uint16_t apicid = j + nodes[node].ht[ht].apic_base + nodes[node].apic_offset;

				if (ht_next_apic < 0x100) {
					struct acpi_core_affinity core;
					memset(&core, 0, sizeof(core));
					core.type     = 0;
					core.len      = sizeof(core);
					core.prox_low = nodes[node].ht[ht].pdom & 0xff;
					core.apic_id  = apicid;
					core.enabled  = 1;
					core.flags    = 0;
					core.sapic_eid = 0;
					core.prox_hi   = nodes[node].ht[ht].pdom >> 8;
					memcpy((unsigned char *)srat + srat->len, &core, core.len);
					srat->len += core.len;
				} else {
					struct acpi_x2apic_affinity x2core;
					memset(&x2core, 0, sizeof(x2core));
					x2core.type     = 2;
					x2core.len      = sizeof(x2core);
					x2core.prox_dom = nodes[node].ht[ht].pdom;
					x2core.x2apic_id = apicid;
					x2core.enabled  = 1;
					x2core.flags    = 0;
					x2core.clock_dom = ~0;
					memcpy((unsigned char *)srat + srat->len, &x2core, x2core.len);
					srat->len += x2core.len;
				}
			}
		}
	}

	srat->checksum += checksum(srat, srat->len);
	tables_add(srat->len);

	if (rsdt) assert(replace_child("SRAT", srat, rsdt, 4));
	if (xsdt) assert(replace_child("SRAT", srat, xsdt, 8));

	/* MCFG table */
	acpi_sdt_p mcfg = (acpi_sdt_p)tables_next;
	memset(mcfg, 0, sizeof(*mcfg) + 8);
	memcpy(mcfg->sig.s, "MCFG", 4);
	mcfg->len = offsetof(struct acpi_sdt, data) + 8 ; /* Count 'reserved' field */
	mcfg->revision = ACPI_REV;
	memcpy(mcfg->oemid, "NUMASC", 6);
	memcpy(mcfg->oemtableid, "N313NUMA", 8);
	mcfg->oemrev = 0;
	memcpy(mcfg->creatorid, "1B47", 4);
	mcfg->creatorrev = 1;

	for (node = 0; node < (remote_io ? dnc_node_count : 1); node++) {
		struct acpi_mcfg_allocation *mcfg_allocation = (struct acpi_mcfg_allocation *)((unsigned char *)mcfg + mcfg->len);
		memset(mcfg_allocation, 0, sizeof(*mcfg_allocation));
		mcfg_allocation->address = DNC_MCFG_BASE | ((uint64_t)nodes[node].sci << 28ULL);
		mcfg_allocation->pci_segment = node;
		mcfg_allocation->start_bus_number = 0;
		mcfg_allocation->end_bus_number = 255;
		mcfg->len += sizeof(*mcfg_allocation);
	}

	mcfg->checksum = 0;
	mcfg->checksum = checksum(mcfg, mcfg->len);
	tables_add(mcfg->len);

	if (rsdt) assert(replace_child("MCFG", mcfg, rsdt, 4));
	if (xsdt) assert(replace_child("MCFG", mcfg, xsdt, 8));
}

static void update_acpi_tables_late(void)
{
	uint32_t extra_len;
	unsigned char *extra = remote_aml(&extra_len);

	if (!acpi_append(rsdt, 4, "SSDT", extra, extra_len))
		if (!acpi_append(rsdt, 4, "DSDT", extra, extra_len)) {
			/* Appending to existing DSDT or SSDT failed; construct new SSDT */
			acpi_sdt_p ssdt = (acpi_sdt_p)tables_next;
			memset(ssdt, 0, sizeof(*ssdt) + 8);
			memcpy(ssdt->sig.s, "SSDT", 4);
			ssdt->revision = ACPI_REV;
			memcpy(ssdt->oemid, "NUMASC", 6);
			memcpy(ssdt->oemtableid, "N313NUMA", 8);
			ssdt->oemrev = 0;
			memcpy(ssdt->creatorid, "1B47", 4);
			ssdt->creatorrev = 1;
			memcpy(ssdt->data, extra, extra_len);
			ssdt->len = offsetof(struct acpi_sdt, data) + extra_len;
			ssdt->checksum = 0;
			ssdt->checksum = checksum(ssdt, ssdt->len);
			tables_add(ssdt->len);

			add_child(ssdt, xsdt, 8);
			add_child(ssdt, rsdt, 4);
		}

	free(extra);

	/* Ensure BIOS-provided DSDT uses ACPI revision 2, to 64-bit intergers are accepted */
	acpi_sdt_p dsdt = find_child("DSDT", rsdt, 4);
	if (dsdt->revision < 2) {
		printf("Promoting DSDT to revision 2\n");
		dsdt->revision = 2;
		dsdt->checksum += checksum(dsdt, dsdt->len);
	}
}

static struct mp_floating_pointer *find_mptable(const char *start, int len) {
	const char *ret = NULL;
	int i;

	for (i = 0; i < len; i += 16) {
		if (*(uint32_t *)(start + i) == STR_DW_H("_MP_")) {
			ret = start + i;
			break;
		}
	}

	return (struct mp_floating_pointer *)ret;
}

static void update_mptable(void)
{
	struct mp_floating_pointer *mpfp;
	struct mp_config_table *mptable;
	unsigned int i;
	const char *ebda = (const char *)(*((unsigned short *)0x40e) * 16);
	mpfp = find_mptable(ebda, 1024);

	if (!mpfp)
		mpfp = find_mptable((const char *)(639 * 1024), 1024);

	if (!mpfp)
		mpfp = find_mptable((const char *)(0xf0000), 0x10000);

	if (!mpfp)
		return;

	printf("sig: %.4s, len: %d, rev: %d, chksum: %x, feat: %02x:%02x:%02x:%02x:%02x\n",
	       mpfp->sig.s, mpfp->len,  mpfp->revision,  mpfp->checksum,
	       mpfp->feature[0], mpfp->feature[1], mpfp->feature[2],
	       mpfp->feature[3], mpfp->feature[4]);

	if ((uint32_t)mpfp > (uint32_t)REL32(new_mpfp)) {
		memcpy((void *)REL32(new_mpfp), mpfp, sizeof(*mpfp));
		mpfp = (mp_floating_pointer *)REL32(new_mpfp);
	}

	mptable = mpfp->mptable;
	memcpy((void *)REL32(new_mptable), mptable, 512);
	mptable = (mp_config_table *)REL32(new_mptable);
	mpfp->mptable = mptable;
	mpfp->checksum += checksum((acpi_sdt_p)mpfp, mpfp->len);
	printf("sig: %.4s, len: %d, rev: %d, chksum: %x, oemid: %.8s, prodid: %.12s,\n"
	       "oemtable: %p, oemsz: %d, entries: %d, lapicaddr: %08x, elen: %d, echk: %x\n",
	       mptable->sig.s, mptable->len,  mptable->revision,  mptable->checksum,
	       mptable->oemid, mptable->prodid, mptable->oemtable, mptable->oemtablesz,
	       mptable->entries, mptable->lapicaddr, mptable->extlen, mptable->extchksum);
	i = 0x1d8; /* First unused entry */
	memcpy(&(mptable->data[i]), &(mptable->data[20]), 20); /* Copy AP 1 data */
	mptable->data[i + 1] = 0xf;
	mptable->checksum += checksum((acpi_sdt_p)mptable, mptable->len);
}

static void setup_apic_atts(void)
{
	uint32_t apic_shift;
	uint16_t i, j;
	apic_shift = 1;

	while (apic_per_node > (1 << apic_shift)) apic_shift++;

	if (apic_shift > 4)
		apic_shift = 4;

	printf("Setting up APIC ATT tables with shift %d...", apic_shift);

	/* Set APIC ATT for remote interrupts */
	for (i = 0; i < dnc_node_count; i++) {
		uint16_t snode = (i == 0) ? 0xfff0 : nodes[i].sci;
		uint16_t dnode, ht;

		dnc_write_csr(snode, H2S_CSR_G3_APIC_MAP_SHIFT, apic_shift - 1);
		dnc_write_csr(snode, H2S_CSR_G3_NC_ATT_MAP_SELECT, NC_ATT_APIC);

		for (j = 0; j < 64; j++)
			dnc_write_csr(snode, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + j * 4, nodes[0].sci);

		for (dnode = 0; dnode < dnc_node_count; dnode++) {
			uint16_t cur, min, max;
			min = ~0;
			max = 0;

			for (ht = nodes[dnode].nb_ht_lo; ht <= nodes[dnode].nb_ht_hi; ht++) {
				cur = nodes[dnode].apic_offset + nodes[dnode].ht[ht].apic_base;

				if (min > cur)
					min = cur;

				if (max < cur + nodes[dnode].ht[ht].cores)
					max = cur + nodes[dnode].ht[ht].cores;
			}

			min = min >> apic_shift;
			max = (max - 1) >> apic_shift;

			dnc_write_csr(snode, H2S_CSR_G3_NC_ATT_MAP_SELECT, NC_ATT_APIC | ((min >> 8) & 0xf));
			for (j = min; j <= max; j++)
				dnc_write_csr(snode, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + (j & 0xff) * 4, nodes[dnode].sci);
		}
	}
	printf("done\n");
}

static void add_scc_hotpatch_att(uint64_t addr, uint16_t node)
{
	uint64_t val;
	uint32_t base, lim;

	if (scc_started) {
		uint32_t att_idx = dnc_read_csr(0xfff0, H2S_CSR_G0_ATT_INDEX);
		att_idx = (att_idx >> 27) & 0xf;
		val = addr >> 20;

		while (att_idx > 0) {
			val = val >> 4;
			att_idx = att_idx >> 1;
		}

		att_idx = dnc_read_csr(0xfff0, H2S_CSR_G0_ATT_INDEX);
		dnc_write_csr(0xfff0, H2S_CSR_G0_ATT_INDEX, (att_idx & 0x78000000) | (val & 0xfff));

		if (val & ~0xfff) {
			val = dnc_read_csr(0xfff0, H2S_CSR_G0_ATT_ENTRY);
			assert(val == node);
		}

		dnc_write_csr(0xfff0, H2S_CSR_G0_ATT_ENTRY, node);
	} else {
		/* Set local ATT */
		dnc_write_csr(0xfff0, H2S_CSR_G0_ATT_INDEX,
		              (0x08000000 << 3) | /* Index Range (3 = 47:36) */
		              (addr >> 36)); /* Start index */
		dnc_write_csr(0xfff0, H2S_CSR_G0_ATT_ENTRY, node);

		for (int i = 0; i < 8; i++) {
			base = cht_read_conf(0, FUNC1_MAPS, 0x40 + (8 * i));
			lim = cht_read_conf(0, FUNC1_MAPS, 0x44 + (8 * i));

			if (base & 3) {
				base = (base >> 8) | (base & 3);
				lim = (lim >> 8) | (lim & 7);
			} else {
				base = 0;
				lim = 0;
			}

			cht_write_conf(nodes[0].nc_ht, 1, H2S_CSR_F1_RESOURCE_MAPPING_ENTRY_INDEX, i);
			cht_write_conf(nodes[0].nc_ht, 1, H2S_CSR_F1_DRAM_LIMIT_ADDRESS_REGISTERS, lim);
			cht_write_conf(nodes[0].nc_ht, 1, H2S_CSR_F1_DRAM_BASE_ADDRESS_REGISTERS, base);
		}
	}
}

static void disable_smm_handler(uint64_t smm_base)
{
	uint64_t val;
	uint64_t smm_addr;
	uint16_t node;
	uint32_t sreq_ctrl;
	int i;
	uint8_t *cur;

	if (smm_base && (smm_base != ~0ULL))
		smm_base += 0x8000;
	else
		return;

	printf("Disabling SMM handler at 0x%llx\n", smm_base);
	smm_addr = 0x200000000000ULL | smm_base;
	val = dnc_read_csr(0xfff0, H2S_CSR_G0_NODE_IDS);
	node = (val >> 16) & 0xfff;

	for (i = 0; i < nodes[0].nc_ht; i++)
		mmio_range(0xfff0, i, 10, 0x200000000000ULL, 0x2fffffffffffULL, nodes[0].nc_ht, 0, 0);

	add_scc_hotpatch_att(smm_addr, node);
	sreq_ctrl = dnc_read_csr(0xfff0, H2S_CSR_G3_SREQ_CTRL);
	dnc_write_csr(0xfff0, H2S_CSR_G3_SREQ_CTRL,
	              (sreq_ctrl & ~0xfff0) | (0xf00 << 4));
	/* val = mem64_read32(smm_addr);
	   printf("MEM %llx: %08lx\n", smm_base, val); */
	cur = &smm_handler_start;

	while (cur + 4 <= &smm_handler_end) {
		mem64_write32(smm_addr, *((uint32_t *)cur));
		smm_addr += 4;
		cur += 4;
	}

	if (cur + 2 <= &smm_handler_end) {
		mem64_write16(smm_addr, *((uint16_t *)cur));
		smm_addr += 2;
		cur += 2;
	}

	if (cur < &smm_handler_end) {
		mem64_write8(smm_addr, *cur);
		smm_addr++;
		cur++;
	}

	dnc_write_csr(0xfff0, H2S_CSR_G3_SREQ_CTRL, sreq_ctrl);

	for (i = 0; i < nodes[0].nc_ht; i++)
		mmio_range_del(0xfff0, i, 10);
}

static void setup_other_cores(void)
{
	uint32_t ht, apicid, oldid, i, val;
	volatile uint32_t *icr, *apic;

	/* Set H2S_Init */
	val = dnc_read_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL);
	dnc_write_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL, val | (1 << 12));

	uint64_t msr = rdmsr(MSR_APIC_BAR);
	apic = (volatile uint32_t *)((uint32_t)msr & ~0xfff);
	icr = (volatile uint32_t *)&apic[0x300 / 4];

	/* Set core watchdog timer to 21s */
	msr = (9 << 3);
	if (enable_nbwdt > 0)
		msr |= 1;
	wrmsr(MSR_CPUWDT, msr);
	*REL64(new_cpuwdt_msr) = msr;

	/* ERRATA #N28: Disable HT Lock mechanism on Fam10h
	 * AMD Email dated 31.05.2011 :
	 * There is a switch that can help with these high contention issues,
	 * but it isn't "productized" due to a very rare potential for live lock if turned on.
	 * Given that HUGE caveat, here is the information that I got from a good source:
	 * LSCFG[44] =1 will disable it. MSR number is C001_1020 */
	msr = rdmsr(MSR_LSCFG);
	if (family == 0x10) {
		msr |= 1ULL << 44;
		wrmsr(MSR_LSCFG, msr);
	}
	*REL64(new_lscfg_msr) = msr;

	/* Enable 64-bit MMIO config access */
	msr = rdmsr(MSR_CU_CFG2) | (1ULL << 50);

	/* AMD Fam 15h Errata #572: Access to PCI Extended Configuration Space in SMM is Blocked
	 * Suggested Workaround: BIOS should set MSRC001_102A[27] = 1b */
	if (family >= 0x15)
		msr |= 1ULL << 27;

	wrmsr(MSR_CU_CFG2, msr);
	*REL64(new_cucfg2_msr) = msr;

	printf("APICs");
	critical_enter();

	/* Start all local cores (not BSP) and let them run our init_trampoline */
	for (ht = nodes[0].nb_ht_lo; ht <= nodes[0].nb_ht_hi; ht++) {
		for (i = 0; i < nodes[0].ht[ht].cores; i++) {
			if ((ht == 0) && (i == 0))
				continue; /* Skip BSP */

			oldid = nodes[0].ht[ht].apic_base + i;
			apicid = nodes[0].apic_offset + oldid;

			*REL8(cpu_apic_renumber) = apicid;
			*REL8(cpu_apic_hi)       = 0;
			*REL32(cpu_status) = VECTOR_TRAMPOLINE;
			*REL64(rem_topmem_msr) = ~0ULL;
			*REL64(rem_smm_base_msr) = ~0ULL;

			apic[0x310 / 4] = oldid << 24;
			*icr = 0x00004500;

			while (*icr & 0x1000)
				cpu_relax();

			apic[0x310 / 4] = apicid << 24;
			assert(((uint32_t)REL32(init_dispatch) & ~0xff000) == 0);
			*icr = 0x00004600 | (((uint32_t)REL32(init_dispatch) >> 12) & 0xff);

			while (*icr & 0x1000)
				cpu_relax();

			while (*REL32(cpu_status) != 0)
				cpu_relax();

			msr = *REL64(rem_smm_base_msr);
			if (verbose > 1)
				printf(" %d/0x%llx", apicid, msr);
			if (disable_smm)
				disable_smm_handler(msr);
		}
	}

	critical_leave();
	printf(" online\n");
}

static void renumber_remote_bsp(node_info_t *const node)
{
	uint8_t i, j;
	uint16_t sci = node->sci;
	uint8_t max_ht = node->nc_ht;
	uint32_t val;
	printf(" [renumbering");

	for (i = 0; i < max_ht; i++) {
		val = dnc_read_conf(sci, 0, 24 + i, FUNC0_HT, 0x0);
		assert(val == 0x12001022 || val == 0x16001022);

		/* Disable traffic distribution */
		dnc_write_conf(sci, 0, 24 + i, FUNC0_HT, 0x164, 0);

		/* Route max_ht + 1 as max_ht */
		val = dnc_read_conf(sci, 0, 24 + i, FUNC0_HT, 0x40 + 4 * max_ht);
		dnc_write_conf(sci, 0, 24 + i, FUNC0_HT, 0x44 + 4 * max_ht, val);
	}

	/* Bump NC to max_ht + 1 */
	dnc_write_conf(sci, 0, 24 + max_ht, 0, H2S_CSR_F0_CHTX_NODE_ID,
	               (max_ht << 8) | (max_ht + 1));
	val = dnc_read_csr(sci, H2S_CSR_G3_HT_NODEID);

	for (i = 0; i < max_ht; i++) {
		val = dnc_read_conf(sci, 0, 24 + i, FUNC0_HT, 0x68);
		dnc_write_conf(sci, 0, 24 + i, FUNC0_HT, 0x68, (val & ~(1 << 15)) | 0x40f);
		/* Increase NodeCnt */
		val = dnc_read_conf(sci, 0, 24 + i, FUNC0_HT, 0x60);
		dnc_write_conf(sci, 0, 24 + i, FUNC0_HT, 0x60, val + 0x10);
		/* Route max_ht as 0 */
		val = dnc_read_conf(sci, 0, 24 + i, FUNC0_HT, 0x40);
		dnc_write_conf(sci, 0, 24 + i, FUNC0_HT, 0x40 + 4 * max_ht, val);
	}

	/* Renumber HT#0 */
	val = dnc_read_conf(sci, 0, 24 + node->bsp_ht, FUNC0_HT, 0x60);
	dnc_write_conf(sci, 0, 24 + node->bsp_ht, FUNC0_HT, 0x60,
	               (val & ~0x7707) | (max_ht << 12) | (max_ht << 8) | max_ht);

	for (i = 1; i <= max_ht; i++) {
		/* Update LkNode, SbNode */
		val = dnc_read_conf(sci, 0, 24 + i, FUNC0_HT, 0x60);
		dnc_write_conf(sci, 0, 24 + i, FUNC0_HT, 0x60,
		               (val & ~0x7700) | (max_ht << 12) | (max_ht << 8));

		/* Update VGA routing */
		val = dnc_read_conf(sci, 0, 24 + i, FUNC1_MAPS, 0xf4);
		dnc_write_conf(sci, 0, 24 + i, FUNC1_MAPS, 0xf4, (val & ~(7 << 4)) | max_ht);

		/* Ensure CC6 state save node is local */
		if (family >= 0x15) {
			val = dnc_read_conf(sci, 0, 24 + i, FUNC4_LINK, 0x128);
			if (((val >> 12) & 0xff) == 0)
				dnc_write_conf(sci, 0, 24 + i, FUNC4_LINK, 0x128, val | (max_ht << 12));
		}

		/* Update DRAM maps */
		for (j = 0; j < 8; j++) {
			val = dnc_read_conf(sci, 0, 24 + i, FUNC1_MAPS, 0x44 + 8 * j);

			if ((val & 7) == 0)
				dnc_write_conf(sci, 0, 24 + i, FUNC1_MAPS, 0x44 + 8 * j, val | max_ht);
		}

		/* Update low MMIO ranges leaving upper ranges */
		for (j = 0; j < 8; j++) {
			val = dnc_read_conf(sci, 0, 24 + i, FUNC1_MAPS, 0x84 + 8 * j);

			if ((val & 7) == 0)
				dnc_write_conf(sci, 0, 24 + i, FUNC1_MAPS, 0x84 + 8 * j, val | max_ht);
		}

		/* Update IO maps */
		for (j = 0; j < 4; j++) {
			val = dnc_read_conf(sci, 0, 24 + i, FUNC1_MAPS, 0xc4 + 8 * j);

			if ((val & 7) == 0)
				dnc_write_conf(sci, 0, 24 + i, FUNC1_MAPS, 0xc4 + 8 * j, val | max_ht);
		}

		/* Update CFG maps */
		for (j = 0; j < 4; j++) {
			val = dnc_read_conf(sci, 0, 24 + i, FUNC1_MAPS, 0xe0 + 4 * j);

			if (((val >> 4) & 7) == 0)
				dnc_write_conf(sci, 0, 24 + i, FUNC1_MAPS, 0xe0 + 4 * j, val | (max_ht << 4));
		}
	}

	for (i = 1; i <= max_ht; i++) {
		val = dnc_read_conf(sci, 0, 24 + i, FUNC0_HT, 0x00);
		assert(val == 0x12001022 || val == 0x16001022);

		/* Route 0 as max_ht + 1 */
		val = dnc_read_conf(sci, 0, 24 + i, FUNC0_HT, 0x44 + 4 * max_ht);
		dnc_write_conf(sci, 0, 24 + i, FUNC0_HT, 0x40, val);
	}

	/* Move NC to HT#0, update SbNode, LkNode */
	dnc_write_conf(sci, 0, 24 + max_ht + 1, 0, H2S_CSR_F0_CHTX_NODE_ID,
	               (max_ht << 24) | (max_ht << 16) | (max_ht << 8) | 0);
	val = dnc_read_csr(sci, H2S_CSR_G3_HT_NODEID);

	/* Decrease NodeCnt */
	for (i = 1; i <= max_ht; i++) {
		val = dnc_read_conf(sci, 0, 24 + i, FUNC0_HT, 0x60);
		dnc_write_conf(sci, 0, 24 + i, FUNC0_HT, 0x60, val - 0x10);
	}

	/* Remove max_ht + 1 routing entry */
	for (i = 1; i <= max_ht; i++)
		dnc_write_conf(sci, 0, 24 + i, FUNC0_HT, 0x44 + 4 * max_ht, 0x40201);

	/* Reenable probes */
	for (i = 1; i <= max_ht; i++) {
		val = dnc_read_conf(sci, 0, 24 + i, FUNC0_HT, 0x68);
		dnc_write_conf(sci, 0, 24 + i, FUNC0_HT, 0x68, (val & ~0x40f) | (1 << 15));
	}

	memcpy(&node->ht[max_ht], &node->ht[0], sizeof(ht_node_info_t));
	node->nb_ht_lo = 1;
	node->nb_ht_hi = max_ht;
	node->bsp_ht = max_ht;
	node->nc_ht = 0;

#ifdef BROKEN
	/* Reorder the individual HT node memory base address so it is in increasing order */
	for (i = 1; i <= max_ht; i++) {
		if (i == 1) node->ht[i].base = node->dram_base;
		else        node->ht[i].base = node->ht[i-1].base + node->ht[i-1].size;
	}
#endif

	/* Rewrite high MMIO ranges to route CSR access to Numachip */
	for (i = 1; i <= max_ht; i++) {
		mmio_range(sci, i, 8, DNC_CSR_BASE, DNC_CSR_LIM, 0, 0, 1);
		mmio_range(sci, i, 9, DNC_MCFG_BASE, DNC_MCFG_LIM, 0, 0, 1);
	}

	printf("]");
}

static void setup_remote_cores(node_info_t *const node)
{
	uint8_t i, map_index;
	uint16_t sci = node->sci;
	uint16_t apicid, oldid;
	uint32_t j;
	uint32_t val;

	printf(" SCI%03x", sci);
	/* Toggle go-ahead flag to remote node */
	do {
		check_error();
		udelay(100000);
		val = dnc_read_csr(sci, H2S_CSR_G3_FAB_CONTROL);
	} while (!(val & 0x40000000UL));

	val |= 0x80000000UL;
	dnc_write_csr(sci, H2S_CSR_G3_FAB_CONTROL, val);

	do {
		udelay(200);
		val = dnc_read_csr(sci, H2S_CSR_G3_FAB_CONTROL);
	} while (val & 0x80000000UL);

	/* Setup remote MMIO */
	for (j = 0; j < 4096; j++) {
		if ((j & 0xff) == 0)
			dnc_write_csr(sci, H2S_CSR_G3_NC_ATT_MAP_SELECT, NC_ATT_MMIO32 | (j >> 8));
		dnc_write_csr(sci, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + (j & 0xff) * 4, nodes[0].sci);
	}

	if (renumber_bsp == 1)
		renumber_remote_bsp(node);

	val = dnc_read_csr(sci, H2S_CSR_G3_HREQ_CTRL);
	dnc_write_csr(sci, H2S_CSR_G3_HREQ_CTRL, val | (1 << 12));

	/* Check additional IO range registers */
	for (i = 0; i < 2; i++) {
		uint64_t qval = rdmsr(MSR_IORR_PHYS_MASK0 + i * 2);
		assertf(!(qval & (1 << 11)), "IO range 0x%llx is enabled", rdmsr(MSR_IORR_PHYS_BASE0 + i * 2) & (~0xfffULL));
	}

	for (i = node->nb_ht_lo; i <= node->nb_ht_hi; i++) {
		int range = 0;

		/* 1st MMIO map pair is set to point to the VGA segment A0000-C0000 */
		mmio_range(sci, i, range++, MMIO_VGA_BASE, MMIO_VGA_LIMIT, node->nc_ht, 0, 1);

		/* 2nd MMIO map pair is set to point to MMIO between TOM and 4G */
		/* FIXME: scope master's PCI bus */
		uint64_t tom = rdmsr(MSR_TOPMEM);
		mmio_range(sci, i, range++, tom, 0xffffffff, node->nc_ht, 0, 1);

		/* Clear low MMIO ranges, leaving high ranges */
		while (range < 8)
			mmio_range_del(sci, i, range++);

		/* Make sure the VGA Enable register is disabled to forward VGA transactions
		 * (MMIO A_0000h - B_FFFFh and I/O 3B0h - 3BBh or 3C0h - 3DFh) to the NumaChip */
		dnc_write_conf(sci, 0, 24 + i, FUNC1_MAPS, 0xf4, 0x0);
		if (dnc_read_conf(sci, 0, 24 + i, FUNC1_MAPS, 0xf4))
			warning("Legacy VGA access is locked to local server; some video card BIOSs may cause any X servers to fail to complete initialisation");
	}

	/* Now, reset all DRAM maps */
	/* Read DRAM Hole register off master BSP */
	uint32_t memhole = cht_read_conf(0, FUNC1_MAPS, 0xf0);

	for (i = node->nb_ht_lo; i <= node->nb_ht_hi; i++) {
		/* If Memory hoisting is enabled on master BSP, copy DramHoleBase[31:24] and DramMemHoistValid[1] */
		dnc_write_conf(sci, 0, 24 + i, FUNC1_MAPS, 0xf0, memhole & 0xff000002);

		/* Re-direct everything below our first local address to NumaChip */
		dram_range(sci, i, 0, (uint64_t)nodes[0].ht[0].base << DRAM_MAP_SHIFT, ((uint64_t)((node - 1)->dram_limit) << DRAM_MAP_SHIFT) - 1, node->nc_ht);

		/* Clear remaining entries */
		for (j = 1; j < 8; j++)
			dram_range_del(sci, i, j);
	}

	/* Reprogram HT node "self" ranges */
	for (i = node->nb_ht_lo; i <= node->nb_ht_hi; i++) {
		/* Check if DRAM channels are unganged */
		val = dnc_read_conf(sci, 0, 24 + i, FUNC2_DRAM, 0x110);
		if (val & 1) {
			/* Note offset from base */
			uint32_t base = dnc_read_conf(sci, 0, 24 + i, FUNC1_MAPS, 0x120) & 0x1fffff;
			uint32_t rlow = ((dnc_read_conf(sci, 0, 24 + i, FUNC2_DRAM, 0x110) >> 11) - base) << (27 - DRAM_MAP_SHIFT);
			uint32_t rhigh = ((dnc_read_conf(sci, 0, 24 + i, FUNC2_DRAM, 0x114) >> 11) - base) << (27 - DRAM_MAP_SHIFT);

			/* Reprogram DCT base/offset values against new base */
			dnc_write_conf(sci, 0, 24 + i, FUNC2_DRAM, 0x110, (val & 0x7ff) |
			               (((node->ht[i].base + rlow) >> (27 - DRAM_MAP_SHIFT)) << 11));
			dnc_write_conf(sci, 0, 24 + i, FUNC2_DRAM, 0x114,
			               ((node->ht[i].base + rhigh) >> (26 - DRAM_MAP_SHIFT)) << 10);
		}

		dnc_write_conf(sci, 0, 24 + i, FUNC1_MAPS, 0x120,
		               node->ht[i].base >> (27 - DRAM_MAP_SHIFT));
		/* Account for Cstate6 save area */
		dnc_write_conf(sci, 0, 24 + i, FUNC1_MAPS, 0x124,
		               (node->ht[i].base + node->ht[i].size - 1 + pf_cstate6) >> (27 - DRAM_MAP_SHIFT));
	}

	/* Program our local DRAM ranges */
	for (map_index = 0, i = node->nb_ht_lo; i <= node->nb_ht_hi; i++) {
		for (j = node->nb_ht_lo; j <= node->nb_ht_hi; j++)
			dram_range(sci, j, map_index + 1, (uint64_t)node->ht[i].base << DRAM_MAP_SHIFT,
				((uint64_t)(node->ht[i].base + node->ht[i].size) << DRAM_MAP_SHIFT) - 1, i);

		uint64_t base = (uint64_t)node->ht[i].base << DRAM_MAP_SHIFT;
		uint64_t limit = ((uint64_t)(node->ht[i].base + node->ht[i].size) << DRAM_MAP_SHIFT) - 1;
		nc_dram_range(sci, map_index++, base, limit, i);
	}

	/* Re-direct everything above our last local DRAM address (if any) to NumaChip */
	if (node < &nodes[dnc_node_count - 1]) {
		for (i = node->nb_ht_lo; i <= node->nb_ht_hi; i++)
			dram_range(sci, i, map_index + 1, (uint64_t)node->dram_limit << DRAM_MAP_SHIFT,
				((uint64_t)nodes[dnc_node_count - 1].dram_limit << DRAM_MAP_SHIFT) - 1, node->nc_ht);

		map_index++;
	}

	dnc_write_csr(sci, H2S_CSR_G3_PCI_SEG0, nodes[0].sci << 16);

	/* Quick and dirty: zero out I/O and config space maps; add
	 * all-covering map towards DNC */
	/* Note that rewriting F1xE0 prevents remote PCI config access hitting the remote
	   bus and it is decoded locally */
	for (i = node->nb_ht_lo; i <= node->nb_ht_hi; i++) {
		dnc_write_conf(sci, 0, 24 + i, FUNC1_MAPS, 0xc4, 0x00fff000 | node->nc_ht);
		dnc_write_conf(sci, 0, 24 + i, FUNC1_MAPS, 0xc0, 0x00000003);

		for (j = 0xc8; j <= 0xdc; j += 4)
			dnc_write_conf(sci, 0, 24 + i, FUNC1_MAPS, j, 0);

		if (!remote_io) {
			dnc_write_conf(sci, 0, 24 + i, FUNC1_MAPS, 0xe0, 0xff000003 | (node->nc_ht << 4));

			for (j = 0xe4; j <= 0xec; j += 4)
				dnc_write_conf(sci, 0, 24 + i, FUNC1_MAPS, j, 0);
		}
	}

	/* Set DRAM range on local NumaChip */
	dnc_write_csr(sci, H2S_CSR_G0_MIU_NGCM0_LIMIT, node->dram_base >> 6);
	dnc_write_csr(sci, H2S_CSR_G0_MIU_NGCM1_LIMIT, (node->dram_limit >> 6) - 1);
	dnc_write_csr(sci, H2S_CSR_G3_DRAM_SHARED_BASE, node->dram_base);
	dnc_write_csr(sci, H2S_CSR_G3_DRAM_SHARED_LIMIT, node->dram_limit);

	/* "Wraparound" entry, lets APIC 0xff00 - 0xffff target 0x0 to 0xff on destination node */
	dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, NC_ATT_APIC | 0xf);
	i = dnc_read_csr(0xfff0, H2S_CSR_G3_APIC_MAP_SHIFT) + 1;

	for (j = (0xff00 >> i) & 0xff; j < 0x100; j++)
		dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + j * 4, sci);

	*REL64(new_mcfg_msr) = DNC_MCFG_BASE | ((uint64_t)sci << 28ULL) | 0x21ULL;

	/* Start all remote cores and let them run our init_trampoline */
	for (ht_t ht = node->nb_ht_lo; ht <= node->nb_ht_hi; ht++) {
		for (i = 0; i < node->ht[ht].cores; i++) {
			oldid = node->ht[ht].apic_base + i;
			apicid = node->apic_offset + oldid;

			*REL8(cpu_apic_renumber) = apicid & 0xff;
			*REL8(cpu_apic_hi)       = (apicid >> 8) & 0x3f;
			*REL64(rem_topmem_msr) = ~0ULL;
			*REL64(rem_smm_base_msr) = ~0ULL;

			wake_core_global(oldid, VECTOR_TRAMPOLINE);

			if (verbose > 1)
				printf(" %d", apicid);

			uint64_t qval = *REL64(rem_smm_base_msr);
			if (disable_smm)
				disable_smm_handler(qval);
		}
	}
}

static char *read_file(const char *filename, int *len)
{
	static com32sys_t inargs, outargs;
	int fd, bsize;

	char *buf = (char *)lmalloc(strlen(filename) + 1);
	strcpy(buf, filename);

	printf("Opening %s...", filename);
	memset(&inargs, 0, sizeof inargs);
	inargs.eax.w[0] = 0x0006; /* Open file */
	inargs.esi.w[0] = OFFS(buf);
	inargs.es = SEG(buf);
	__intcall(0x22, &inargs, &outargs);
	lfree(buf);

	fd = outargs.esi.w[0];
	*len = outargs.eax.l;
	bsize = outargs.ecx.w[0];

	if (!fd || *len < 0) {
		*len = 0;
		printf("not found\n");
		return NULL;
	}

	buf = (char *)lzalloc(roundup(*len, bsize));
	assert(buf);

	memset(&inargs, 0, sizeof inargs);
	inargs.eax.w[0] = 0x0007; /* Read file */
	inargs.esi.w[0] = fd;
	inargs.ecx.w[0] = (*len / bsize) + 1;
	inargs.ebx.w[0] = OFFS(buf);
	inargs.es = SEG(buf);
	__intcall(0x22, &inargs, &outargs);
	*len = outargs.ecx.l;

	memset(&inargs, 0, sizeof inargs);
	inargs.eax.w[0] = 0x0008; /* Close file */
	inargs.esi.w[0] = fd;
	__intcall(0x22, &inargs, NULL);

	printf("done\n");
	return buf;
}

#ifdef UNUSED
static int convert_buf_uint32_t(char *src, uint32_t *dst, int max_offset)
{
	char *b;
	int offs = 0;

	for (b = strtok(src, " \n"); b != NULL && offs < max_offset; b = strtok(NULL, "\n")) {
		if (b[0] == '@') {
			offs = strtol(&b[1], NULL, 16);

			if (offs >= max_offset) {
				printf("Error converting offset %s, value too large\n", b);
				return -1;
			}
		} else {
			dst[offs++] = strtol(b, NULL, 16);
		}
	}

	return offs - 1;
}

static int convert_buf_uint16_t(char *src, uint16_t *dst, int max_offset)
{
	char *b;
	int offs = 0;

	for (b = strtok(src, " \n"); b != NULL && offs < max_offset; b = strtok(NULL, "\n")) {
		if (b[0] == '@') {
			offs = strtol(&b[1], NULL, 16);
			assert(offs < max_offset);
		} else
			dst[offs++] = strtol(b, NULL, 16);
	}

	return offs - 1;
}

static uint32_t mseq_ucode_update[1024];
static uint16_t mseq_table_update[128];

static void read_microcode_update(void)
{
	char path[512];
	int psep;
	int ucode_len, table_len;
	uint32_t ucode_xor, table_xor;
	uint16_t i;
	ucode_xor = 0;

	for (i = 0; i < mseq_ucode_length; i++) {
		ucode_xor = ucode_xor ^(i * mseq_ucode[i]);
	}

	table_xor = 0;

	for (i = 0; i < mseq_table_length; i++) {
		table_xor = table_xor ^(i * mseq_table[i]);
	}

	memset(path, 0, sizeof(path));
	strncpy(path, microcode_path, sizeof(path));

	psep = strlen(path);
	if (psep == 0) {
		printf("Using internal microcode (xor %u, %u)\n", ucode_xor, table_xor);
		return;
	}

	if ((psep > 0) && (path[psep - 1] != '/'))
		path[psep++] = '/';

	strcat(path, "mseq.code");
	int len;
	char *data = read_file(path, &len);
	assertf(data, "Microcode mseq.code not found");

	ucode_len = convert_buf_uint32_t(data, mseq_ucode_update, len);
	assertf(ucode_len >= 0, "Microcode mseq.code corrupted");
	lfree(data);

	path[psep] = '\0';
	strcat(path, "mseq.table");
	data = read_file(path, &len);
	assertf(data, "Microcode mseq.table not found");

	table_len = convert_buf_uint16_t(data, mseq_table_update, len);
	assetf(table_len >= 0, "Microcode mseq.table corrupted");
	lfree(data);

	mseq_ucode = mseq_ucode_update;
	mseq_table = mseq_table_update;
	mseq_ucode_length = ucode_len;
	mseq_table_length = table_len;
	ucode_xor = 0;

	for (i = 0; i < mseq_ucode_length; i++) {
		ucode_xor = ucode_xor ^(i * mseq_ucode[i]);
	}

	table_xor = 0;

	for (i = 0; i < mseq_table_length; i++) {
		table_xor = table_xor ^(i * mseq_table[i]);
	}

	printf("Using updated microcode (xor %u, %u)\n", ucode_xor, table_xor);
}
#endif

int read_config_file(const char *filename)
{
	int config_len;
	char *config = read_file(filename, &config_len);
	assertf(config, "Fabric configuration file <%s> not found", filename);

	config[config_len] = '\0';
	if (!parse_config_file(config))
		return -1;

	lfree(config);
	return 0;
}

static int pxeapi_call(int func, const uint8_t *buf)
{
	static com32sys_t inargs, outargs;
	inargs.eax.w[0] = 0x0009; /* Call PXE Stack */
	inargs.ebx.w[0] = func; /* PXE function number */
	inargs.edi.w[0] = OFFS(buf);
	inargs.es = SEG(buf);
	__intcall(0x22, &inargs, &outargs);
	return outargs.eax.w[0] == PXENV_EXIT_SUCCESS;
}

void udp_open(void)
{
	t_PXENV_TFTP_CLOSE *tftp_close_param = (t_PXENV_TFTP_CLOSE *)lzalloc(sizeof(t_PXENV_TFTP_CLOSE));
	assert(tftp_close_param);
	pxeapi_call(PXENV_TFTP_CLOSE, (uint8_t *)tftp_close_param);
	printf("TFTP close returns: %d\n", tftp_close_param->Status);
	lfree(tftp_close_param);

	t_PXENV_UDP_OPEN *pxe_open_param = (t_PXENV_UDP_OPEN *)lzalloc(sizeof(t_PXENV_UDP_OPEN));
	assert(pxe_open_param);
	pxe_open_param->src_ip = myip.s_addr;
	pxeapi_call(PXENV_UDP_OPEN, (uint8_t *)pxe_open_param);
	printf("PXE UDP open returns: %d\n", pxe_open_param->status);
	lfree(pxe_open_param);
}

void udp_broadcast_state(const void *buf, const size_t len)
{
	assert(len >= sizeof(struct state_bcast));

	t_PXENV_UDP_WRITE *pxe_write_param = (t_PXENV_UDP_WRITE *)lzalloc(sizeof(t_PXENV_UDP_WRITE) + len);
	assert(pxe_write_param);
	char *buf_reloc = (char *)pxe_write_param + sizeof(*pxe_write_param);

	pxe_write_param->ip = 0xffffffff;
	pxe_write_param->src_port = htons(4711);
	pxe_write_param->dst_port = htons(4711);
	pxe_write_param->buffer.seg = SEG(buf_reloc);
	pxe_write_param->buffer.offs = OFFS(buf_reloc);
	pxe_write_param->buffer_size = len;

	memcpy(buf_reloc, buf, len);
	pxeapi_call(PXENV_UDP_WRITE, (uint8_t *)pxe_write_param);
	lfree(pxe_write_param);
}

int udp_read_state(void *buf, const size_t len, uint32_t *ip)
{
	int ret = 0;

	t_PXENV_UDP_READ *pxe_read_param = (t_PXENV_UDP_READ *)lzalloc(sizeof(t_PXENV_UDP_READ) + len);
	assert(pxe_read_param);
	char *buf_reloc = (char *)pxe_read_param + sizeof(*pxe_read_param);

	pxe_read_param->s_port = htons(4711);
	pxe_read_param->d_port = htons(4711);
	pxe_read_param->buffer.seg = SEG(buf_reloc);
	pxe_read_param->buffer.offs = OFFS(buf_reloc);
	pxe_read_param->buffer_size = len;
	pxeapi_call(PXENV_UDP_READ, (uint8_t *)pxe_read_param);

	if ((pxe_read_param->status == PXENV_STATUS_SUCCESS) &&
	    (pxe_read_param->s_port == htons(4711))) {
		memcpy(buf, buf_reloc, pxe_read_param->buffer_size);
		*ip = pxe_read_param->src_ip;
		ret = pxe_read_param->buffer_size;
	}

	lfree(pxe_read_param);
	return ret;
}

static void wait_status(struct node_info *info)
{
	printf("Waiting for");

	for (int i = 0; i < cfg_nodes; i++) {
		if (config_local(&cfg_nodelist[i], info->uuid)) /* Self */
			continue;

		if (nodedata[cfg_nodelist[i].sci] != 0x80)
			printf(" SCI%03x/%s",
			       cfg_nodelist[i].sci, cfg_nodelist[i].desc);
	}

	printf("\n");
}

static void wait_for_slaves(struct node_info *info, struct part_info *part)
{
	struct state_bcast cmd;
	bool ready_pending = 1;
	int count, backoff, last_stat, i;
	bool do_restart = 0;
	enum node_state waitfor, own_state;
	uint32_t ip, last_cmd = ~0;
	size_t len;
	char buf[UDP_MAXLEN];
	struct state_bcast *rsp = (struct state_bcast *)buf;

	udp_open();

	cmd.sig = UDP_SIG;
	cmd.state = CMD_STARTUP;
	cmd.uuid  = info->uuid;
	cmd.sci = info->sci;
	cmd.tid   = 0; /* Must match initial rsp.tid for RSP_SLAVE_READY */
	waitfor = RSP_SLAVE_READY;
	printf("Waiting for %d nodes...\n", cfg_nodes - 1);
	count = 0;
	backoff = 1;
	last_stat = 0;

	while (1) {
		if (++count >= backoff) {
			udp_broadcast_state(&cmd, sizeof(cmd));

			udelay(100 * backoff);
			last_stat += backoff;

			if (backoff < 32)
				backoff = backoff * 2;

			count = 0;
		}

		if (cmd.state == CMD_CONTINUE)
			break;

		if (last_cmd != cmd.tid) {
			/* Perform commands locally as well */
			if (handle_command(cmd.state, &own_state, info, part))
				do_restart = own_state != waitfor;

			if (do_restart)
				printf("Command did not complete successfully on master (reason %s), resetting...\n",
				       node_state_name[own_state]);

			nodedata[info->sci] = 0x80;
			last_cmd = cmd.tid;
		}

		if (cfg_nodes > 1) {
			len = udp_read_state(rsp, UDP_MAXLEN, &ip);
			if (!do_restart) {
				if (!do_restart) {
					if (last_stat > 64) {
						last_stat = 0;
						wait_status(info);
					}

					if (!len)
						continue;
				}
			}
		} else
			len = 0;

		if (len >= sizeof(rsp) && rsp->sig == UDP_SIG) {
			if (rsp->uuid == 0xffffffff) {
					error_remote(0xffffffff, "", ip, (char *)rsp + sizeof(struct state_bcast));
					continue;
			}

			bool ours = 0;

			for (i = 0; i < cfg_nodes; i++)
				if ((!name_matching && (cfg_nodelist[i].uuid == rsp->uuid)) ||
				    ( name_matching && (cfg_nodelist[i].sci == rsp->sci))) {
					ours = 1;
					break;
				}

			if (ours) {
				if ((rsp->state == waitfor) && (rsp->tid == cmd.tid)) {
					if (nodedata[rsp->sci] != 0x80)
						nodedata[rsp->sci] = 0x80;
				} else if ((rsp->state == RSP_PHY_NOT_TRAINED) ||
				           (rsp->state == RSP_RINGS_NOT_OK) ||
				           (rsp->state == RSP_FABRIC_NOT_READY) ||
				           (rsp->state == RSP_FABRIC_NOT_OK)) {
					if (nodedata[rsp->sci] != 0x80) {
						printf("SCI%03x (%s) aborted with state %d; restarting process...\n",
						       rsp->sci, cfg_nodelist[i].desc, rsp->state);
						do_restart = 1;
						nodedata[rsp->sci] = 0x80;
					}
				} else if (rsp->state == RSP_ERROR)
					error_remote(rsp->sci, cfg_nodelist[i].desc, ip, (char *)rsp + sizeof(struct state_bcast));
			}
		}

		ready_pending = 0;

		for (i = 0; i < cfg_nodes; i++) {
			if (config_local(&cfg_nodelist[i], info->uuid)) /* Self */
				continue;

			if (!(nodedata[cfg_nodelist[i].sci] & 0x80)) {
				ready_pending = 1;
				break;
			}
		}

		if (!ready_pending || do_restart) {
			if (do_restart) {
				cmd.state = CMD_RESET_FABRIC;
				waitfor = RSP_RESET_COMPLETE;
				do_restart = 0;
			} else if (cmd.state == CMD_STARTUP) {
				/* Skip over resetting fabric, as that's just if training fails */
				cmd.state = CMD_TRAIN_FABRIC;
				waitfor = RSP_PHY_TRAINED;
			} else if (cmd.state == CMD_TRAIN_FABRIC) {
				cmd.state = CMD_VALIDATE_RINGS;
				waitfor = RSP_RINGS_OK;
			} else if (cmd.state == CMD_RESET_FABRIC) {
				/* When invoked, continue at fabric training */
				cmd.state = CMD_TRAIN_FABRIC;
				waitfor = RSP_PHY_TRAINED;
			} else if (cmd.state == CMD_VALIDATE_RINGS) {
				cmd.state = CMD_SETUP_FABRIC;
				waitfor = RSP_FABRIC_READY;
			} else if (cmd.state == CMD_SETUP_FABRIC) {
				cmd.state = CMD_VALIDATE_FABRIC;
				waitfor = RSP_FABRIC_OK;
			} else if (cmd.state == CMD_VALIDATE_FABRIC) {
				cmd.state = CMD_CONTINUE;
				waitfor = RSP_NONE;
			}

			/* Clear seen flag */
			for (i = 0; i < cfg_nodes; i++)
				nodedata[cfg_nodelist[i].sci] &= 0x7f;

			cmd.tid++;
			count = 0;
			backoff = 1;
			printf("Issuing %s; expecting %s\n",
			       node_state_name[cmd.state], node_state_name[waitfor]);
		}
	}
}

void mtrr_range(const uint64_t base, const uint64_t limit, const int type)
{
	uint64_t val;
	int i = -1;

	/* Find next unused entry */
	do {
		i++;
		assert(i < 8);
		val = rdmsr(MSR_MTRR_PHYS_MASK0 + i * 2);
	} while (val);

	uint64_t *mtrr_var_base = REL64(new_mtrr_var_base);
	uint64_t *mtrr_var_mask = REL64(new_mtrr_var_mask);

	mtrr_var_base[i] = base | type;
	wrmsr(MSR_MTRR_PHYS_BASE0 + i * 2, mtrr_var_base[i]);
	mtrr_var_mask[i] = (((1ULL << 48) - 1) &~ (limit - base - 1)) | 0x800;
	wrmsr(MSR_MTRR_PHYS_MASK0 + i * 2, mtrr_var_mask[i]);
}

static void update_mtrr(void)
{
	/* Ensure Tom2ForceMemTypeWB (bit 22) is set, so memory between 4G and TOM2 is writeback */
	uint64_t *syscfg_msr = REL64(new_syscfg_msr);
	*syscfg_msr = rdmsr(MSR_SYSCFG) | (1 << 22);
	wrmsr(MSR_SYSCFG, *syscfg_msr);

	/* Ensure default memory type is uncacheable */
	uint64_t *mtrr_default = REL64(new_mtrr_default);
	*mtrr_default = 3 << 10;
	wrmsr(MSR_MTRR_DEFAULT, *mtrr_default);

	/* Store fixed MTRRs */
	uint64_t *new_mtrr_fixed = REL64(new_mtrr_fixed);
	uint32_t *fixed_mtrr_regs = REL32(fixed_mtrr_regs);

	printf("Fixed MTRRs:\n");
	for (int i = 0; fixed_mtrr_regs[i] != 0xffffffff; i++) {
		new_mtrr_fixed[i] = rdmsr(fixed_mtrr_regs[i]);
		printf("- 0x%016llx\n", new_mtrr_fixed[i]);
	}

	/* Store variable MTRRs */
	uint64_t *mtrr_var_base = REL64(new_mtrr_var_base);
	uint64_t *mtrr_var_mask = REL64(new_mtrr_var_mask);
	printf("Variable MTRRs:\n");

	for (int i = 0; i < 8; i++) {
		mtrr_var_base[i] = rdmsr(MSR_MTRR_PHYS_BASE0 + i * 2);
		mtrr_var_mask[i] = rdmsr(MSR_MTRR_PHYS_MASK0 + i * 2);

		if (mtrr_var_mask[i] & 0x800ULL) {
			printf("- 0x%011llx:0x%011llx %s\n", mtrr_var_base[i] & ~0xfffULL,
			       mtrr_var_mask[i] & ~0xfffULL, MTRR_TYPE(mtrr_var_base[i] & 0xffULL));
		}
	}
}

#ifdef UNUSED
static void disable_iommu(void)
{
	uint32_t val, val2;
	/* 0x60/64 is SR56x0 NBMISCIND port */
	/* Enable access to device function 2 if needed */
	dnc_write_conf(0xfff0, 0, 0, 0, 0x60, 0x75);
	val = dnc_read_conf(0xfff0, 0, 0, 0, 0x64);

	if ((val & 1) == 0) {
		dnc_write_conf(0xfff0, 0, 0, 0, 0x60, 0x75 | 0x80);
		dnc_write_conf(0xfff0, 0, 0, 0, 0x64, val | 1);
	};

	/* SR56x0 F2 0x44/48 is IOMMU config index/data port
	 * 0x18 is IOMMU_MMIO_CNTRL_0 */
	dnc_write_conf(0xfff0, 0, 0, 2, 0x44, 0x18);

	val2 = dnc_read_conf(0xfff0, 0, 0, 2, 0x48);

	if (val2 & 1) {
		printf("- disabling IOMMU (0x%x)\n", val2);
		dnc_write_conf(0xfff0, 0, 0, 2, 0x44, 0x18);
		dnc_write_conf(0xfff0, 0, 0, 2, 0x48, 0);
	}

	/* Hide device function 2 if previously hidden */
	if ((val & 1) == 0) {
		dnc_write_conf(0xfff0, 0, 0, 0, 0x60, 0x75 | 0x80);
		dnc_write_conf(0xfff0, 0, 0, 0, 0x64, val);
	}
}
#endif

static void lvt_setup(void)
{
	uint32_t val;

	for (int ht = 0; ht < nodes[0].nc_ht; ht++) {
		val = cht_read_conf(ht, FUNC3_MISC, 0xb0);
		if (((val >> 14) & 3) == 2) {
			printf("- disabling ECC error SMI on HT#%d\n", ht);
			cht_write_conf(ht, FUNC3_MISC, 0xb0, val & ~(3 << 14));
		}

		val = cht_read_conf(ht, FUNC3_MISC, 0xb0);
		if (((val >> 12) & 3) == 2) {
			printf("- disabling online DIMM swap complete SMI on HT#%d\n", ht);
			cht_write_conf(ht, FUNC3_MISC, 0xb0, val & ~(3 << 12));
		}

		val = cht_read_conf(ht, FUNC3_MISC, 0x1d4);
		if (((val >> 22) & 3) == 2) {
			printf("- disabling probe filter error SMI on HT#%d\n", ht);
			cht_write_conf(ht, FUNC3_MISC, 0x1d4, val & ~(3 << 22));
		}
	}

}

static void local_chipset_fixup(const bool master)
{
	uint32_t val;
	val = dnc_read_conf(0xfff0, 0, 0x14, 0, 0);

	if (val == VENDEV_SP5100) {
		uint8_t val8 = pmio_readb(0x00);
		if (val8 & 6) {
			printf("- disabling PM IO timers\n");
			pmio_writeb(0x00, val8 & ~6);
		}

		val8 = pmio_readb(0xa8);
		if (val8 & 0x30) {
			printf("- disabling config-space triggered SMI\n");
			pmio_writeb(0xa8, val8 & ~0x30);
		}

		val8 = pmio_readb(0xd2);
		if (val8 & 1) {
			printf("- disabling BMC I2C link\n");
			pmio_writeb(0xd2, val8 & ~1);
		}

		const uint8_t sources[] = {0x02, 0x03, 0x04, 0x1c, 0xa8};
		for (unsigned int i = 0; i < sizeof(sources); i++) {
			val8 = pmio_readb(sources[i]);
			if (val8) {
				printf("- disabling SMI sources 0x%02x in PM IO register 0x%02x\n", val8, sources[i]);
				pmio_writeb(sources[i], 0);
			}
		}

		/* Enable SP5100 SATA MSI support */
		uint32_t val2 = dnc_read_conf(0xfff0, 0, 17, 0, 0x40);
		dnc_write_conf(0xfff0, 0, 17, 0, 0x40, val2 | 1);
		val = dnc_read_conf(0xfff0, 0, 17, 0, 0x60);
		dnc_write_conf(0xfff0, 0, 17, 0, 0x60, (val & ~0xff00) | 0x5000);
		dnc_write_conf(0xfff0, 0, 17, 0, 0x40, val2);

		if (!master) {
			/* Disable and hide SP5100 IDE controller */
			dnc_write_conf(0xfff0, 0, 20, 1, 4, 0);
			val = dnc_read_conf(0xfff0, 0, 20, 0, 0xac);
			dnc_write_conf(0xfff0, 0, 20, 0, 0xac, val | (1 << 19));

			/* Disable and hide VGA controller */
			dnc_write_conf(0xfff0, 4, 6, 0, 4, 0);
			val = dnc_read_conf(0xfff0, 0, 20, 4, 0xfc);
			dnc_write_conf(0xfff0, 0, 20, 4, 0x5c, val & ~0xffff);

			/* Disable the LPC controller */
			dnc_write_conf(0xfff0, 0, 14, 3, 4, 0);
			val = dnc_read_conf(0xfff0, 0, 20, 4, 0x64);
			dnc_write_conf(0xfff0, 0, 20, 4, 0x64, val & ~(1 << 20));

			/* Disable OHCI */
			dnc_write_conf(0xfff0, 0, 12, 0, 4, 0);
			dnc_write_conf(0xfff0, 0, 12, 1, 4, 0);
			dnc_write_conf(0xfff0, 0, 12, 2, 4, 0);
			dnc_write_conf(0xfff0, 0, 13, 0, 4, 0);
			dnc_write_conf(0xfff0, 0, 13, 1, 4, 0);
			dnc_write_conf(0xfff0, 0, 13, 2, 4, 0);

			val = dnc_read_conf(0xfff0, 0, 20, 0, 0x68);
			dnc_write_conf(0xfff0, 0, 20, 0, 0x68, val & ~0xf7);

			/* Disable and hide HD audio */
			dnc_write_conf(0xfff0, 1, 0, 1, 4, 0);
			val8 = pmio_readb(0x59);
			pmio_writeb(0x59, val8 & ~(1 << 3));

			val = dnc_read_conf(0xfff0, 0, 20, 4, 0x5c);
			dnc_write_conf(0xfff0, 0, 20, 4, 0x5c, val & 0xffff);

			/* Hide SMBus controller */
			dnc_write_conf(0xfff0, 0, 20, 0, 4, 0);
			val8 = pmio_readb(0xba);
			pmio_writeb(0xba, val8 | (1 << 6));

			val = dnc_read_conf(0xfff0, 0, 20, 0, 0x40);
			dnc_write_conf(0xfff0, 0, 20, 0, 0x40, val & ~(1 << 28));
		}
	}
	/* Only needed to workaround rev A/B issue */
	if (dnc_asic_mode && dnc_chip_rev < 2) {
		uint16_t node;
		uint64_t addr;
		int i;
		uint32_t sreq_ctrl;
		addr = rdmsr(MSR_SMM_BASE) + 0x8000 + 0x37c40;
		addr += 0x200000000000ULL;
		val = dnc_read_csr(0xfff0, H2S_CSR_G0_NODE_IDS);
		node = (val >> 16) & 0xfff;

		for (i = 0; i < nodes[0].nc_ht; i++)
			mmio_range(0xfff0, i, 10, 0x200000000000ULL, 0x2fffffffffffULL, nodes[0].nc_ht, 0, 0);

		add_scc_hotpatch_att(addr, node);
		sreq_ctrl = dnc_read_csr(0xfff0, H2S_CSR_G3_SREQ_CTRL);
		dnc_write_csr(0xfff0, H2S_CSR_G3_SREQ_CTRL,
		              (sreq_ctrl & ~0xfff0) | (0xf00 << 4));
		val = mem64_read32(addr + 4);
		printf("SMM fingerprint: %08x\n", val);

		if (val == 0x3160bf66) {
			printf("SMM coh config space trigger fingerprint found, patching...\n");
			val = mem64_read32(addr);
			mem64_write32(addr, (val & 0xff) | 0x9040eb00);
		}

		val = mem64_read32(addr);
		printf("MEM %llx: %08x\n", addr, val);
		dnc_write_csr(0xfff0, H2S_CSR_G3_SREQ_CTRL, sreq_ctrl);

		for (i = 0; i < nodes[0].nc_ht; i++)
			mmio_range_del(0xfff0, i, 10);
	}

	pci_setup();
	lvt_setup();
}

static void global_chipset_fixup(void)
{
	uint16_t node;
	uint32_t val;
	int i;
	printf("Performing global chipset configuration...");

	for (i = 0; i < dnc_node_count; i++) {
		node = nodes[i].sci;
		uint32_t vendev = dnc_read_conf(node, 0, 0, 0, 0);

		if ((vendev == VENDEV_SR5690) || (vendev == VENDEV_SR5670) || (vendev == VENDEV_SR5650)) {
			/* Limit TOM2 to HyperTransport address */
			uint64_t limit = min(ht_base, (uint64_t)dnc_top_of_mem << DRAM_MAP_SHIFT);
			ioh_htiu_write(node, SR56X0_HTIU_TOM2LO, (limit & 0xff800000) | 1);
			ioh_htiu_write(node, SR56X0_HTIU_TOM2HI, limit >> 32);

			if (dnc_top_of_mem >= (1 << (40 - DRAM_MAP_SHIFT)))
				ioh_nbmiscind_write(node, SR56X0_MISC_TOM3, ((dnc_top_of_mem << (DRAM_MAP_SHIFT - 22)) - 1) | (1 << 31));
			else
				ioh_nbmiscind_write(node, SR56X0_MISC_TOM3, 0);
		} else if ((vendev == VENDEV_MCP55)) {
			val = dnc_read_conf(node, 0, 0, 0, 0x90);
			/* Disable mmcfg setting in bridge to avoid OS confusion */
			dnc_write_conf(node, 0, 0, 0, 0x90, val & ~(1 << 31));
		}
	}

	printf("done\n");
}

#ifdef BROKEN
static void setup_c1e_osvw(void)
{
	uint64_t msr;
	/* Disable C1E in MSRs */
	msr = rdmsr(MSR_HWCR) & ~(1 << 12);
	wrmsr(MSR_HWCR, msr);
	*REL64(new_hwcr_msr) = msr;
	msr = 0;
	wrmsr(MSR_INT_HALT, msr);
	*REL64(new_int_halt_msr) = msr;
	/* Disable OS Vendor Workaround bit for errata #400, as C1E is disabled */
	msr = rdmsr(MSR_OSVW_ID_LEN);

	if (msr < 2) {
		/* Extend ID length to cover errata 400 status bit */
		wrmsr(MSR_OSVW_ID_LEN, 2);
		*REL64(new_osvw_id_len_msr) = 2;
		msr = rdmsr(MSR_OSVW_STATUS) & ~2;
		wrmsr(MSR_OSVW_STATUS, msr);
		*REL64(new_osvw_status_msr) = msr;
		printf("Enabled OSVW errata #400 workaround status, as C1E disabled\n");
	} else {
		*REL64(new_osvw_id_len_msr) = msr;
		msr = rdmsr(MSR_OSVW_STATUS);

		if (msr & 2) {
			msr &= ~2;
			wrmsr(MSR_OSVW_STATUS, msr);
			printf("Cleared OSVW errata #400 bit status, as C1E disabled\n");
		}

		*REL64(new_osvw_status_msr) = msr;
	}
}
#endif

static void setup_mc4_thresholds(void)
{
	uint64_t msr;

	/* Ensure MCEs aren't redirected into SMIs */
	wrmsr(MSR_MCE_REDIR, 0);

	/* Set McStatusWrEn in HWCR first or we might get a GPF */
	msr = rdmsr(MSR_HWCR);
	msr |= (1ULL << 18);
	wrmsr(MSR_HWCR, msr);
	*REL64(new_hwcr_msr) = msr & ~(1ULL << 17); /* Don't let the Wrap32Dis bit follow through to other cores */

	msr = rdmsr(MSR_MC4_MISC0);
	if (msr != MSR_VECTOR_MASKED) {
		printf("- disabling %slocked DRAM thresholding SMI (was 0x%llx)\n", (msr >> 61) ? "" : "un", msr);
		msr = MSR_VECTOR_MASKED;
		wrmsr(MSR_MC4_MISC0, msr);
	}
	*REL64(new_mc4_misc0_msr) = msr;

	msr = rdmsr(MSR_MC4_MISC1);
	if (msr != MSR_VECTOR_MASKED) {
		printf("- disabling %slocked link thresholding SMI (was 0x%llx)\n", (msr >> 61) ? "" : "un", msr);
		msr = MSR_VECTOR_MASKED;
		wrmsr(MSR_MC4_MISC1, msr);
	}
	*REL64(new_mc4_misc1_msr) = msr;

	msr = rdmsr(MSR_MC4_MISC2);
	if (msr != MSR_VECTOR_MASKED) {
		printf("- disabling %slocked L3 cache thresholding SMI (was 0x%llx)\n", (msr >> 61) ? "" : "un", msr);
		msr = MSR_VECTOR_MASKED;
		wrmsr(MSR_MC4_MISC2, msr);
	}
	*REL64(new_mc4_misc2_msr) = msr;
}

static void enable_cstate6(void)
{
	printf("Enabling C-state 6...");

	for (node_info_t *node = &nodes[0]; node < &nodes[dnc_node_count]; node++) {
		for (ht_t ht = node->nb_ht_lo; ht <= node->nb_ht_hi; ht++) {
			/* Action field 0, 1 */
			uint32_t val = dnc_read_conf(node->sci, 0, 24 + ht, FUNC4_LINK, 0x118);
			val |= (1 << 17) | (1 << 1); /* CacheFlushEn */
			val = (val & ~((3 << 2) | (3 << 18))) | (1 << 2) | (1 << 18); /* CacheFlushTmrSel */
			val |= (1 << 8) | (1 << 24); /* PwrGateEn */
			dnc_write_conf(node->sci, 0, 24 + ht, FUNC4_LINK, 0x118, val); //0x0107000b);

			/* Action field 2 */
			val = dnc_read_conf(node->sci, 0, 24 + ht, FUNC4_LINK, 0x11c);
			val |= (1 << 8); /* CacheFlushEn */
			val = (val & ~(3 << 2)) | (1 << 2); /* CacheFlushTmrSel */
			val |= (1 << 8); /* PwrGateEn */
			dnc_write_conf(node->sci, 0, 24 + ht, FUNC4_LINK, 0x11c, val); //00000000);

			/* Set StateSaveDestNode to itself; clear CoreCstateMode */
			val = dnc_read_conf(node->sci, 0, 24 + ht, FUNC4_LINK, 0x128);
			val = (val & ~0x3f000) | (ht << 12);
			val &= ~1;
			dnc_write_conf(node->sci, 0, 24 + ht, FUNC4_LINK, 0x128, val);

			/* Set CC6SaveEn, LockDramCfg */
			val = dnc_read_conf(node->sci, 0, 24 + ht, FUNC2_DRAM, 0x118);
			dnc_write_conf(node->sci, 0, 24 + ht, FUNC2_DRAM, 0x118, val | (3 << 18));
		}
	}

	printf("done\n");
}

static void calibrate_nb_tscs(void)
{
	uint32_t t1, t2, adjustment[8];

	printf("Calibrating nortbridge timestamp counters:\n");

	/* Calculate round-trip delta */
	printf("- roundtrip in clocks:");
	for (ht_t ht = nodes[0].nb_ht_lo; ht <= nodes[0].nb_ht_hi; ht++) {
		adjustment[ht] = -1;
		for (int i = 0; i < 100000; i++) {
			t1 = dnc_read_conf(0x000, 0, 0x18 + ht, FUNC2_DRAM, 0xb0);
			t2 = dnc_read_conf(0x000, 0, 0x18 + ht, FUNC2_DRAM, 0xb0);
			if ((t2 - t1) < adjustment[ht])
				adjustment[ht] = t2 - t1;
		}
		printf(" %d", adjustment[ht]);
	}

	printf("\n- uncalibrated offset in clocks:");
	/* Show clock offsets before calibration */
	for (ht_t ht = nodes[0].nb_ht_lo; ht <= nodes[0].nb_ht_hi; ht++) {
		t1 = dnc_read_conf(0x000, 0, 0x18 + 0, FUNC2_DRAM, 0xb0);
		t2 = dnc_read_conf(0x000, 0, 0x18 + ht, FUNC2_DRAM, 0xb0);
		printf(" %d", t2 - t1 - (adjustment[0] + adjustment[ht]) / 2);
	}

	int skip = (rdmsr(MSR_NODE_ID) >> 3) & 7;

	for (ht_t ht = nodes[0].nb_ht_lo + skip + 1; ht <= nodes[0].nb_ht_hi; ht++) {
		/* Set clock value on secondary sockets, adjusting for two half rount-trips */
		t1 = dnc_read_conf(0x000, 0, 0x18 + 0, FUNC2_DRAM, 0xb0);
		t1 += (adjustment[0] + adjustment[ht]) / 2;
		dnc_write_conf(0x000, 0, 0x18 + ht, FUNC2_DRAM, 0xb0, t1);
	}

	printf("\n- calibrated offset in clocks:");
	for (ht_t ht = nodes[0].nb_ht_lo; ht <= nodes[0].nb_ht_hi; ht++) {
		t1 = dnc_read_conf(0x000, 0, 0x18 + 0, FUNC2_DRAM, 0xb0);
		t2 = dnc_read_conf(0x000, 0, 0x18 + ht, FUNC2_DRAM, 0xb0);
		printf(" %d", t2 - t1 - (adjustment[0] + adjustment[ht]) / 2);
	}
	printf("\n");
}

static void unify_all_nodes(void)
{
	uint16_t i;
	uint16_t node;
	bool abort = 0;
	int model, model_first = 0;
	dnc_node_count = 0;
	ht_pdom_count  = 0;
	tally_local_node();

	if (mem_gap) {
		printf("Inserting gap of %lldMB\n", mem_gap >> 20);
		dnc_top_of_mem += mem_gap >> DRAM_MAP_SHIFT;
	}

	nodes[1].ht[0].base = dnc_top_of_mem;

	tally_all_remote_nodes();

	/* Remove last 16MB if CC6 enabled */
	if (pf_cstate6)
		dnc_top_of_mem -= 1;

	for (node = 0; node < dnc_node_count; node++) {
		for (i = nodes[node].nb_ht_lo; i <= nodes[node].nb_ht_hi; i++) {
			/* Ensure all processors are the same for compatibility */
			model = cpu_family(nodes[node].sci, i);

			if (!model_first)
				model_first = model;
			else if (model != model_first) {
				error("SCI%03x (%s) has varying processor models 0x%08x and 0x%08x",
					nodes[node].sci, get_master_name(nodes[node].sci), model_first, model);
				abort = 1;
			}

			/* 6200/4200 processors lack the HT lock mechanism, so abort */
			if ((family >> 8) == 0x1501) {
				error("SCI%03x (%s) has incompatible 6200/4200 processors; please use 6300/4300 or later",
					nodes[node].sci, get_master_name(nodes[node].sci));
				abort = 1;
			}

			if ((model >> 16) >= 0x15) {
				uint32_t val = dnc_read_conf(nodes[node].sci, 0, 24 + i, FUNC2_DRAM, 0x118);

				if (val & (1 << 19)) {
					error("SCI%03x (%s) has CState C6 enabled in the BIOS",
					       nodes[node].sci, get_master_name(nodes[node].sci));
					abort = 1;
				}
			}
		}
	}

	if (abort)
		fatal("Processor/BIOS configuration issues need resolving");

	/* Set up local mapping registers etc from 0 - master node max */
	for (i = 0; i < nodes[0].nc_ht; i++) {
		uint64_t base = (uint64_t)nodes[0].ht[i].base << DRAM_MAP_SHIFT;
		uint64_t limit = ((uint64_t)(nodes[0].ht[i].base + nodes[0].ht[i].size) << DRAM_MAP_SHIFT) - 1;

		nc_dram_range(0xfff0, i, base, limit, i);
	}

	/* DRAM map on local CPUs to redirect all accesses outside our local range to NC
	 * NB: Assuming that memory is assigned sequentially to SCI nodes */
	for (i = 0; i < nodes[0].nc_ht; i++) {
		int range = dram_range_unused(0xfff0, i);

		/* Don't add if the second node's base is the not above the first's, since it'll be a 1-node partition */
		if (nodes[1].dram_base > nodes[0].dram_base)
			dram_range(0xfff0, i, range, (uint64_t)nodes[1].dram_base << DRAM_MAP_SHIFT,
				((uint64_t)dnc_top_of_mem << DRAM_MAP_SHIFT) - 1, nodes[0].nc_ht);
	}

	dnc_write_csr(0xfff0, H2S_CSR_G0_MIU_NGCM0_LIMIT, nodes[0].dram_base >> 6);
	dnc_write_csr(0xfff0, H2S_CSR_G0_MIU_NGCM1_LIMIT, (nodes[0].dram_limit >> 6) - 1);
	dnc_write_csr(0xfff0, H2S_CSR_G3_DRAM_SHARED_BASE, nodes[0].dram_base);
	dnc_write_csr(0xfff0, H2S_CSR_G3_DRAM_SHARED_LIMIT, nodes[0].dram_limit);

	for (i = 0; i < dnc_node_count; i++) {
		uint16_t dnode;
		uint32_t addr = 0;
		node = (i == 0) ? 0xfff0 : nodes[i].sci;

		for (dnode = 0; dnode < dnc_node_count; dnode++) {
			addr = nodes[dnode].dram_base;
			uint32_t end  = nodes[dnode].dram_limit;

			dnc_write_csr(node, H2S_CSR_G0_ATT_INDEX, (1 << 31) |
			  (1 << (27 + SCC_ATT_INDEX_RANGE)) | (addr / SCC_ATT_GRAN));

			while (addr < end) {
				dnc_write_csr(node, H2S_CSR_G0_ATT_ENTRY, nodes[dnode].sci);
				addr += SCC_ATT_GRAN;
			}
		}

		/* Point the rest at the master */
		while (addr < (4096 * SCC_ATT_GRAN)) {
			dnc_write_csr(node, H2S_CSR_G0_ATT_ENTRY, 0x000);
			addr += SCC_ATT_GRAN;
		}
	}

	load_scc_microcode();
	scc_started = 1;
	update_mtrr();
	/* Set TOPMEM2 for ourselves and other cores */
	wrmsr(MSR_TOPMEM2, (uint64_t)dnc_top_of_mem << DRAM_MAP_SHIFT);
	*REL64(new_topmem2_msr) = (uint64_t)dnc_top_of_mem << DRAM_MAP_SHIFT;
	/* Harmonize TOPMEM */
	*REL64(new_topmem_msr) = rdmsr(MSR_TOPMEM);

#ifdef BROKEN
	/* Update OS visible workaround MSRs */
	if (disable_c1e)
		setup_c1e_osvw();
#endif

	/* Make sure MC4 threshold regs are appropriate */
	setup_mc4_thresholds();

	/* If SMM is going to be disabled, do handover now */
	if (disable_smm)
		handover_legacy();

	update_acpi_tables();
	update_mptable();

	if (verbose > 0)
		debug_acpi();

	setup_apic_atts();

	/* Decode remote coherent access to MMIO ranges to the SRI */
	uint64_t tom = rdmsr(MSR_TOPMEM);
	uint8_t ioh_ht = (cht_read_conf(0, FUNC0_HT, 0x60) >> 8) & 7;
	nc_mmio_range(0xfff0, 0, MMIO_VGA_BASE, MMIO_VGA_LIMIT, ioh_ht);
	nc_mmio_range(0xfff0, 1, tom, 0xffffffff, ioh_ht);

	old_mcfg = rdmsr(MSR_MCFG_BASE) & ~0x3f;
	printf("Old MCFG space 0x%08llx\n", old_mcfg);

	*REL64(new_mcfg_msr) = DNC_MCFG_BASE | ((uint64_t)nodes[0].sci << 28ULL) | 0x21ULL;
	wrmsr(MSR_MCFG_BASE, *REL64(new_mcfg_msr));

	/* Drop redundant MMIO ranges pointing to old MCFG space */
	for (int range = 0; range < 8; range++) {
		uint64_t base, limit;
		int dest, link;
		bool lock;

		if (!mmio_range_read(0xfff0, i, range, &base, &limit, &dest, &link, &lock))
			continue;

		if (base == old_mcfg) {
			mmio_range_del(0xfff0, i, range);
			break;
		}
	}

	/* Make chipset-specific adjustments */
	global_chipset_fixup();

	/* Must run after SCI is operational */
	if (disable_smm) {
		printf("BSP SMM:");
		disable_smm_handler(rdmsr(MSR_SMM_BASE));
		printf("\n");
	}

	setup_other_cores();

	printf("Joining:");
	for (i = 1; i < dnc_node_count; i++)
		setup_remote_cores(&nodes[i]);
	printf("\n");

	if (remote_io) {
		setup_mmio();
		setup_mmio_late();
	}

	update_acpi_tables_late();

	printf("Clearing remote memory...");
	for (node = 1; node < dnc_node_count; node++) {
		uint16_t sci = nodes[node].sci;

		for (int ht = nodes[node].nb_ht_lo; ht <= nodes[node].nb_ht_hi; ht++) {
			uint32_t val;

			/* Disable memory controller prefetch */
			val = dnc_read_conf(sci, 0, 24 + ht, FUNC2_DRAM, 0x11c);
			dnc_write_conf(sci, 0, 24 + ht, FUNC2_DRAM, 0x11c, val | (3 << 12));

			/* Start memory clearing */
			val = dnc_read_conf(sci, 0, 24 + ht, FUNC2_DRAM, 0x110);
			dnc_write_conf(sci, 0, 24 + ht, FUNC2_DRAM, 0x110, val | (1 << 3));
		}
	}

	/* Wait for clear to finish */
	for (node = 1; node < dnc_node_count; node++) {
		uint16_t sci = nodes[node].sci;

		for (int ht = nodes[node].nb_ht_lo; ht <= nodes[node].nb_ht_hi; ht++) {
			uint32_t val;

			while (1) {
				val = dnc_read_conf(sci, 0, 24 + ht, FUNC2_DRAM, 0x110);
				if (!(val & (1 << 9)))
					break;
				udelay(100);
			}

			/* Reenable memory controller prefetch */
			val = dnc_read_conf(sci, 0, 24 + ht, FUNC2_DRAM, 0x11c);
			dnc_write_conf(sci, 0, 24 + ht, FUNC2_DRAM, 0x11c, val & ~(3 << 12));
		}
	}
	printf("done\n");

	/* Re-enable DRAM scrubbers with our new memory map, required by fam15h BKDG; see D18F2xC0 b0 */
	if (!trace_buf_size) {
		printf("Enabling DRAM scrubbers...");

		for (node = 0; node < dnc_node_count; node++) {
			uint16_t sci = (node == 0) ? 0xfff0 : nodes[node].sci;

			for (i = nodes[node].nb_ht_lo; i <= nodes[node].nb_ht_hi; i++) {
				if (!(nodes[node].ht[i].scrub & 0x1f))
					continue;

				uint64_t base = (uint64_t)nodes[node].ht[i].base << DRAM_MAP_SHIFT;
				uint32_t redir = dnc_read_conf(sci, 0, 24 + i, FUNC3_MISC, 0x5c) & 1;
				dnc_write_conf(sci, 0, 24 + i, FUNC3_MISC, 0x5c, base | redir);
				dnc_write_conf(sci, 0, 24 + i, FUNC3_MISC, 0x60, base >> 32);

				/* Fam15h: Accesses to this register must first set F1x10C [DctCfgSel]=0;
				   Accesses to this register with F1x10C [DctCfgSel]=1 are undefined;
				   See erratum 505 */
				if (family >= 0x15)
					dnc_write_conf(sci, 0, 24 + i, FUNC1_MAPS, 0x10c, 0);

				dnc_write_conf(sci, 0, 24 + i, FUNC3_MISC, 0x58, nodes[node].ht[i].scrub);
			}
		}

		printf("done\n");
	}

	if (pf_cstate6)
		enable_cstate6();

	calibrate_nb_tscs();
}

static void start_user_os(void)
{
	static com32sys_t rm;
	/* Restore 32-bit only access */
	set_wrap32_enable();

	char *param = (char *)lmalloc(sizeof next_label + 1);
	assert(param);
	strcpy(param, next_label);
	rm.eax.w[0] = 0x0003;
	rm.ebx.w[0] = OFFS(param);
	rm.es = SEG(param);
	printf(BANNER "Unification succeeded at 20%02d-%02d-%02d %02d:%02d:%02d; loading %s..." COL_DEFAULT "\n",
		rtc_read(RTC_YEAR), rtc_read(RTC_MONTH), rtc_read(RTC_DAY),
		rtc_read(RTC_HOURS), rtc_read(RTC_MINUTES), rtc_read(RTC_SECONDS), next_label);

	if (boot_wait)
		wait_key();

	__intcall(0x22, &rm, NULL);
	fatal("Failed to boot");
}

static void cleanup_stack(void)
{
	static com32sys_t rm;
	rm.eax.w[0] = 0x000C;
	rm.edx.w[0] = 0x0000;
	printf("Unloading bootloader stack...");
	__intcall(0x22, &rm, NULL);
	printf("done\n");
}

static void get_hostname(void)
{
	int sts;
	char *dhcpdata;
	size_t dhcplen;

	if ((sts = pxe_get_cached_info(PXENV_PACKET_TYPE_DHCP_ACK, (void **)&dhcpdata, &dhcplen)) != 0) {
		printf("pxe_get_cached_info() returned status %d\n", sts);
		return;
	}

	/* Save MyIP for later (in udp_open) */
	myip.s_addr = ((pxe_bootp_t *)dhcpdata)->yip;

	/* Skip standard fields, as hostname is an option */
	unsigned int offset = 4 + offsetof(pxe_bootp_t, vendor.d);

	while (offset < dhcplen) {
		int code = dhcpdata[offset];
		int len = dhcpdata[offset + 1];

		/* Sanity-check length */
		if (len == 0)
			break;

		/* Skip non-hostname options */
		if (code != 12) {
			offset += 2 + len;
			continue;
		}

		/* Sanity-check length */
		if ((offset + len) > dhcplen)
			break;

		/* Create a private copy */
		hostname = strndup(&dhcpdata[offset + 2], len);
		assert(hostname);
		printf("Hostname %s, IP address %s\n", hostname, inet_ntoa(myip));
		return;
	}

	printf("IP address %s\n", inet_ntoa(myip));
	hostname = NULL;
}

#define ERR_API_VERSION            -1
#define ERR_MASTER_HT_ID           -2
#define ERR_NODE_CONFIG            -3
#define ERR_PARTITION_CONFIG       -4
#define ERR_SETUP_FABRIC           -5
#define ERR_INIT_CACHES            -6
#define ERR_INSTALL_E820_HANDLER   -7
#define ERR_UNIFY_ALL_NODES        -8
#define ERR_GENERAL_NC_START_ERROR -9

static void constants(void)
{
	family = cpu_family(0xfff0, 0) >> 16;

	if (family >= 0x15) {
		uint32_t val = cht_read_conf(0, FUNC5_EXTD, 0x160);
		tsc_mhz = 200 * (((val >> 1) & 0x1f) + 4) / (1 + ((val >> 7) & 1));
	} else {
		uint32_t val = cht_read_conf(0, FUNC3_MISC, 0xd4);
		uint64_t val6 = rdmsr(0xc0010071);
		tsc_mhz = 200 * ((val & 0x1f) + 4) / (1 + ((val6 >> 22) & 1));

		/* C-state 6 not supported before Fam15h */
		pf_cstate6 = 0;
	}

	printf("Family %xh with NB/TSC frequency %dMHz\n", family, tsc_mhz);
}

#define STEP_MIN (64)
#define STEP_MAX (4 << 20)

static void selftest_late_memmap(void)
{
	struct e820entry *e820 = (struct e820entry *)REL32(new_e820_map);
	uint16_t *len = REL16(new_e820_len);

	for (int i = 0; i < *len; i++) {
		/* Test only regions marked usable */
		if (e820[i].type != 1)
			continue;

		uint64_t start = e820[i].base;
		uint64_t end = e820[i].base + e820[i].length;
		printf("Testing memory permissions from %011llx to %011llx...", start, end - 1);

		uint64_t pos = start;
		uint64_t mid = start + (end - start) / 2;
		uint64_t step = STEP_MIN;

		while (pos < mid) {
			mem64_write32(pos, mem64_read32(pos));
			pos = (pos + step) & ~3;
			step = min(step << 1, STEP_MAX);
		}

		while (pos < end) {
			mem64_write32(pos, mem64_read32(pos));
			step = min((end - pos) / 2, STEP_MAX);
			pos += max(step, STEP_MIN) & ~3;
		}

		printf("done\n");
	}
}

/* Assumes slaves are configured with the same BIOS */
static void check_renumbering(void)
{
	if (renumber_bsp != -1)
		return;

	foreach_nb(ht, nodes) {
		for (int i = 0; i < 8; i++) {
			uint32_t val = dnc_read_conf(nodes->sci, 0, 24 + ht, FUNC1_MAPS, 0x80 + i * 8);
			if (val & (1 << 3)) {
				renumber_bsp = 1;
				goto out;
			}
		}

		if (family < 0x15)
			continue;

		for (int i = 0; i < 4; i++) {
			uint32_t val = dnc_read_conf(nodes->sci, 0, 24 + ht, FUNC1_MAPS, 0x1a0 + i * 8);
			if (val & (1 << 3)) {
				renumber_bsp = 1;
				goto out;
			}
		}
	}
out:
	printf("Renumbering flag %d\n", renumber_bsp);
}

static int nc_start(void)
{
	struct part_info *part;

	/* Set local info for early error reporting */
	local_info = (node_info *)malloc(sizeof *local_info);
	assert(local_info);
	memset(local_info, 0xff, sizeof *local_info);

	nodes = (node_info_t *)zalloc(sizeof *nodes);
	assert(nodes);

	get_hostname();

	int rc = dnc_init_bootloader(&dnc_chip_rev, dnc_card_type, &dnc_asic_mode);
	if (rc == -2)
		start_user_os();

	nodes[0].nc_ht = rc;

	if (singleton) {
		make_singleton_config();
	} else {
		if (read_config_file(config_file_name) < 0)
			return ERR_NODE_CONFIG;
	}

	nodes = (node_info_t *)realloc(nodes, sizeof(*nodes) * cfg_nodes);
	assert(nodes);
	memset(&nodes[1], 0, sizeof(*nodes) * (cfg_nodes - 1));

	get_node_config();
	nodes[0].sci = local_info->sci;

	if (name_matching)
		printf("Node: <%s> sciid: 0x%03x, partition: %d, osc: %d, sync_only: %d\n",
		       local_info->desc, local_info->sci, local_info->partition, local_info->osc, local_info->sync_only);
	else
		printf("Node: <%s> uuid: %08X, sciid: 0x%03x, partition: %d, osc: %d, sync_only: %d\n",
		       local_info->desc, local_info->uuid, local_info->sci, local_info->partition, local_info->osc, local_info->sync_only);

	part = get_partition_config(local_info->partition);
	if (!part)
		return ERR_PARTITION_CONFIG;

	printf("Partition master: 0x%03x; builder: 0x%03x\n", part->master, part->builder);
	printf("Fabric dimensions: x: %d, y: %x, z: %d\n",
	       cfg_fabric.x_size, cfg_fabric.y_size, cfg_fabric.z_size);

	for (int i = 0; i < cfg_nodes; i++) {
		if (config_local(&cfg_nodelist[i], local_info->uuid))
			continue;

		if (name_matching)
			printf("Remote node: <%s> sciid: 0x%03x, partition: %d, osc: %d\n",
			       cfg_nodelist[i].desc,
			       cfg_nodelist[i].sci,
			       cfg_nodelist[i].partition,
			       cfg_nodelist[i].osc);
		else
			printf("Remote node: <%s> uuid: %08X, sciid: 0x%03x, partition: %d, osc: %d\n",
			       cfg_nodelist[i].desc,
			       cfg_nodelist[i].uuid,
			       cfg_nodelist[i].sci,
			       cfg_nodelist[i].partition,
			       cfg_nodelist[i].osc);
	}

	adjust_oscillator(dnc_card_type, local_info->osc);

	/* Copy this into NC ram so its available remotely */
	load_existing_apic_map();

	if (part->builder == local_info->sci)
		wait_for_slaves(local_info, part);
	else
		wait_for_master(local_info, part);

	/* Must run after SCI is operational */
	local_chipset_fixup(part->master == local_info->sci || local_info->sync_only);

	if (verbose > 0)
		debug_acpi();

	load_orig_e820_map();
	check_renumbering();
	update_acpi_tables_early();

	if (local_info->sync_only) {
		/* Release resources to reduce allocator fragmentation */
		free(cfg_nodelist);
		free(cfg_partlist);
		lfree(orig_e820_map);
		start_user_os();
	}

	dnc_init_caches();

	if (!install_e820_handler())
		return ERR_INSTALL_E820_HANDLER;

	if (force_probefilteron && !force_probefilteroff) {
		enable_probefilter(nodes[0].nc_ht - 1);
		probefilter_tokens(nodes[0].nc_ht - 1);
	}

	if (part->master == local_info->sci) {
		/* Master */
		for (int i = 0; i < cfg_nodes; i++) {
			if (config_local(&cfg_nodelist[i], local_info->uuid))
				continue;

			if (cfg_nodelist[i].partition != local_info->partition)
				continue;

			nodedata[cfg_nodelist[i].sci] = 0x80;
		}

#ifdef UNUSED
		read_microcode_update();
#endif
		unify_all_nodes();

		dnc_check_mctr_status(0);
		dnc_check_mctr_status(1);

		update_e820_map();

		/* Release resources to reduce allocator fragmentation */
		free(cfg_nodelist);
		free(cfg_partlist);
		lfree(orig_e820_map);

		if (disable_kvm > -1)
			disable_kvm_ports(disable_kvm);

#if BROKEN	/* Disabling APIC LVT entries causes boot failure on eg Dell R815s */
		selftest_late_apiclvt();
#endif
		if (verbose) {
			ranges_print();
			selftest_late_memmap();

			/* Do this ahead of the self-test to prevent false positives */
			/* Restore 32-bit only access */
			set_wrap32_enable();
		}

		start_user_os();
	} else {
		/* Slave */
		uint32_t val;

		/* On non-root servers, prevent writing to unexpected locations */
		cleanup_stack();
		handover_legacy();

		/* Set G3x02c FAB_CONTROL bit 30 */
		dnc_write_csr(0xfff0, H2S_CSR_G3_FAB_CONTROL, 1 << 30);
		printf("Numascale NumaChip awaiting fabric set-up by master node...");

		do {
			if (((dnc_check_mctr_status(0) & 0xfbc) != 0) ||
			    ((dnc_check_mctr_status(1) & 0xfbc) != 0) ||
			    (dnc_check_fabric(local_info)))
				fatal("Late memory controller or fabric issues detected");
			udelay(1000);
			val = dnc_read_csr(0xfff0, H2S_CSR_G3_FAB_CONTROL);
		} while (!(val & (1 << 31)));

		printf(BANNER "\n\nThis server '%s' is part of a %d-server NumaConnect system; refer to the console on server '%s'\n",
		       local_info->desc, cfg_nodes, get_master_name(part->master));

		disable_smi();
		if (!remote_io)
			disable_dma_all(); /* Disables VGA refresh and other DMA traffic */
		if (disable_kvm > -1)
			disable_kvm_ports(disable_kvm);
		clear_bsp_flag();
		disable_xtpic();
		disable_cache();

		/* Let master know we're ready for remapping/integration */
		dnc_write_csr(0xfff0, H2S_CSR_G3_FAB_CONTROL, val & ~(1 << 31));

		/* Restore 32-bit only access */
		set_wrap32_enable();

		while (1) {
			cli();
			asm volatile("hlt" ::: "memory");
		}
	}

	return ERR_GENERAL_NC_START_ERROR;
}

int main(const int argc, const char *argv[])
{
	int ret;
	openconsole(&dev_rawcon_r, &dev_stdcon_w);

	printf(CLEAR BANNER "NumaConnect system unification module " VER " at 20%02d-%02d-%02d %02d:%02d:%02d" COL_DEFAULT "\n",
		rtc_read(RTC_YEAR), rtc_read(RTC_MONTH), rtc_read(RTC_DAY),
		rtc_read(RTC_HOURS), rtc_read(RTC_MINUTES), rtc_read(RTC_SECONDS));

	/* Enable CF8 extended access for first Northbridge; we do others for Linux later */
	set_cf8extcfg_enable(0);

	/* Disable 32-bit address wrapping to allow 64-bit access in 32-bit code */
	set_wrap32_disable();

	constants();
	parse_cmdline(argc, argv);

	ret = nc_start();
	if (ret < 0) {
		error("nc_start() failed with error code %d; check configuration files match hardware and UUIDs\n", ret);
		wait_key();
	}

	/* Restore 32-bit only access */
	set_wrap32_enable();
	return ret;
}
