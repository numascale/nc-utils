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

#include <inttypes.h>
#include <stdbool.h>

#include "dnc-config.h"
#include "ddr_spd.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define roundup(x, n) (((x) + ((n) - 1)) & (~((n) - 1)))
#define cpu_relax() asm volatile("pause" ::: "memory")
#define PRInode "node 0x%03x (%s)"
#define nodestr_offset(x) cfg_nodelist[x].sciid, cfg_nodelist[x].desc

#define COL_DEFAULT   "\033[0m"
#define COL_RED       "\033[31m"
#define COL_YELLOW    "\033[33m"
#define CLEAR         "\033\143"
#define BANNER        "\033[1m\033[37m"
#define DRAM_SEGMENT_SHIFT 28 /* 256MB; ~20s test time */

#define assert(cond) do { if (!(cond)) {				\
	printf(COL_RED "Error: assertion '%s' failed in %s at %s:%d\n",	\
	    #cond, __FUNCTION__, __FILE__, __LINE__);			\
	printf(COL_DEFAULT);					\
	while (1) cpu_relax();						\
    } } while (0)

#define assertf(cond, format, args...) do { if (!(cond)) {			\
	printf(COL_RED "Error: ");						\
	printf(format, ## args);					\
	printf(COL_DEFAULT);					\
	while (1) cpu_relax();						\
    } } while(0)

#define fatal(format, args...) do {						\
	printf(COL_RED "Error: ");						\
	printf(format, ## args);					\
	printf(COL_DEFAULT);					\
	while (1) cpu_relax();						\
   } while (0)

#define fatal_reboot(format, args...) do {						\
	printf(COL_RED "Error: ");						\
	printf(format, ## args);					\
	printf("; rebooting in 15s...");		\
	printf(COL_DEFAULT);					\
	udelay(15000000);						\
	reset_cf9(0xa, 0);						\
   } while (0)

#define warning(format, args...) do {						\
	printf(COL_YELLOW "Warning: ");						\
	printf(format, ## args);					\
	printf(COL_DEFAULT "\n");					\
   } while (0)

#define error(format, args...) do {						\
	printf(COL_RED "Error: ");						\
	printf(format, ## args);					\
	printf("\n");					\
   } while (0)

#ifdef __i386__
#define disable_cache() do { \
    asm volatile( \
	"mov %%cr0, %%eax\n" \
	"or $0x40000000, %%eax\n" \
	"mov %%eax, %%cr0\n" \
	"wbinvd\n" ::: "eax", "memory"); \
	} while (0)

#define enable_cache() do { \
    asm volatile( \
	"mov %%cr0, %%eax\n" \
	"and $~0x40000000, %%eax\n" \
	"mov %%eax, %%cr0\n" ::: "eax", "memory"); \
	} while (0)
#endif

#define DRAM_MAP_SHIFT 24ULL

#define NODE_SYNC_STATES(state) \
    state(CMD_STARTUP) \
    state(RSP_SLAVE_READY) \
    state(CMD_ENTER_RESET) \
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
    state(RSP_ERROR) \
    state(RSP_NONE)

#define ENUM_DEF(state) state,
#define ENUM_NAMES(state) #state,

enum node_state { NODE_SYNC_STATES(ENUM_DEF) };

struct state_bcast {
	enum node_state state;
	uint32_t uuid;
	uint32_t sciid;
	uint32_t tid;
};

/* Structs to hold DIMM configuration from SPD readout */
struct dimm_config {
	ddr2_spd_eeprom_t spd;
	uint8_t addr_pins;
	uint8_t column_size;
	uint8_t cs_map;
	uint8_t width;
	uint8_t eight_bank;
	int mem_size; /* Size of DIMM in GByte powers of 2 */
};

const char *pr_size(uint64_t val);
void udelay(uint32_t usecs);
void wait_key(void);
int cpu_family(uint16_t scinode, uint8_t node);
void probefilter_tokens(int nodes);
void detect_southbridge(void);
void disable_smi(void);
void enable_smi(void);
void critical_enter(void);
void critical_leave(void);
int adjust_oscillator(char p_type[16], uint32_t osc_setting);
int dnc_init_bootloader(uint32_t *p_uuid, uint32_t *p_chip_rev, char p_type[16], bool *p_asic_mode, const char *cmdline);
int dnc_check_fabric(struct node_info *info);
uint32_t dnc_check_mctr_status(int cdata);
void dnc_dram_initialise(void);
int dnc_init_caches(void);
int handle_command(enum node_state cstate, enum node_state *rstate,
                   struct node_info *info, struct part_info *part);
void wait_for_master(struct node_info *info, struct part_info *part);
void wake_core_local(const int apicid, const int vector);
void wake_core_global(const int apicid, const int vector);
void enable_probefilter(const int nodes);
void selftest_late_msrs(void);
void dnc_dimmtest(const int testmask, const struct dimm_config dimms[2]);

extern bool dnc_asic_mode;
extern uint32_t dnc_chip_rev;
extern char dnc_card_type[16];

extern char *config_file_name;
extern char *next_label;
extern char *microcode_path;
extern bool disable_smm;
extern bool disable_c1e;
extern int renumber_bsp;
extern bool mem_offline;
extern uint64_t trace_buf;
extern uint32_t trace_buf_size;
extern int verbose;
extern int nc_neigh, nc_neigh_link;
extern int forwarding_mode;
extern int sync_interval;
extern bool enable_relfreq;
extern bool singleton;
extern bool handover_acpi;
extern int remote_io;
extern bool boot_wait;
extern int family;
extern uint32_t tsc_mhz;
extern uint32_t pf_maxmem;
extern bool pf_vga_local;
extern uint32_t max_mem_per_node;
extern int force_probefilteroff;
extern int force_probefilteron;
extern uint64_t mem_gap;
extern bool workaround_locks;
extern int enable_nbwdt;

extern const char *node_state_name[];

#endif
