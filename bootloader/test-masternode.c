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

#define _GNU_SOURCE 1

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <sched.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "dnc-regs.h"
#include "dnc-types.h"
#include "dnc-access.h"
#include "dnc-route.h"
#include "dnc-config.h"

#include "dnc-commonlib.h"
#include "dnc-masterlib.h"

int link_watchdog = 0;

void wait_key(void)
{
    char ch;
    printf("... (press any key to continue) ... ");
    while (fread(&ch, 1, 1, stdin) == 0)
        ;
    printf("\n");
}

void tsc_wait(u32 mticks) {
    usleep((useconds_t)mticks*1000);
}

void sighandler(int sig)
{
    printf("Received signal %d. aborting!\n", sig);
    dnc_write_csr(0xfff0, H2S_CSR_G0_RAW_CONTROL, 0x1000); // Reset RAW engine
    exit(-1);
}

/* Not implemented here. */
int udp_open(void) {
    return 1;
}
void udp_broadcast_state(int handle, void *buf, int len) {
}
int udp_read_state(int handle, void *buf, int len) {
    return 0;
}

int dnc_master_ht_id;     /* HT id of NC on master node, equivalent to nc_node[0].nc_ht_id */
int dnc_asic_mode;
u16 dnc_node_count = 0;
nc_node_info_t nc_node[128];
u16 ht_pdom_count = 0;
u16 apic_per_node;
u16 ht_next_apic;
u32 dnc_top_of_mem;       /* Top of mem, in 16MB chunks */
u8 post_apic_mapping[256]; /* POST APIC assigments */

/* Traversal info per node.  Bit 7: seen, bits 5:0 rings walked. */
u8 nodedata[4096];

#define DRAM_MAP_SHIFT 24

int
sci_fabric_setup(void)
{
    u32 val;
    u16 i, node;

    tally_local_node(0);
    tally_all_remote_nodes();
    
    for (i = 0; i < dnc_node_count; i++) {
        node = (i == 0) ? 0xfff0 : nc_node[i].sci_id;
        printf("Loading SCC microcode on node %03x.\n", nc_node[i].sci_id);
        load_scc_microcode(node);
        printf("Disabling SCC MIB timeout on node %03x.\n", nc_node[i].sci_id);
        val = dnc_read_csr(node, H2S_CSR_G0_MIB_IBC);
        dnc_write_csr(node, H2S_CSR_G0_MIB_IBC, val | 0x40);
    }

    printf("SCC microcode loaded.\n");

    return 0;
    
    //[    0.000000] Bootmem setup node 1 0000000220000000-0000000420000000
    // Shared memory on node: 300000000-400000000 (4G window)
    cht_write_config(dnc_master_ht_id, 1, H2S_CSR_F1_RESOURCE_MAPPING_ENTRY_INDEX, 0);
    cht_write_config(dnc_master_ht_id, 1, H2S_CSR_F1_DRAM_BASE_ADDRESS_REGISTERS, 0x00030003);
    cht_write_config(dnc_master_ht_id, 1, H2S_CSR_F1_DRAM_LIMIT_ADDRESS_REGISTERS, 0x0003ff01);

    // SCC NGC window overlaps a bit because of the boundaries
    dnc_write_csr(0xfff0, H2S_CSR_G0_MIU_NGCM0_LIMIT, 12); // 200000000
    dnc_write_csr(0xfff0, H2S_CSR_G0_MIU_NGCM1_LIMIT, 16); // 400000000

    dnc_write_csr(0xfff0, H2S_CSR_G3_DRAM_SHARED_BASE, 0x300);
    dnc_write_csr(0xfff0, H2S_CSR_G3_DRAM_SHARED_LIMIT, 0x400);

    // Set ATTs.
    for (i = 1; i < dnc_node_count; i++) {
        node = nc_node[i].sci_id;
        dnc_write_csr(node, H2S_CSR_G0_ATT_INDEX, 0xa0000003); // Enable AutoInc, use 43:32 as index to ATT and set index to 3 (12G)
        dnc_write_csr(node, H2S_CSR_G0_ATT_ENTRY, nc_node[0].sci_id); //  12G-16G  (0x300000000 - 0x400000000)
    }

    for (i = 1; i < dnc_node_count; i++) {
        u8 ht_id = nc_node[i].nc_ht_id;
        node = nc_node[i].sci_id;
    
        // Set DRAM Limit on HT#1 to 0x2ffffffff
        dnc_write_conf(node, 0, 24+1, 1, 0x124, 0x5f);
        // Adjust Limit on HT#1 window to 0x2ffffffff
        dnc_write_conf(node, 0, 24+0, 1, 0x4c, 0x02ff0001);
        dnc_write_conf(node, 0, 24+1, 1, 0x4c, 0x02ff0001);

        // Insert new window for 0x300000000 - 0x3ffffffff to point to NC
        dnc_write_conf(node, 0, 24+0, 1, 0x54, 0x03ff0000 | ht_id);
        dnc_write_conf(node, 0, 24+0, 1, 0x50, 0x03000000 | 3);
        dnc_write_conf(node, 0, 24+1, 1, 0x54, 0x03ff0000 | ht_id);
        dnc_write_conf(node, 0, 24+1, 1, 0x50, 0x03000000 | 3);
    }

    for (i = 0; i < dnc_node_count; i++) {
        node = (i == 0) ? 0xfff0 : nc_node[i].sci_id;
        printf("Setting HReq_Ctrl on node %03x.\n", nc_node[i].sci_id);
//        dnc_write_csr(node, H2S_CSR_G3_HREQ_CTRL, (0<<26) | (1<<17) | (1<<12)); // CacheSize=0 (2GB), Error, H2S_Init
        dnc_write_csr(node, H2S_CSR_G3_HREQ_CTRL, (0<<26) | (1<<17)); // CacheSize=0 (2GB), Error

        val = dnc_read_csr(node, H2S_CSR_G3_FAB_CONTROL);
        printf("fab_control: %x\n", val);
        val |= 0x80000000UL;
        dnc_write_csr(node, H2S_CSR_G3_FAB_CONTROL, val);
    }
    
    return 0;
}

int read_config_file(char *file_name)
{
    int fd, len;
    char buf[16*1024];

    fd = open(file_name, O_RDONLY);
    if (fd < 0) {
	fprintf(stderr, "Unable to read <%s>.\n", file_name);
	return -1;
    }
    len = read(fd, buf, sizeof(buf));
    if (buf <= 0) {
	fprintf(stderr, "No config file contents?\n");
	return -1;
    }
    buf[len] = '\0';
    if (!parse_config_file(buf)) {
	printf("Error reading config file!\n");
	return -1;
    }

    return 0;
}

int main(int argc, char **argv)
{
    uint32_t val;
    int cpu_fam  = -1;
    cpu_set_t cset;
    u32 uuid;
    int chip_rev;
    struct node_info *info;
    struct part_info *part;
    int i;

    (void) signal(SIGINT, sighandler);
    
    /* Bind to core 0 */
    CPU_ZERO(&cset);
    CPU_SET(0, &cset);
    if (sched_setaffinity(0, sizeof(cset), &cset) != 0) {
	fprintf(stderr, "Unable to bind to core 0.\n");
	return -1;
    }
    
    asm volatile("mov $1, %%eax; cpuid" : "=a"(val) :: "ebx", "ecx", "edx");
    cpu_fam = (val >> 8) & 0xf;
    if (cpu_fam == 0xf)
        cpu_fam += (val >> 20) & 0xf;

    if (cpu_fam <= 0xf) {
        fprintf(stderr, "*** Unsupported CPU family %d\n", cpu_fam);
        return -1;
    }

    // Since our memory is allocated dynamically, we need to zero out this
    memset(nodedata, 0, sizeof(nodedata));

    dnc_master_ht_id = dnc_init_bootloader(&uuid, &dnc_asic_mode, &chip_rev,
					   argc > 1 ? argv[1] : NULL);
    if (dnc_master_ht_id < 0)
        return -1;

    info = get_node_config(uuid);
    if (!info)
	return -1;

    part = get_partition_config(info->partition);
    if (!part)
	return -1;

    if (part->master != info->sciid) {
        printf("This node (%03x) is not a master of partition %d!\n",
               info->sciid, info->partition);
        return -1;
    }

    for (i = 0; i < cfg_nodes; i++) {
	if (cfg_nodelist[i].uuid == uuid)
	    continue;
        if (cfg_nodelist[i].partition != info->partition)
            continue;
        nodedata[cfg_nodelist[i].sciid] = 0x80;
    }
    
    if (dnc_setup_fabric(info) < 0)
        return -1;

    if (dnc_init_caches() < 0)
        return -1;
    
    sci_fabric_setup();
    
    return 0;
}
