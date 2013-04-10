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
#include <syslinux/pxe.h>
#include <sys/io.h>

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
#include "dnc-version.h"

#define PIC_MASTER_CMD          0x20
#define PIC_MASTER_IMR          0x21
#define PIC_SLAVE_CMD           0xa0
#define PIC_SLAVE_IMR           0xa1

#define MTRR_TYPE(x) (x) == 0 ? "uncacheable" : (x) == 1 ? "write-combining" : (x) == 4 ? "write-through" : (x) == 5 ? "write-protect" : (x) == 6 ? "write-back" : "unknown"

#define TABLE_AREA_SIZE		256*1024
#define MIN_NODE_MEM		256*1024*1024

int dnc_master_ht_id;     /* HT id of NC on master node, equivalent nc_node[0].nc_ht_id */
bool dnc_asic_mode;
uint32_t dnc_chip_rev;
char dnc_card_type[16];
uint16_t dnc_node_count = 0;
nc_node_info_t nc_node[128];
uint16_t ht_pdom_count = 0;
uint16_t apic_per_node;
uint16_t ht_next_apic;
uint32_t dnc_top_of_dram;      /* Top of DRAM, before MMIO, in 16MB chunks */
uint32_t dnc_top_of_mem;       /* Top of MMIO, in 16MB chunks */
uint8_t post_apic_mapping[256]; /* POST APIC assigments */
static bool scc_started = 0;
static struct in_addr myip = {0xffffffff};
uint64_t ht_base = HT_BASE;

/* Traversal info per node.  Bit 7: seen, bits 5:0 rings walked */
uint8_t nodedata[4096];

char *asm_relocated;
static char *tables_relocated;
void *tables_next;

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

extern uint8_t smm_handler_start;
extern uint8_t smm_handler_end;

static struct e820entry *orig_e820_map;
static int orig_e820_len;

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
	static com32sys_t rm;
	unsigned int e820_len;
	struct e820entry *e820_map = lmalloc(E820_MAX_LEN);
	assert(e820_map);
	memset(e820_map, 0, E820_MAX_LEN);
	rm.eax.l = 0x0000e820;
	rm.edx.l = STR_DW_N("SMAP");
	rm.ebx.l = 0;
	rm.ecx.l = sizeof(struct e820entry);
	rm.edi.w[0] = OFFS(e820_map);
	rm.es = SEG(e820_map);
	__intcall(0x15, &rm, &rm);

	assert(rm.eax.l == STR_DW_N("SMAP"));
	printf("--- E820 memory map\n");
	e820_len = rm.ecx.l;

	while (rm.ebx.l > 0) {
		rm.eax.l = 0x0000e820;
		rm.edx.l = STR_DW_N("SMAP");
		rm.ecx.l = sizeof(struct e820entry);
		rm.edi.w[0] = OFFS(e820_map) + e820_len;
		rm.es = SEG(e820_map);
		__intcall(0x15, &rm, &rm);
		e820_len += rm.ecx.l ? rm.ecx.l : sizeof(struct e820entry);
		assert(e820_len < E820_MAX_LEN);
	}

	int i, j, len;
	struct e820entry elem;
	len = e820_len / sizeof(elem);

	/* Sort e820 map entries */
	for (j = 1; j < len; j++) {
		memcpy(&elem, &e820_map[j], sizeof(elem));

		for (i = j - 1; (i >= 0) && (e820_map[i].base > elem.base); i--)
			memcpy(&e820_map[i + 1], &e820_map[i], sizeof(elem));

		memcpy(&e820_map[i + 1], &elem, sizeof(elem));
	}

	for (i = 0; i < len; i++) {
		printf(" %016llx - %016llx (%016llx) [%x]\n",
		       e820_map[i].base, e820_map[i].base + e820_map[i].length,
		       e820_map[i].length, e820_map[i].type);
	}

	orig_e820_map = e820_map;
	orig_e820_len = e820_len;
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
	asm_relocated = (void *)((tom_lower - relocate_size) & ~0xfff);
	/* http://groups.google.com/group/comp.lang.asm.x86/msg/9b848f2359f78cdf
	 * *bda_tom_lower = ((uint32_t)asm_relocated) >> 10; */
	memcpy(asm_relocated, &asm_relocate_start, relocate_size);
	e820 = (void *)REL32(new_e820_map);
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
		    (orig_e820_map[i].length > TABLE_AREA_SIZE) &&
		    (orig_e820_map[i].type == 1))
			last_32b = j - 1;
	}

	free(orig_e820_map);
	*REL16(new_e820_len)  = j;
	*REL32(old_int15_vec) = int_vecs[0x15];

	if (last_32b < 0) {
		error("Unable to allocate room for ACPI tables");
		return 0;
    }

	e820[last_32b].length -= TABLE_AREA_SIZE;
	tables_relocated = (void *)(long)e820[last_32b].base + (long)e820[last_32b].length;
	tables_next = tables_relocated;
	int_vecs[0x15] = (((uint32_t)asm_relocated) << 12) |
	                 ((uint32_t)(&new_e820_handler_relocate - &asm_relocate_start));
	printf("Persistent code relocated to %p\n", asm_relocated);
	printf("Allocating ACPI tables at %p\n", tables_relocated);
	return 1;
}

static void update_e820_map(void)
{
	uint64_t prev_end;
	unsigned int i, j, max;
	struct e820entry *e820;
	uint16_t *len;
	e820 = (void *)REL32(new_e820_map);
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
	e820[max].length = ((uint64_t)nc_node[0].ht[0].size << DRAM_MAP_SHIFT) - e820[max].base;
	prev_end = e820[max].base + e820[max].length;

	if ((trace_buf_size > 0) && (e820[max].length > trace_buf_size)) {
		e820[max].length -= trace_buf_size;
		trace_buf = e820[max].base + e820[max].length;
		printf("SCI%03x#%x tracebuffer reserved @ 0x%llx - 0x%llx\n",
		       nc_node[0].sci_id, 0, trace_buf, trace_buf + trace_buf_size);
	}

	/* Add remote nodes */
	for (i = 0; i < dnc_node_count; i++) {
		for (j = 0; j < 8; j++) {
			uint64_t base, length;

			if (!nc_node[i].ht[j].cpuid)
				continue;

			if ((i == 0) && (j == 0))
				continue; /* Skip BSP */

			base   = ((uint64_t)nc_node[i].ht[j].base << DRAM_MAP_SHIFT);
			length = ((uint64_t)nc_node[i].ht[j].size << DRAM_MAP_SHIFT);

			if (mem_offline && (i > 0)) {
				if (length > MIN_NODE_MEM)
					length = MIN_NODE_MEM;
			} else {
				if ((trace_buf_size > 0) && (length > trace_buf_size)) {
					length -= trace_buf_size;
					printf("SCI%03x#%x tracebuffer reserved @ 0x%llx - 0x%llx\n",
					       nc_node[i].sci_id, j, base + length, base + length + trace_buf_size);
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

	/* Reserve MCFG address range so Linux accepts it */
	e820[*len].base   = DNC_MCFG_BASE;
	e820[*len].length = DNC_MCFG_LIM - DNC_MCFG_BASE + 1;
	e820[*len].type   = 2;
	(*len)++;

	assert((len - REL16(new_e820_len)) < E820_MAX_LEN);
	printf("Updated E820 map:\n");

	for (i = 0; i < *len; i++) {
		printf(" %016llx - %016llx (%016llx) [%x]\n",
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
			    (struct acpi_core_affinity *) & (srat->data[i]);
			post_apic_mapping[c] = af->apic_id;
			apic_used[af->apic_id >> 4] =
			    apic_used[af->apic_id >> 4] | (1 << (af->apic_id & 0xf));
			c++;
			i += af->len;
		} else if (srat->data[i] == 1) {
			struct acpi_mem_affinity *af =
			    (struct acpi_mem_affinity *) & (srat->data[i]);
			i += af->len;
		} else {
			break;
		}
	}

	/* Use APIC ATT map as scratch area to communicate APIC maps to master */
	dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00000020); /* Select APIC ATT */

	for (i = 0; i < 16; i++)
		dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4, apic_used[i]);
}

static int dist_fn(int src_node, int src_ht, int dst_node, int dst_ht)
{
	if (src_node != dst_node)
		return 120;

	if (src_ht != dst_ht)
		return 16;

	return 10;
}

static void disable_fixed_mtrrs(void)
{
	disable_cache();
	wrmsr(MSR_MTRR_DEFAULT, rdmsr(MSR_MTRR_DEFAULT) & ~(1 << 10));
	enable_cache();
}

static void enable_fixed_mtrrs(void)
{
	disable_cache();
	wrmsr(MSR_MTRR_DEFAULT, rdmsr(MSR_MTRR_DEFAULT) | (1 << 10));
	enable_cache();
}

static void tables_add(const size_t len)
{
	tables_next += roundup(len, TABLE_ALIGNMENT);
	assert((char *)tables_next < (tables_relocated + TABLE_AREA_SIZE));
}

static void update_acpi_tables_early(void)
{
	/* Get table root points and check for space */
	acpi_sdt_p rsdt = find_root("RSDT");
	assert(rsdt);

	acpi_sdt_p xsdt = find_root("XSDT");
	assert(xsdt);

	/* Find e820 entry with ACPI tables */
	struct e820entry *e820 = orig_e820_map;
	while (((uint32_t)rsdt < e820->base) || ((uint32_t)rsdt >= (e820->base + e820->length)))
		e820++;

	printf("Existing ACPI tables in e820 range %016llx - %016llx\n", e820->base, e820->base + e820->length);

	acpi_sdt_p oemn = acpi_build_oemn();
	acpi_sdt_p gap = acpi_gap(e820, oemn->len);
	if (!gap) {
		warning("No space for OEMN table");
		return;
	}

	printf("Adding OEMN at %08x with length %d\n", (uint32_t)gap, oemn->len);
	memcpy((char *)gap, oemn, oemn->len);
	free(oemn);

	/* Fixed MTRRs may mark the RSDT and XSDT pointers r/o */
	disable_fixed_mtrrs();

	add_child(gap, rsdt, 4);
	add_child(gap, xsdt, 8);

	enable_fixed_mtrrs();
}

static void update_acpi_tables(void)
{
	acpi_sdt_p oroot;
	uint8_t *dist;
	unsigned int i, j, apicid, pnum;
	unsigned int node, ht;

	/* Fixed MTRRs may mark the RSDT and XSDT pointers r/o */
	disable_fixed_mtrrs();

	/* replace_root may fail if rptr is r/o, so we read the pointers
	 * back. In case of failure, we'll assume the existing rsdt/xsdt
	 * tables can be extended where they are */
	acpi_sdt_p rsdt = tables_next;
	tables_add(RSDT_MAX);
	acpi_sdt_p xsdt = tables_next;
	tables_add(RSDT_MAX);

	oroot = find_root("RSDT");
	if (oroot) {
		memcpy(rsdt, oroot, oroot->len);
		replace_root("RSDT", rsdt);
		rsdt = find_root("RSDT");
	} else
		rsdt = NULL;

	oroot = find_root("XSDT");
	if (oroot) {
		memcpy(xsdt, oroot, oroot->len);
		replace_root("XSDT", xsdt);
		xsdt = find_root("XSDT");
	} else
		xsdt = NULL;

	if (!rsdt && !xsdt) {
		printf("Neither RSDT or XSDT found!\n");
		return;
	}

	acpi_sdt_p oapic = find_sdt("APIC");
	if (!oapic) {
		printf("Default ACPI APIC table not found\n");
		return;
	}

	/* With APIC we reuse the old info and add our new entries */
	acpi_sdt_p apic = tables_next;
	memcpy(apic, oapic, oapic->len);
	memcpy(apic->oemid, "NUMASC", 6);
	apic->len = offsetof(struct acpi_sdt, data) + 8; /* Count 'Local Interrupt Controller' and 'Flags' fields */

	/* Apply enable mask to existing APICs, find first unused ACPI ProcessorId */
	pnum = 0;
	j = 0;

	for (i = 44; i < oapic->len;) {
		struct acpi_local_apic *lapic = (void *)&oapic->data[i - sizeof(*oapic)];

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
		for (ht = 0; ht < 8; ht++) {
			if (!nc_node[node].ht[ht].cpuid)
				continue;

			for (j = 0; j < nc_node[node].ht[ht].cores; j++) {
				apicid = j + nc_node[node].ht[ht].apic_base + nc_node[node].apic_offset;

				if (ht_next_apic < 0x100) {
					struct acpi_local_apic *lapic = (void *)&apic->data[apic->len - sizeof(*apic)];
					lapic->type = 0;
					lapic->len = 8;
					lapic->proc_id = pnum; /* ACPI Processor ID */
					lapic->apic_id = apicid; /* APIC ID */
					lapic->flags = 1;
					apic->len += lapic->len;
				} else {
					struct acpi_local_x2apic *x2apic = (void *)&apic->data[apic->len - sizeof(*apic)];
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

	apic->checksum -= checksum(apic, apic->len);
	tables_add(apic->len);

	if (rsdt) replace_child("APIC", apic, rsdt, 4);
	if (xsdt) replace_child("APIC", apic, xsdt, 8);

	/* Make SLIT info from scratch (ie replace existing table if any) */
	acpi_sdt_p slit = tables_next;

	memcpy(slit->sig.s, "SLIT", 4);
	slit->revision = ACPI_REV;
	memcpy(slit->oemid, "NUMASC", 6);
	memcpy(slit->oemtableid, "N313NUMA", 8);
	slit->oemrev = 1;
	memcpy(slit->creatorid, "1B47", 4);
	slit->creatorrev = 1;
	memset(slit->data, 0, 8); /* Number of System Localities */
	dist = (void *) & (slit->data[8]);
	int k, l, index = 0;

	for (i = 0; i < dnc_node_count; i++)
		for (j = 0; j < 8; j++) {
			if (!nc_node[i].ht[j].cpuid)
				break;

			for (k = 0; k < dnc_node_count; k++) {
				for (l = 0; l < 8; l++) {
					if (!nc_node[k].ht[l].cpuid)
						break;

					dist[index++] = min(254, dist_fn(i, j, k, l)); /* 255 is unreachable */
				}
			}
		}

	memcpy(slit->data, &ht_pdom_count, sizeof(ht_pdom_count));
	slit->len = 44 + ht_pdom_count * ht_pdom_count;
	slit->checksum -= checksum(slit, slit->len);
	tables_add(slit->len);

	if (rsdt) {
		/* If original bios doesn't have room/entry for SLIT table
		 * see if we can use the BOOT entry instead */
		if (!replace_child("SLIT", slit, rsdt, 4))
			replace_child("BOOT", slit, rsdt, 4);
	}

	if (xsdt) {
		/* If original bios doesn't have room/entry for SLIT table
		 * see if we can use the BOOT entry instead */
		if (!replace_child("SLIT", slit, xsdt, 8))
			replace_child("BOOT", slit, xsdt, 8);
	}

	/* With SRAT we reuse the old info and add our new entries */
	acpi_sdt_p osrat = find_sdt("SRAT");
	if (!osrat) {
		printf("Default ACPI SRAT table not found\n");
		return;
	}

	acpi_sdt_p srat = tables_next;

	memcpy(srat, osrat, osrat->len);
	memcpy(srat->oemid, "NUMASC", 6);
	srat->len = 48;

	for (node = 0; node < dnc_node_count; node++) {
		for (ht = 0; ht < 8; ht++) {
			struct acpi_mem_affinity mem;

			if (!nc_node[node].ht[ht].cpuid)
				continue;

			memset(&mem, 0, sizeof(mem));
			mem.type     = 1;
			mem.len      = sizeof(mem);
			mem.prox_dom = nc_node[node].ht[ht].pdom;
			mem.mem_base = (uint64_t)nc_node[node].ht[ht].base << DRAM_MAP_SHIFT;
			mem.mem_size = (uint64_t)nc_node[node].ht[ht].size << DRAM_MAP_SHIFT;
			mem.enabled  = 1;
			mem.hotplug  = 0;
			mem.nonvol   = 0;
			memcpy((unsigned char *)srat + srat->len, &mem, mem.len);
			srat->len += mem.len;
		}
	}

	for (node = 0; node < dnc_node_count; node++) {
		for (ht = 0; ht < 8; ht++) {
			if (!nc_node[node].ht[ht].cpuid)
				continue;

			for (j = 0; j < nc_node[node].ht[ht].cores; j++) {
				uint16_t apicid = j + nc_node[node].ht[ht].apic_base + nc_node[node].apic_offset;

				if (ht_next_apic < 0x100) {
					struct acpi_core_affinity core;
					memset(&core, 0, sizeof(core));
					core.type     = 0;
					core.len      = sizeof(core);
					core.prox_low = nc_node[node].ht[ht].pdom & 0xff;
					core.apic_id  = apicid;
					core.enabled  = 1;
					core.flags    = 0;
					core.sapic_eid = 0;
					core.prox_hi   = nc_node[node].ht[ht].pdom >> 8;
					memcpy((unsigned char *)srat + srat->len, &core, core.len);
					srat->len += core.len;
				} else {
					struct acpi_x2apic_affinity x2core;
					memset(&x2core, 0, sizeof(x2core));
					x2core.type     = 2;
					x2core.len      = sizeof(x2core);
					x2core.prox_dom = nc_node[node].ht[ht].pdom;
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

	srat->checksum -= checksum(srat, srat->len);
	tables_add(srat->len);

	if (rsdt) replace_child("SRAT", srat, rsdt, 4);
	if (xsdt) replace_child("SRAT", srat, xsdt, 8);

	/* MCFG table */
	acpi_sdt_p mcfg = tables_next;
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
		struct acpi_mcfg_allocation mcfg_allocation;
		memset(&mcfg_allocation, 0, sizeof(mcfg_allocation));
		mcfg_allocation.address = DNC_MCFG_BASE | ((uint64_t)nc_node[node].sci_id << 28ULL);
		mcfg_allocation.pci_segment = node;
		mcfg_allocation.start_bus_number = 0;
		mcfg_allocation.end_bus_number = 255;
		memcpy((unsigned char *)mcfg + mcfg->len, &mcfg_allocation, sizeof(mcfg_allocation));
		mcfg->len += sizeof(mcfg_allocation);
	}

	mcfg->checksum = -checksum(mcfg, mcfg->len);
	tables_add(mcfg->len);

	if (rsdt) replace_child("MCFG", mcfg, rsdt, 4);
	if (xsdt) replace_child("MCFG", mcfg, xsdt, 8);

	if (remote_io) {
		uint32_t extra_len;
		unsigned char *extra = remote_aml(&extra_len);

		if (!acpi_append(rsdt, 4, "SSDT", extra, extra_len))
			if (!acpi_append(rsdt, 4, "DSDT", extra, extra_len)) {
				/* Appending to existing DSDT or SSDT failed; construct new SSDT */
				acpi_sdt_p ssdt = tables_next;
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
				ssdt->checksum = -checksum(ssdt, ssdt->len);
				tables_add(ssdt->len);

				bool failed = 0;

				if (rsdt) failed |= add_child(ssdt, rsdt, 4);
				if (xsdt) failed |= add_child(ssdt, xsdt, 8);

				if (failed) {
					printf("Warning: failed to inject AML; remote I/O will be unavailable\n");
					remote_io = 0;
				}
			}

		free(extra);
	}

	enable_fixed_mtrrs();
}

static struct mp_floating_pointer *find_mptable(void *start, int len) {
	void *ret = NULL;
	int i;

	for (i = 0; i < len; i += 16) {
		if (*(uint32_t *)(start + i) == STR_DW_H("_MP_")) {
			ret = start + i;
			break;
		}
	}

	return ret;
}

static void update_mptable(void)
{
	struct mp_floating_pointer *mpfp;
	struct mp_config_table *mptable;
	unsigned int i;
	void *ebda = (void *)(*((unsigned short *)0x40e) * 16);
	mpfp = find_mptable(ebda, 1024);

	if (!mpfp)
		mpfp = find_mptable((void *)(639 * 1024), 1024);

	if (!mpfp)
		mpfp = find_mptable((void *)(0xf0000), 0x10000);

	printf("mpfp: %p\n", mpfp);

	if (!mpfp)
		return;

	printf("sig: %.4s, len: %d, rev: %d, chksum: %x, feat: %02x:%02x:%02x:%02x:%02x\n",
	       mpfp->sig.s, mpfp->len,  mpfp->revision,  mpfp->checksum,
	       mpfp->feature[0], mpfp->feature[1], mpfp->feature[2],
	       mpfp->feature[3], mpfp->feature[4]);

	if ((uint32_t)mpfp > (uint32_t)REL32(new_mpfp)) {
		memcpy((void *)REL32(new_mpfp), mpfp, sizeof(*mpfp));
		mpfp = (void *)REL32(new_mpfp);
	}

	mptable = mpfp->mptable;
	printf("mptable @%p\n", mptable);
	memcpy((void *)REL32(new_mptable), mptable, 512);
	mptable = (void *)REL32(new_mptable);
	printf("mptable @%p\n", mptable);
	mpfp->mptable = mptable;
	mpfp->checksum -= checksum(mpfp, mpfp->len);
	printf("sig: %.4s, len: %d, rev: %d, chksum: %x, oemid: %.8s, prodid: %.12s,\n"
	       "oemtable: %p, oemsz: %d, entries: %d, lapicaddr: %08x, elen: %d, echk: %x\n",
	       mptable->sig.s, mptable->len,  mptable->revision,  mptable->checksum,
	       mptable->oemid, mptable->prodid, mptable->oemtable, mptable->oemtablesz,
	       mptable->entries, mptable->lapicaddr, mptable->extlen, mptable->extchksum);
	i = 0x1d8; /* First unused entry */
	memcpy(&(mptable->data[i]), &(mptable->data[20]), 20); /* Copy AP 1 data */
	mptable->data[i + 1] = 0xf;
	mptable->checksum -= checksum(mptable, mptable->len);
}

static void setup_apic_atts(void)
{
	uint32_t apic_shift;
	uint16_t i, j;
	apic_shift = 1;

	while (apic_per_node > (1 << apic_shift)) apic_shift++;

	if (apic_shift > 4)
		apic_shift = 4;

	printf("Setting up APIC ATT tables with shift %d:", apic_shift);

	/* Set APIC ATT for remote interrupts */
	for (i = 0; i < dnc_node_count; i++) {
		uint16_t snode = (i == 0) ? 0xfff0 : nc_node[i].sci_id;
		uint16_t dnode, ht;

		printf(" SCI%03x", nc_node[i].sci_id);
		dnc_write_csr(snode, H2S_CSR_G3_APIC_MAP_SHIFT, apic_shift - 1);
		dnc_write_csr(snode, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00000020); /* Select APIC ATT */

		for (j = 0; j < 64; j++)
			dnc_write_csr(snode, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + j * 4, nc_node[0].sci_id);

		for (dnode = 0; dnode < dnc_node_count; dnode++) {
			uint16_t cur, min, max;
			min = ~0;
			max = 0;

			for (ht = 0; ht < 8; ht++) {
				if (!nc_node[dnode].ht[ht].cpuid)
					continue;

				cur = nc_node[dnode].apic_offset + nc_node[dnode].ht[ht].apic_base;

				if (min > cur)
					min = cur;

				if (max < cur + nc_node[dnode].ht[ht].cores)
					max = cur + nc_node[dnode].ht[ht].cores;
			}

			min = min >> apic_shift;
			max = (max - 1) >> apic_shift;

			for (j = min; j <= max; j++)
				dnc_write_csr(snode, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + j * 4, nc_node[dnode].sci_id);
		}
	}
	printf("\n");
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

			cht_write_conf(dnc_master_ht_id, 1, H2S_CSR_F1_RESOURCE_MAPPING_ENTRY_INDEX, i);
			cht_write_conf(dnc_master_ht_id, 1, H2S_CSR_F1_DRAM_LIMIT_ADDRESS_REGISTERS, lim);
			cht_write_conf(dnc_master_ht_id, 1, H2S_CSR_F1_DRAM_BASE_ADDRESS_REGISTERS, base);
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

	if (verbose)
		printf(" (SMM @ 0x%" PRIx64")", smm_base);

	if (!disable_smm)
		return;

	if (smm_base && (smm_base != ~0ULL))
		smm_base += 0x8000;
	else
		return;

	printf("Disabling SMM handler...\n");
	smm_addr = 0x200000000000ULL | smm_base;
	val = dnc_read_csr(0xfff0, H2S_CSR_G0_NODE_IDS);
	node = (val >> 16) & 0xfff;

	for (i = 0; i < dnc_master_ht_id; i++)
		mmio_range(0xfff0, i, 10, 0x200000000000ULL, 0x2fffffffffffULL, dnc_master_ht_id);

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

	for (i = 0; i < dnc_master_ht_id; i++)
		mmio_range_del(0xfff0, i, 10);
}

static void setup_other_cores(void)
{
	uint16_t node = nc_node[0].sci_id;
	uint32_t ht, apicid, oldid, i;
	volatile uint32_t *icr;
	volatile uint32_t *apic;
	uint32_t val;
	uint64_t msr;
	/* Set H2S_Init */
	printf("Setting SCI%03x H2S_Init...\n", node);
	val = dnc_read_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL);
	dnc_write_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL, val | (1 << 12));
	msr = rdmsr(MSR_APIC_BAR);
	printf("MSR APIC_BAR: %012llx\n", msr);
	apic = (void *)((uint32_t)msr & ~0xfff);
	icr = &apic[0x300 / 4];
	printf("apic: %08x, apicid: %08x, icr: %08x, %08x\n",
	       (uint32_t)apic, apic[0x20 / 4], (uint32_t)icr, *icr);

	/* Set core watchdog timer to 21s */
	msr = (9 << 3) | 1;
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

	/* AMD Fam 15h Errata #572: Access to PCI Extended Configuration Space in SMM is Blocked
	 * Suggested Workaround: BIOS should set MSRC001_102A[27] = 1b */
	msr = rdmsr(MSR_CU_CFG2);
	if (family >= 0x15) {
		msr |= 1ULL << 27;
		wrmsr(MSR_CU_CFG2, msr);
	}
	*REL64(new_cucfg2_msr) = msr;

	printf("APICs:");
	critical_enter();

	/* Start all local cores (not BSP) and let them run our init_trampoline */
	for (ht = 0; ht < 8; ht++) {
		for (i = 0; i < nc_node[0].ht[ht].cores; i++) {
			if (!nc_node[0].ht[ht].cpuid)
				continue;

			if ((ht == 0) && (i == 0))
				continue; /* Skip BSP */

			oldid = nc_node[0].ht[ht].apic_base + i;
			apicid = nc_node[0].apic_offset + oldid;

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

			printf(" %d", apicid);

			if (*REL64(rem_topmem_msr) != *REL64(new_topmem_msr))
				printf(" (topmem 0x%llx->0x%llx)", *REL64(rem_topmem_msr), *REL64(new_topmem_msr));

			msr = *REL64(rem_smm_base_msr);
			disable_smm_handler(msr);
		}
	}

	critical_leave();
	printf("\n");
}

static void renumber_remote_bsp(const uint16_t num)
{
	uint8_t i, j;
	nc_node_info_t *cur_node = &nc_node[num];
	uint16_t node = cur_node->sci_id;
	uint8_t maxnode = cur_node->nc_ht_id;
	uint32_t val;
	printf("Renumbering BSP to HT#%d on SCI%03x\n", maxnode, node);

	for (i = 0; i < maxnode; i++) {
		val = dnc_read_conf(node, 0, 24 + i, FUNC0_HT, 0x0);
		if ((val != 0x12001022) && (val != 0x16001022)) {
			error("F0x00 value 0x%08x does not indicate an AMD Opteron processor on SCI%03x#%x",
			       val, node, i);
			return;
		}

		/* Disable traffic distribution */
		dnc_write_conf(node, 0, 24 + i, FUNC0_HT, 0x164, 0);

		/* Route maxnode + 1 as maxnode */
		val = dnc_read_conf(node, 0, 24 + i, FUNC0_HT, 0x40 + 4 * maxnode);
		dnc_write_conf(node, 0, 24 + i, FUNC0_HT, 0x44 + 4 * maxnode, val);
	}

	/* Bump NC to maxnode + 1 */
	dnc_write_conf(node, 0, 24 + maxnode, 0, H2S_CSR_F0_CHTX_NODE_ID,
	               (maxnode << 8) | (maxnode + 1));
	val = dnc_read_csr(node, H2S_CSR_G3_HT_NODEID);
	printf("- moving NC to HT#%d\n", val);

	for (i = 0; i < maxnode; i++) {
		val = dnc_read_conf(node, 0, 24 + i, FUNC0_HT, 0x68);
		dnc_write_conf(node, 0, 24 + i, FUNC0_HT, 0x68, (val & ~(1 << 15)) | 0x40f);
		/* Increase NodeCnt */
		val = dnc_read_conf(node, 0, 24 + i, FUNC0_HT, 0x60);
		dnc_write_conf(node, 0, 24 + i, FUNC0_HT, 0x60, val + 0x10);
		/* Route maxnode as 0 */
		val = dnc_read_conf(node, 0, 24 + i, FUNC0_HT, 0x40);
		dnc_write_conf(node, 0, 24 + i, FUNC0_HT, 0x40 + 4 * maxnode, val);
	}

	/* Renumber HT#0 */
	val = dnc_read_conf(node, 0, 24 + 0, FUNC0_HT, 0x60);
	dnc_write_conf(node, 0, 24 + 0, FUNC0_HT, 0x60,
	               (val & ~0x7707) | (maxnode << 12) | (maxnode << 8) | maxnode);

	for (i = 1; i <= maxnode; i++) {
		/* Update LkNode, SbNode */
		val = dnc_read_conf(node, 0, 24 + i, FUNC0_HT, 0x60);
		dnc_write_conf(node, 0, 24 + i, FUNC0_HT, 0x60,
		               (val & ~0x7700) | (maxnode << 12) | (maxnode << 8));

		/* Update VGA routing */
		val = dnc_read_conf(node, 0, 24 + i, FUNC1_MAPS, 0xf4);
		dnc_write_conf(node, 0, 24 + i, FUNC1_MAPS, 0xf4, (val & ~(7 << 4)) | maxnode);

		/* Ensure CC6 state save node is local */
		if (family >= 0x15) {
			val = dnc_read_conf(node, 0, 24 + i, FUNC4_LINK, 0x128);
			if (((val >> 12) & 0xff) == 0)
				dnc_write_conf(node, 0, 24 + i, FUNC4_LINK, 0x128, val | (maxnode << 12));
		}

		/* Update DRAM maps */
		for (j = 0; j < 8; j++) {
			val = dnc_read_conf(node, 0, 24 + i, FUNC1_MAPS, 0x44 + 8 * j);

			if ((val & 7) == 0)
				dnc_write_conf(node, 0, 24 + i, FUNC1_MAPS, 0x44 + 8 * j, val | maxnode);
		}

		/* Update low MMIO ranges leaving upper ranges */
		for (j = 0; j < 8; j++) {
			val = dnc_read_conf(node, 0, 24 + i, FUNC1_MAPS, 0x84 + 8 * j);

			if ((val & 7) == 0)
				dnc_write_conf(node, 0, 24 + i, FUNC1_MAPS, 0x84 + 8 * j, val | maxnode);
		}

		/* Update IO maps */
		for (j = 0; j < 4; j++) {
			val = dnc_read_conf(node, 0, 24 + i, FUNC1_MAPS, 0xc4 + 8 * j);

			if ((val & 7) == 0)
				dnc_write_conf(node, 0, 24 + i, FUNC1_MAPS, 0xc4 + 8 * j, val | maxnode);
		}

		/* Update CFG maps */
		for (j = 0; j < 4; j++) {
			val = dnc_read_conf(node, 0, 24 + i, FUNC1_MAPS, 0xe0 + 4 * j);

			if (((val >> 4) & 7) == 0)
				dnc_write_conf(node, 0, 24 + i, FUNC1_MAPS, 0xe0 + 4 * j, val | (maxnode << 4));
		}
	}

	for (i = 1; i <= maxnode; i++) {
		val = dnc_read_conf(node, 0, 24 + i, FUNC0_HT, 0x00);

		if ((val != 0x12001022) && (val != 0x16001022)) {
			error("F0x00 value 0x%08x does not indicate an AMD Opteron processor on SCI%03x#%x",
			       i, val, node);
			return;
		}

		/* Route 0 as maxnode + 1 */
		val = dnc_read_conf(node, 0, 24 + i, FUNC0_HT, 0x44 + 4 * maxnode);
		dnc_write_conf(node, 0, 24 + i, FUNC0_HT, 0x40, val);
	}

	/* Move NC to HT#0, update SbNode, LkNode */
	dnc_write_conf(node, 0, 24 + maxnode + 1, 0, H2S_CSR_F0_CHTX_NODE_ID,
	               (maxnode << 24) | (maxnode << 16) | (maxnode << 8) | 0);
	val = dnc_read_csr(node, H2S_CSR_G3_HT_NODEID);
	printf("- moving NC to HT#%d on SCI%03x...", val, node);

	/* Decrease NodeCnt */
	for (i = 1; i <= maxnode; i++) {
		val = dnc_read_conf(node, 0, 24 + i, FUNC0_HT, 0x60);
		dnc_write_conf(node, 0, 24 + i, FUNC0_HT, 0x60, val - 0x10);
	}

	/* Remove maxnode + 1 routing entry */
	for (i = 1; i <= maxnode; i++)
		dnc_write_conf(node, 0, 24 + i, FUNC0_HT, 0x44 + 4 * maxnode, 0x40201);

	/* Reenable probes */
	for (i = 1; i <= maxnode; i++) {
		val = dnc_read_conf(node, 0, 24 + i, FUNC0_HT, 0x68);
		dnc_write_conf(node, 0, 24 + i, FUNC0_HT, 0x68, (val & ~0x40f) | (1 << 15));
	}

	memcpy(&cur_node->ht[maxnode], &cur_node->ht[0], sizeof(ht_node_info_t));
	cur_node->ht[0].cpuid = 0;
	cur_node->nc_ht_id = 0;

#ifdef BROKEN
	/* Reorder the individual HT node memory base address so it is in increasing order */
	for (i = 1; i <= maxnode; i++) {
		if (i == 1) cur_node->ht[i].base = cur_node->dram_base;
		else        cur_node->ht[i].base = cur_node->ht[i-1].base + cur_node->ht[i-1].size;
	}
#endif

	/* Rewrite high MMIO ranges to route CSR access to Numachip */
	for (i = 1; i <= maxnode; i++) {
		mmio_range_del(node, i, 8);
		mmio_range(node, i, 8, DNC_CSR_BASE, DNC_CSR_LIM, 0);
		mmio_range_del(node, i, 9);
		mmio_range(node, i, 9, DNC_MCFG_BASE, DNC_MCFG_LIM, 0);
	}

	printf("done\n");
}

void mmio_range_print(const uint16_t sci, const int ht, uint8_t range)
{
	if (family >= 0x15) {
		assert(range < 12);

		int loff = 0, hoff = 0;
		if (range > 7) {
			loff = 0xe0;
			hoff = 0x20;
		}

		/* Skip disabled ranges */
		uint32_t low = dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x80 + loff + range * 8);
		if (!(low & 3))
			return;

		printf("SCI%03x#%d MMIO range %d: %08x %08x %08x\n", sci, ht, range, low,
			dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x84 + loff + range * 8),
			dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x180 + hoff + range * 4));
		return;
	}

	/* Family 10h */
	if (range < 8) {
		/* Skip disabled ranges */
		uint32_t low = dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x80 + range * 8);
		if (!(low & 3))
			return;

		printf("SCI%03x#%d MMIO range %d: %08x %08x\n", sci, ht, range, low,
			dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x84 + range * 8));
		return;
	}

	assert(range < 12);
	range -= 8;

	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x110, (2 << 28) | range);
	uint32_t val1 = dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x114);
	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x110, (3 << 28) | range);
	uint32_t val2 = dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x114);

	printf("SCI%03x#%d MMIO range %d: %08x %08x\n", sci, ht, range, val1, val2);
}

void mmio_range(const uint16_t sci, const int ht, uint8_t range, uint64_t base, uint64_t limit, const int dest)
{
	if (verbose > 1)
		printf("Adding MMIO range %d on SCI%03x#%x from 0x%llx to 0x%llx towards %d\n",
			range, sci, ht, base, limit, dest);

	if (family >= 0x15) {
		assert(range < 12);

		int loff = 0, hoff = 0;
		if (range > 7) {
			loff = 0xe0;
			hoff = 0x20;
		}

		uint32_t val = dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x80 + loff + range * 8);
		if (val & (1 << 3))
			return; /* Locked */
		assert((val & 3) == 0); /* Unused */

		dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x180 + hoff + range * 4, ((limit >> 40) << 16) | (base >> 40));
		dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x84 + loff + range * 8, ((limit >> 16) << 8) | dest);
		dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x80 + loff + range * 8, ((base >> 16) << 8 | 3));
		return;
	}

	/* Family 10h */
	if (range < 8) {
		assert(limit < (1ULL << 40));
		uint32_t val = dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x80 + range * 8);
		if (val & (1 << 3))
			return; /* Locked */
		assert((val & 3) == 0); /* Unused */

		dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x84 + range * 8, ((limit >> 16) << 8) | dest);
		dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x80 + range * 8, ((base >> 16) << 8 | 3));
		return;
	}

	assert(range < 12);
	range -= 8;

	/* Reading an uninitialised extended MMIO ranges results in MCE, so can't assert */

	uint64_t mask = 0;
	base  >>= 27;
	limit >>= 27;

	while ((base | mask) != (limit | mask))
		mask = (mask << 1) | 1;

	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x110, (2 << 28) | range);
	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x114, (base << 8) | dest);
	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x110, (3 << 28) | range);
	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x114, (mask << 8) | 1);
}

void mmio_range_del(const uint16_t sci, const int ht, uint8_t range)
{
	if (verbose > 1)
		printf("Deleting MMIO range %d on SCI%03x#%x\n", range, sci, ht);

	if (family >= 0x15) {
		assert(range < 12);

		int loff = 0, hoff = 0;
		if (range > 7) {
			loff = 0xe0;
			hoff = 0x20;
		}

		dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x80 + loff + range * 8, 0);
		dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x84 + loff + range * 8, 0);
		dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x180 + hoff + range * 4, 0);
		return;
	}

	/* Family 10h */
	if (range < 8) {
		dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x84 + range * 8, 0);
		dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x80 + range * 8, 0);
		return;
	}

	assert(range < 12);
	range -= 8;

	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x110, (2 << 28) | range);
	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x114, 0);
	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x110, (3 << 28) | range);
	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x114, 0);
}

static bool dram_range_read(const uint16_t sci, const int ht, const int range, uint64_t *base, uint64_t *limit, int *dst)
{
	assert(range < 8);

	uint32_t base_l = dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x40 + range * 8);
	uint32_t limit_l = dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x44 + range * 8);
	uint32_t base_h = dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x140 + range * 8);
	uint32_t limit_h = dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x144 + range * 8);

	*base = ((uint64_t)(base_l & ~0xffff) << (24 - 16)) | ((uint64_t)(base_h & 0xff) << 40);
	*limit = ((uint64_t)(limit_l & ~0xffff) << (24 - 16)) | ((uint64_t)(limit_h & 0xff) << 40);
	*dst = limit_l & 7;

	/* Ensure both read and write are enabled or neither */
	assert((base_l & 1) == ((base_l >> 1) & 1));
	bool en = base_l & 1;
	if (en)
		*limit |= 0xffffff;

	return en;
}

static int dram_range_unused(const uint16_t sci, const int ht)
{
	uint64_t base, limit;
	int dst;

	for (int range = 0; range < 8; range++)
		if (!dram_range_read(sci, ht, range, &base, &limit, &dst))
			return range;

	fatal("No free DRAM ranges on SCI%03x\n", sci);
}

static void dram_range_print(const uint16_t sci, const int ht, const int range)
{
	uint64_t base, limit;
	int dst;

	assert(range < 8);

	if (dram_range_read(sci, ht, range, &base, &limit, &dst))
		printf("SCI%03x#%d DRAM range %d: 0x%016llx - 0x%016llx towards %d\n", sci, ht, range, base, limit, dst);
}

static void dram_range(const uint16_t sci, const int ht, const int range, const uint32_t base, const uint32_t limit, const int dest)
{
	assert(dest < 8);
	assert(range < 8);
	assert((dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x40 + range * 8) & 3) == 0);

	if (verbose > 1)
		printf("SCI%03x#%d adding DRAM range %d: 0x%08x - 0x%08x towards %d\n", sci, ht, range, base, limit, dest);

	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x144 + range * 8, limit >> (40 - DRAM_MAP_SHIFT));
	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x44 + range * 8, (limit << 16) | dest);
	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x140 + range * 8, base >> (40 - DRAM_MAP_SHIFT));
	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x40 + range * 8, (base << 16) | 3);
}

static void dram_range_del(const uint16_t sci, const int ht, const int range)
{
	assert(range < 8);

	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x144 + range * 8, 0);
	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x44 + range * 8, 0);
	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x140 + range * 8, 0);
	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x40 + range * 8, 0);
}

static void setup_remote_cores(const uint16_t num)
{
	uint8_t i, map_index;
	nc_node_info_t *cur_node = &nc_node[num];
	uint16_t node = cur_node->sci_id;
	uint8_t ht_id = cur_node->nc_ht_id;
	uint16_t ht, apicid, oldid;
	uint32_t j;
	uint32_t val;
	uint64_t tom;
	uint64_t qval;
	printf("Setting up cores on node #%d (SCI%03x), %d HT nodes\n",
	       num, node, ht_id);
	/* Toggle go-ahead flag to remote node */
	printf("Checking if SCI%03x is ready\n", node);

	do {
		udelay(200);
		val = dnc_read_csr(node, H2S_CSR_G3_FAB_CONTROL);
	} while (!(val & 0x40000000UL));

	val |= 0x80000000UL;
	dnc_write_csr(node, H2S_CSR_G3_FAB_CONTROL, val);
	printf("Waiting for SCI%03x to acknowledge\n", node);

	do {
		udelay(200);
		val = dnc_read_csr(node, H2S_CSR_G3_FAB_CONTROL);
	} while (val & 0x80000000UL);

	/* Map MMIO 0x00000000 - 0xffffffff to master node */
	printf("Setting remote H2S MMIO32 ATT pages...\n");

	for (j = 0; j < 0x1000; j++) {
		if ((j & 0xff) == 0)
			dnc_write_csr(node, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00000010 | j >> 8);

		dnc_write_csr(node, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + (j & 0xff) * 4,
		              nc_node[0].sci_id);
	}

	if (renumber_bsp == -1) {
		/* Check if renumbering is needed */
		for (int ht = 0; ht < 8; ht++) {
			if (!cur_node->ht[ht].cpuid)
				continue;

			for (j = 0; j < 8; j++) {
				if (dnc_read_conf(node, 0, 24 + ht, FUNC1_MAPS, 0x80 + j * 8) & (1 << 3)) {
					renumber_bsp = 1;
					break;
				}
			}

			if (family >= 0x15) {
				for (j = 0; j < 4; j++) {
					if (dnc_read_conf(node, 0, 24 + ht, FUNC1_MAPS, 0x1a0 + j * 8) & (1 << 3)) {
						renumber_bsp = 1;
						break;
					}
				}
			}

			/* Break out of outer loop if needed */
			if (renumber_bsp == 1) {
				printf("SCI%03x has locked MMIO ranges\n", node);
				break;
			}
		}
	}

	if (renumber_bsp == 1)
		renumber_remote_bsp(num);

	ht_id = cur_node->nc_ht_id;
	printf("Remote H2S MMIO32 ATT set...\n");
	/* Set H2S_Init */
	printf("Setting SCI%03x H2S_Init...\n", node);
	val = dnc_read_csr(node, H2S_CSR_G3_HREQ_CTRL);
	dnc_write_csr(node, H2S_CSR_G3_HREQ_CTRL, val | (1 << 12));

	/* Check additional IO range registers */
	for (i = 0; i < 2; i++) {
		uint64_t qval = rdmsr(MSR_IORR_PHYS_MASK0 + i * 2);

		if (qval & (1 << 11))
			printf("Warning: IO range 0x%llx is enabled\n", rdmsr(MSR_IORR_PHYS_BASE0 + i * 2) & (~0xfffULL));
	}

	/* Insert coverall MMIO maps */
	tom = rdmsr(MSR_TOPMEM) >> 8;
	printf("Inserting coverall MMIO maps on SCI%03x\n", node);

	for (i = 0; i < 8; i++) {
		if (!cur_node->ht[i].cpuid)
			continue;

		/* Clear low MMIO ranges, leaving high ranges */
		for (j = 0; j < 8; j++)
			mmio_range_del(node, i, j);

		/* 1st MMIO map pair is set to point to the VGA segment A0000-C0000 */
		mmio_range(node, i, 0, 0xa0000, 0xbffff, ht_id);

		/* 2nd MMIO map pair is set to point to MMIO between TOM and 4G */
		mmio_range(node, i, 1, tom, 0xffffffff, ht_id);

		/* Make sure the VGA Enable register is disabled to forward VGA transactions
		 * (MMIO A_0000h - B_FFFFh and I/O 3B0h - 3BBh or 3C0h - 3DFh) to the NumaChip */
		if (!pf_vga_local) {
			dnc_write_conf(node, 0, 24 + i, FUNC1_MAPS, 0xf4, 0x0);
			if (dnc_read_conf(node, 0, 24 + i, FUNC1_MAPS, 0xf4))
				printf("Warning: Legacy VGA access is locked to local server; some video card BIOSs may cause any X servers to fail to complete initialisation\n");
		}
	}

	/* Now, reset all DRAM maps */
	printf("Resetting DRAM maps on SCI%03x\n", node);

	/* Read DRAM Hole register off master BSP */
	uint32_t memhole = cht_read_conf(0, FUNC1_MAPS, 0xf0);

	for (i = 0; i < 8; i++) {
		if (!cur_node->ht[i].cpuid)
			continue;

		/* Clear all entries */
		for (j = 0; j < 8; j++)
			dram_range_del(node, i, j);

		/* If Memory hoisting is enabled on master BSP, copy DramHoleBase[31:24] and DramMemHoistValid[1] */
		dnc_write_conf(node, 0, 24 + i, FUNC1_MAPS, 0xf0, memhole & 0xff000002);

		/* Re-direct everything below our first local address to NumaChip */
		dram_range(node, i, 0, nc_node[0].ht[0].base, cur_node->ht[0].base - 1, ht_id);
	}

	/* Reprogram HT node "self" ranges */
	printf("Reprogramming HT node \"self\" ranges on SCI%03x\n", node);

	for (i = 0; i < 8; i++) {
		if (!cur_node->ht[i].cpuid)
			continue;

		/* Check if DRAM channels are unganged */
		val = dnc_read_conf(node, 0, 24 + i, FUNC2_DRAM, 0x110);
		if (val & 1) {
			/* Note offset from base */
			uint32_t base = dnc_read_conf(node, 0, 24 + i, FUNC1_MAPS, 0x120) & 0x1fffff;
			uint32_t rlow = ((dnc_read_conf(node, 0, 24 + i, FUNC2_DRAM, 0x110) >> 11) - base) << (27 - DRAM_MAP_SHIFT);
			uint32_t rhigh = ((dnc_read_conf(node, 0, 24 + i, FUNC2_DRAM, 0x114) >> 11) - base) << (27 - DRAM_MAP_SHIFT);

			/* Reprogram DCT base/offset values against new base */
			dnc_write_conf(node, 0, 24 + i, FUNC2_DRAM, 0x110, (val & 0x7ff) |
			               (((cur_node->ht[i].base + rlow) >> (27 - DRAM_MAP_SHIFT)) << 11));
			dnc_write_conf(node, 0, 24 + i, FUNC2_DRAM, 0x114,
			               ((cur_node->ht[i].base + rhigh) >> (26 - DRAM_MAP_SHIFT)) << 10);
			printf("SCI%03x#%d: F2x110 %08x, F2x114 %08x\n",
			       node, i,
			       dnc_read_conf(node, 0, 24 + i, FUNC2_DRAM, 0x110),
			       dnc_read_conf(node, 0, 24 + i, FUNC2_DRAM, 0x114));
		}

		dnc_write_conf(node, 0, 24 + i, FUNC1_MAPS, 0x120,
		               cur_node->ht[i].base >> (27 - DRAM_MAP_SHIFT));
		dnc_write_conf(node, 0, 24 + i, FUNC1_MAPS, 0x124,
		               (cur_node->ht[i].base + cur_node->ht[i].size - 1) >> (27 - DRAM_MAP_SHIFT));
	}

	/* Program our local DRAM ranges */
	for (map_index = 0, i = 0; i < 8; i++) {
		if (!cur_node->ht[i].cpuid)
			continue;

		for (j = 0; j < 8; j++) {
			if (!cur_node->ht[j].cpuid)
				continue;

			dram_range(node, j, map_index + 1, cur_node->ht[i].base, cur_node->ht[i].base + cur_node->ht[i].size - 1, i);
		}

		dnc_write_conf(node, 0, 24 + ht_id, FUNC1_MAPS, H2S_CSR_F1_RESOURCE_MAPPING_ENTRY_INDEX,
		               map_index);
		dnc_write_conf(node, 0, 24 + ht_id, FUNC1_MAPS, H2S_CSR_F1_DRAM_LIMIT_ADDRESS_REGISTERS,
		               ((cur_node->ht[i].base + cur_node->ht[i].size - 1) << 8) | i);
		dnc_write_conf(node, 0, 24 + ht_id, FUNC1_MAPS, H2S_CSR_F1_DRAM_BASE_ADDRESS_REGISTERS,
		               (cur_node->ht[i].base << 8) | 3);
		map_index++;
	}

	/* Re-direct everything above our last local DRAM address (if any) to NumaChip */
	if (num != dnc_node_count - 1) {
		for (i = 0; i < 8; i++) {
			if (!cur_node->ht[i].cpuid)
				continue;

			dram_range(node, i, map_index + 1, cur_node->dram_limit, nc_node[dnc_node_count - 1].dram_limit - 1, ht_id);
		}

		map_index++;
	}

	if (verbose > 0) {
		printf("Master DRAM and MMIO ranges:\n");
		for (i = 0; i < 8; i++) {
			if (!cur_node->ht[i].cpuid)
				continue;

			for (j = 0; j < 8; j++)
				dram_range_print(node, i, j);

			for (j = 0; j < 8; j++)
				mmio_range_print(node, i, j);
		}
	}

	dnc_write_csr(node, H2S_CSR_G3_PCI_SEG0, nc_node[0].sci_id << 16);

	/* Quick and dirty: zero out I/O and config space maps; add
	 * all-covering map towards DNC */
	/* Note that rewriting F1xE0 prevents remote PCI config access hitting the remote
	   bus and it is decoded locally */
	for (i = 0; i < 8; i++) {
		if (!cur_node->ht[i].cpuid)
			continue;

		dnc_write_conf(node, 0, 24 + i, FUNC1_MAPS, 0xc4, 0x00fff000 | ht_id);
		dnc_write_conf(node, 0, 24 + i, FUNC1_MAPS, 0xc0, 0x00000003);

		for (j = 0xc8; j <= 0xdc; j += 4)
			dnc_write_conf(node, 0, 24 + i, FUNC1_MAPS, j, 0);

		if (!remote_io) {
			dnc_write_conf(node, 0, 24 + i, FUNC1_MAPS, 0xe0, 0xff000003 | (ht_id << 4));

			for (j = 0xe4; j <= 0xec; j += 4)
				dnc_write_conf(node, 0, 24 + i, FUNC1_MAPS, j, 0);
		}
	}

	/* Set DRAM range on local NumaChip */
	dnc_write_csr(node, H2S_CSR_G0_MIU_NGCM0_LIMIT, cur_node->dram_base >> 6);
	dnc_write_csr(node, H2S_CSR_G0_MIU_NGCM1_LIMIT, (cur_node->dram_limit >> 6) - 1);
	printf("SCI%03x NGCM0 0x%x, NGCM1 0x%x\n", node,
		dnc_read_csr(node, H2S_CSR_G0_MIU_NGCM0_LIMIT),
		dnc_read_csr(node, H2S_CSR_G0_MIU_NGCM1_LIMIT));
	dnc_write_csr(node, H2S_CSR_G3_DRAM_SHARED_BASE, cur_node->dram_base);
	dnc_write_csr(node, H2S_CSR_G3_DRAM_SHARED_LIMIT, cur_node->dram_limit);

	/* "Wraparound" entry, lets APIC 0xff00 - 0xffff target 0x0 to 0xff on destination node */
	dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x0000002f);
	i = dnc_read_csr(0xfff0, H2S_CSR_G3_APIC_MAP_SHIFT) + 1;

	for (j = (0xff00 >> i) & 0xff; j < 0x100; j++)
		dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + j * 4, node);

	printf("int_status: %x\n", dnc_read_csr(0xfff0, H2S_CSR_G3_EXT_INTERRUPT_STATUS));
	udelay(200);
	*REL64(new_mcfg_msr) = DNC_MCFG_BASE | ((uint64_t)node << 28ULL) | 0x21ULL;

	printf("APICs:");

	/* Start all remote cores and let them run our init_trampoline */
	for (ht = 0; ht < 8; ht++) {
		for (i = 0; i < cur_node->ht[ht].cores; i++) {
			if (!cur_node->ht[ht].cpuid)
				continue;

			oldid = cur_node->ht[ht].apic_base + i;
			apicid = cur_node->apic_offset + oldid;

			*REL8(cpu_apic_renumber) = apicid & 0xff;
			*REL8(cpu_apic_hi)       = (apicid >> 8) & 0x3f;
			*REL64(rem_topmem_msr) = ~0ULL;
			*REL64(rem_smm_base_msr) = ~0ULL;

			wake_core_global(oldid, VECTOR_TRAMPOLINE);
			printf(" %d", apicid);

			if (*REL64(rem_topmem_msr) != *REL64(new_topmem_msr))
				printf(" (topmem 0x016%llx->0x016%llx)", *REL64(rem_topmem_msr), *REL64(new_topmem_msr));

			qval = *REL64(rem_smm_base_msr);
			disable_smm_handler(qval);
		}
	}

	printf("\n");
}

static void setup_local_mmio_maps(void)
{
	int i, j, next;
	uint64_t tom;
	uint32_t base[8];
	uint32_t lim[8];
	uint32_t dst[8];
	uint32_t curbase, curlim, curdst;
	unsigned int sbnode;

	tom = rdmsr(MSR_TOPMEM);
	printf("Setting MMIO maps on local DNC with TOM %lldMB...\n", tom >> 20);
	assert(tom < 0x100000000);

	for (i = 0; i < 8; i++) {
		base[i] = 0;
		lim[i] = 0;
		dst[i] = 0;
	}

	sbnode = (cht_read_conf(0, FUNC0_HT, 0x60) >> 8) & 7;
	base[0] = (tom >> 8) & ~0xff;
	lim[0] = 0x00ffff00;
	dst[0] = (sbnode << 8) | 3;
	next = 1;

	/* Apply default maps so we can bail without losing all hope */
	for (i = 0; i < 8; i++) {
		cht_write_conf(dnc_master_ht_id, 1, H2S_CSR_F1_RESOURCE_MAPPING_ENTRY_INDEX, i);
		cht_write_conf(dnc_master_ht_id, 1, H2S_CSR_F1_MMIO_LIMIT_ADDRESS_REGISTERS, lim[i] | (dst[i] >> 8));
		cht_write_conf(dnc_master_ht_id, 1, H2S_CSR_F1_MMIO_BASE_ADDRESS_REGISTERS, base[i] | (dst[i] & 0x3));
	}

	for (i = 0; i < 8; i++) {
		curbase = cht_read_conf(sbnode, FUNC1_MAPS, 0x80 + i * 8);
		curlim = cht_read_conf(sbnode, FUNC1_MAPS, 0x84 + i * 8);
		curdst = ((curlim & 0x7) << 8) | (curbase & 0x3);

		/* This strips NP-bit */
		curbase = curbase & ~0xff;
		curlim = curlim & ~0xff;

		if (verbose > 0)
			printf("CPU MMIO region #%d base: %08x, lim: %08x, dst: %04x\n",
			       i, curbase, curlim, curdst);

		if (curdst & 3) {
			bool found = 0;
			bool placed = 0;

			for (j = 0; j < next; j++) {
				if (((curbase < base[j]) && (curlim > base[j])) ||
				    ((curbase < lim[j])  && (curlim > lim[j]))) {
					error("MMIO range #%d (%x-%x) overlaps registered window #%d (%x-%x)",
					       i, curbase, curlim, j, base[j], lim[j]);
					return;
				}

				if (curbase == base[j]) {
					found = 1;

					if ((curdst >> 8) == sbnode) {
						placed = 1;
					}

					if (curlim == lim[j]) {
						/* Complete overlap */
						dst[j] = curdst;
						placed = 1;
					} else {
						/* Equal base */
						base[j] = curlim + 0x100;
					}

					break;
				} else if ((curbase > base[j]) && (curbase <= lim[j])) {
					found = 1;

					if ((curdst >> 8) == sbnode) {
						placed = 1;
					} else if (curlim == lim[j]) {
						/* Equal limit */
						lim[j] = curbase - 0x100;
					} else {
						/* Enclosed region */
						if (next >= 8) {
							error("Ran out of MMIO regions trying to place #%d (%x-%x)",
							       i, curbase, curlim);
							return;
						}

						base[next] = curlim + 0x100;
						lim[next] = lim[j];
						dst[next] = dst[j];
						lim[j] = curbase - 0x100;
						next++;
					}

					break;
				} else if ((curbase < 0x1000) && (curlim < 0x1000)) {
					/* Sub-1M ranges */
					found = 1;
				}
			}

			if (found) {
				if (!placed) {
					if (next >= 8) {
						error("Ran out of MMIO regions trying to place #%d (%x-%x)",
						       i, curbase, curlim);
						return;
					}

					base[next] = curbase;
					lim[next] = curlim;
					dst[next] = curdst;
					next++;
				}
			} else {
				error("Enclosing window not found for MMIO range #%d (%x-%x)",
				       i, curbase, curlim);
				return;
			}
		}
	}

	if (verbose > 0)
		for (i = 0; i < 8; i++) {
			printf("NC MMIO region #%d base %08x, lim %08x, dst %04x\n", i, base[i], lim[i], dst[i]);
			cht_write_conf(dnc_master_ht_id, 1, H2S_CSR_F1_RESOURCE_MAPPING_ENTRY_INDEX, i);
			cht_write_conf(dnc_master_ht_id, 1, H2S_CSR_F1_MMIO_LIMIT_ADDRESS_REGISTERS, lim[i] | (dst[i] >> 8));
			cht_write_conf(dnc_master_ht_id, 1, H2S_CSR_F1_MMIO_BASE_ADDRESS_REGISTERS, base[i] | (dst[i] & 0x3));
		}
}

static int read_file(const char *filename, void *buf, int bufsz)
{
	static com32sys_t inargs, outargs;
	int fd, len, bsize, blocks;

	assert(bufsz >= ((int)(strlen(filename) + 1)));

	printf("Trying to open %s....", filename);
	strcpy(buf, filename);
	inargs.eax.w[0] = 0x0006; /* Open file */
	inargs.esi.w[0] = OFFS(buf);
	inargs.es = SEG(buf);
	__intcall(0x22, &inargs, &outargs);
	fd = outargs.esi.w[0];
	len = outargs.eax.l;
	bsize = outargs.ecx.w[0];

	if (fd == 0 || len < 0) {
		error("File not found");
		return -1;
	}

	if ((len + 1) > bufsz) {
		error("File to large at %d bytes", len);
		return -1;
	}

	printf("Ok\n");
	memset(buf, 0, len + 1);
	blocks = (len / bsize) + 1;
	printf("Reading %s (%d bytes, %d blocks)\n", filename, len, blocks);
	inargs.eax.w[0] = 0x0007; /* Read file */
	inargs.esi.w[0] = fd;
	inargs.ecx.w[0] = blocks;
	inargs.ebx.w[0] = OFFS(buf);
	inargs.es = SEG(buf);
	__intcall(0x22, &inargs, &outargs);
	len = outargs.ecx.l;

	if (outargs.esi.w[0] != 0) {
		inargs.eax.w[0] = 0x0008; /* Close file */
		inargs.esi.w[0] = outargs.esi.w[0];
		__intcall(0x22, &inargs, NULL);
	}

	return len;
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
		printf("Microcode update not specified, continuing with built-in version (xor = %u, %u)\n",
		       ucode_xor, table_xor);
		return;
	}

	if ((psep > 0) && (path[psep - 1] != '/'))
		path[psep++] = '/';

	strcat(path, "mseq.code");
	ucode_len = read_file(path, __com32.cs_bounce, __com32.cs_bounce_size);

	if (ucode_len < 0) {
		printf("Microcode update not found, continuing with built-in version (xor = %u, %u)\n",
		       ucode_xor, table_xor);
		return;
	}

	ucode_len = convert_buf_uint32_t(__com32.cs_bounce, mseq_ucode_update, 1024);

	if (ucode_len < 0) {
		printf("Error decoding microcode update, continuing with built-in version (xor = %u, %u)\n",
		       ucode_xor, table_xor);
		return;
	}

	path[psep] = '\0';
	strcat(path, "mseq.table");
	table_len = read_file(path, __com32.cs_bounce, __com32.cs_bounce_size);

	if (table_len < 0) {
		printf("Microcode update not found, continuing with built-in version (xor = %u, %u)\n",
		       ucode_xor, table_xor);
		return;
	}

	table_len = convert_buf_uint16_t(__com32.cs_bounce, mseq_table_update, 128);

	if (table_len < 0) {
		printf("Error decoding microcode update, continuing with built-in version (xor = %u, %u)\n",
		       ucode_xor, table_xor);
		return;
	}

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

	printf("Using updated microcode (xor = %u, %u)\n", ucode_xor, table_xor);
}
#endif

int read_config_file(char *file_name)
{
	int config_len;
	char *config;
	config_len = read_file(file_name, __com32.cs_bounce, __com32.cs_bounce_size);

	if (config_len < 0) {
		error("Fabric configuration file <%s> not found", file_name);
		return -1;
	}

	config = __com32.cs_bounce;
	config[config_len] = '\0';

	if (!parse_config_file(config))
		return -1;

	return 0;
}

static int pxeapi_call(int func, uint8_t *buf)
{
	static com32sys_t inargs, outargs;
	inargs.eax.w[0] = 0x0009; /* Call PXE Stack */
	inargs.ebx.w[0] = func; /* PXE function number */
	inargs.edi.w[0] = OFFS(buf);
	inargs.es = SEG(buf);
	__intcall(0x22, &inargs, &outargs);
	return outargs.eax.w[0] == PXENV_EXIT_SUCCESS;
}

int udp_open(void)
{
	t_PXENV_TFTP_CLOSE *tftp_close_param;
	t_PXENV_UDP_OPEN *pxe_open_param;
	tftp_close_param = __com32.cs_bounce;
	memset(tftp_close_param, 0, sizeof(*tftp_close_param));
	pxeapi_call(PXENV_TFTP_CLOSE, (uint8_t *)tftp_close_param);
	printf("TFTP close returns: %d\n", tftp_close_param->Status);

	pxe_open_param = __com32.cs_bounce;
	memset(pxe_open_param, 0, sizeof(*pxe_open_param));
	pxe_open_param->src_ip = myip.s_addr;
	pxeapi_call(PXENV_UDP_OPEN, (uint8_t *)pxe_open_param);
	printf("PXE UDP open returns: %d\n", pxe_open_param->status);
	return 1;
}

void udp_broadcast_state(int handle __attribute__((unused)),
                         void *buf, int len)
{
	t_PXENV_UDP_WRITE *pxe_write_param;
	char *buf_reloc;
	pxe_write_param = __com32.cs_bounce;
	buf_reloc = (char *)__com32.cs_bounce + sizeof(*pxe_write_param);
	memset(pxe_write_param, 0, sizeof(*pxe_write_param));
	pxe_write_param->ip = 0xffffffff;
	pxe_write_param->src_port = htons(4711);
	pxe_write_param->dst_port = htons(4711);
	pxe_write_param->buffer.seg = SEG(buf_reloc);
	pxe_write_param->buffer.offs = OFFS(buf_reloc);
	memcpy(buf_reloc, buf, len);
	pxe_write_param->buffer_size = len;
	pxeapi_call(PXENV_UDP_WRITE, (uint8_t *)pxe_write_param);
}

int udp_read_state(int handle __attribute__((unused)),
                   void *buf, int len)
{
	t_PXENV_UDP_READ *pxe_read_param;
	char *buf_reloc;
	pxe_read_param = __com32.cs_bounce;
	buf_reloc = (char *)__com32.cs_bounce + sizeof(*pxe_read_param);
	memset(pxe_read_param, 0, sizeof(*pxe_read_param));
	pxe_read_param->s_port = htons(4711);
	pxe_read_param->d_port = htons(4711);
	pxe_read_param->buffer.seg = SEG(buf_reloc);
	pxe_read_param->buffer.offs = OFFS(buf_reloc);
	pxe_read_param->buffer_size = len;
	pxeapi_call(PXENV_UDP_READ, (uint8_t *)pxe_read_param);

	if ((pxe_read_param->status == PXENV_STATUS_SUCCESS) &&
	    (pxe_read_param->s_port == htons(4711))) {
		memcpy(buf, buf_reloc, pxe_read_param->buffer_size);
		return pxe_read_param->buffer_size;
	} else {
		return 0;
	}
}

static void wait_status(struct node_info *info)
{
	printf("Waiting for");

	for (int i = 0; i < cfg_nodes; i++) {
		if (config_local(&cfg_nodelist[i], info->uuid)) /* Self */
			continue;

		if (nodedata[cfg_nodelist[i].sciid] != 0x80)
			printf(" SCI%03x (%s)",
			       cfg_nodelist[i].sciid, cfg_nodelist[i].desc);
	}

	printf("\n");
}

static void wait_for_slaves(struct node_info *info, struct part_info *part)
{
	struct state_bcast cmd, rsp;
	bool ready_pending = 1;
	int count, backoff, last_stat;
	bool do_restart = 0;
	enum node_state waitfor, own_state;
	uint32_t last_cmd = ~0;
	int len, i;
	int handle;
	handle = udp_open();
	cmd.state = CMD_STARTUP;
	cmd.uuid  = info->uuid;
	cmd.sciid = info->sciid;
	cmd.tid   = 0; /* Must match initial rsp.tid for RSP_SLAVE_READY */
	waitfor = RSP_SLAVE_READY;
	printf("Waiting for %d nodes...\n", cfg_nodes - 1);
	count = 0;
	backoff = 1;
	last_stat = 0;

	while (1) {
		if (++count >= backoff) {
			udp_broadcast_state(handle, &cmd, sizeof(cmd));

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

			nodedata[info->sciid] = 0x80;
			last_cmd = cmd.tid;
		}

		if (cfg_nodes > 1) {
			len = udp_read_state(handle, &rsp, sizeof(rsp));

			if (!len && !do_restart) {
				if (last_stat > 64) {
					last_stat = 0;
					wait_status(info);
				}

				continue;
			}
		} else
			len = 0;

		if (len == sizeof(rsp)) {
			int ours = 0;

			for (i = 0; i < cfg_nodes; i++)
				if ((!name_matching && (cfg_nodelist[i].uuid == rsp.uuid)) ||
				    ( name_matching && (cfg_nodelist[i].sciid == rsp.sciid))) {
					ours = 1;
					break;
				}

			if (ours) {
				if ((rsp.state == waitfor) && (rsp.tid == cmd.tid)) {
					if (nodedata[rsp.sciid] != 0x80) {
						printf("SCI%03x (%s) entered %s\n",
						       rsp.sciid, cfg_nodelist[i].desc,
						       node_state_name[waitfor]);
						nodedata[rsp.sciid] = 0x80;
					}
				} else if ((rsp.state == RSP_PHY_NOT_TRAINED) ||
				           (rsp.state == RSP_RINGS_NOT_OK) ||
				           (rsp.state == RSP_FABRIC_NOT_READY) ||
				           (rsp.state == RSP_FABRIC_NOT_OK)) {
					if (nodedata[rsp.sciid] != 0x80) {
						printf("SCI%03x (%s) aborted with state %d; restarting process...\n",
						       rsp.sciid, cfg_nodelist[i].desc, rsp.state);
						do_restart = 1;
						nodedata[rsp.sciid] = 0x80;
					}
				}
			}
		}

		ready_pending = 0;

		for (i = 0; i < cfg_nodes; i++) {
			if (config_local(&cfg_nodelist[i], info->uuid)) /* Self */
				continue;

			if (!(nodedata[cfg_nodelist[i].sciid] & 0x80)) {
				ready_pending = 1;
				break;
			}
		}

		if (!ready_pending || do_restart) {
			if (do_restart) {
				cmd.state = CMD_ENTER_RESET;
				waitfor = RSP_RINGS_OK;
				do_restart = 0;
			} else if (cmd.state == CMD_STARTUP) {
				cmd.state = CMD_ENTER_RESET;
				waitfor = RSP_PHY_TRAINED;
			} else if (cmd.state == CMD_ENTER_RESET) {
				cmd.state = CMD_VALIDATE_RINGS;
				waitfor = RSP_RINGS_OK;
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
				nodedata[cfg_nodelist[i].sciid] &= 0x7f;

			cmd.tid++;
			count = 0;
			backoff = 1;
			printf("Issuing %s, desired response %s (tid = %d)\n",
			       node_state_name[cmd.state], node_state_name[waitfor], cmd.tid);
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

	uint64_t *mtrr_var_base = (void *)REL64(new_mtrr_var_base);
	uint64_t *mtrr_var_mask = (void *)REL64(new_mtrr_var_mask);

	mtrr_var_base[i] = base | type;
	wrmsr(MSR_MTRR_PHYS_BASE0 + i * 2, mtrr_var_base[i]);
	mtrr_var_mask[i] = (((1ULL << 48) - 1) &~ (limit - base - 1)) | 0x800;
	wrmsr(MSR_MTRR_PHYS_MASK0 + i * 2, mtrr_var_mask[i]);
}

static void update_mtrr(void)
{
	/* Ensure Tom2ForceMemTypeWB (bit 22) is set, so memory between 4G and TOM2 is writeback */
	uint64_t *syscfg_msr = (void *)REL64(new_syscfg_msr);
	*syscfg_msr = rdmsr(MSR_SYSCFG) | (1 << 22);
	wrmsr(MSR_SYSCFG, *syscfg_msr);

	/* Ensure default memory type is uncacheable */
	uint64_t *mtrr_default = (void *)REL64(new_mtrr_default);
	*mtrr_default = 3 << 10;
	wrmsr(MSR_MTRR_DEFAULT, *mtrr_default);

	/* Store fixed MTRRs */
	uint64_t *new_mtrr_fixed = (void *)REL64(new_mtrr_fixed);
	uint32_t *fixed_mtrr_regs = (void *)REL64(fixed_mtrr_regs);

	for (int i = 0; fixed_mtrr_regs[i] != 0xffffffff; i++)
		new_mtrr_fixed[i] = rdmsr(fixed_mtrr_regs[i]);

	/* Store variable MTRRs */
	uint64_t *mtrr_var_base = (void *)REL64(new_mtrr_var_base);
	uint64_t *mtrr_var_mask = (void *)REL64(new_mtrr_var_mask);
	printf("Variable MTRRs:\n");

	for (int i = 0; i < 8; i++) {
		mtrr_var_base[i] = rdmsr(MSR_MTRR_PHYS_BASE0 + i * 2);
		mtrr_var_mask[i] = rdmsr(MSR_MTRR_PHYS_MASK0 + i * 2);

		if (mtrr_var_mask[i] & 0x800ULL) {
			printf("  [%d] base=0x%012llx, mask=0x%012llx : %s\n", i, mtrr_var_base[i] & ~0xfffULL,
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

	for (int ht = 0; ht < dnc_master_ht_id; ht++) {
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

static void local_chipset_fixup(bool master)
{
	uint32_t val;
	printf("Scanning for known chipsets, local pass...\n");
	val = dnc_read_conf(0xfff0, 0, 0x14, 0, 0);

	if (val == VENDEV_SP5100) {
		printf("Adjusting local configuration of AMD SP5100...\n");
		/* Disable config-space triggered SMI */
		val = pmio_readb(0xa8);
		pmio_writeb(0xa8, val & ~(3 << 4)); /* Clear bit4 and bit5 */
	}

	val = dnc_read_conf(0xfff0, 0, 0, 0, 0);
	if (val == VENDEV_SR5690 || val == VENDEV_SR5670 || val == VENDEV_SR5650) {
		if (!remote_io && !master) {
			printf("- disabling IOAPIC\n");
			/* SR56x0 F0 0xf8/fc is IOAPIC config index/data port
			 * Write 0 to IOAPIC config register 0 to disable */
			dnc_write_conf(0xfff0, 0, 0, 2, 0xf8, 0);
			dnc_write_conf(0xfff0, 0, 0, 2, 0xfc, 0);
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

		for (i = 0; i < dnc_master_ht_id; i++)
			mmio_range(0xfff0, i, 10, 0x200000000000ULL, 0x2fffffffffffULL, dnc_master_ht_id);

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

		for (i = 0; i < dnc_master_ht_id; i++)
			mmio_range_del(0xfff0, i, 10);
	}

	pci_setup();
	lvt_setup();

	printf("Chipset-specific setup done\n");
}

static void global_chipset_fixup(void)
{
	uint16_t node;
	uint32_t val;
	int i;
	printf("Scanning for known chipsets, global pass...\n");

	for (i = 0; i < dnc_node_count; i++) {
		node = nc_node[i].sci_id;
		val = dnc_read_conf(node, 0, 0, 0, 0);

		if ((val == VENDEV_SR5690) || (val == VENDEV_SR5670) || (val == VENDEV_SR5650)) {
			printf("Adjusting configuration of AMD SR56x0 on SCI%03x...", node);
			/* Limit TOM2 to HyperTransport address */
			uint64_t limit = min(ht_base, (uint64_t)dnc_top_of_mem << DRAM_MAP_SHIFT);
			ioh_htiu_write(node, SR56X0_HTIU_TOM2LO, (limit & 0xff800000) | 1);
			ioh_htiu_write(node, SR56X0_HTIU_TOM2HI, limit >> 32);

			if (dnc_top_of_mem >= (1 << (40 - DRAM_MAP_SHIFT)))
				ioh_nbmiscind_write(node, SR56X0_MISC_TOM3, ((dnc_top_of_mem << (DRAM_MAP_SHIFT - 22)) - 1) | (1 << 31));
			else
				ioh_nbmiscind_write(node, SR56X0_MISC_TOM3, 0);

			if (verbose)
				printf("TOM2LO 0x%08x TOM2HI 0x%08x TOM3 0x%08x ",
				       ioh_htiu_read(node, SR56X0_HTIU_TOM2LO), ioh_htiu_read(node, SR56X0_HTIU_TOM2HI),
				       ioh_nbmiscind_read(node, SR56X0_MISC_TOM3));

			/* 0xf8/fc IOAPIC config space access
			 * write 0 to register 0 to disable IOAPIC */
			dnc_write_conf(node, 0, 0, 2, 0xf8, 0);
			dnc_write_conf(node, 0, 0, 2, 0xfc, 0);

			printf("done\n");
		} else if ((val == VENDEV_MCP55)) {
			uint32_t val = dnc_read_conf(node, 0, 0, 0, 0x90);
			printf("Adjusting configuration of nVidia MCP55 on SCI%03x...\n",
			       node);
			/* Disable mmcfg setting in bridge to avoid OS confusion */
			dnc_write_conf(node, 0, 0, 0, 0x90, val & ~(1 << 31));
		}
	}

	printf("Chipset-specific setup done\n");
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

void setup_mc4_thresholds(void)
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
	if (((msr >> 49) & 3) == 2)
		printf("- disabling DRAM thresholding SMI (%s)\n", (msr >> 61) ? "locked" : "not locked");
	msr &= ~((3ULL << 49) | (1ULL << 61));
	wrmsr(MSR_MC4_MISC0, msr);
	*REL64(new_mc4_misc0_msr) = msr;

	msr = rdmsr(MSR_MC4_MISC1);
	if (((msr >> 49) & 3) == 2)
		printf("- disabling link thresholding SMI (%s)\n", (msr >> 61) ? "locked" : "not locked");
	msr &= ~((3ULL << 49) | (1ULL << 61));
	wrmsr(MSR_MC4_MISC1, msr);
	*REL64(new_mc4_misc1_msr) = msr;

	msr = rdmsr(MSR_MC4_MISC2);
	if (((msr >> 49) & 3) == 2)
		printf("- disabling L3 cache thresholding SMI (%s)\n", (msr >> 61) ? "locked" : "not locked");
	msr &= ~((3ULL << 49) | (1ULL << 61));
	wrmsr(MSR_MC4_MISC2, msr);
	*REL64(new_mc4_misc2_msr) = msr;
}

static int unify_all_nodes(void)
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

	nc_node[1].ht[0].base = dnc_top_of_mem;

	if (!tally_all_remote_nodes()) {
		printf("Unable to reach all nodes, aborting...\n");
		return 0;
	}

	for (node = 0; node < dnc_node_count; node++) {
		for (i = 0; i < 8; i++) {
			if (!nc_node[node].ht[i].cpuid)
				continue;

			/* Ensure all processors are the same for compatibility */
			model = cpu_family(nc_node[node].sci_id, i);

			if (!model_first)
				model_first = model;
			else if (model != model_first) {
				error("SCI%03x (%s) has varying processor models 0x%08x and 0x%08x",
					nc_node[node].sci_id, get_master_name(nc_node[node].sci_id), model_first, model);
				abort = 1;
			}

			/* 6200/4200 processors lack the HT lock mechanism, so abort */
			if ((family >> 8) == 0x1501) {
				error("SCI%03x (%s) has incompatible 6200/4200 processors; please use 6300/4300 or later",
					nc_node[node].sci_id, get_master_name(nc_node[node].sci_id));
				abort = 1;
			}

			if ((model >> 16) >= 0x15) {
				uint32_t val = dnc_read_conf(nc_node[node].sci_id, 0, 24 + i, FUNC2_DRAM, 0x118);

				if (val & (1 << 19)) {
					error("SCI%03x (%s) has CState C6 enabled in the BIOS",
					       nc_node[node].sci_id, get_master_name(nc_node[node].sci_id));
					abort = 1;
				}
			}
		}
	}

	if (abort)
		return -1;

	if (verbose > 0) {
		printf("Global memory map:\n");

		for (node = 0; node < dnc_node_count; node++) {
			for (i = 0; i < 8; i++) {
				if (!nc_node[node].ht[i].cpuid)
					continue;

				printf("SCI%03x/HT#%d: %012llx - %012llx\n",
				       nc_node[node].sci_id, i,
				       (uint64_t)nc_node[node].ht[i].base << DRAM_MAP_SHIFT,
				       (uint64_t)(nc_node[node].ht[i].base + nc_node[node].ht[i].size) << DRAM_MAP_SHIFT);
			}
		}
	}

	/* Set up local mapping registers etc from 0 - master node max */
	for (i = 0; i < dnc_master_ht_id; i++) {
		cht_write_conf(dnc_master_ht_id, 1, H2S_CSR_F1_RESOURCE_MAPPING_ENTRY_INDEX, i);
		cht_write_conf(dnc_master_ht_id, 1, H2S_CSR_F1_DRAM_LIMIT_ADDRESS_REGISTERS,
		               ((nc_node[0].ht[i].base + nc_node[0].ht[i].size - 1) << 8) | i);
		cht_write_conf(dnc_master_ht_id, 1, H2S_CSR_F1_DRAM_BASE_ADDRESS_REGISTERS,
		               (nc_node[0].ht[i].base << 8) | 3);
	}

	/* DRAM map on local CPUs to redirect all accesses outside our local range to NC
	 * NB: Assuming that memory is assigned sequentially to SCI nodes */
	for (i = 0; i < dnc_master_ht_id; i++) {
		assert(cht_read_conf(i, FUNC1_MAPS, 0x78) == 0);
		int range = dram_range_unused(0xfff0, i);

		/* Don't add if the second node's base is the not above the first's, since it'll be a 1-node partition */
		if (nc_node[1].dram_base > nc_node[0].dram_base)
			dram_range(0xfff0, i, range, nc_node[1].dram_base, dnc_top_of_mem - 1, dnc_master_ht_id);
	}

	if (verbose > 0) {
		printf("DRAM and MMIO ranges:\n");
		for (i = 0; i < dnc_master_ht_id; i++) {
			for (int j = 0; j < 8; j++)
				dram_range_print(0xfff0, i, j);

			for (int j = 0; j < 8; j++)
				mmio_range_print(0xfff0, i, j);
		}
	}

	dnc_write_csr(0xfff0, H2S_CSR_G0_MIU_NGCM0_LIMIT, nc_node[0].dram_base >> 6);
	dnc_write_csr(0xfff0, H2S_CSR_G0_MIU_NGCM1_LIMIT, (nc_node[0].dram_limit >> 6) - 1);
	dnc_write_csr(0xfff0, H2S_CSR_G3_DRAM_SHARED_BASE, nc_node[0].dram_base);
	dnc_write_csr(0xfff0, H2S_CSR_G3_DRAM_SHARED_LIMIT, nc_node[0].dram_limit);

	for (i = 0; i < dnc_node_count; i++) {
		uint16_t dnode;
		node = (i == 0) ? 0xfff0 : nc_node[i].sci_id;

		for (dnode = 0; dnode < dnc_node_count; dnode++) {
			uint32_t addr = nc_node[dnode].dram_base;
			uint32_t end  = nc_node[dnode].dram_limit;

			dnc_write_csr(node, H2S_CSR_G0_ATT_INDEX,
			              0x80000000 | /* AutoInc */
			              (0x08000000 << SCC_ATT_INDEX_RANGE) | /* Index Range */
			              (addr / SCC_ATT_GRAN)); /* Start index for current node */

			while (addr < end) {
				dnc_write_csr(node, H2S_CSR_G0_ATT_ENTRY, nc_node[dnode].sci_id);
				addr += SCC_ATT_GRAN;
			}
		}
	}

	printf("Loading SCC microcode:");
	for (i = 0; i < dnc_node_count; i++) {
		node = (i == 0) ? 0xfff0 : nc_node[i].sci_id;
		printf(" SCI%03x", nc_node[i].sci_id);
		load_scc_microcode(node);
	}

	printf("\n");
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

	setup_local_mmio_maps();
	setup_apic_atts();
	*REL64(new_mcfg_msr) = DNC_MCFG_BASE | ((uint64_t)nc_node[0].sci_id << 28ULL) | 0x21ULL;
	wrmsr(MSR_MCFG_BASE, *REL64(new_mcfg_msr));

	/* Make chipset-specific adjustments */
	global_chipset_fixup();

	/* Must run after SCI is operational */
	printf("BSP SMM:");
	disable_smm_handler(rdmsr(MSR_SMM_BASE));
	printf("\n");

	setup_other_cores();

	for (i = 1; i < dnc_node_count; i++)
		setup_remote_cores(i);

	printf("Clearing remote memory...");
	for (node = 1; node < dnc_node_count; node++) {
		for (int ht = 0; ht < 8; ht++) {
			if (!nc_node[node].ht[ht].cpuid)
				continue;
			uint16_t sci = nc_node[node].sci_id;
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
		for (int ht = 0; ht < 8; ht++) {
			if (!nc_node[node].ht[ht].cpuid)
				continue;
			uint16_t sci = nc_node[node].sci_id;
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
		printf("Enabling DRAM scrubbers:");

		for (node = 0; node < dnc_node_count; node++) {
			uint16_t sciid = (node == 0) ? 0xfff0 : nc_node[node].sci_id;
			printf(" SCI%03x", nc_node[node].sci_id);

			for (i = 0; i < 8; i++) {
				if (!nc_node[0].ht[i].cpuid)
					continue;

				if (nc_node[node].ht[i].scrub & 0x1f) {
					uint64_t base = (uint64_t)nc_node[node].ht[i].base << DRAM_MAP_SHIFT;
					uint32_t redir = dnc_read_conf(sciid, 0, 24 + i, FUNC3_MISC, 0x5c) & 1;
					dnc_write_conf(sciid, 0, 24 + i, FUNC3_MISC, 0x5c, base | redir);
					dnc_write_conf(sciid, 0, 24 + i, FUNC3_MISC, 0x60, base >> 32);

					/* Fam15h: Accesses to this register must first set F1x10C [DctCfgSel]=0;
					   Accesses to this register with F1x10C [DctCfgSel]=1 are undefined;
					   See erratum 505 */
					if (family >= 0x15)
						dnc_write_conf(sciid, 0, 24 + i, FUNC1_MAPS, 0x10c, 0);

					dnc_write_conf(sciid, 0, 24 + i, FUNC3_MISC, 0x58, nc_node[node].ht[i].scrub);
				}
			}
		}

		printf("\n");
	}

	return 1;
}

static int check_api_version(void)
{
	static com32sys_t inargs, outargs;
	int major, minor;
	inargs.eax.w[0] = 0x0001;
	__intcall(0x22, &inargs, &outargs);
	major = outargs.ecx.b[1];
	minor = outargs.ecx.b[0];
	printf("Detected SYSLINUX API version %d.%02d\n", major, minor);

	if ((major * 100 + minor) < 372) {
		error("SYSLINUX API version >= 3.72 is required");
		return -1;
	}

	return 0;
}

static void start_user_os(void)
{
	static com32sys_t rm;
	/* Restore 32-bit only access */
	set_wrap32_enable();
	strcpy(__com32.cs_bounce, next_label);
	rm.eax.w[0] = 0x0003;
	rm.ebx.w[0] = OFFS(__com32.cs_bounce);
	rm.es = SEG(__com32.cs_bounce);
	printf("Unification succeeded; loading %s...\n", next_label);

	if (boot_wait)
		wait_key();

	__intcall(0x22, &rm, NULL);
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
		printf("pxe_get_cached_info() returned status : %d\n", sts);
		return;
	}

	/* Save MyIP for later (in udp_open) */
	myip.s_addr = ((pxe_bootp_t *)dhcpdata)->yip;
	printf("My IP address is %s\n", inet_ntoa(myip));

	/* Skip standard fields, as hostname is an option */
	unsigned int offset = 4 + offsetof(pxe_bootp_t, vendor.d);

	while (offset < dhcplen) {
		int code = dhcpdata[offset];
		int len = dhcpdata[offset + 1];

		/* Sanity-check length */
		if (len == 0)
			return;

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
		printf("Hostname is %s\n", hostname);
		return;
	}

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
	}

	printf("NB/TSC frequency is %dMHz\n", tsc_mhz);
}

#define STEP_MIN (64)
#define STEP_MAX (4 << 20)

static void selftest_late_memmap(void)
{
	struct e820entry *e820 = (void *)REL32(new_e820_map);
	uint16_t *len = REL16(new_e820_len);

	for (int i = 0; i < *len; i++) {
		/* Test only regions marked usable */
		if (e820[i].type != 1)
			continue;

		uint64_t start = e820[i].base;
		uint64_t end = e820[i].base + e820[i].length;
		printf("Testing memory permissions from %016llx to %016llx...", start, end - 1);

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

static int nc_start(void)
{
	uint32_t uuid;
	struct node_info *info;
	struct part_info *part;
	int i;

	if (check_api_version() < 0)
		return ERR_API_VERSION;

	constants();
	get_hostname();
	dnc_master_ht_id = dnc_init_bootloader(&uuid, &dnc_chip_rev, dnc_card_type, &dnc_asic_mode,
	                                       __com32.cs_cmdline);

	if (dnc_master_ht_id == -2)
		start_user_os();

	if (dnc_master_ht_id < 0)
		return ERR_MASTER_HT_ID;

	if (singleton) {
		make_singleton_config(uuid);
	} else {
		if (read_config_file(config_file_name) < 0)
			return ERR_NODE_CONFIG;
	}

	info = get_node_config(uuid);

	if (!info)
		return ERR_NODE_CONFIG;

	if (name_matching)
		printf("Node: <%s> sciid: 0x%03x, partition: %d, osc: %d\n",
		       info->desc, info->sciid, info->partition, info->osc);
	else
		printf("Node: <%s> uuid: %08X, sciid: 0x%03x, partition: %d, osc: %d\n",
		       info->desc, info->uuid, info->sciid, info->partition, info->osc);
	part = get_partition_config(info->partition);

	if (!part)
		return ERR_PARTITION_CONFIG;

	printf("Partition master: 0x%03x; builder: 0x%03x\n", part->master, part->builder);
	printf("Fabric dimensions: x: %d, y: %x, z: %d\n",
	       cfg_fabric.x_size, cfg_fabric.y_size, cfg_fabric.z_size);

	for (i = 0; i < cfg_nodes; i++) {
		if (config_local(&cfg_nodelist[i], uuid))
			continue;

		if (name_matching)
			printf("Remote node: <%s> sciid: 0x%03x, partition: %d, osc: %d\n",
			       cfg_nodelist[i].desc,
			       cfg_nodelist[i].sciid,
			       cfg_nodelist[i].partition,
			       cfg_nodelist[i].osc);
		else
			printf("Remote node: <%s> uuid: %08X, sciid: 0x%03x, partition: %d, osc: %d\n",
			       cfg_nodelist[i].desc,
			       cfg_nodelist[i].uuid,
			       cfg_nodelist[i].sciid,
			       cfg_nodelist[i].partition,
			       cfg_nodelist[i].osc);
	}

	if (adjust_oscillator(dnc_card_type, info->osc) < 0)
		return -1;

	/* Copy this into NC ram so its available remotely */
	load_existing_apic_map();

	if (part->builder == info->sciid)
		wait_for_slaves(info, part);
	else
		wait_for_master(info, part);

	/* Must run after SCI is operational */
	local_chipset_fixup(part->master == info->sciid);

	if (verbose > 0)
		debug_acpi();

	load_orig_e820_map();
	update_acpi_tables_early();

	if (info->sync_only) {
		/* Release resources to reduce allocator fragmentation */
		free(cfg_nodelist);
		free(cfg_partlist);
		start_user_os();
	}

	if (dnc_init_caches() < 0)
		return ERR_INIT_CACHES;

	if (!install_e820_handler())
		return ERR_INSTALL_E820_HANDLER;

	if (force_probefilteron && !force_probefilteroff) {
		enable_probefilter(dnc_master_ht_id - 1);
		probefilter_tokens(dnc_master_ht_id - 1);
	}

	if (part->master == info->sciid) {
		int i;

		/* Master */
		for (i = 0; i < cfg_nodes; i++) {
			if (config_local(&cfg_nodelist[i], uuid))
				continue;

			if (cfg_nodelist[i].partition != info->partition)
				continue;

			nodedata[cfg_nodelist[i].sciid] = 0x80;
		}

#ifdef UNUSED
		read_microcode_update();
#endif
		if (unify_all_nodes() == 0)
			return ERR_UNIFY_ALL_NODES;

		(void)dnc_check_mctr_status(0);
		(void)dnc_check_mctr_status(1);
		update_e820_map();

		if (remote_io > 1)
			setup_mmio_late();

		/* Release resources to reduce allocator fragmentation */
		free(cfg_nodelist);
		free(cfg_partlist);

		if (verbose) {
			selftest_late_memmap();

			/* Do this ahead of the self-test to prevent false positives */
			/* Restore 32-bit only access */
			set_wrap32_enable();
			selftest_late_msrs();
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
			    (!dnc_check_fabric(info))) {
				printf("\nErrors detected, halting!\n");
				while (1) {
					cli();
					asm volatile("hlt" ::: "memory");
				}
			}
			udelay(1000);
			val = dnc_read_csr(0xfff0, H2S_CSR_G3_FAB_CONTROL);
		} while (!(val & (1 << 31)));

		printf("\n\n");
		printf(BANNER "This server '%s' is part of a %d-server NumaConnect system; refer to the console on server '%s'",
		       info->desc, cfg_nodes, get_master_name(part->master));

		disable_smi();
		disable_dma_all(); /* disables VGA refresh */
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

int main(void)
{
	int ret;
	openconsole(&dev_rawcon_r, &dev_stdcon_w);
	printf(BANNER "NumaConnect system unification module " VER COL_DEFAULT "\n");

	/* Enable CF8 extended access for first Northbridge; we do others for Linux later */
	set_cf8extcfg_enable(0);

	/* Disable 32-bit address wrapping to allow 64-bit access in 32-bit code */
	set_wrap32_disable();

	ret = nc_start();
	if (ret < 0) {
		error("nc_start() failed with error code %d; check configuration files match hardware and UUIDs\n", ret);
		wait_key();
	}

	/* Restore 32-bit only access */
	set_wrap32_enable();
	return ret;
}
