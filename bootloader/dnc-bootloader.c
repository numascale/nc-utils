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
#include "dnc-types.h"
#include "dnc-access.h"
#include "dnc-route.h"
#include "dnc-acpi.h"
#include "dnc-fabric.h"
#include "dnc-config.h"

#include "dnc-commonlib.h"
#include "dnc-masterlib.h"

#include "hw-config.h"

#define PIC_MASTER_CMD          0x20
#define PIC_MASTER_IMR          0x21
#define PIC_SLAVE_CMD           0xa0
#define PIC_SLAVE_IMR           0xa1

#define IMPORT_RELOCATED(sym) extern volatile u8 sym ## _relocate
#define REL(sym) ((volatile u8 *)asm_relocated + ((volatile u8 *)&sym ## _relocate - (volatile u8 *)&asm_relocate_start))

#define TABLE_AREA_SIZE		1024*1024

int dnc_master_ht_id;     /* HT id of NC on master node, equivalent nc_node[0].nc_ht_id */
extern int nc_neigh, nc_neigh_link;
int dnc_asic_mode;
int dnc_chip_rev;
u16 dnc_node_count = 0;
nc_node_info_t nc_node[128];
u16 ht_pdom_count = 0;
u16 apic_per_node;
u16 ht_next_apic;
u32 dnc_top_of_mem;       /* Top of mem, in 16MB chunks */
u8 post_apic_mapping[256]; /* POST APIC assigments */
int scc_started = 0;

/* Traversal info per node.  Bit 7: seen, bits 5:0 rings walked. */
u8 nodedata[4096];

const long long unsigned fixed_mtrr_base[] = {0x00000, 0x80000, 0xA0000,
    0xC0000, 0xC8000, 0xD0000, 0xD8000, 0xE0000, 0xE8000, 0xF0000, 0xF8000};
const int fixed_mtrr_reg[] = {MSR_MTRR_FIX64K_00000, MSR_MTRR_FIX16K_80000,
    MSR_MTRR_FIX16K_A0000, MSR_MTRR_FIX4K_C0000, MSR_MTRR_FIX4K_C8000,
    MSR_MTRR_FIX4K_D0000, MSR_MTRR_FIX4K_D8000, MSR_MTRR_FIX4K_E0000,
    MSR_MTRR_FIX4K_E8000, MSR_MTRR_FIX4K_F0000, MSR_MTRR_FIX4K_F8000};

extern unsigned char asm_relocate_start;
extern unsigned char asm_relocate_end;
char *asm_relocated;
char *tables_relocated;

IMPORT_RELOCATED(new_e820_handler);
IMPORT_RELOCATED(old_int15_vec);
IMPORT_RELOCATED(init_trampoline);
IMPORT_RELOCATED(cpu_init_finished);
IMPORT_RELOCATED(cpu_apic_renumber);
IMPORT_RELOCATED(cpu_apic_hi);
IMPORT_RELOCATED(new_mcfg_msr);
IMPORT_RELOCATED(new_topmem_msr);
IMPORT_RELOCATED(new_e820_len);
IMPORT_RELOCATED(new_e820_map);
IMPORT_RELOCATED(new_mpfp);
IMPORT_RELOCATED(new_mptable);
IMPORT_RELOCATED(new_mtrr_base);
IMPORT_RELOCATED(new_mtrr_mask);
IMPORT_RELOCATED(rem_topmem_msr);
IMPORT_RELOCATED(rem_smm_base_msr);

extern u8 smm_handler_start;
extern u8 smm_handler_end;

struct e820entry {
    u64 base;
    u64 length;
    u32 type;
} __attribute__((packed));
struct e820entry *orig_e820_map;
int orig_e820_len;

com32sys_t inreg, outreg;


static inline u64 rdtscll(void)
{
    u64 val;
    asm volatile ("rdtsc" : "=A" (val));
    return val;
}

void tsc_wait(u32 mticks) {
    u64 count;
    u64 stop;

    count = rdtscll() >> 1;
    stop = count + ((u64)mticks << (20 - 1));
    while(stop > count) {
        count = rdtscll() >> 1;
    }
}

void wait_key(void)
{
    char ch;
    printf("... (press any key to continue) ... ");
    while (fread(&ch, 1, 1, stdin) == 0)
        ;
    printf("\n");
}

unsigned char sleep(unsigned int msec)
{
    unsigned long micro = 1000*msec;

    inreg.eax.b[1] = 0x86;
    inreg.ecx.w[0] = (micro >> 16);
    inreg.edx.w[0] = (micro & 0xFFFF);
    __intcall(0x15, &inreg, &outreg);
    return outreg.eax.b[1];
}

static void disable_xtpic(void) {
    u8 dummy;

    dummy = inb(PIC_MASTER_IMR);
    outb(0xff, PIC_MASTER_IMR);
    dummy = inb(PIC_SLAVE_IMR);
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
      
        int i;
        for (i = 0; i*sizeof(struct e820entry) < e820_len; i++) {
            printf(" %016llx - %016llx (%016llx) [%x]\n",
                   e820_map[i].base, e820_map[i].base+e820_map[i].length,
                   e820_map[i].length, e820_map[i].type);
        }
        orig_e820_map = malloc(e820_len);
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
    /* http://groups.google.com/group/comp.lang.asm.x86/msg/9b848f2359f78cdf */
//    *bda_tom_lower = ((u32)asm_relocated) >> 10;

    memcpy(asm_relocated, &asm_relocate_start, relocate_size);

    e820 = (void *)REL(new_e820_map);

    unsigned int i, j = 0;
    for (i = 0; i < orig_e820_len / sizeof(struct e820entry); i++) {
        u64 orig_end = orig_e820_map[i].base + orig_e820_map[i].length;

	if (orig_e820_map[i].base > MAX_MEM_PER_NODE) {
	    /* Skip entry altogether */
	    continue;
	}

	if (orig_end > MAX_MEM_PER_NODE) {
	    /* Adjust length to fit */
	    orig_end = MAX_MEM_PER_NODE;
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

	if ((orig_end < 0x100000000) &&
	    (orig_e820_map[i].length > TABLE_AREA_SIZE) &&
	    (orig_e820_map[i].type == 1))
	    last_32b = j-1;
    }
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
    u64 prev_end, rest, buffer;
    unsigned int i, j;
    struct e820entry *e820;
    u16 *len;

    e820 = (void *)REL(new_e820_map);
    len  = (u16 *)REL(new_e820_len);

    /* Truncate to SCI 000/HT 0 end. Rest added below. */
    e820[*len-1].length = ((u64)nc_node[0].ht[0].size << DRAM_MAP_SHIFT)
	- e820[*len-1].base;

    prev_end = e820[*len-1].base + e820[*len-1].length;
    if (nc_node[0].nc_ht_id == 1) {
	/* Truncate SCI 000/HT 0 to SCC ATT granularity if only HT
	 * node on SCI 000.  Existing adjustment of ht_node_size
	 * handles rest. */
	rest = prev_end & ((SCC_ATT_GRAN << DRAM_MAP_SHIFT) - 1);
	if (rest) {
	    printf("Deducting %x from e820 entry...\n", (u32)rest);
	    e820[*len-1].length -= rest;
	    prev_end -= rest;
	}
    }

#ifdef TRACE_BUF_SIZE
    if (e820[*len-1].length > TRACE_BUF_SIZE)
	e820[*len-1].length -= TRACE_BUF_SIZE;
#endif

    buffer = 0;
    /* Add remote nodes */
    for (i = 0; i < dnc_node_count; i++) {
	for (j = 0; j < 8; j++) {
	    if ((i == 0) && (j == 0))
		continue;
	    if (!nc_node[i].ht[j].cpuid)
		continue;

            e820[*len].base   = ((u64)nc_node[i].ht[j].base << DRAM_MAP_SHIFT) + buffer;
            e820[*len].length = ((u64)nc_node[i].ht[j].size << DRAM_MAP_SHIFT) - buffer;
#ifdef TRACE_BUF_SIZE		
	    if (e820[*len].length > TRACE_BUF_SIZE)
		e820[*len].length -= TRACE_BUF_SIZE;
#endif
            e820[*len].type   = 1;
            prev_end = e820[*len].base + e820[*len].length;
            (*len)++;

//            buffer = (buffer + 4096) % (1<<20);
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
	    printf("core%d apic_id: 0x%x\n", c, af->apic_id);
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
    dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00000020); // Select APIC ATT
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
    const int socket_cost = 5;	/* typically 2.2GHz HyperTransport link */
    const int nc_cost = 90;	/* fixed NC costs */
    const int fabric_cost = 20;	/* scaled variable NC costs: 2xSERDES latency + route lookup */
    int total_cost = 10;	/* memory-controller + SDRAM latency */

    int src_sci = cfg_nodelist[src_node].sciid;
    int dst_sci = cfg_nodelist[dst_node].sciid;
    int size = cfg_fabric.z_size << 8 | cfg_fabric.y_size << 4 | cfg_fabric.x_size;
    int hops = 0;

    /* sum hops per axis */
    for (int dim = 0; dim < 3; dim++) {
	if (cfg_fabric.strict)
	    /* compute shortest path */
	    hops += linear_hops(src_sci & 0xf, dst_sci & 0xf, size & 0xf);
	else
	    /* assume average of half ring length */
	    hops += (src_sci == dst_sci) ? 0 : (size / 2);

	src_sci >>= 4;
	dst_sci >>= 4;
	size >>= 4;
    }

    if (hops) {
	/* assume linear socket distance to NC */
	int src_socket_offset = abs(src_ht - nc_node[src_node].nc_neigh);
	int dst_socket_offset = abs(dst_ht - nc_node[dst_node].nc_neigh);

	total_cost += (src_socket_offset + dst_socket_offset) * socket_cost;
	total_cost += nc_cost; /* fixed cost of NC */
	total_cost += hops * fabric_cost; /* variable cost of NC */
    } else
	total_cost += abs(dst_ht - src_ht) * socket_cost;

    return total_cost;
}

void disable_fixed_mtrrs(void)
{
    disable_cache();
    dnc_wrmsr(MSR_MTRR_DEFAULT, dnc_rdmsr(MSR_MTRR_DEFAULT) & ~(1 << 10));
    enable_cache();
}

void enable_fixed_mtrrs(void)
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

    /* fixed MTRRs may mark the RSDT and XSDT pointers r/o */
    disable_fixed_mtrrs();

    /* replace_root may fail if rptr is r/o, so we read the pointers
     * back.  In case of failure, we'll assume the existing rsdt/xsdt
     * tables can be extended where they are. */
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
	printf("Default ACPI APIC table not found.\n");
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
#ifdef APIC_ENABLE_MASK
		    lapic->flags = APIC_ENABLE_MASK(pnum);
#else
		    lapic->flags = 1;
#endif
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
#ifdef APIC_ENABLE_MASK
		    x2apic->flags = APIC_ENABLE_MASK(pnum);
#else
		    x2apic->flags = 1;
#endif
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
	printf("Default ACPI SRAT table not found.\n");
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


struct mp_config_table {
    union {
        char s[4];
        uint32_t l;
    } sig;
    uint16_t len;
    uint8_t revision;
    uint8_t checksum;
    unsigned char oemid[8];
    unsigned char prodid[12];
    uint8_t *oemtable;
    uint16_t oemtablesz;
    uint16_t entries;
    uint32_t lapicaddr;
    uint16_t extlen;
    uint16_t extchksum;
    char reserved;
    unsigned char data[0];
} __attribute__((packed));

struct mp_floating_pointer {
    union {
        char s[4];
        uint32_t l;
    } sig;
    struct mp_config_table *mptable;
    uint8_t len;
    uint8_t revision;
    uint8_t checksum;
    unsigned char feature[5];
} __attribute__((packed));


struct mp_floating_pointer *find_mptable(void *start, int len)
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
    printf("Using apic shift: %d (%d)\n", apic_shift, apic_per_node);
    
    // Set APIC ATT for remote interrupts
    for (i = 0; i < dnc_node_count; i++) {
        u16 snode = (i == 0) ? 0xfff0 : nc_node[i].sci_id;
	u16 dnode, ht;

        printf("Initializing sci node %03x APIC ATT tables...\n", nc_node[i].sci_id);
        dnc_write_csr(snode, H2S_CSR_G3_APIC_MAP_SHIFT,    apic_shift-1);
        dnc_write_csr(snode, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00000020); // Select APIC ATT

	for (j = 0; j < 64; j++) {
	    dnc_write_csr(snode, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + j*4, nc_node[0].sci_id);
	}
		    
	for (dnode = 0; dnode < dnc_node_count; dnode++) {
	    for (ht = 0; ht < 8; ht++) {
		if (!nc_node[dnode].ht[ht].cpuid)
		    continue;
		for (j = 0; j < nc_node[dnode].ht[ht].cores; j += (1<<apic_shift)) {
		    u16 offs = 4 * ((nc_node[dnode].apic_offset + nc_node[dnode].ht[ht].apic_base + j) >> apic_shift);
		    printf("Adding apic entry: %03x : %02x -> %03x\n", nc_node[i].sci_id, offs, nc_node[dnode].sci_id);
		    dnc_write_csr(snode, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + offs, nc_node[dnode].sci_id);
		}
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
		       val, node);
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
	add_extd_mmio_maps(0xfff0, i, 7, 0x200000000000ULL, 0x2fffffffffffULL,
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
	del_extd_mmio_maps(0xfff0, i, 7);
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

    // ERRATA #N28: Disable HT Lock mechanism. 
    // AMD Email dated 31.05.2011 :
    // There is a switch that can help with these high contention issues,
    // but it isn't "productized" due to a very rare potential for live lock if turned on.
    // Given that HUGE caveat, here is the information that I got from a good source:
    // LSCFG[44] =1 will disable it. MSR number is C001_1020.
    val = dnc_rdmsr(MSR_LSCFG);
    val = val | (1ULL << 44);
    dnc_wrmsr(MSR_LSCFG, val);
   
    // Start all local cores (not BSP) and let them run our init_trampoline
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
	    *icr = 0x00004500;
	    for (j = 0; j < 1000; j++) {
		if (!(*icr & 0x1000))
		    break;
		tsc_wait(10);
	    }
	    if (*icr & 0x1000)
		printf("apic %d INIT IPI not delivered.\n", apicid);

	    apic[0x310/4] = apicid << 24;
	    *icr = 0x00004600 | (((u32)REL(init_trampoline) >> 12) & 0xff);
	    for (j = 0; j < 1000; j++) {
		if (!(*icr & 0x1000))
		    break;
		tsc_wait(10);
	    }
	    if (*icr & 0x1000)
		printf("apic %d STARTUP IPI not delivered.\n", apicid);
	    
	    for (j = 0; j < 100; j++) {
		if (*REL(cpu_init_finished))
		    break;
		tsc_wait(10);
	    }
	    if (*REL(cpu_init_finished)) {
		printf("apic %d reported done.\n", apicid);
		val = *((u64 *)REL(rem_topmem_msr));
		printf("remote topmem: %llx\n", val);
		val = *((u64 *)REL(rem_smm_base_msr));
		printf("remote smm_base: %llx\n", val);
		disable_smm_handler(val);
	    }
	    else {
		printf("apic %d did not toggle init flag.\n", apicid);
	    }
	}
    }
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
	if (val != 0x12001022) {
	    fprintf(stderr,
		    "[%04x#%x]F0x00 does not indicate an AMD Opteron CPU: 0x%08x\n",
		    node, i, val);
	    return;
	}

        // Disable traffic distribution for now..
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
	if (val != 0x12001022) {
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
    printf("Done.\n");

    memcpy(&nc_node[num].ht[maxnode], &nc_node[num].ht[0], sizeof(ht_node_info_t));
    nc_node[num].ht[0].cpuid = 0;
    nc_node[num].nc_ht_id = 0;
}

static void setup_remote_cores(u16 num)
{
    u8 i;
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
            printf("setting remote mmio32 maps page %d...\n", j >> 8);
            dnc_write_csr(node, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00000010 | j >> 8);
        }
        dnc_write_csr(node, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + (j & 0xff) * 4,
                      nc_node[0].sci_id);
    }

    if (renumber_bsp)
	renumber_remote_bsp(num);
    ht_id = nc_node[num].nc_ht_id;

    printf("remote dnc mmio32 maps set...\n");

    /* Set H2S_Init */
    printf("Setting sci node %03x H2S_Init...\n", node);
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
#if !defined(USE_LOCAL_VGA)
        // Apparently the HP DL165 modes can't handle non-posted writes to the VGA ports...
        /* Make sure the VGA Enable register is disabled to forward VGA transactions
           (MMIO A_0000h - B_FFFFh and I/O 3B0h - 3BBh or 3C0h - 3DFh) to the NumaChip */
        dnc_write_conf(node, 0, 24+i, NB_FUNC_MAPS, 0xf4, 0x0);
#endif
    }

    /* Now, reset all DRAM maps */
    printf("Resetting DRAM maps on sci node %03x\n", node);
    for (i = 0; i < 8; i++) {
	if (!cur_node->ht[i].cpuid)
	    continue;
        // Re-direct everything below our first local address to NumaChip
        dnc_write_conf(node, 0, 24+i, NB_FUNC_MAPS, 0x44,
                       ((cur_node->ht[0].base - 1) << 16) | ht_id);
        dnc_write_conf(node, 0, 24+i, NB_FUNC_MAPS, 0x40,
                       (nc_node[0].ht[0].base << 16) | 3);
        dnc_write_conf(node, 0, 24+i, NB_FUNC_MAPS, 0xf0,  0);
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
	val = dnc_read_conf(node, 0, 24+i, 2, 0x110);
	if (val & 1) {
	    /* Reprogram DCT base/offset values */
	    dnc_write_conf(node, 0, 24+i, 2, 0x110, (val & ~0xfffff800) |
			   ((cur_node->ht[i].base >> (27 - DRAM_MAP_SHIFT)) << 11));
	    dnc_write_conf(node, 0, 24+i, 2, 0x114, 
			   (cur_node->ht[i].base >> (26 - DRAM_MAP_SHIFT)) << 10);
	    printf("[%03x#%d] F2x110: %08x, F2x114: %08x\n",
		   node, i,
		   dnc_read_conf(node, 0, 24+i, 2, 0x110),
		   dnc_read_conf(node, 0, 24+i, 2, 0x114));
	}
    }

    for (i = 1; i < 8; i++) {
	for (j = 0; j < 8; j++) {
	    if (!cur_node->ht[j].cpuid)
		continue;
            dnc_write_conf(node, 0, 24+j, NB_FUNC_MAPS, 0x44 + i*8, 0);
            dnc_write_conf(node, 0, 24+j, NB_FUNC_MAPS, 0x40 + i*8, 0);
        }
    }

    // Program our local ranges
    for (i = 0; i < 8; i++) {
	if (!cur_node->ht[i].cpuid)
	    continue;
	for (j = 0; j < 8; j++) {
	    if (!cur_node->ht[j].cpuid)
		continue;
            dnc_write_conf(node, 0, 24+j, NB_FUNC_MAPS, 0x4c + i*8,
                           ((cur_node->ht[i].base + cur_node->ht[i].size - 1) << 16) | i);
            dnc_write_conf(node, 0, 24+j, NB_FUNC_MAPS, 0x48 + i*8,
                           (cur_node->ht[i].base << 16) | 3);
        }
        dnc_write_conf(node, 0, 24+ht_id, NB_FUNC_MAPS, H2S_CSR_F1_RESOURCE_MAPPING_ENTRY_INDEX,
                       i);
        dnc_write_conf(node, 0, 24+ht_id, NB_FUNC_MAPS, H2S_CSR_F1_DRAM_LIMIT_ADDRESS_REGISTERS,
                       ((cur_node->ht[i].base + cur_node->ht[i].size - 1) << 8) | i);
        dnc_write_conf(node, 0, 24+ht_id, NB_FUNC_MAPS, H2S_CSR_F1_DRAM_BASE_ADDRESS_REGISTERS,
                       (cur_node->ht[i].base << 8) | 3);
    }

    // Re-direct everything above our last local address (if any) to NumaChip
    if (num != dnc_node_count-1) {
	for (j = 0; j < 8; j++) {
	    if (!cur_node->ht[j].cpuid)
		continue;
            dnc_write_conf(node, 0, 24+j, NB_FUNC_MAPS, 0x7c,
                           ((nc_node[dnc_node_count-1].addr_end - 1) << 16) | ht_id);
            dnc_write_conf(node, 0, 24+j, NB_FUNC_MAPS, 0x78,
                           (cur_node->addr_end << 16) | 3);
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

    // Start all remote cores and let them run our init_trampoline
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
	    for (j = 0; j < 100; j++) {
		if (*REL(cpu_init_finished))
		    break;
		tsc_wait(10);
	    }

	    if (*REL(cpu_init_finished)) {
		printf("apic %d (%d) reported done.\n", apicid, oldid);
		qval = *((u64 *)REL(rem_topmem_msr));
		printf("remote topmem: %llx\n", qval);
		qval = *((u64 *)REL(rem_smm_base_msr));
		printf("remote smm_base: %llx\n", qval);
		disable_smm_handler(qval);
	    }
	    else {
		printf("apic %d (%d) did not toggle init flag.\n", apicid, oldid);
	    }
	}
    }
}

#if 0
static void fix_legacy_mmio_maps(void)
{
    u8 node;
    u32 val;
    int i;
    
    for (node = 0; node < dnc_master_ht_id; node++) {
        for (i = 0; i < 8; i++) {
            val = cht_read_config(node, NB_FUNC_MAPS, 0x80 + i*8);
            if (val & 2) {
                // Set the non-posted bit to force non-posted requests
                printf("Enablig the non-posted bit for HT#%d MMIO range %010llx - %010llx\n", node,
                       ((u64)cht_read_config(node, NB_FUNC_MAPS, 0x80 + i*8) << 8) & 0xffffff0000ULL,
                       ((u64)cht_read_config(node, NB_FUNC_MAPS, 0x84 + i*8) << 8) & 0xffffff0000ULL);
                val = cht_read_config(node, NB_FUNC_MAPS, 0x84 + i*8);
                cht_write_config(node, NB_FUNC_MAPS, 0x84 + i*8, val | (1<<7));
            }
        }
#if !defined(USE_LOCAL_VGA)
        // Apparently the HP DL165 modes can't handle non-posted writes to the VGA ports...
        // Fix VGA enable register also, if in use
        val = cht_read_config(node, NB_FUNC_MAPS, 0xf4);
        if (val & 1) {
            printf("Enablig the non-posted bit for HT#%d VGA range (0xA0000 - 0xBFFFF)\n", node);
            cht_write_config(node, NB_FUNC_MAPS, 0xf4, val | (1<<1));
        }
#endif
    }
}
#endif

static void setup_local_mmio_maps(void)
{
    int i, j, next;
    u64 tom;
    u32 base[8];
    u32 lim[8];
    u32 dst[8];
    u32 curbase, curlim, curdst;
    int sbnode;

    printf("Setting MMIO maps on local DNC...\n");

    for (i = 0; i < 8; i++) {
	base[i] = 0;
	lim[i] = 0;
	dst[i] = 0;
    }

    tom = dnc_rdmsr(MSR_TOPMEM);
    if (tom >= 0x100000000) {
	printf("TOP_MEM above 4G boundary, aborting!\n");
	return;
    }

    sbnode = (cht_read_config(0, NB_FUNC_HT, 0x60) >> 8) & 7;
    base[0] = (tom >> 8) & ~0xff;
    lim[0] = 0x00ffff00;
    dst[0] = (sbnode << 8) | 3;
    next = 1;

    /* Apply default maps so we can bail without losing all hope. */
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
	curdst = ((curlim & 0x7) << 8) | curbase & 0x3;
	/* This strips NP-bit. */
	curbase = curbase & ~0xff;
	curlim = curlim & ~0xff;
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
    inargs.eax.w[0] = 0x0006; // Open file
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
    inargs.eax.w[0] = 0x0007; // Read file
    inargs.esi.w[0] = fd;
    inargs.ecx.w[0] = blocks;
    inargs.ebx.w[0] = OFFS(buf);
    inargs.es = SEG(buf);
    __intcall(0x22, &inargs, &outargs);

    len = outargs.ecx.l;
    
    if (outargs.esi.w[0] != 0) {
        inargs.eax.w[0] = 0x0008; // Close file
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

u32 mseq_ucode_update[1024];
u16 mseq_table_update[128];

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
    strncpy(path, microcode_path, 500);
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
	printf("Config file <%s> not found!\n", file_name);
        return -1;
    }

    config = __com32.cs_bounce;
    config[config_len] = '\0';

    if (!parse_config_file(config)) {
	printf("Error reading config file!\n");
	return -1;
    }
    return 0;
}

static int pxeapi_call(int func, u8 *buf)
{
    static com32sys_t inargs, outargs;

    inargs.eax.w[0] = 0x0009; // Call PXE Stack
    inargs.ebx.w[0] = func; // PXE function number
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

void udp_broadcast_state(int handle, void *buf, int len)
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

int udp_read_state(int handle, void *buf, int len)
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
    static int count = 0;

    /* print once in 10 200ms loop timeouts */
    if (count++ < 10)
	return;

    count = 0;
    printf("waiting for");

    /* skip first node */
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
    int count, backoff;
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
    while (1) {
	if (cmd.state != CMD_STARTUP)
	    if (++count >= backoff) {
		udp_broadcast_state(handle, &cmd, sizeof(cmd));
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
		printf("Command did not complete successfully on master (reason %d), resetting...\n", own_state);

	    last_cmd = cmd.tid;
	}

	if (cfg_nodes > 1) {
	    len = udp_read_state(handle, &rsp, sizeof(rsp));
	    if (!len && !do_restart) {
		wait_status();
		tsc_wait(200);
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
			printf("Node 0x%03x (%d) entered %d\n", rsp.sciid, rsp.uuid, waitfor);
			nodedata[rsp.sciid] = 0x80;
		    }
		}
		else if ((rsp.state == RSP_PHY_NOT_TRAINED) ||
			 (rsp.state == RSP_RINGS_NOT_OK) ||
			 (rsp.state == RSP_FABRIC_NOT_READY) ||
			 (rsp.state == RSP_FABRIC_NOT_OK))
		{
		    if (nodedata[rsp.sciid] != 0x80) {
			printf("Node 0x%03x aborted!  Restarting process...\n", rsp.sciid);
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

	    printf("Transition to %d, desired response %d.\n", cmd.state, waitfor);
	    cmd.tid++;
	    count = 0;
	    backoff = 1;
	}
    }
}

static int update_mtrr(void)
{
    u64 base, mask, prev;
    u64 *mtrr_base;
    u64 *mtrr_mask;
    int i;

    mtrr_base = (void *)REL(new_mtrr_base);
    mtrr_mask = (void *)REL(new_mtrr_mask);

    printf("Setting BSP MTRR...\n");
    base = (u64)dnc_top_of_mem << DRAM_MAP_SHIFT;
    mask = ~0ULL;
    prev = mask;
    i = 0;
    while (base & mask) {
	if (i >= 8) {
	    printf("*** Not enough room for required MTRR entries!\n");
	    return -1;
	}
	if (base & ~mask) {
	    base = base & mask;
	    mtrr_base[i] = (base & 0x0000fffffffff000) | 0x006;
	    mtrr_mask[i] = (prev & 0x0000fffffffff000) | 0x800;
	    i++;
	}
	prev = mask;
	mask = mask << 1ULL;
    }
    if ((prev != ~0ULL) && (prev != 0ULL)) {
	if (i >= 8) {
	    printf("*** Not enough room for required MTRR entries!\n");
	    return -1;
	}
	mtrr_base[i] = 0x006;
	mtrr_mask[i] = (prev & 0x0000fffffffff000) | 0x800;
	i++;
    }

    /* Add entries for MMIO hole under 4G */
    base = dnc_rdmsr(MSR_TOPMEM);
    mask = ~0ULL;
    prev = mask;
    
    while (base & mask) {
	if (i >= 8) {
	    printf("*** Not enough room for required MTRR entries!\n");
	    return -1;
	}
	prev = mask;
	mask = mask << 1ULL;
	if (((base & mask) < base) || ((base + ~mask + 1) > 0x100000000)) {
	    mtrr_base[i] = (base & 0x0000fffffffff000) | 0x000;
	    mtrr_mask[i] = (prev & 0x0000fffffffff000) | 0x800;
	    i++;
	    base = base + ~prev + 1;
	    if (base >= 0x100000000)
		break;
	    mask = ~0;
	    prev = mask;
	}
    }

    /* Blank remaining */
    while (i < 8) {
	mtrr_base[i] = 0;
	mtrr_mask[i] = 0;
	i++;
    }

    printf("Disabling MTRRs...\n");
    disable_cache();

    /* Disable all entries */
    for (i = 0; i < 8; i++)
	dnc_wrmsr(MSR_MTRR_PHYS_MASK0 + i*2, 0);

    printf("MTRRs disabled...\n");
    
    for (i = 0; i < 8; i++) {
	dnc_wrmsr(MSR_MTRR_PHYS_BASE0 + i*2, mtrr_base[i]);
	dnc_wrmsr(MSR_MTRR_PHYS_MASK0 + i*2, mtrr_mask[i]);
    }

    enable_cache();
    printf("MTRRs enabled.\n");
    printf("default type: %016llx\n", dnc_rdmsr(MSR_MTRR_DEFAULT));

    for (i = 0; i < 11; i++)
	printf("MTRR F%d: %016llx: %016llx\n", i, fixed_mtrr_base[i], dnc_rdmsr(fixed_mtrr_reg[i]));

    for (i = 0; i < 8; i++)
        printf("MTRR %d: %012llx - %012llx\n", 
               i,
               dnc_rdmsr(MSR_MTRR_PHYS_BASE0 + i*2),
               dnc_rdmsr(MSR_MTRR_PHYS_MASK0 + i*2));
    return 0;
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
	    add_extd_mmio_maps(0xfff0, i, 7, 0x200000000000ULL, 0x2fffffffffffULL,
			       dnc_master_ht_id);
	}

	add_scc_hotpatch_att(addr, node);

	sreq_ctrl = dnc_read_csr(0xfff0, H2S_CSR_G3_SREQ_CTRL);
	dnc_write_csr(0xfff0, H2S_CSR_G3_SREQ_CTRL,
		      (sreq_ctrl & ~0xfff0) | (0xf00 << 4));

	val = mem64_read32(addr + 4);
	printf("SMM fingerprint: %16llx\n", val);
    
	if (val == 0x3160bf66) {
	    printf("SMM coh config space trigger fingerprint found, patching...\n");
	    val = mem64_read32(addr);
	    mem64_write32(addr, (val & 0xff) | 0x9040eb00);
	}
    
	val = mem64_read32(addr);
	printf("MEM %llx: %08lx\n", addr, val);
	
	dnc_write_csr(0xfff0, H2S_CSR_G3_SREQ_CTRL, sreq_ctrl);
	for (i = 0; i < dnc_master_ht_id; i++)
	    del_extd_mmio_maps(0xfff0, i, 7);
    }
    printf("Chipset-specific setup done.\n");
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
	    /* NBHTIU_INDEX is 0x94 */
	    /* Set TOM2; HTIU 0x30/0x31 are TOM2 lo/hi */
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
    printf("Chipset-specific setup done.\n");
}

static int unify_all_nodes(void)
{
    u64 val;
    u16 i;
    u16 node;
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

//    if (dnc_node_count <= 1) {
//        printf("No remote nodes!\n");
//        return 1;
//    }
 
    // Set up local mapping registers etc. from 0 - master node max
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

    // DRAM map on local CPUs to redirect all accesses outside our local range to NC
    // NB: Assuming that memory is assigned sequentially to SCI nodes and HT nodes.
    for (i = 0; i < dnc_master_ht_id; i++) {
	// TODO: Verify that this DRAM slot is actually unused
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
        printf("Loading SCC microcode on node %03x.\n", nc_node[i].sci_id);
        load_scc_microcode(node);
    }

    scc_started = 1;
    printf("SCC microcode loaded.\n");

    if (update_mtrr() < 0) {
        printf("Error updating MTRRs!\n");
	return 1;
    }

    // Set TOPMEM2 for ourselves and other cores
    dnc_wrmsr(MSR_TOPMEM2, (u64)dnc_top_of_mem << DRAM_MAP_SHIFT);
    *((u64 *)REL(new_topmem_msr)) = (u64)dnc_top_of_mem << DRAM_MAP_SHIFT;

    debug_acpi();
    update_acpi_tables();
    update_mptable();
    debug_acpi();

    setup_local_mmio_maps();

//    fix_legacy_mmio_maps();
    
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
        printf("SYSLINUX API version >= 3.72 is required! Exiting.\n");
        return -1;
    }

    return 0;
}

static void start_user_os(void)
{
    static com32sys_t rm;

    strcpy(__com32.cs_bounce, next_label);
    rm.eax.w[0] = 0x0003;
    rm.ebx.w[0] = OFFS(__com32.cs_bounce);
    rm.es = SEG(__com32.cs_bounce);
    printf("Loading %s...\n", next_label);
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

#define HcRevision 0x00

#define HcControl 0x04

/*
 * HcControl (control) register masks
 */
#define OHCI_CTRL_CBSR  (3 << 0)        /* control/bulk service ratio */
#define OHCI_CTRL_PLE   (1 << 2)        /* periodic list enable */
#define OHCI_CTRL_IE    (1 << 3)        /* isochronous enable */
#define OHCI_CTRL_CLE   (1 << 4)        /* control list enable */
#define OHCI_CTRL_BLE   (1 << 5)        /* bulk list enable */
#define OHCI_CTRL_HCFS  (3 << 6)        /* host controller functional state */
#define OHCI_CTRL_IR    (1 << 8)        /* interrupt routing */
#define OHCI_CTRL_RWC   (1 << 9)        /* remote wakeup connected */
#define OHCI_CTRL_RWE   (1 << 10)       /* remote wakeup enable */


#define HcCommandStatus 0x08

/*
 * HcCommandStatus (cmdstatus) register masks
 */
#define OHCI_HCR        (1 << 0)        /* host controller reset */
#define OHCI_CLF        (1 << 1)        /* control list filled */
#define OHCI_BLF        (1 << 2)        /* bulk list filled */
#define OHCI_OCR        (1 << 3)        /* ownership change request */
#define OHCI_SOC        (3 << 16)       /* scheduling overrun count */

#define HcInterruptStatus 0x0C
#define HcInterruptEnable 0x10
#define HcInterruptDisable 0x14

/*
 * masks used with interrupt registers:
 * HcInterruptStatus (intrstatus)
 * HcInterruptEnable (intrenable)
 * HcInterruptDisable (intrdisable)
 */
#define OHCI_INTR_SO	(1 << 0)	/* scheduling overrun */
#define OHCI_INTR_WDH	(1 << 1)	/* writeback of done_head */
#define OHCI_INTR_SF	(1 << 2)	/* start frame */
#define OHCI_INTR_RD	(1 << 3)	/* resume detect */
#define OHCI_INTR_UE	(1 << 4)	/* unrecoverable error */
#define OHCI_INTR_FNO	(1 << 5)	/* frame number overflow */
#define OHCI_INTR_RHSC	(1 << 6)	/* root hub status change */
#define OHCI_INTR_OC	(1 << 30)	/* ownership change */
#define OHCI_INTR_MIE	(1 << 31)	/* master interrupt enable */

#define HcHCCA 0x18

// Legacy emulation registers (if enabled in the revision register, bit8)
#define HceControl 0x100

static int _stop_ohci(u8 bus, u8 dev, u8 fn)
{
    u32 val, bar0;

    bar0 = dnc_read_conf(0xfff0, bus, dev, fn, 0x10);
    if ((bar0 != 0xffffffff) && (bar0 != 0)) {
        val = mem64_read32(bar0 + HcHCCA);
        printf("Found OHCI controller at %02x:%02x:%x, BAR0 @%08x, HCCA @%08x\n",
               bus, dev, fn, bar0, val);
        val = mem64_read32(bar0 + HcControl);
        if (val & OHCI_CTRL_IR) { // Interrupt routing enabled, we must request change of ownership
            u32 temp;
            printf("Requesting Change of Ownership on OHCI controller %02x:%02x:%x\n",
                   bus, dev, fn);
            /* this timeout is arbitrary.  we make it long, so systems
             * depending on usb keyboards may be usable even if the
             * BIOS/SMM code seems pretty broken.
             */
            temp = 500;	/* arbitrary: five seconds */
            
            mem64_write32(bar0 + HcInterruptEnable, OHCI_INTR_OC); // Enable OwnershipChange interrupt
            mem64_write32(bar0 + HcCommandStatus, OHCI_OCR); // Request OwnershipChange
            while (mem64_read32(bar0 + HcControl) & OHCI_CTRL_IR) {
                tsc_wait(1000);
                if (--temp == 0) {
                    printf("OHCI HC takeover failed on %02x:%02x:%x ! (BIOS/SMM bug)\n",
                           bus, dev, fn);
                    return -1;
                }
            }

            // Shutdown
            mem64_write32(bar0 + HcInterruptDisable, OHCI_INTR_MIE);
            val = mem64_read32(bar0 + HcControl);
            val &= OHCI_CTRL_RWC;
            mem64_write32(bar0 + HcControl, val);
            /* flush the writes */
            val = mem64_read32(bar0 + HcControl);
        } else {
            printf("OHCI controller %02x:%02x:%x not in use by BIOS/SMM, skipping\n",
                   bus, dev, fn);
        }
        val = mem64_read32(bar0 + HcRevision);
        if (val & (1 << 8)) { // Legacy emulation is supported
            val = mem64_read32(bar0 + HceControl);
            if (val & (1 << 0)) {
                printf("Legacy support enabled!\n");
            }
        }
    }

    return 0;
}

#define PCI_CLASS_SERIAL_USB_OHCI     0x0c0310

static void stop_usb(void)
{
    u32 val;
    u8 bus = 0;
    u8 dev;
    u8 fn;

    // If SMM is disabled, USB functionality may have been lost.
    if (disable_smm)
	return;

    // Scan bus 0 for OHCI controllers and disable them if used by BIOS/SMM
    for (dev=0; dev<0x18; dev++) {
        for (fn=0; fn<7; fn++) {
            val = dnc_read_conf(0xfff0, bus, dev, fn, 8);
            if ((val>>8) == PCI_CLASS_SERIAL_USB_OHCI)
                (void)_stop_ohci(bus, dev, fn);
        }
    }
}

static void set_wrap32_disable(void)
{
    u32 lo, hi;
    asm volatile("rdmsr" : "=d"(hi), "=a"(lo) : "c"(MSR_HWCR));
    lo |= (1UL<<17);
    asm volatile("wrmsr" :: "d"(hi), "a"(lo), "c"(MSR_HWCR));
}

static void clear_bsp_flag(void)
{
    u32 lo, hi;
    asm volatile("rdmsr" : "=d"(hi), "=a"(lo) : "c"(MSR_APIC_BAR));
    lo &= ~(1UL<<8);
    asm volatile("wrmsr" :: "d"(hi), "a"(lo), "c"(MSR_APIC_BAR));
}

int main(void)
{
    u32 uuid;
    struct node_info *info;
    struct part_info *part;
    int wait;
    
    openconsole(&dev_rawcon_r, &dev_stdcon_w);

    if (check_api_version() < 0)
        return -1;

    /* Only in effect on core0, but only required for 32-bit code. */
    set_wrap32_disable();
 
    dnc_master_ht_id = dnc_init_bootloader(&uuid, &dnc_asic_mode, &dnc_chip_rev,
					   __com32.cs_cmdline);
    if (dnc_master_ht_id == -2)
	start_user_os();

    if (dnc_master_ht_id < 0)
        return -1;

    info = get_node_config(uuid);
    if (!info)
        return -1;

    part = get_partition_config(info->partition);
    if (!part)
        return -1;

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
            return -1;
    }

    /* Must run after SCI is operational */
    local_chipset_fixup();

    if (info->sync_only) {
	tsc_wait(5000);
	start_user_os();
    }

    if (dnc_init_caches() < 0)
        return -1;

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
            return -1;

	wait = 100;
	while (!unify_all_nodes()) {
	    if (wait > 100000) {
		wait = 100;
	    }
	    tsc_wait(wait);
	    wait <<= 2;
	    dnc_check_fabric(info);
	}

        (void)dnc_check_mctr_status(0);
        (void)dnc_check_mctr_status(1);
        
        update_e820_map();
        start_user_os();
    } else {
        /* Slave */
	u32 val;

        clear_bsp_flag();

        cleanup_stack();

        stop_usb();

        // Set G3x02c FAB_CONTROL bit 30.
        dnc_write_csr(0xfff0, H2S_CSR_G3_FAB_CONTROL, 1<<30);
 
        printf("Numascale NumaChip awaiting fabric set-up by master node...\n");
	// print_status();
        while (1) {
            val = dnc_check_mctr_status(0);
            val = dnc_check_mctr_status(1);
            
            val = dnc_read_csr(0xfff0, H2S_CSR_G3_FAB_CONTROL);
            if ((val & (1<<31))) {
                printf("Go-ahead seen, jumping to trampoline...\n");
                break;
            }

	    dnc_check_fabric(info);
            tsc_wait(1000);
        }

        /* Disable legacy PIC interrupts and cache*/
        disable_xtpic();
	disable_cache();

	/* Let master know we're ready for remapping/integration */
	dnc_write_csr(0xfff0, H2S_CSR_G3_FAB_CONTROL, val & ~(1<<31));
        while (1) {
            asm volatile("hlt" ::: "memory");
        }
    }

    return -1;
}
