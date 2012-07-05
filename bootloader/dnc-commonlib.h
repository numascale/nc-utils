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

#ifndef __DNC_COMMONLIB_H
#define __DNC_COMMONLIB_H 1

#include "dnc-types.h"
#include "dnc-config.h"

#define assert(cond) do { if (!(cond)) {				\
	printf("Error: assertion '%s' failed at %s:%d\n",		\
	    #cond, __FUNCTION__, __LINE__);				\
	wait_key();							\
    } } while (0)

#define assertf(cond, format, ...) do { if (!(cond)) {			\
	printf("Error: ");						\
	printf(format, __VA_ARGS__);					\
	wait_key();							\
    } } while(0)

#define fatal(format, ...) do {						\
	printf("Error: ");						\
	printf(format, __VA_ARGS__);					\
	wait_key();							\
   } while (0)

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

struct state_bcast {
    enum node_state state;
    u32 uuid;
    u32 sciid;
    u32 tid;
};

void wait_key(void);
int cpu_family(u16 scinode, u8 node);
void add_extd_mmio_maps(u16 scinode, u8 node, u8 idx, u64 start, u64 end, u8 dest);
void del_extd_mmio_maps(u16 scinode, u8 node, u8 idx);
void detect_southbridge(void);
void disable_smi(void);
void enable_smi(void);
void critical_enter(void);
void critical_leave(void);
int dnc_init_bootloader(u32 *p_uuid, int *p_asic_mode, int *p_chip_rev, const char *cmdline);
int dnc_setup_fabric(struct node_info *info);
int dnc_check_fabric(struct node_info *info);
u32 dnc_check_mctr_status(int cdata);
int dnc_init_caches(void);
int handle_command(enum node_state cstate, enum node_state *rstate,
		   struct node_info *info, struct part_info *part);
void wait_for_master(struct node_info *info, struct part_info *part);

extern int dnc_asic_mode;
extern int dnc_chip_rev;
extern u32 max_mem_per_node;

extern char *next_label;
extern int sync_mode;
extern char *microcode_path;
extern int enable_vga_redir;
extern int disable_smm;
extern int disable_c1e;
extern int renumber_bsp;
extern int mem_offline;
extern u64 trace_buf;
extern u32 trace_buf_size;
extern int verbose;
extern int nc_neigh, nc_neigh_link;
extern int forwarding_mode;
extern int remote_io;

extern const char* node_state_name[];

#endif
