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

#define ASSERT(cond) do { if (!(cond)) {				\
	    printf("%s(%d): assert (%s) failed!\n",			\
		   __FUNCTION__, __LINE__,				\
		   #cond);						\
	   } } while (0)

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

#define PRInode "node 0x%03x (%s)"
#define nodestr_offset(x) cfg_nodelist[x].sciid, cfg_nodelist[x].desc

#define DRAM_MAP_SHIFT 24ULL

int cpu_family(u16 scinode, u8 node);
void add_extd_mmio_maps(u16 scinode, u8 node, u8 idx, u64 start, u64 end, u8 dest);
void del_extd_mmio_maps(u16 scinode, u8 node, u8 idx);

int dnc_init_bootloader(u32 *p_uuid, int *p_asic_mode, int *p_chip_rev, const char *cmdline);
int dnc_setup_fabric(struct node_info *info);
int dnc_check_fabric(struct node_info *info);
u32 dnc_check_mctr_status(int cdata);
int dnc_init_caches(void);

enum node_state enter_reset(struct node_info *info);
enum node_state release_reset(struct node_info *info);
enum node_state validate_rings(struct node_info *info);
int handle_command(enum node_state cstate, enum node_state *rstate, 
		   struct node_info *info, struct part_info *part);
void wait_for_master(struct node_info *info, struct part_info *part);
                               
// These are helper functions used by various routines.
// The actual code is *not* located in dnc-commonlib.c
extern void tsc_wait(u32 mticks);
extern int read_config_file(char *file_name);

extern int dnc_asic_mode;
extern int dnc_chip_rev;
extern u32 max_mem_per_node;

extern char *next_label;
extern int sync_mode;
extern char *microcode_path;
extern int enable_vga_redir;
extern int disable_smm;
extern int renumber_bsp;
extern int mem_offline;
extern u64 trace_buf_size;
extern int verbose;

extern u16 shadow_rtbll[7][256];
extern u16 shadow_rtblm[7][256];
extern u16 shadow_rtblh[7][256];
extern u16 shadow_ltbl[7][256];

#define NODE_SYNC_STATES(state) \
    state(CMD_STARTUP) \
    state(RSP_SLAVE_READY) \
    state(CMD_ENTER_RESET) \
    state(RSP_RESET_ACTIVE) \
    state(CMD_RELEASE_RESET) \
    state(RSP_PHY_TRAINED) \
    state(RSP_PHY_NOT_TRAINED) \
    state(CMD_VALIDATE_RINGS) \
    state(RSP_RINGS_OK) \
    state(RSP_RINGS_NOT_OK) \
    state(CMD_SETUP_FABRIC) \
    state(RSP_FABRIC_READY) \
    state(RSP_FABRIC_NOT_READY) \
    state(CMD_VALIDATE_FABRIC) \
    state(RSP_FABRIC_OK) \
    state(RSP_FABRIC_NOT_OK) \
    state(CMD_CONTINUE) \
    state(RSP_NONE)

#define ENUM_DEF(state) state,
#define ENUM_NAMES(state) #state,

enum node_state { NODE_SYNC_STATES(ENUM_DEF) };
extern const char* node_state_name[];

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
