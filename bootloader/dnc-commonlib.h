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

#ifndef __DNC_COMMONLIB_H
#define __DNC_COMMONLIB_H 1

#include "dnc-types.h"
#include "dnc-config.h"

#define disable_cache() do { \
    asm volatile("wbinvd\n" \
	"mov %%cr0, %%eax\n" \
	"or $0x40000000, %%eax\n" \
	"mov %%eax, %%cr0\n" ::: "eax", "memory"); \
	} while (0)

#define enable_cache() do { \
    asm volatile("wbinvd\n" \
	"mov %%cr0, %%eax\n" \
	"and $~0x40000000, %%eax\n" \
	"mov %%eax, %%cr0\n" ::: "eax", "memory"); \
	} while (0)

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

void add_extd_mmio_maps(u16 scinode, u8 node, u8 idx, u64 start, u64 end, u8 dest);
void del_extd_mmio_maps(u16 scinode, u8 node, u8 idx);

int dnc_init_bootloader(u32 *p_uuid, int *p_asic_mode, int *p_chip_rev, const char *cmdline);
int dnc_setup_fabric(struct node_info *info);
int dnc_check_fabric(struct node_info *info);
u32 dnc_check_mctr_status(int cdata);
int dnc_init_caches(void);

enum node_state enter_reset(struct node_info *info, struct part_info *part);
enum node_state release_reset(struct node_info *info, struct part_info *part);
enum node_state validate_rings(struct node_info *info, struct part_info *part);
int handle_command(enum node_state cstate, enum node_state *rstate, 
		   struct node_info *info, struct part_info *part);
void wait_for_master(struct node_info *info, struct part_info *part);
                               
// These are helper functions used by various routines.
// The actual code is *not* located in dnc-commonlib.c
extern void tsc_wait(u32 mticks);
extern int read_config_file(char *file_name);

extern int dnc_asic_mode;
extern int dnc_chip_rev;

extern char *next_label;
extern int sync_mode;
extern char *microcode_path;
extern int disable_smm;

extern u16 shadow_rtbll[7][256];
extern u16 shadow_rtblm[7][256];
extern u16 shadow_rtblh[7][256];
extern u16 shadow_ltbl[7][256];

enum node_state {
    CMD_STARTUP,
    RSP_SLAVE_READY,
    CMD_ENTER_RESET,
    RSP_RESET_ACTIVE,
    CMD_RELEASE_RESET,
    RSP_PHY_TRAINED,
    RSP_PHY_NOT_TRAINED,
    CMD_VALIDATE_RINGS,
    RSP_RINGS_OK,
    RSP_RINGS_NOT_OK,
    CMD_SETUP_FABRIC,
    RSP_FABRIC_READY,
    RSP_FABRIC_NOT_READY,
    CMD_VALIDATE_FABRIC,
    RSP_FABRIC_OK,
    RSP_FABRIC_NOT_OK,
    CMD_CONTINUE,
    RSP_NONE,
};

struct state_bcast {
    enum node_state state;
    u32 uuid;
    u32 sciid;
    u32 tid;
};

typedef struct ht_node_info {
    u32 base;		/* Start of DRAM at individual HT nodes, in 16MB chunks */
    u32 size;		/* Amount of DRAM at individual HT nodes, in 16MB chunks */
    u16 pdom;		/* Proximity domain of individual HT nodes */
    u16 cores;		/* Number of cores at individual HT nodes */
    u16 apic_base;
    u32 cpuid;
} ht_node_info_t;

typedef struct nc_node_info {
    u16 sci_id;		/* Maps logical dnc node ids to physical (sci) ids */
    u32 node_mem;	/* Amount of DRAM at dnc nodes, in 16MB chunks */
    u32 addr_base;
    u32 addr_end;
    ht_node_info_t ht[8];
    u16 nc_ht_id;	/* HT id of dnc node dnc controller on local system */
    u16 apic_offset;	/* Offset to shift APIC ids by when unifying */
    u8 nc_neigh;	/* Our nearest neighbour HT node on local system */
} nc_node_info_t;


extern int udp_open(void);
extern void udp_broadcast_state(int handle, void *buf, int len);
extern int udp_read_state(int handle, void *buf, int len);

#endif
