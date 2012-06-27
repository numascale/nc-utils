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
#include <console.h>
#include <com32.h>
#include <inttypes.h>
#include <syslinux/pxe.h>
#include <sys/io.h>

#include "dnc-regs.h"
#include "dnc-defs.h"
#include "dnc-types.h"
#include "dnc-access.h"
#include "dnc-route.h"
#include "dnc-acpi.h"
#include "dnc-fabric.h"
#include "dnc-config.h"

#include "dnc-bootloader.h"
#include "dnc-commonlib.h"
#include "dnc-masterlib.h"
#include "dnc-devices.h"
#include "auto-dnc-gitlog.h"

#include "hw-config.h"

#define PIC_MASTER_CMD          0x20
#define PIC_MASTER_IMR          0x21
#define PIC_SLAVE_CMD           0xa0
#define PIC_SLAVE_IMR           0xa1

#define IMPORT_RELOCATED(sym) extern volatile u8 sym ## _relocate
#define REL(sym) ((volatile u8 *)asm_relocated + ((volatile u8 *)&sym ## _relocate - (volatile u8 *)&asm_relocate_start))
#define MTRR_TYPE(x) (x) == 0 ? "uncacheable" : (x) == 1 ? "write-combining" : (x) == 4 ? "write-through" : (x) == 5 ? "write-protect" : (x) == 6 ? "write-back" : "unknown"

#define TABLE_AREA_SIZE		1024*1024
#define MIN_NODE_MEM		256*1024*1024

int dnc_master_ht_id;     /* HT id of NC on master node, equivalent nc_node[0].nc_ht_id */
int dnc_asic_mode;
int dnc_chip_rev;
u16 dnc_node_count = 0;
nc_node_info_t nc_node[128];
u16 ht_pdom_count = 0;
u16 apic_per_node;
u16 ht_next_apic;
u32 dnc_top_of_dram;      /* Top of DRAM, before MMIO, in 16MB chunks */
u32 dnc_top_of_mem;       /* Top of MMIO, in 16MB chunks */
u8 post_apic_mapping[256]; /* POST APIC assigments */
static int scc_started = 0;

/* Traversal info per node.  Bit 7: seen, bits 5:0 rings walked */
u8 nodedata[4096];

extern unsigned char asm_relocate_start;
extern unsigned char asm_relocate_end;
static char *asm_relocated;
static char *tables_relocated;

IMPORT_RELOCATED(new_e820_handler);
IMPORT_RELOCATED(old_int15_vec);
IMPORT_RELOCATED(init_trampoline);
IMPORT_RELOCATED(cpu_init_finished);
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
IMPORT_RELOCATED(new_osvw_id_len_msr);
IMPORT_RELOCATED(new_osvw_status_msr);
IMPORT_RELOCATED(new_hwcr_msr);
IMPORT_RELOCATED(new_int_halt_msr);

extern u8 smm_handler_start;
extern u8 smm_handler_end;

static struct e820entry *orig_e820_map;
static int orig_e820_len;

void tsc_wait(u32 mticks) {
    u64 count;
    u64 stop;

    count = rdtscll() >> 1;
    stop = count + ((u64)mticks << (20 - 1));
    while(stop > count) {
        count = rdtscll() >> 1;
    }
}

static void disable_xtpic(void) {
    inb(PIC_MASTER_IMR);
    outb(0xff, PIC_MASTER_IMR);
    inb(PIC_SLAVE_IMR);
    outb(0xff, PIC_SLAVE_IMR);
}

static void load_orig_e820_map(void)
{
    static com32sys_t rm;
    struct e820entry *e820_map;
    unsigned int e820_len = 0;
     
    e820_map = __com32.cs_bounce;
    memset(e820_map, 0, 4096);
    rm.eax.l = 0x0000e820;
    rm.edx.l = STR_DW_N("SMAP");
    rm.ebx.l = 0;
    rm.ecx.l = sizeof(struct e820entry);
    rm.edi.w[0] = OFFS(e820_map);
    rm.es = SEG(e820_map);
    __intcall(0x15, &rm, &rm);

    if (rm.eax.l == STR_DW_N("SMAP")) {
        /* Function supported. */
        printf("--- E820 memory map\n");
        e820_len = e820_len + rm.ecx.l;
        while (rm.ebx.l > 0) {
            rm.eax.l = 0x0000e820;
            rm.edx.l = STR_DW_N("SMAP");
            rm.ecx.l = sizeof(struct e820entry);
            rm.edi.w[0] = OFFS(e820_map) + e820_len;
            rm.es = SEG(e820_map);
            __intcall(0x15, &rm, &rm);

            e820_len += rm.ecx.l ? rm.ecx.l : sizeof(struct e820entry);
        }
      
        int i, j, len;
	struct e820entry elem;

	len = e820_len / sizeof(elem);
	/* Sort e820 map entries */
	for (j = 1; j < len; j++) {
	    memcpy(&elem, &e820_map[j], sizeof(elem));
	    for (i = j - 1; (i >= 0) && (e820_map[i].base > elem.base); i--)
		memcpy(&e820_map[i+1], &e820_map[i], sizeof(elem));
	    memcpy(&e820_map[i+1], &elem, sizeof(elem));
	}

        for (i = 0; i < len; i++) {
            printf(" %016llx - %016llx (%016llx) [%x]\n",
                   e820_map[i].base, e820_map[i].base+e820_map[i].length,
                   e820_map[i].length, e820_map[i].type);
        }

        orig_e820_map = malloc(e820_len);
        assert(orig_e820_map);
        memcpy(orig_e820_map, e820_map, e820_len);
        orig_e820_len = e820_len;
    }
}

static int install_e820_handler(void)
{
    u32 *int_vecs = 0x0;
    struct e820entry *e820;
    volatile u16 *bda_tom_lower = (u16 *)0x413;
    u32 tom_lower = *bda_tom_lower << 10;
    u32 relocate_size;
    int last_32b = -1;

    relocate_size = (&asm_relocate_end - &asm_relocate_start + 1023) / 1024;
    relocate_size *= 1024;
    asm_relocated = (void *)((tom_lower - relocate_size) & ~0xfff);
    /* http://groups.google.com/group/comp.lang.asm.x86/msg/9b848f2359f78cdf
     * *bda_tom_lower = ((u32)asm_relocated) >> 10; */

    memcpy(asm_relocated, &asm_relocate_start, relocate_size);

    e820 = (void *)REL(new_e820_map);

    unsigned int i, j = 0;
    for (i = 0; i < orig_e820_len / sizeof(struct e820entry); i++) {
        u64 orig_end = orig_e820_map[i].base + orig_e820_map[i].length;

	if ((orig_e820_map[i].base >> DRAM_MAP_SHIFT) > max_mem_per_node) {
	    /* Skip entry altogether */
	    continue;
	}

	if ((orig_end >> DRAM_MAP_SHIFT) > max_mem_per_node) {
	    /* Adjust length to fit */
	    printf("Master node exceeds cachable memory range, clamping...\n");
	    orig_end = (u64)max_mem_per_node << DRAM_MAP_SHIFT;
	    orig_e820_map[i].length = orig_end - orig_e820_map[i].base;
	}

	/* Reserve space for relocated code */
        if ((orig_end > (u32)asm_relocated) && (orig_end <= tom_lower)) {
	    /* Current entry ends within relocate space */
            if (orig_e820_map[i].base > (u32)asm_relocated)
                continue;
            e820[j].base = orig_e820_map[i].base;
            e820[j].length = 
                (u32)asm_relocated - orig_e820_map[i].base;
            e820[j].type = orig_e820_map[i].type;
            j++;
        }
        else if ((orig_e820_map[i].base > (u32)asm_relocated) &&
                 (orig_e820_map[i].base < tom_lower))
        {
	    /* Current entry starts within relocate space */
            e820[j].base = tom_lower;
            e820[j].length = orig_end - tom_lower;
            e820[j].type = orig_e820_map[i].type;
            j++;
        }
        else if ((orig_e820_map[i].base < (u32)asm_relocated) &&
                 (orig_end > tom_lower))
        {
	    /* Current entry covers relocate space */
            e820[j].base = orig_e820_map[i].base;
            e820[j].length = 
                (u32)asm_relocated - orig_e820_map[i].base;
            e820[j].type = orig_e820_map[i].type;
            j++;

            e820[j].base = tom_lower;
            e820[j].length = orig_end - tom_lower;
            e820[j].type = orig_e820_map[i].type;
            j++;
        }
        else {
            e820[j].base = orig_e820_map[i].base;
            e820[j].length = orig_e820_map[i].length;
            e820[j].type = orig_e820_map[i].type;
            j++;
        }

	if ((orig_end < 0x100000000ULL) &&
	    (orig_e820_map[i].length > TABLE_AREA_SIZE) &&
	    (orig_e820_map[i].type == 1))
	    last_32b = j-1;
    }

    free(orig_e820_map);

    *((u16 *)REL(new_e820_len))  = j;
    *((u32 *)REL(old_int15_vec)) = int_vecs[0x15];

    if (last_32b < 0) {
	printf("*** Unable to allocate room for ACPI tables\n");
	return 0;
    }
    else {
	e820[last_32b].length -= TABLE_AREA_SIZE;
	tables_relocated = (void *)(long)e820[last_32b].base + (long)e820[last_32b].length;
    }
    int_vecs[0x15] = (((u32)asm_relocated)<<12)|
        ((u32)(&new_e820_handler_relocate - &asm_relocate_start));

    printf("Persistent code relocated to %p\n", asm_relocated);
    printf("Allocating ACPI tables at %p\n", tables_relocated);
    return 1;
}

static void update_e820_map(void)
{
    u64 prev_end, rest;
    unsigned int i, j, max;
    struct e820entry *e820;
    u16 *len;

    e820 = (void *)REL(new_e820_map);
    len  = (u16 *)REL(new_e820_len);

    prev_end = 0;
    max = 0;
    for (i = 0; i < *len; i++) {
	if (prev_end < e820[i].base + e820[i].length) {
	    max = i;
	    prev_end = e820[max].base + e820[max].length;
	}
    }

    /* Truncate to SCI 000/HT 0 end. Rest added below. */
    e820[max].length = ((u64)nc_node[0].ht[0].size << DRAM_MAP_SHIFT)
	- e820[max].base;

    prev_end = e820[max].base + e820[max].length;
    if (nc_node[0].nc_ht_id == 1) {
	/* Truncate SCI 000/HT 0 to SCC ATT granularity if only HT
	 * node on SCI 000.  Existing adjustment of ht_node_size
	 * handles rest */
	rest = prev_end & ((SCC_ATT_GRAN << DRAM_MAP_SHIFT) - 1);
	if (rest) {
	    printf("Deducting 0x%x from e820 entry...\n", (u32)rest);
	    e820[max].length -= rest;
	    prev_end -= rest;
	}
    }

    if ((trace_buf_size > 0) && (e820[max].length > trace_buf_size)) {
	e820[max].length -= trace_buf_size;
	trace_buf = e820[max].base + e820[max].length;
    }

    /* Add remote nodes */
    for (i = 0; i < dnc_node_count; i++) {
	for (j = 0; j < 8; j++) {
	    if ((i == 0) && (j == 0))
		continue;
	    if (!nc_node[i].ht[j].cpuid)
		continue;

            e820[*len].base   = ((u64)nc_node[i].ht[j].base << DRAM_MAP_SHIFT);
            e820[*len].length = ((u64)nc_node[i].ht[j].size << DRAM_MAP_SHIFT);
            e820[*len].type   = 1;
	    if (mem_offline && (i > 0)) {
		if (e820[*len].length > MIN_NODE_MEM)
		    e820[*len].length = MIN_NODE_MEM;
	    }
	    else {
		if ((trace_buf_size > 0) && (e820[*len].length > trace_buf_size))
		    e820[*len].length -= trace_buf_size;
	    }
	    prev_end = e820[*len].base + e820[*len].length;
            (*len)++;
	}
    }

    e820[*len].base   = DNC_MCFG_BASE;
    e820[*len].length = DNC_MCFG_LIM - DNC_MCFG_BASE + 1;
    e820[*len].type   = 2;
    (*len)++;
    
    printf("Updated E820 map:\n");
    for (i = 0; i < *len; i++) {
	printf(" %016llx - %016llx (%016llx) [%x]\n",
	       e820[i].base, e820[i].base+e820[i].length,
	       e820[i].length, e820[i].type);
    }
}

static void load_existing_apic_map(void)
{
    acpi_sdt_p srat = find_sdt("SRAT");
    u16 apic_used[16];
    int i, c;
    memset(post_apic_mapping, ~0, sizeof(post_apic_mapping));
    memset(apic_used, 0, sizeof(apic_used));
    i = 12;
    c = 0;
    while (i + sizeof(*srat) < srat->len) {
	if (srat->data[i] == 0) {
	    struct acpi_core_affinity *af = 
		(struct acpi_core_affinity *)&(srat->data[i]);
	    post_apic_mapping[c] = af->apic_id;
	    apic_used[af->apic_id >> 4] = 
		apic_used[af->apic_id >> 4] | (1 << (af->apic_id & 0xf));
	    c++;
	    i += af->len;
	}
	else if (srat->data[i] == 1) {
	    struct acpi_mem_affinity *af = 
		(struct acpi_mem_affinity *)&(srat->data[i]);
	    i += af->len;
	}
	else {
	    break;
	}
    }

    /* Use APIC ATT map as scracth area to communicate APIC maps to master */
    dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00000020); /* Select APIC ATT */
    for (i = 0; i < 16; i++)
	dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i*4, apic_used[i]);
}

static int linear_hops(int src, int dst, int size) {
    if (src == dst)
	return 0;

    int fw = ((size - src) + dst) % size;
    int bw = ((src + (size - dst)) + size) % size;

    return min(fw, bw);
}

static int dist_fn(int src_node, int src_ht, int dst_node, int dst_ht)
{
    const int socket_cost = 5;	/* Typically 2.2GHz HyperTransport link */
    const int nc_cost = 90;	/* Fixed NC costs */
    const int fabric_cost = 20;	/* Scaled variable NC costs: 2xSERDES latency + route lookup */
    int total_cost = 10;	/* Memory-controller + SDRAM latency */

    int src_sci = cfg_nodelist[src_node].sciid;
    int dst_sci = cfg_nodelist[dst_node].sciid;
    int size = cfg_fabric.z_size << 8 | cfg_fabric.y_size << 4 | cfg_fabric.x_size;
    int hops = 0;

    /* Sum hops per axis */
    for (int dim = 0; dim < 3; dim++) {
	if (cfg_fabric.strict)
	    /* Compute shortest path */
	    hops += linear_hops(src_sci & 0xf, dst_sci & 0xf, size & 0xf);
	else
	    /* Assume average of half ring length */
	    hops += (src_sci == dst_sci) ? 0 : (size / 2);

	src_sci >>= 4;
	dst_sci >>= 4;
	size >>= 4;
    }

    if (hops) {
	/* Assume linear socket distance to NC */
	int src_socket_offset = abs(src_ht - nc_node[src_node].nc_neigh);
	int dst_socket_offset = abs(dst_ht - nc_node[dst_node].nc_neigh);

	total_cost += (src_socket_offset + dst_socket_offset) * socket_cost;
	total_cost += nc_cost; /* Fixed cost of NC */
	total_cost += hops * fabric_cost; /* Variable cost of NC */
    } else
	total_cost += abs(dst_ht - src_ht) * socket_cost;

    return total_cost;
}

static void disable_fixed_mtrrs(void)
{
    disable_cache();
    dnc_wrmsr(MSR_MTRR_DEFAULT, dnc_rdmsr(MSR_MTRR_DEFAULT) & ~(1 << 10));
    enable_cache();
}

static void enable_fixed_mtrrs(void)
{
    disable_cache();
    dnc_wrmsr(MSR_MTRR_DEFAULT, dnc_rdmsr(MSR_MTRR_DEFAULT) | (1 << 10));
    enable_cache();
}

static void update_acpi_tables(void) {
    acpi_sdt_p oroot;
    acpi_sdt_p osrat = find_sdt("SRAT");
    acpi_sdt_p oapic = find_sdt("APIC");
    acpi_sdt_p rsdt = (void *)tables_relocated;
    acpi_sdt_p xsdt = (void *)tables_relocated +   4 * 1024;
    acpi_sdt_p srat = (void *)tables_relocated +   8 * 1024;
    acpi_sdt_p slit = (void *)tables_relocated + 128 * 1024;
    acpi_sdt_p apic = (void *)tables_relocated + 256 * 1024;
    acpi_sdt_p mcfg = (void *)tables_relocated + 512 * 1024;
    u8 *dist;

    unsigned int i, j, apicid, pnum;
    unsigned int node, ht;

    /* Fixed MTRRs may mark the RSDT and XSDT pointers r/o */
    disable_fixed_mtrrs();

    /* replace_root may fail if rptr is r/o, so we read the pointers
     * back. In case of failure, we'll assume the existing rsdt/xsdt
     * tables can be extended where they are */
    oroot = find_root("RSDT");
    if (oroot) {
	memcpy(rsdt, oroot, oroot->len);
	replace_root("RSDT", rsdt);
	rsdt = find_root("RSDT");
    }
    else
	rsdt = NULL;

    oroot = find_root("XSDT");
    if (oroot) {
	memcpy(xsdt, oroot, oroot->len);
	replace_root("XSDT", xsdt);
	xsdt = find_root("XSDT");
    }
    else
	xsdt = NULL;

    if (!rsdt && !xsdt) {
	printf("Neither RSDT or XSDT found!\n");
	return;
    }

    if (!oapic) {
	printf("Default ACPI APIC table not found\n");
        return;
    }

    /* With APIC we reuse the old info and add our new entries */
    load_existing_apic_map();

    memcpy(apic, oapic, oapic->len);
    memcpy(apic->oemid, "NUMASC", 6);
    apic->len = 44;

    /* Apply enable mask to existing APICs, find first unused ACPI ProcessorId */
    pnum = 0;
    j = 0;
    for (i = 44; i < oapic->len; ) {
	struct acpi_local_apic *lapic = (void *)&oapic->data[i - sizeof(*oapic)];
	if (lapic->type != 0) {
	    memcpy(&apic->data[apic->len - sizeof(*apic)], lapic, lapic->len);
	    apic->len += lapic->len;
	}
	if (lapic->len == 0) {
	    printf("APIC entry at %p (offset %d) reports len 0, aborting!\n",
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
		}
		else {
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
    if (rsdt) replace_child("APIC", apic, rsdt, 4);
    if (xsdt) replace_child("APIC", apic, xsdt, 8);

    /* Make SLIT info from scratch (i.e replace existing table if any) */
    memcpy(slit->sig.s, "SLIT", 4);
    slit->len = 0;
    slit->revision = 1;
    memcpy(slit->oemid, "NUMASC", 6);
    memcpy(slit->oemtableid, "N313NUMA", 8);
    slit->oemrev = 1;
    memcpy(slit->creatorid, "1B47", 4);
    slit->creatorrev = 1;
    memset(slit->data, 0, 8); /* Number of System Localities */
    dist = (void *)&(slit->data[8]);

    int k,l, src_numa = 0, index = 0;

    printf("Updating SLIT table: (%s path)\nnode ", cfg_fabric.strict ? "shortest" : "unoptimised");

    for (i = 0; i < dnc_node_count; i++)
	for (j = 0; j < 8; j++) {
	    if (!nc_node[i].ht[j].cpuid)
		break;

	    printf("%3d ", src_numa++);
	}

    printf("\n");
    src_numa = 0;

    for (i = 0; i < dnc_node_count; i++)
	for (j = 0; j < 8; j++) {
	    if (!nc_node[i].ht[j].cpuid)
		break;

	    printf("%3d: ", src_numa++);

	    for (k = 0; k < dnc_node_count; k++) {
		for (l = 0; l < 8; l++) {
		    if (!nc_node[k].ht[l].cpuid)
			break;

		    printf("%3d ", dist[index++] = min(254, dist_fn(i, j, k, l))); /* 255 is unreachable */
		}
	    }

	    printf("\n");
	}

    memcpy(slit->data, &ht_pdom_count, sizeof(ht_pdom_count));
    slit->len = 44 + ht_pdom_count * ht_pdom_count;

    slit->checksum -= checksum(slit, slit->len);
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
    if (!osrat) {
	printf("Default ACPI SRAT table not found\n");
	return;
    }

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
	    mem.mem_base = (u64)nc_node[node].ht[ht].base << DRAM_MAP_SHIFT;
	    mem.mem_size = (u64)nc_node[node].ht[ht].size << DRAM_MAP_SHIFT;
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
		u16 apicid = j + nc_node[node].ht[ht].apic_base + nc_node[node].apic_offset;
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
		}
		else {
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
    
    if (rsdt) replace_child("SRAT", srat, rsdt, 4);
    if (xsdt) replace_child("SRAT", srat, xsdt, 8);

    /* MMCFG table */
    memset(mcfg, 0, sizeof(*mcfg) + 8);
    memcpy(mcfg->sig.s, "MCFG", 4);
    mcfg->len = 44;
    mcfg->revision = 1;
    memcpy(mcfg->oemid, "NUMASC", 6);
    memcpy(mcfg->oemtableid, "N313NUMA", 8);
    mcfg->oemrev = 0;
    memcpy(mcfg->creatorid, "1B47", 4);
    mcfg->creatorrev = 1;

    for (node = 0; node < dnc_node_count; node++) {
        struct acpi_mcfg_allocation mcfg_allocation;

        memset(&mcfg_allocation, 0, sizeof(mcfg_allocation));
        mcfg_allocation.address = DNC_MCFG_BASE | ((u64)nc_node[node].sci_id << 28ULL);
        mcfg_allocation.pci_segment = node;
        mcfg_allocation.start_bus_number = 0;
        mcfg_allocation.end_bus_number = 255;

        memcpy((unsigned char *)mcfg + mcfg->len, &mcfg_allocation, sizeof(mcfg_allocation));
        mcfg->len += sizeof(mcfg_allocation);
    }

    mcfg->checksum -= checksum(mcfg, mcfg->len);
    
    if (rsdt) replace_child("MCFG", mcfg, rsdt, 4);
    if (xsdt) replace_child("MCFG", mcfg, xsdt, 8);

    enable_fixed_mtrrs();
}

static struct mp_floating_pointer *find_mptable(void *start, int len)
{
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
    void *ebda = (void *)(*((unsigned short *)0x40e)*16);

    mpfp = find_mptable(ebda, 1024);
    if (!mpfp)
        mpfp = find_mptable((void *)(639*1024), 1024);
    if (!mpfp)
        mpfp = find_mptable((void *)(0xf0000), 0x10000);
    printf("mpfp: %p\n", mpfp);
    if (!mpfp)
        return;

    printf("sig: %.4s, len: %d, rev: %d, chksum: %x, feat: %02x:%02x:%02x:%02x:%02x\n",
           mpfp->sig.s, mpfp->len,  mpfp->revision,  mpfp->checksum,
           mpfp->feature[0], mpfp->feature[1], mpfp->feature[2],
           mpfp->feature[3], mpfp->feature[4]);

    if ((u32)mpfp > (u32)REL(new_mpfp)) {
        memcpy((void *)REL(new_mpfp), mpfp, sizeof(*mpfp));
        mpfp = (void *)REL(new_mpfp);
    }

    mptable = mpfp->mptable;
    printf("mptable @%p\n", mptable);

    memcpy((void *)REL(new_mptable), mptable, 512);
    mptable = (void *)REL(new_mptable);
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
    mptable->data[i+1] = 0xf;

    mptable->checksum -= checksum(mptable, mptable->len);
}

static void setup_apic_atts(void)
{
    u32 apic_shift;
    u16 i, j;
    
    apic_shift = 1;
    while (apic_per_node > (1 << apic_shift)) apic_shift++;
    if (apic_shift > 4)
	apic_shift = 4;
    printf("Using apic shift: %d (%d)\n", apic_shift, apic_per_node);
    
    /* Set APIC ATT for remote interrupts */
    for (i = 0; i < dnc_node_count; i++) {
        u16 snode = (i == 0) ? 0xfff0 : nc_node[i].sci_id;
	u16 dnode, ht;

        printf("Initializing sci node %03x APIC ATT tables...\n", nc_node[i].sci_id);
        dnc_write_csr(snode, H2S_CSR_G3_APIC_MAP_SHIFT,    apic_shift-1);
        dnc_write_csr(snode, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00000020); /* Select APIC ATT */

	for (j = 0; j < 64; j++) {
	    dnc_write_csr(snode, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + j*4, nc_node[0].sci_id);
	}
		    
	for (dnode = 0; dnode < dnc_node_count; dnode++) {
	    u16 cur, min, max;
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
	    for (j = min; j <= max; j++) {
		printf("Adding apic entry: %03x : %02x -> %03x\n", nc_node[i].sci_id, j*4, nc_node[dnode].sci_id);
		dnc_write_csr(snode, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + j*4, nc_node[dnode].sci_id);
	    }
	}
    }
}
    
static void add_scc_hotpatch_att(u64 addr, u16 node)
{
    u64 val;
    u32 base, lim;
    int i;

    if (scc_started) {
	u32 att_idx = dnc_read_csr(0xfff0, H2S_CSR_G0_ATT_INDEX);
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
	    if (val != node) {
		printf("add_att: Existing ATT entry differs from given node! %08x != %08x\n",
		       (u32)val, node);
	    }
	}
	dnc_write_csr(0xfff0, H2S_CSR_G0_ATT_ENTRY, node);
    }
    else {
	/* Set local ATT */
	dnc_write_csr(0xfff0, H2S_CSR_G0_ATT_INDEX,
		      (0x08000000 << 3) | /* Index Range (3 = 47:36) */
		      (addr >> 36)); /* Start index */
	dnc_write_csr(0xfff0, H2S_CSR_G0_ATT_ENTRY, node);

	for (i = 0; i < 8; i++) {
	    base = cht_read_config(0, NB_FUNC_MAPS, 0x40 + (8 * i));
	    lim = cht_read_config(0, NB_FUNC_MAPS, 0x44 + (8 * i));
	    if (base & 3) {
		base = (base >> 8) | (base & 3);
		lim = (lim >> 8) | (lim & 7);
	    }
	    else {
		base = 0;
		lim = 0;
	    }

	    cht_write_config(dnc_master_ht_id, 1, H2S_CSR_F1_RESOURCE_MAPPING_ENTRY_INDEX, i);
	    cht_write_config(dnc_master_ht_id, 1, H2S_CSR_F1_DRAM_LIMIT_ADDRESS_REGISTERS, lim);
	    cht_write_config(dnc_master_ht_id, 1, H2S_CSR_F1_DRAM_BASE_ADDRESS_REGISTERS, base);
	}
    }
}

static void disable_smm_handler(u64 smm_base)
{
    u64 val;
    u64 smm_addr;
    u16 node;
    u32 sreq_ctrl;
    int i;
    u8 *cur;

    if (!disable_smm)
	return;

    if (smm_base && (smm_base != ~0ULL))
	smm_base += 0x8000;
    else
	return;

    printf("Disabling SMM handler at %llx...\n", smm_base);
    smm_addr = 0x200000000000ULL | smm_base;

    val = dnc_read_csr(0xfff0, H2S_CSR_G0_NODE_IDS);
    node = (val >> 16) & 0xfff;

    for (i = 0; i < dnc_master_ht_id; i++) {
	add_extd_mmio_maps(0xfff0, i, 3, 0x200000000000ULL, 0x2fffffffffffULL,
			   dnc_master_ht_id);
    }

    add_scc_hotpatch_att(smm_addr, node);

    sreq_ctrl = dnc_read_csr(0xfff0, H2S_CSR_G3_SREQ_CTRL);
    dnc_write_csr(0xfff0, H2S_CSR_G3_SREQ_CTRL,
		  (sreq_ctrl & ~0xfff0) | (0xf00 << 4));

    /* val = mem64_read32(smm_addr);
       printf("MEM %llx: %08lx\n", smm_base, val); */
	
    cur = &smm_handler_start;
    while (cur + 4 <= &smm_handler_end) {
	mem64_write32(smm_addr, *((u32 *)cur));
	smm_addr += 4;
	cur += 4;
    }
    if (cur + 2 <= &smm_handler_end) {
	mem64_write16(smm_addr, *((u16 *)cur));
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
	del_extd_mmio_maps(0xfff0, i, 3);
}

static void setup_other_cores(void)
{
    u16 node = nc_node[0].sci_id;
    u32 ht, apicid, oldid, i, j;
    volatile u32 *icr;
    volatile u32 *apic;
    u64 val;

    /* Set H2S_Init */
    printf("Setting sci node %03x H2S_Init...\n", node);
    val = dnc_read_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL);
    dnc_write_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL, val | (1<<12));

    val = dnc_rdmsr(MSR_APIC_BAR);
    printf("MSR APIC_BAR: %012llx\n", val);
    apic = (void *)((u32)val & ~0xfff);
    icr = &apic[0x300/4];

    printf("apic: %08x, apicid: %08x, icr: %08x, %08x\n",
           (u32)apic, apic[0x20/4], (u32)icr, *icr);

    /* ERRATA #N28: Disable HT Lock mechanism. 
     * AMD Email dated 31.05.2011 :
     * There is a switch that can help with these high contention issues,
     * but it isn't "productized" due to a very rare potential for live lock if turned on.
     * Given that HUGE caveat, here is the information that I got from a good source:
     * LSCFG[44] =1 will disable it. MSR number is C001_1020 */
    val = dnc_rdmsr(MSR_LSCFG);
    val = val | (1ULL << 44);
    dnc_wrmsr(MSR_LSCFG, val);

    /* AMD Fam 15h Errata #572: Access to PCI Extended Configuration Space in SMM is Blocked
     * Suggested Workaround
     * BIOS should set MSRC001_102A[27] = 1b
     * We do this unconditionally (ie on fam10h as well) */
    val = dnc_rdmsr(MSR_CU_CFG2);
    val = val | (1ULL << 27);
    dnc_wrmsr(MSR_CU_CFG2, val);
   
    critical_enter();

    /* Start all local cores (not BSP) and let them run our init_trampoline */
    for (ht = 0; ht < 8; ht++) {
	for (i = 0; i < nc_node[0].ht[ht].cores; i++) {
	    if (!nc_node[0].ht[ht].cpuid)
		continue;
	    oldid = nc_node[0].ht[ht].apic_base + i;
	    if ((ht == 0) && (i == 0))
		continue; /* Skip BSP */

	    apicid = nc_node[0].apic_offset + oldid;
	    
	    *REL(cpu_apic_renumber) = apicid;
	    *REL(cpu_apic_hi)       = 0;
	    *REL(cpu_init_finished) = 0;
	    *((u64 *)REL(rem_topmem_msr)) = ~0ULL;
	    *((u64 *)REL(rem_smm_base_msr)) = ~0ULL;

	    apic[0x310/4] = oldid << 24;

	    printf("APIC %d ", apicid);

	    *icr = 0x00004500;
	    for (j = 0; j < BOOTSTRAP_DELAY; j++) {
		if (!(*icr & 0x1000))
		    break;
		tsc_wait(10);
	    }
	    if (*icr & 0x1000)
		printf("init IPI not delivered\n");

	    apic[0x310/4] = apicid << 24;
	    *icr = 0x00004600 | (((u32)REL(init_trampoline) >> 12) & 0xff);
	    for (j = 0; j < BOOTSTRAP_DELAY; j++) {
		if (!(*icr & 0x1000))
		    break;
		tsc_wait(10);
	    }
	    if (*icr & 0x1000)
		printf("startup IPI not delivered\n");
	    
	    for (j = 0; j < BOOTSTRAP_DELAY; j++) {
		if (*REL(cpu_init_finished))
		    break;
		tsc_wait(10);
	    }
	    if (*REL(cpu_init_finished)) {
		printf("reported done\n");
		if (*((u64 *)REL(rem_topmem_msr)) != *((u64 *)REL(new_topmem_msr)))
		    printf("Adjusted topmem from 0x%llx to 0x%llx\n",
			   *((u64 *)REL(rem_topmem_msr)), *((u64 *)REL(new_topmem_msr)));
		val = *((u64 *)REL(rem_smm_base_msr));
		disable_smm_handler(val);
	    }
	    else {
		printf("did not toggle init flag\n");
	    }
	}
    }

    critical_leave();
}

static void renumber_remote_bsp(u16 num)
{
    u8 i, j;
    u16 node = nc_node[num].sci_id;
    u8 maxnode = nc_node[num].nc_ht_id;
    u32 val;

    printf("[%04x#0] Renumbering BSP to HT#%d...\n", node, maxnode);

    for (i = 0; i < maxnode; i++) {
	val = dnc_read_conf(node, 0, 24+i, 0, 0x00);
	if ((val != 0x12001022) && (val != 0x16001022)) {
	    fprintf(stderr,
		    "[%04x#%x]F0x00 does not indicate an AMD Opteron CPU: 0x%08x\n",
		    node, i, val);
	    return;
	}

        /* Disable traffic distribution for now.. */
	dnc_write_conf(node, 0, 24+i, 0, 0x164, 0);
	
	/* Route maxnode + 1 as maxnode */
	val = dnc_read_conf(node, 0, 24+i, 0, 0x40 + 4 * maxnode);
	dnc_write_conf(node, 0, 24+i, 0, 0x44 + 4 * maxnode, val);
    }

    /* Bump NC to maxnode + 1 */
    dnc_write_conf(node, 0, 24+maxnode, 0, H2S_CSR_F0_CHTX_NODE_ID,
		   (maxnode << 8) | (maxnode + 1));
    val = dnc_read_csr(node, H2S_CSR_G3_HT_NODEID);
    printf("[%04x] Moving NC to HT#%d...\n", node, val);

    for (i = 0; i < maxnode; i++) {
	val = dnc_read_conf(node, 0, 24+i, 0, 0x68);
	dnc_write_conf(node, 0, 24+i, 0, 0x68, (val & ~(1<<15)) | 0x40f);

	/* Increase NodeCnt */
	val = dnc_read_conf(node, 0, 24+i, 0, 0x60);
	dnc_write_conf(node, 0, 24+i, 0, 0x60, val + 0x10);

	/* Route maxnode as 0 */
	val = dnc_read_conf(node, 0, 24+i, 0, 0x40);
	dnc_write_conf(node, 0, 24+i, 0, 0x40 + 4 * maxnode, val);
    }
	
    /* Renumber HT#0 */
    val = dnc_read_conf(node, 0, 24+0, 0, 0x60);
    dnc_write_conf(node, 0, 24+0, 0, 0x60,
		   (val & ~0xff0f) | (maxnode << 12) | (maxnode << 8) | maxnode);
    val = dnc_read_conf(node, 0, 24+maxnode, 0, 0x60);
    printf("[%04x#%x]F0x60: 0x%08x (BSP)...\n", node, maxnode, val);

    for (i = 1; i <= maxnode; i++) {
	/* Update LkNode, SbNode */
	val = dnc_read_conf(node, 0, 24+i, 0, 0x60);
	dnc_write_conf(node, 0, 24+i, 0, 0x60,
		       (val & ~0xff00) | (maxnode << 12) | (maxnode << 8));

	/* Update DRAM maps */
	for (j = 0; j < 8; j++) {
	    val = dnc_read_conf(node, 0, 24+i, 1, 0x44 + 8 * j);
	    if ((val & 7) == 0)
		dnc_write_conf(node, 0, 24+i, 1, 0x44 + 8 * j, val | maxnode);
	}

	/* Update MMIO maps */
	for (j = 0; j < 8; j++) {
	    val = dnc_read_conf(node, 0, 24+i, 1, 0x84 + 8 * j);
	    if ((val & 7) == 0)
		dnc_write_conf(node, 0, 24+i, 1, 0x84 + 8 * j, val | maxnode);
	}

	/* Update IO maps */
	for (j = 0; j < 4; j++) {
	    val = dnc_read_conf(node, 0, 24+i, 1, 0xc4 + 8 * j);
	    if ((val & 7) == 0)
		dnc_write_conf(node, 0, 24+i, 1, 0xc4 + 8 * j, val | maxnode);
	}

	/* Update CFG maps */
	for (j = 0; j < 4; j++) {
	    val = dnc_read_conf(node, 0, 24+i, 1, 0xe0 + 4 * j);
	    if (((val >> 4) & 7) == 0)
		dnc_write_conf(node, 0, 24+i, 1, 0xe0 + 4 * j, val | (maxnode << 4));
	}
    }
    
    for (i = 1; i <= maxnode; i++) {
	val = dnc_read_conf(node, 0, 24+i, 0, 0x00);
	if ((val != 0x12001022) && (val != 0x16001022)) {
	    fprintf(stderr,
		    "[%04x#%x]F0x00 does not indicate an AMD Opteron CPU: 0x%08x\n",
		    node, i, val);
	    return;
	}

	/* Route 0 as maxnode + 1 */
	val = dnc_read_conf(node, 0, 24+i, 0, 0x44 + 4 * maxnode);
	dnc_write_conf(node, 0, 24+i, 0, 0x40, val);
    }

    /* Move NC to HT#0, update SbNode, LkNode */
    dnc_write_conf(node, 0, 24+maxnode+1, 0, H2S_CSR_F0_CHTX_NODE_ID,
		   (maxnode << 24) | (maxnode << 16) | (maxnode << 8) | 0);
    val = dnc_read_csr(node, H2S_CSR_G3_HT_NODEID);
    printf("[%04x] Moving NC to HT#%d...\n", node, val);

    for (i = 1; i <= maxnode; i++) {
	/* Decrease NodeCnt */
	val = dnc_read_conf(node, 0, 24+i, 0, 0x60);
	dnc_write_conf(node, 0, 24+i, 0, 0x60, val - 0x10);
    }

    for (i = 1; i <= maxnode; i++) {
	/* Remote maxnode + 1 routing entry */
	dnc_write_conf(node, 0, 24+i, 0, 0x44 + 4 * maxnode, 0x40201);
    }

    for (i = 1; i <= maxnode; i++) {
	/* Reenable probes */
	val = dnc_read_conf(node, 0, 24+i, 0, 0x68);
	dnc_write_conf(node, 0, 24+i, 0, 0x68, (val & ~0x40f) | (1 << 15));
    }

    val = dnc_read_conf(node, 0, 24+0, 0, H2S_CSR_F0_CHTX_NODE_ID);
    printf("Done\n");

    memcpy(&nc_node[num].ht[maxnode], &nc_node[num].ht[0], sizeof(ht_node_info_t));
    nc_node[num].ht[0].cpuid = 0;
    nc_node[num].nc_ht_id = 0;
}

static void setup_remote_cores(u16 num)
{
    u8 i, map_index;
    u16 node = nc_node[num].sci_id;
    u8 ht_id = nc_node[num].nc_ht_id;
    nc_node_info_t *cur_node = &nc_node[num];
    u16 ht, apicid, oldid;
    u32 j;
    u32 val;
    u64 tom;
    u64 qval;

    printf("Setting up cores on node #%d (0x%03x), %d HT nodes\n",
           num, node, ht_id);
    
    /* Toggle go-ahead flag to remote node */
    printf("Checking if SCI node %03x is ready\n", node);
    do {
        val = dnc_read_csr(node, H2S_CSR_G3_FAB_CONTROL);
        tsc_wait(200);
    } while (!(val & 0x40000000UL));

    val |= 0x80000000UL;
    dnc_write_csr(node, H2S_CSR_G3_FAB_CONTROL, val);
    printf("Waiting for SCI node %03x to acknowledge\n", node);
    while (val & 0x80000000UL) {
        val = dnc_read_csr(node, H2S_CSR_G3_FAB_CONTROL);
        tsc_wait(200);
    }

    /* Map MMIO 0x00000000 - 0xffffffff to master node */
    for (j = 0; j < 0x1000; j++) {
        if ((j & 0xff) == 0) {
	    if (verbose > 0)
		printf("Setting remote MMIO32 maps page %d...\n", j >> 8);
            dnc_write_csr(node, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00000010 | j >> 8);
        }
        dnc_write_csr(node, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + (j & 0xff) * 4,
                      nc_node[0].sci_id);
    }

    if (renumber_bsp)
	renumber_remote_bsp(num);
    ht_id = nc_node[num].nc_ht_id;

    printf("Remote MMIO32 maps set...\n");

    /* Set H2S_Init */
    printf("Setting SCI node %03x H2S_Init...\n", node);
    val = dnc_read_csr(node, H2S_CSR_G3_HREQ_CTRL);
    dnc_write_csr(node, H2S_CSR_G3_HREQ_CTRL, val | (1<<12));

    /* Insert coverall MMIO maps */
    tom = dnc_rdmsr(MSR_TOPMEM) >> 8;
    printf("Inserting coverall MMIO maps on sci node %03x\n", node);
    for (i = 0; i < 8; i++) {
	if (!cur_node->ht[i].cpuid)
	    continue;
        dnc_write_conf(node, 0, 24+i, NB_FUNC_MAPS, 0x84, 0x00000f00 | ht_id);
        dnc_write_conf(node, 0, 24+i, NB_FUNC_MAPS, 0x80, 0x00000a03);
        for (j = 1; j < 8; j++) {
            dnc_write_conf(node, 0, 24+i, NB_FUNC_MAPS, 0x80 + j*8, 0x0);
            dnc_write_conf(node, 0, 24+i, NB_FUNC_MAPS, 0x84 + j*8, 0x0);
        }
        dnc_write_conf(node, 0, 24+i, NB_FUNC_MAPS, 0x8c, 0x00ffff00 | ht_id);
        dnc_write_conf(node, 0, 24+i, NB_FUNC_MAPS, 0x88, tom | 3);
    
	/* Enable redirect of VGA to master, default disable where local cores will access local VGA on each node */
        if (enable_vga_redir) {
            /* Apparently the HP DL165 modes can't handle non-posted writes to the VGA ports...
             * Make sure the VGA Enable register is disabled to forward VGA transactions
             * (MMIO A_0000h - B_FFFFh and I/O 3B0h - 3BBh or 3C0h - 3DFh) to the NumaChip */
            dnc_write_conf(node, 0, 24+i, NB_FUNC_MAPS, 0xf4, 0x0);
        }
    }

    critical_enter();

    /* Now, reset all DRAM maps */
    printf("Resetting DRAM maps on sci node %03x\n", node);
    for (i = 0; i < 8; i++) {
	if (!cur_node->ht[i].cpuid)
	    continue;
        /* Re-direct everything below our first local address to NumaChip */
        dnc_write_conf(node, 0, 24+i, NB_FUNC_MAPS, 0x44,
                       ((cur_node->ht[0].base - 1) << 16) | ht_id);
        dnc_write_conf(node, 0, 24+i, NB_FUNC_MAPS, 0x40,
                       (nc_node[0].ht[0].base << 16) | 3);
        dnc_write_conf(node, 0, 24+i, NB_FUNC_MAPS, 0xf0,  0);
        /* Clear all other entries for now; we'll get to them later */
        for (j = 1; j < 8; j++) {
            dnc_write_conf(node, 0, 24+i, NB_FUNC_MAPS, 0x40 + j*8, 0);
            dnc_write_conf(node, 0, 24+i, NB_FUNC_MAPS, 0x44 + j*8, 0);
        }
    }

    /* Reprogram HT node "self" ranges */
    printf("Reprogramming HT node \"self\" ranges on sci node %03x\n", node);
    for (i = 0; i < 8; i++) {
	if (!cur_node->ht[i].cpuid)
	    continue;
        dnc_write_conf(node, 0, 24+i, NB_FUNC_MAPS, 0x120,
                       cur_node->ht[i].base >> (27 - DRAM_MAP_SHIFT));
        dnc_write_conf(node, 0, 24+i, NB_FUNC_MAPS, 0x124,
                       (cur_node->ht[i].base + cur_node->ht[i].size - 1) >> (27 - DRAM_MAP_SHIFT));
	val = dnc_read_conf(node, 0, 24+i, NB_FUNC_DRAM, 0x110);
	if (val & 1) {
	    /* Reprogram DCT base/offset values */
	    dnc_write_conf(node, 0, 24+i, NB_FUNC_DRAM, 0x110, (val & ~0xfffff800) |
			   ((cur_node->ht[i].base >> (27 - DRAM_MAP_SHIFT)) << 11));
	    dnc_write_conf(node, 0, 24+i, NB_FUNC_DRAM, 0x114, 
			   (cur_node->ht[i].base >> (26 - DRAM_MAP_SHIFT)) << 10);
	    printf("[%03x#%d] F2x110: %08x, F2x114: %08x\n",
		   node, i,
		   dnc_read_conf(node, 0, 24+i, NB_FUNC_DRAM, 0x110),
		   dnc_read_conf(node, 0, 24+i, NB_FUNC_DRAM, 0x114));
	}
    }

    /* Program our local ranges */
    for (map_index = 0, i = 0; i < 8; i++) {
	if (!cur_node->ht[i].cpuid)
	    continue;
	for (j = 0; j < 8; j++) {
	    if (!cur_node->ht[j].cpuid)
		continue;
            dnc_write_conf(node, 0, 24+j, NB_FUNC_MAPS, 0x4c + map_index*8,
                           ((cur_node->ht[i].base + cur_node->ht[i].size - 1) << 16) | i);
            dnc_write_conf(node, 0, 24+j, NB_FUNC_MAPS, 0x48 + map_index*8,
                           (cur_node->ht[i].base << 16) | 3);
        }
        dnc_write_conf(node, 0, 24+ht_id, NB_FUNC_MAPS, H2S_CSR_F1_RESOURCE_MAPPING_ENTRY_INDEX,
                       map_index);
        dnc_write_conf(node, 0, 24+ht_id, NB_FUNC_MAPS, H2S_CSR_F1_DRAM_LIMIT_ADDRESS_REGISTERS,
                       ((cur_node->ht[i].base + cur_node->ht[i].size - 1) << 8) | i);
        dnc_write_conf(node, 0, 24+ht_id, NB_FUNC_MAPS, H2S_CSR_F1_DRAM_BASE_ADDRESS_REGISTERS,
                       (cur_node->ht[i].base << 8) | 3);
        map_index++;
    }

    /* Re-direct everything above our last local address (if any) to NumaChip */
    if (num != dnc_node_count-1) {
	if (map_index > 6)
	    printf("Error: Too many DRAM maps on SCI%03x, cannot fit last overflow map\n", node);

	for (i = 0; i < 8; i++) {
	    if (!cur_node->ht[i].cpuid)
		continue;
            dnc_write_conf(node, 0, 24+i, NB_FUNC_MAPS, 0x4c + map_index*8,
                           ((nc_node[dnc_node_count-1].addr_end - 1) << 16) | ht_id);
            dnc_write_conf(node, 0, 24+i, NB_FUNC_MAPS, 0x48 + map_index*8,
                           (cur_node->addr_end << 16) | 3);
        }
    }

    critical_leave();

    if (verbose > 0) {
	for (i = 0; i < 8; i++) {
	    if (!cur_node->ht[i].cpuid)
		continue;
	    for (j = 0; j < 8; j++) {
		printf("SCI%03x/HT#%d DRAM base/limit[%d] %08x/%08x\n",
		       node, i, j,
		       dnc_read_conf(node, 0, 24+i, NB_FUNC_MAPS, 0x40 + j*8),
		       dnc_read_conf(node, 0, 24+i, NB_FUNC_MAPS, 0x44 + j*8));
	    }
	}
    }

    printf("%03x/G3xPCI_SEG0: %x\n", node, dnc_read_csr(node, H2S_CSR_G3_PCI_SEG0));
    dnc_write_csr(node, H2S_CSR_G3_PCI_SEG0, nc_node[0].sci_id << 16);
    printf("%03x/G3xPCI_SEG0: %x\n", node, dnc_read_csr(node, H2S_CSR_G3_PCI_SEG0));

    /* Quick and dirty: zero out I/O and config space maps; add
     * all-covering map towards DNC */
    for (i = 0; i < 8; i++) {
	if (!cur_node->ht[i].cpuid)
	    continue;
        for (j = 0xc0; j < 0xf0; j += 4) {
            dnc_write_conf(node, 0, 24+i, NB_FUNC_MAPS, j, 0);
        }
        dnc_write_conf(node, 0, 24+i, NB_FUNC_MAPS, 0xc4, 0x00fff000 | ht_id);
        dnc_write_conf(node, 0, 24+i, NB_FUNC_MAPS, 0xc0, 0x00000003);
        dnc_write_conf(node, 0, 24+i, NB_FUNC_MAPS, 0xe0, 0xff000003 | (ht_id << 4));
    }

    dnc_write_csr(node, H2S_CSR_G0_MIU_NGCM0_LIMIT, cur_node->addr_base >> 6);
    dnc_write_csr(node, H2S_CSR_G0_MIU_NGCM1_LIMIT, (cur_node->addr_end >> 6) - 1);
    printf("%03x/NGCM0: %x\n", node, dnc_read_csr(node, H2S_CSR_G0_MIU_NGCM0_LIMIT));
    printf("%03x/NGCM1: %x\n", node, dnc_read_csr(node, H2S_CSR_G0_MIU_NGCM1_LIMIT));

    dnc_write_csr(node, H2S_CSR_G3_DRAM_SHARED_BASE, cur_node->addr_base);
    dnc_write_csr(node, H2S_CSR_G3_DRAM_SHARED_LIMIT, cur_node->addr_end);

    /* Redo the Global CSR and MMCFG maps here just in case master and slave are out of sync */
    for (i = 0; i < 8; i++) {
	if (!cur_node->ht[i].cpuid)
	    continue;
        add_extd_mmio_maps(node, i, 0, DNC_CSR_BASE, DNC_CSR_LIM, ht_id);
        add_extd_mmio_maps(node, i, 1, DNC_MCFG_BASE, DNC_MCFG_LIM, ht_id);
    }

    /* "Wraparound" entry, lets APIC 0xff00 - 0xffff target 0x0 to 0xff on destination node */
    dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x0000002f);
    i = dnc_read_csr(0xfff0, H2S_CSR_G3_APIC_MAP_SHIFT) + 1;
    for (j = (0xff00 >> i) & 0xff; j < 0x100; j++) {
	dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + j * 4, node);
    }
    printf("int_status: %x\n", dnc_read_csr(0xfff0, H2S_CSR_G3_EXT_INTERRUPT_STATUS));

    tsc_wait(200);
    
    *((u64 *)REL(new_mcfg_msr)) = DNC_MCFG_BASE | ((u64)node << 28ULL) | 0x21ULL;

    critical_enter();

    /* Start all remote cores and let them run our init_trampoline */
    for (ht = 0; ht < 8; ht++) {
	for (i = 0; i < cur_node->ht[ht].cores; i++) {
	    if (!cur_node->ht[ht].cpuid)
		continue;
	    oldid = cur_node->ht[ht].apic_base + i;
	    apicid = cur_node->apic_offset + oldid;
	
	    *REL(cpu_apic_renumber) = apicid & 0xff;
	    *REL(cpu_apic_hi)       = (apicid >> 8) & 0x3f;
	    *REL(cpu_init_finished) = 0;
	    *((u64 *)REL(rem_topmem_msr)) = ~0ULL;
	    *((u64 *)REL(rem_smm_base_msr)) = ~0ULL;

	    dnc_write_csr(0xfff0, H2S_CSR_G3_EXT_INTERRUPT_GEN, 0xff001500 | (oldid<<16));
	    tsc_wait(50);
	    dnc_write_csr(0xfff0, H2S_CSR_G3_EXT_INTERRUPT_GEN,
			  0xff002600 | (oldid<<16) | (((u32)REL(init_trampoline) >> 12) & 0xff));
	    for (j = 0; j < BOOTSTRAP_DELAY; j++) {
		if (*REL(cpu_init_finished))
		    break;
		tsc_wait(10);
	    }

	    if (*REL(cpu_init_finished)) {
		printf("APIC %d (%d) reported done\n", apicid, oldid);
		if (*((u64 *)REL(rem_topmem_msr)) != *((u64 *)REL(new_topmem_msr)))
		    printf("Adjusted topmem value from %llx to %llx\n",
			   *((u64 *)REL(rem_topmem_msr)), *((u64 *)REL(new_topmem_msr)));
		qval = *((u64 *)REL(rem_smm_base_msr));
		disable_smm_handler(qval);
	    }
	    else {
		printf("APIC %d (%d) did not toggle init flag\n", apicid, oldid);
	    }
	}
    }

    critical_leave();
}

static void setup_local_mmio_maps(void)
{
    int i, j, next;
    u64 tom;
    u32 base[8];
    u32 lim[8];
    u32 dst[8];
    u32 curbase, curlim, curdst;
    unsigned int sbnode;

    printf("Setting MMIO maps on local DNC...\n");

    for (i = 0; i < 8; i++) {
	base[i] = 0;
	lim[i] = 0;
	dst[i] = 0;
    }

    tom = dnc_rdmsr(MSR_TOPMEM);
    if (tom >= 0x100000000) {
	printf("Error: TOP_MEM above 4G boundary\n");
	return;
    }

    sbnode = (cht_read_config(0, NB_FUNC_HT, 0x60) >> 8) & 7;
    base[0] = (tom >> 8) & ~0xff;
    lim[0] = 0x00ffff00;
    dst[0] = (sbnode << 8) | 3;
    next = 1;

    /* Apply default maps so we can bail without losing all hope */
    for (i = 0; i < 8; i++) {
	cht_write_config_nc(dnc_master_ht_id, 1, nc_neigh, nc_neigh_link,
			    H2S_CSR_F1_RESOURCE_MAPPING_ENTRY_INDEX, i);
	cht_write_config_nc(dnc_master_ht_id, 1, nc_neigh, nc_neigh_link,
			    H2S_CSR_F1_MMIO_LIMIT_ADDRESS_REGISTERS, lim[i] | (dst[i] >> 8));
	cht_write_config_nc(dnc_master_ht_id, 1, nc_neigh, nc_neigh_link,
			    H2S_CSR_F1_MMIO_BASE_ADDRESS_REGISTERS, base[i] | (dst[i] & 0x3));
    }

    for (i = 0; i < 8; i++) {
        curbase = cht_read_config(sbnode, NB_FUNC_MAPS, 0x80 + i*8);
        curlim = cht_read_config(sbnode, NB_FUNC_MAPS, 0x84 + i*8);
	curdst = ((curlim & 0x7) << 8) | (curbase & 0x3);
	/* This strips NP-bit */
	curbase = curbase & ~0xff;
	curlim = curlim & ~0xff;
	if (verbose > 0)
	    printf("CPU MMIO region #%d base: %08x, lim: %08x, dst: %04x\n",
		   i, curbase, curlim, curdst);
	if (curdst & 3) {
	    int found = 0;
	    int placed = 0;
	    for (j = 0; j < next; j++) {
		if (((curbase < base[j]) && (curlim > base[j])) ||
		    ((curbase < lim[j])  && (curlim > lim[j])))
		{
		    printf("MMIO range #%d (%x-%x) overlaps registered window #%d (%x-%x)!\n",
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
		    }
		    else {
			/* Equal base */
			base[j] = curlim + 0x100;
		    }
		    break;
		}
		else if ((curbase > base[j]) && (curbase <= lim[j])) {
		    found = 1;
		    if ((curdst >> 8) == sbnode) {
			placed = 1;
		    }
		    else if (curlim == lim[j]) {
			/* Equal limit */
			lim[j] = curbase - 0x100;
		    }
		    else {
			/* Enclosed region */
			if (next >= 8) {
			    printf("Ran out of MMIO regions trying to place #%d (%x-%x)!\n",
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
		}
		else if ((curbase < 0x1000) && (curlim < 0x1000)) {
		    /* Sub-1M ranges */
		    found = 1;
		}
	    }
	    if (found) {
		if (!placed) {
		    if (next >= 8) {
			printf("Ran out of MMIO regions trying to place #%d (%x-%x)!\n",
			       i, curbase, curlim);
			return;
		    }

		    base[next] = curbase;
		    lim[next] = curlim;
		    dst[next] = curdst;
		    next++;
		}
	    }
	    else {
		printf("Enclosing window not found for MMIO range #%d (%x-%x)!\n",
		       i, curbase, curlim);
		return;
	    }
	}
    }
    if (verbose > 0)
	for (i = 0; i < 8; i++) {
	    printf("NC MMIO region #%d base: %08x, lim: %08x, dst: %04x\n", i, base[i], lim[i], dst[i]);
	    cht_write_config_nc(dnc_master_ht_id, 1, nc_neigh, nc_neigh_link,
				H2S_CSR_F1_RESOURCE_MAPPING_ENTRY_INDEX, i);
	    cht_write_config_nc(dnc_master_ht_id, 1, nc_neigh, nc_neigh_link,
				H2S_CSR_F1_MMIO_LIMIT_ADDRESS_REGISTERS, lim[i] | (dst[i] >> 8));
	    cht_write_config_nc(dnc_master_ht_id, 1, nc_neigh, nc_neigh_link,
				H2S_CSR_F1_MMIO_BASE_ADDRESS_REGISTERS, base[i] | (dst[i] & 0x3));
	}
}

static int read_file(const char *filename, void *buf, int bufsz)
{
    static com32sys_t inargs, outargs;
    int fd, len, bsize, blocks;

    if (bufsz < (int)(strlen(filename) + 1)) {
        printf("Buffer too small (%d bytes)\n", bufsz);
        return -1;
    }
    
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
        printf("File not found!\n");
        return -1;
    }

    if ((len+1) > bufsz) {
        printf("File to large (%d bytes)\n", len);
        return -1;
    }

    printf("Ok\n");

    memset(buf, 0, len+1);
    
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

static int convert_buf_u32(char *src, u32 *dst, int max_offset)
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

    return offs-1;
}

static int convert_buf_u16(char *src, u16 *dst, int max_offset)
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

    return offs-1;
}

static u32 mseq_ucode_update[1024];
static u16 mseq_table_update[128];

static void read_microcode_update(void)
{
    char path[512];
    int psep;
    int ucode_len, table_len;
    u32 ucode_xor, table_xor;
    u16 i;

    ucode_xor = 0;
    for (i = 0; i < mseq_ucode_length; i++) {
        ucode_xor = ucode_xor ^ (i * mseq_ucode[i]);
    }

    table_xor = 0;
    for (i = 0; i < mseq_table_length; i++) {
        table_xor = table_xor ^ (i * mseq_table[i]);
    }

    memset(path, 0, sizeof(path));
    strncpy(path, microcode_path, sizeof(path));
    psep = strlen(path);
    if (psep == 0) {
        printf("Microcode update not specified, continuing with built-in version (xor = %u, %u)\n",
               ucode_xor, table_xor);
        return;
    }
    
    if ((psep > 0) && (path[psep-1] != '/'))
	path[psep++] = '/';

    strcat(path, "mseq.code");
    ucode_len = read_file(path, __com32.cs_bounce, __com32.cs_bounce_size);
    if (ucode_len < 0) {
        printf("Microcode update not found, continuing with built-in version (xor = %u, %u)\n",
               ucode_xor, table_xor);
        return;
    }

    ucode_len = convert_buf_u32(__com32.cs_bounce, mseq_ucode_update, 1024);
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

    table_len = convert_buf_u16(__com32.cs_bounce, mseq_table_update, 128);
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
        ucode_xor = ucode_xor ^ (i * mseq_ucode[i]);
    }

    table_xor = 0;
    for (i = 0; i < mseq_table_length; i++) {
        table_xor = table_xor ^ (i * mseq_table[i]);
    }

    printf("Using updated microcode (xor = %u, %u)\n", ucode_xor, table_xor);
}

int read_config_file(char *file_name)
{
    int config_len;
    char *config;

    config_len = read_file(file_name, __com32.cs_bounce, __com32.cs_bounce_size);
    if (config_len < 0) {
	printf("Error: Fabric configuration file <%s> not found\n", file_name);
        return -1;
    }

    config = __com32.cs_bounce;
    config[config_len] = '\0';

    if (!parse_config_file(config))
	return -1;

    return 0;
}

static int pxeapi_call(int func, u8 *buf)
{
    static com32sys_t inargs, outargs;

    inargs.eax.w[0] = 0x0009; /* Call PXE Stack */
    inargs.ebx.w[0] = func; /* PXE function number */
    inargs.esi.w[0] = OFFS(buf);
    inargs.es = SEG(buf);
    __intcall(0x22, &inargs, &outargs);
    
    return outargs.eax.w[0] == PXENV_EXIT_SUCCESS;
}

int udp_open(void)
{
    t_PXENV_TFTP_CLOSE *tftp_close_param;
    t_PXENV_UDP_OPEN *pxe_open_param;
    pxe_bootp_t *buf;
    in_addr_t srcip;
    size_t len;

    tftp_close_param = __com32.cs_bounce;
    memset(tftp_close_param, 0, sizeof(*tftp_close_param));
    pxeapi_call(PXENV_TFTP_CLOSE, (u8 *)tftp_close_param);
    printf("TFTP close returns: %d\n", tftp_close_param->Status);

    srcip = 0xffffffff;
    if (pxe_get_cached_info(2, (void **)&buf, &len) >= 0) {
	srcip = buf->yip;
	free(buf);
    }

    pxe_open_param = __com32.cs_bounce;
    memset(pxe_open_param, 0, sizeof(*pxe_open_param));
    pxe_open_param->src_ip = srcip;
    pxeapi_call(PXENV_UDP_OPEN, (u8 *)pxe_open_param);
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
    
    pxeapi_call(PXENV_UDP_WRITE, (u8 *)pxe_write_param);
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
    pxe_read_param->buffer.seg = SEG(buf_reloc);
    pxe_read_param->buffer.offs = OFFS(buf_reloc);
    pxe_read_param->buffer_size = len;
	
    pxeapi_call(PXENV_UDP_READ, (u8 *)pxe_read_param);
    if ((pxe_read_param->status == PXENV_STATUS_SUCCESS) &&
	(pxe_read_param->s_port == htons(4711)))
    {
	memcpy(buf, buf_reloc, pxe_read_param->buffer_size);
	return pxe_read_param->buffer_size;
    }
    else {
	return 0;
    }
}

static void wait_status(void)
{
    printf("Waiting for");

    /* Skip first node */
    for (int i = 1; i < cfg_nodes; i++)
	if (nodedata[cfg_nodelist[i].sciid] != 0x80)
	    printf(" 0x%03x (%s)",
		   cfg_nodelist[i].sciid, cfg_nodelist[i].desc);
    printf("\n");
}

static void wait_for_slaves(struct node_info *info, struct part_info *part)
{
    struct state_bcast cmd, rsp;
    int ready_pending = 1;
    int count, backoff, last_stat;
    int do_restart;
    enum node_state waitfor, own_state;
    u32 last_cmd = ~0;
    int len, i;
    int handle;

    handle = udp_open();
    cmd.state = CMD_STARTUP;
    cmd.uuid  = info->uuid;
    cmd.sciid = info->sciid;
    cmd.tid   = 0; /* Must match initial rsp.tid for RSP_SLAVE_READY */
    waitfor = RSP_SLAVE_READY;
    do_restart = 0;

    printf("Waiting for %d nodes...\n", cfg_nodes - 1);

    count = 0;
    backoff = 1;
    last_stat = 0;
    while (1) {
	if (++count >= backoff) {
	    if (cmd.state != CMD_STARTUP)
		udp_broadcast_state(handle, &cmd, sizeof(cmd));
	    tsc_wait(100 * backoff);
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
		    wait_status();
		}
		continue;
	    }
	} else
	    len = 0;

	if (len == sizeof(rsp)) {
	    int ours = 0;
	    for (i = 0; i < cfg_nodes; i++)
		if (cfg_nodelist[i].uuid == rsp.uuid) {
		    ours = 1;
		    break;
		}

	    if (ours) {
		if ((rsp.state == waitfor) && (rsp.tid == cmd.tid)) {
		    if (nodedata[rsp.sciid] != 0x80) {
			printf("Node 0x%03x (%s) entered %s\n",
			       rsp.sciid, cfg_nodelist[i].desc,
			       node_state_name[waitfor]);
			nodedata[rsp.sciid] = 0x80;
		    }
		}
		else if ((rsp.state == RSP_PHY_NOT_TRAINED) ||
			 (rsp.state == RSP_RINGS_NOT_OK) ||
			 (rsp.state == RSP_FABRIC_NOT_READY) ||
			 (rsp.state == RSP_FABRIC_NOT_OK))
		{
		    if (nodedata[rsp.sciid] != 0x80) {
			printf("Node 0x%03x (%s) aborted! Restarting process...\n",
			       rsp.sciid, cfg_nodelist[i].desc);
			do_restart = 1;
			nodedata[rsp.sciid] = 0x80;
		    }
		}
	    }
	}
	ready_pending = 0;
        for (i = 0; i < cfg_nodes; i++) {
            if (cfg_nodelist[i].uuid == info->uuid) /* Self */
                continue;
	    if (!(nodedata[cfg_nodelist[i].sciid] & 0x80)) {
		ready_pending = 1;
		break;
	    }
        }

	if (!ready_pending || do_restart) {
	    if (do_restart) {
		cmd.state = CMD_ENTER_RESET;
		waitfor = RSP_RESET_ACTIVE;
		do_restart = 0;
	    }
	    else if (cmd.state == CMD_STARTUP) {
		cmd.state = CMD_ENTER_RESET;
		waitfor = RSP_RESET_ACTIVE;
	    }
	    else if (cmd.state == CMD_ENTER_RESET) {
		cmd.state = CMD_RELEASE_RESET;
		waitfor = RSP_PHY_TRAINED;
	    }
	    else if (cmd.state == CMD_RELEASE_RESET) {
		cmd.state = CMD_VALIDATE_RINGS;
		waitfor = RSP_RINGS_OK;
	    }
	    else if (cmd.state == CMD_VALIDATE_RINGS) {
		cmd.state = CMD_SETUP_FABRIC;
		waitfor = RSP_FABRIC_READY;
	    }
	    else if (cmd.state == CMD_SETUP_FABRIC) {
		cmd.state = CMD_VALIDATE_FABRIC;
		waitfor = RSP_FABRIC_OK;
	    }
	    else if (cmd.state == CMD_VALIDATE_FABRIC) {
		cmd.state = CMD_CONTINUE;
		waitfor = RSP_NONE;
	    }
	    /* Clear seen flag */
	    for (i = 0; i < cfg_nodes; i++)
		nodedata[cfg_nodelist[i].sciid] &= 0x7f;

	    cmd.tid++;
	    count = 0;
	    backoff = 1;

	    printf("Issuing %s, desired response %s (tid = %d).\n",
		   node_state_name[cmd.state], node_state_name[waitfor], cmd.tid);
	}
    }
}

static void update_mtrr(void)
{
    /* Ensure Tom2ForceMemTypeWB (bit 22) is set, so memory between 4G and TOM2 is writeback */
    uint64_t *syscfg_msr = (void *)REL(new_syscfg_msr);
    *syscfg_msr = dnc_rdmsr(MSR_SYSCFG) | (1 << 22);
    dnc_wrmsr(MSR_SYSCFG, *syscfg_msr);

    /* Ensure default memory type is uncacheable */
    uint64_t *mtrr_default = (void *)REL(new_mtrr_default);
    *mtrr_default = 3 << 10;
    dnc_wrmsr(MSR_MTRR_DEFAULT, *mtrr_default);

    /* Store fixed MTRRs */
    uint64_t *mtrr_fixed = (void *)REL(new_mtrr_fixed);
    uint32_t *fixed_mtrr_regs = (void *)REL(fixed_mtrr_regs);

    for (int i = 0; i < 11; i++)
	mtrr_fixed[i] = dnc_rdmsr(fixed_mtrr_regs[i]);

    /* Store variable MTRRs */
    uint64_t *mtrr_var_base = (void *)REL(new_mtrr_var_base);
    uint64_t *mtrr_var_mask = (void *)REL(new_mtrr_var_mask);

    printf("Variable MTRRs :\n");
    for (int i = 0; i < 8; i++) {
	mtrr_var_base[i] = dnc_rdmsr(MSR_MTRR_PHYS_BASE0 + i * 2);
	mtrr_var_mask[i] = dnc_rdmsr(MSR_MTRR_PHYS_MASK0 + i * 2);
	if (mtrr_var_mask[i] & 0x800ULL) {
	    printf("  [%d] base=0x%012llx, mask=0x%012llx : %s\n", i, mtrr_var_base[i] & ~0xfffULL,
		   mtrr_var_mask[i] & ~0xfffULL, MTRR_TYPE(mtrr_var_base[i] & 0xffULL));
	}
    }
}

static void local_chipset_fixup(void)
{
    u16 node;
    u64 addr;
    u32 sreq_ctrl, val;
    int i;

    printf("Scanning for known chipsets, local pass...\n");
    val = dnc_read_conf(0xfff0, 0, 0x14, 0, 0);
    if (val == 0x43851002) {
	printf("Adjusting local configuration of AMD SP5100...\n");
	if (!disable_smm) {
	    /* Disable config-space triggered SMI */
	    outb(0xa8, 0xcd6);
	    val = inb(0xcd7);
	    val = val & ~0x10;
	    outb(val, 0xcd7);
	}

	addr = dnc_rdmsr(MSR_SMM_BASE) + 0x8000 + 0x37c40;
	addr += 0x200000000000ULL;

	val = dnc_read_csr(0xfff0, H2S_CSR_G0_NODE_IDS);
	node = (val >> 16) & 0xfff;

	for (i = 0; i < dnc_master_ht_id; i++) {
	    add_extd_mmio_maps(0xfff0, i, 3, 0x200000000000ULL, 0x2fffffffffffULL,
			       dnc_master_ht_id);
	}

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
	    del_extd_mmio_maps(0xfff0, i, 3);
    }
    printf("Chipset-specific setup done\n");
}

static void global_chipset_fixup(void)
{
    u16 node;
    u32 val;
    int i;

    printf("Scanning for known chipsets, global pass...\n");
    for (i = 0; i < dnc_node_count; i++) {
	node = nc_node[i].sci_id;
	val = dnc_read_conf(node, 0, 0, 0, 0);
	if ((val == 0x5a101002) || (val == 0x5a121002) || (val == 0x5a131002)) {
	    printf("Adjusting configuration of AMD SR56x0 on node 0x%04x...\n",
		   node);
	    /* NBHTIU_INDEX is 0x94
	     * Set TOM2; HTIU 0x30/0x31 are TOM2 lo/hi */
	    dnc_write_conf(node, 0, 0, 0, 0x94, 0x30|0x100);
	    dnc_write_conf(node, 0, 0, 0, 0x98, 
			   ((dnc_top_of_mem << DRAM_MAP_SHIFT) & 0xffffffff) | 1);
	    dnc_write_conf(node, 0, 0, 0, 0x94, 0x31|0x100);
	    dnc_write_conf(node, 0, 0, 0, 0x98,
			   dnc_top_of_mem >> (32 - DRAM_MAP_SHIFT));
	}
	if ((val == 0x036910de)) {
            u32 val = dnc_read_conf(node, 0, 0, 0, 0x90);
	    printf("Adjusting configuration of nVidia MCP55 on node 0x%04x...\n",
		   node);
            /* Disable mmcfg setting in bridge to avoid OS confusion */
	    dnc_write_conf(node, 0, 0, 0, 0x90, val & ~(1<<31));
	}
    }
    printf("Chipset-specific setup done\n");
}

static void setup_c1e_osvw(void)
{
    u64 msr;

    /* Disable C1E in MSRs */
    msr = dnc_rdmsr(MSR_HWCR) & ~(1 << 12);
    dnc_wrmsr(MSR_HWCR, msr);
    *((u64 *)REL(new_hwcr_msr)) = msr;

    msr = 0;
    dnc_wrmsr(MSR_INT_HALT, msr);
    *((u64 *)REL(new_int_halt_msr)) = msr;

    /* Disable OS Vendor Workaround bit for errata #400, as C1E is disabled */
    msr = dnc_rdmsr(MSR_OSVW_ID_LEN);
    if (msr < 2) {
	/* Extend ID length to cover errata 400 status bit */
	dnc_wrmsr(MSR_OSVW_ID_LEN, 2);
	*((u64 *)REL(new_osvw_id_len_msr)) = 2;
	msr = dnc_rdmsr(MSR_OSVW_STATUS) & ~2;
	dnc_wrmsr(MSR_OSVW_STATUS, msr);
	*((u64 *)REL(new_osvw_status_msr)) = msr;
	printf("Enabled OSVW errata #400 workaround status, as C1E disabled\n");
    } else {
	*((u64 *)REL(new_osvw_id_len_msr)) = msr;
	msr = dnc_rdmsr(MSR_OSVW_STATUS);
	if (msr & 2) {
	    msr &= ~2;
	    dnc_wrmsr(MSR_OSVW_STATUS, msr);
	    printf("Cleared OSVW errata #400 bit status, as C1E disabled\n");
	}

	*((u64 *)REL(new_osvw_status_msr)) = msr;
    }
}

static int unify_all_nodes(void)
{
    u64 val;
    u16 i;
    u16 node;
    u8 abort = 0;
    int model, model_first = 0;
    volatile u32 *apic;

    val = dnc_rdmsr(MSR_APIC_BAR);
    apic = (void *)((u32)val & ~0xfff);

    dnc_node_count = 0;
    ht_pdom_count  = 0;
    ht_next_apic   = apic[0x20/4] >> 24;
    printf("ht_next_apic: %x\n", ht_next_apic);

    tally_local_node(1);
    nc_node[1].ht[0].base = dnc_top_of_mem;
    if (!tally_all_remote_nodes()) {
        printf("Unable to reach all nodes, retrying...\n");
	return 0;
    }

    for (node = 0; node < dnc_node_count; node++)
	for (i = 0; i < 8; i++) {
	    if (!nc_node[node].ht[i].cpuid)
		continue;

	    /* Ensure all processors are the same for compatibility */
	    model = cpu_family(nc_node[node].sci_id, i);
	    if (!model_first)
		model_first = model;
	    else if (model != model_first) {
		printf("[%04x#%d] has processor model 0x%08x, different than first processor model 0x%08x\n", nc_node[node].sci_id, i, model, model_first);
		abort = 1;
	    }

	    if ((model >> 16) >= 0x15) {
		val = dnc_read_conf(nc_node[node].sci_id, 0, 24+i, NB_FUNC_DRAM, 0x118);
		if (val & (1<<19)) {
		    printf("[%04x#%d] has LockDramCfg set (F2x118 = 0x%08x), cannot continue\n",
			   nc_node[node].sci_id, i, (u32)val);
		    abort = 2;
		}
	    }
	}

    if (abort == 1) {
	printf("Error: Please install processors with consistent model and stepping\n");
	return -1;
    } else if (abort == 2) {
	printf("Error: Please ensure that CState C6 isn't enabled in the BIOS\n");
	return -1;
    }

    if (verbose > 0) {
	printf("Global memory map:\n");
	for (node = 0; node < dnc_node_count; node++) {
	    for (i = 0; i < 8; i++) {
		if (!nc_node[node].ht[i].cpuid)
		    continue;
		printf("SCI%03x/HT#%d : %012llx - %012llx\n",
		       node, i,
		       (u64)nc_node[node].ht[i].base << DRAM_MAP_SHIFT,
		       (u64)(nc_node[node].ht[i].base + nc_node[node].ht[i].size) << DRAM_MAP_SHIFT);
	    }
	}
    }

    /* Set up local mapping registers etc from 0 - master node max */
    for (i = 0; i < dnc_master_ht_id; i++) {
	cht_write_config_nc(dnc_master_ht_id, 1, nc_neigh, nc_neigh_link,
			    H2S_CSR_F1_RESOURCE_MAPPING_ENTRY_INDEX, i);
	cht_write_config_nc(dnc_master_ht_id, 1, nc_neigh, nc_neigh_link,
			    H2S_CSR_F1_DRAM_LIMIT_ADDRESS_REGISTERS,
			    ((nc_node[0].ht[i].base + nc_node[0].ht[i].size - 1) << 8) | i);
	cht_write_config_nc(dnc_master_ht_id, 1, nc_neigh, nc_neigh_link,
			    H2S_CSR_F1_DRAM_BASE_ADDRESS_REGISTERS,
			    (nc_node[0].ht[i].base << 8) | 3);
    }

    /* DRAM map on local CPUs to redirect all accesses outside our local range to NC
     * NB: Assuming that memory is assigned sequentially to SCI nodes and HT nodes */
    for (i = 0; i < dnc_master_ht_id; i++) {
	/* TODO: Verify that this DRAM slot is actually unused */
	cht_write_config(i, 1, 0x7c, ((dnc_top_of_mem - 1) << 16) | dnc_master_ht_id);
	cht_write_config(i, 1, 0x78, (nc_node[1].ht[0].base << 16) | 3);
    }

    dnc_write_csr(0xfff0, H2S_CSR_G0_MIU_NGCM0_LIMIT,
		  nc_node[0].ht[0].base >> 6);
    dnc_write_csr(0xfff0, H2S_CSR_G0_MIU_NGCM1_LIMIT,
		  ((nc_node[0].ht[dnc_master_ht_id - 1].base +
		    nc_node[0].ht[dnc_master_ht_id - 1].size) >> 6) - 1);
    printf("%03x/NGCM0: %x\n", nc_node[0].sci_id, dnc_read_csr(0xfff0, H2S_CSR_G0_MIU_NGCM0_LIMIT));
    printf("%03x/NGCM1: %x\n", nc_node[0].sci_id, dnc_read_csr(0xfff0, H2S_CSR_G0_MIU_NGCM1_LIMIT));

    dnc_write_csr(0xfff0, H2S_CSR_G3_DRAM_SHARED_BASE,
		  nc_node[0].ht[0].base);
    dnc_write_csr(0xfff0, H2S_CSR_G3_DRAM_SHARED_LIMIT,
		  nc_node[0].ht[dnc_master_ht_id - 1].base +
		  nc_node[0].ht[dnc_master_ht_id - 1].size);

    for (i = 0; i < dnc_node_count; i++) {
	u16 dnode;
	u32 addr, end, ht;

        node = (i == 0) ? 0xfff0 : nc_node[i].sci_id;
	for (dnode = 0; dnode < dnc_node_count; dnode++) {
	    addr = nc_node[dnode].ht[0].base;
	    end  = addr + nc_node[dnode].ht[0].size;
	    for (ht = 1; ht < 8; ht++)
		if (nc_node[dnode].ht[ht].base > addr)
		    end = nc_node[dnode].ht[ht].base + nc_node[dnode].ht[ht].size;
	    dnc_write_csr(node, H2S_CSR_G0_ATT_INDEX,
			  0x80000000 | /* AutoInc */
			  (0x08000000 << SCC_ATT_INDEX_RANGE) | /* Index Range */
			  (addr/SCC_ATT_GRAN)); /* Start index for current node */
	    printf("Node %03x ATT_INDEX: %x (%x, %x) SCI %03x\n", nc_node[i].sci_id,
		   dnc_read_csr(node, H2S_CSR_G0_ATT_INDEX), addr, end, nc_node[dnode].sci_id);
	    while (addr < end) {
		dnc_write_csr(node, H2S_CSR_G0_ATT_ENTRY, nc_node[dnode].sci_id);
		addr += SCC_ATT_GRAN;
	    }
	}
    }

    for (i = 0; i < dnc_node_count; i++) {
        node = (i == 0) ? 0xfff0 : nc_node[i].sci_id;
        printf("Loading SCC microcode on node %03x\n", nc_node[i].sci_id);
        load_scc_microcode(node);
    }

    scc_started = 1;
    printf("SCC microcode loaded\n");

    update_mtrr();

    /* Set TOPMEM2 for ourselves and other cores */
    dnc_wrmsr(MSR_TOPMEM2, (u64)dnc_top_of_mem << DRAM_MAP_SHIFT);
    *((u64 *)REL(new_topmem2_msr)) = (u64)dnc_top_of_mem << DRAM_MAP_SHIFT;
    /* Harmonize TOPMEM */
    *((u64 *)REL(new_topmem_msr)) = dnc_rdmsr(MSR_TOPMEM);

    /* Update OS visible workaround MSRs */
    if (disable_c1e)
	setup_c1e_osvw();

    if (verbose > 0)
	debug_acpi();
    update_acpi_tables();
    update_mptable();
    if (verbose > 0)
	debug_acpi();

    setup_local_mmio_maps();

    setup_apic_atts();

    dnc_wrmsr(MSR_MCFG_BASE, DNC_MCFG_BASE | ((u64)nc_node[0].sci_id << 28ULL) | 0x21ULL);
    *((u64 *)REL(new_mcfg_msr)) = DNC_MCFG_BASE | ((u64)nc_node[0].sci_id << 28ULL) | 0x21ULL;

    /* Make chipset-specific adjustments */
    global_chipset_fixup();

    /* Must run after SCI is operational */
    disable_smm_handler(dnc_rdmsr(MSR_SMM_BASE));

    setup_other_cores();

    for (i = 1; i < dnc_node_count; i++)
        setup_remote_cores(i);

    printf("\n");
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
        printf("Error: SYSLINUX API version >= 3.72 is required\n");
        return -1;
    }

    return 0;
}

static void start_user_os(void)
{
    static com32sys_t rm;

    /* Release resources to reduce allocator fragmentation */
    free(cfg_nodelist);
    free(cfg_partlist);

    strcpy(__com32.cs_bounce, next_label);
    rm.eax.w[0] = 0x0003;
    rm.ebx.w[0] = OFFS(__com32.cs_bounce);
    rm.es = SEG(__com32.cs_bounce);
    printf("Unification succeeded; loading %s...\n", next_label);
    __intcall(0x22, &rm, NULL);
}

static void cleanup_stack(void)
{
    static com32sys_t rm;
    rm.eax.w[0] = 0x000C;
    rm.edx.w[0] = 0x0000;
    printf("Unloading bootloader stack...\n");
    __intcall(0x22, &rm, NULL);
}

static void set_wrap32_disable(void)
{
    u64 val = dnc_rdmsr(MSR_HWCR);
    dnc_wrmsr(MSR_HWCR, val | (1ULL << 17));
}

static void clear_bsp_flag(void)
{
    u64 val = dnc_rdmsr(MSR_APIC_BAR);
    dnc_wrmsr(MSR_APIC_BAR, val &= ~(1ULL << 8));
}

static void get_hostname(void)
{
    char *dhcpdata;
    size_t dhcplen;

    if (pxe_get_cached_info(PXENV_PACKET_TYPE_DHCP_ACK, (void **)&dhcpdata, &dhcplen))
	return;

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
#define ERR_DNC_SETUP_FABRIC       -5
#define ERR_INIT_CACHES            -6
#define ERR_INSTALL_E820_HANDLER   -7
#define ERR_UNIFY_ALL_NODES        -8 
#define ERR_GENERAL_NC_START_ERROR -9

static int nc_start(void)
{
    u32 uuid;
    struct node_info *info;
    struct part_info *part;
    int wait;
    
    if (check_api_version() < 0)
        return ERR_API_VERSION;

    get_hostname();

    /* Only in effect on core0, but only required for 32-bit code. */
    set_wrap32_disable();
 
    dnc_master_ht_id = dnc_init_bootloader(&uuid, &dnc_asic_mode, &dnc_chip_rev,
					   __com32.cs_cmdline);
    if (dnc_master_ht_id == -2)
	start_user_os();

    if (dnc_master_ht_id < 0)
        return ERR_MASTER_HT_ID;

    info = get_node_config(uuid);
    if (!info)
        return ERR_NODE_CONFIG;

    part = get_partition_config(info->partition);
    if (!part)
        return ERR_PARTITION_CONFIG;

    /* Copy this into NC ram so its available remotely */
    load_existing_apic_map();

    if (sync_mode >= 1) {
	if (part->builder == info->sciid) {
	    wait_for_slaves(info, part);
	}
	else {
	    wait_for_master(info, part);
	}
    }
    else {
	printf("Sync mode disabled...\n"); 

	if (dnc_setup_fabric(info) < 0)
            return ERR_DNC_SETUP_FABRIC;
    }

    /* Must run after SCI is operational */
    local_chipset_fixup();

    if (info->sync_only) {
	tsc_wait(5000);
	start_user_os();
    }

    if (dnc_init_caches() < 0)
        return ERR_INIT_CACHES;

    if (part->master == info->sciid) {
        int i;
        /* Master */
        for (i = 0; i < cfg_nodes; i++) {
            if (cfg_nodelist[i].uuid == uuid)
                continue;
	    if (cfg_nodelist[i].partition != info->partition)
		continue;
            nodedata[cfg_nodelist[i].sciid] = 0x80;
        }

        read_microcode_update();

	load_orig_e820_map();
        if (!install_e820_handler())
            return ERR_INSTALL_E820_HANDLER;

	wait = 100;
	while ((i = unify_all_nodes()) == 0) {
	    if (wait > 100000) {
		wait = 100;
	    }
	    tsc_wait(wait);
	    wait <<= 2;
	    dnc_check_fabric(info);
	}
	if (i < 0)
	    return ERR_UNIFY_ALL_NODES;

        (void)dnc_check_mctr_status(0);
        (void)dnc_check_mctr_status(1);
        
        update_e820_map();
        start_user_os();
    } else {
        /* Slave */
	u32 val;

	/* On non-root servers, prevent writing to unexpected locations */
	handover_legacy();
	disable_smi();

        clear_bsp_flag();

        cleanup_stack();

        /* Set G3x02c FAB_CONTROL bit 30 */
        dnc_write_csr(0xfff0, H2S_CSR_G3_FAB_CONTROL, 1<<30);
 
        printf("Numascale NumaChip awaiting fabric set-up by master node...\n");
	/* print_status(); */
        while (1) {
            val = dnc_check_mctr_status(0);
            val = dnc_check_mctr_status(1);
            
            val = dnc_read_csr(0xfff0, H2S_CSR_G3_FAB_CONTROL);
            if ((val & (1<<31))) {
                printf("*** Go-ahead seen, joining system to master node ***\n");
                break;
            }

	    dnc_check_fabric(info);
            tsc_wait(1000);
        }

        /* Disable legacy PIC interrupts and cache */
        disable_xtpic();
	disable_cache();

	/* Let master know we're ready for remapping/integration */
	dnc_write_csr(0xfff0, H2S_CSR_G3_FAB_CONTROL, val & ~(1<<31));
        while (1) {
            asm volatile("hlt" ::: "memory");
        }
    }

    return ERR_GENERAL_NC_START_ERROR;
}

int main(void)
{
    int ret;
    openconsole(&dev_rawcon_r, &dev_stdcon_w);
    printf("*** NumaConnect system unification module starting, rev %.7s%s ***\n",
	   gitlog_dnc_bootloader_sha, 
	   (strlen(gitlog_dnc_bootloader_diff) > 0) ? " (modified)" : "");

    ret = nc_start();
    if (ret < 0)
      {
	printf("Error: nc_start() failed with error code %d; check configuration files match hardware and UUIDs\n", ret);
	wait_key();
      }
    return ret;
}
