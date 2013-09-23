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
#define nodestr_offset(x) cfg_nodelist[x].sci, cfg_nodelist[x].desc

#define COL_DEFAULT   "\033[0m"
#define COL_RED       "\033[31m"
#define COL_YELLOW    "\033[33m"
#define CLEAR         "\033\143"
#define BANNER        "\033[1m\033[34m"
#define DRAM_SEGMENT_SHIFT 28 /* 256MB; ~20s test time */

#define assert(cond) do { if (!(cond)) {				\
	printf(COL_RED "Error: assertion '%s' failed in %s at %s:%d\n" COL_DEFAULT,	\
	    #cond, __FUNCTION__, __FILE__, __LINE__);			\
	broadcast_error(1, "Assertion '%s' failed in %s at %s:%d", \
	    #cond, __FUNCTION__, __FILE__, __LINE__); \
    } } while (0)

#define assertf(cond, format, args...) do { if (!(cond)) {			\
	printf(COL_RED "Error: " format COL_DEFAULT, ## args);					\
	broadcast_error(1, format, ## args); \
    } } while(0)

#define fatal(format, args...) do {						\
	printf(COL_RED "Error: " format COL_DEFAULT, ## args);					\
	broadcast_error(1, format, ## args); \
   } while (1)

#define fatal_reboot(format, args...) do {						\
	printf(COL_RED "Error: " format "; rebooting in 5s..." COL_DEFAULT, ## args);					\
	broadcast_error(0, format "; rebooting", ## args); \
	udelay(5000000);						\
	reset_cf9(0xa, 0);						\
   } while (1)

#define warning(format, args...) do {						\
	printf(COL_YELLOW "Warning: " format COL_DEFAULT "\n", ## args);					\
   } while (0)

#define error(format, args...) do {						\
	printf(COL_RED "Error: " format COL_DEFAULT "\n", ## args);					\
	broadcast_error(0, format, ## args); \
   } while (0)

#define error_remote(sci, name, ip, msg) do { \
	if (sci != 0xffffffff) \
		printf(COL_RED "Error on SCI%03x (%s): %s" COL_DEFAULT "\n", sci, name, msg); \
	else \
		printf(COL_RED "Error on %d.%d.%d.%d: %s" COL_DEFAULT "\n", \
			ip & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff, (ip >> 24) & 0xff, msg); \
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
    state(CMD_RESET_FABRIC) \
    state(RSP_RESET_COMPLETE) \
    state(CMD_TRAIN_FABRIC) \
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
#define UDP_SIG 0xdeafcafe
#define UDP_MAXLEN 256

enum node_state { NODE_SYNC_STATES(ENUM_DEF) };

struct state_bcast {
	uint32_t sig;
	enum node_state state;
	uint32_t uuid;
	uint32_t sci;
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

struct kvm_port {
	const char *name;
	int port;
};

const char *pr_size(uint64_t val);
void udelay(uint32_t usecs);
void wait_key(void);
int cpu_family(uint16_t scinode, uint8_t node);
void probefilter_tokens(const ht_t max_ht);
void detect_southbridge(void);
void disable_smi(void);
void enable_smi(void);
void critical_enter(void);
void critical_leave(void);
void adjust_oscillator(const char p_type[16], const uint32_t osc_setting);
int dnc_init_bootloader(uint32_t *p_chip_rev, char p_type[16], bool *p_asic_mode);
bool dnc_check_fabric(struct node_info *info);
uint32_t dnc_check_mctr_status(const int cdata);
void dnc_dram_initialise(void);
int dnc_init_caches(void);
int handle_command(enum node_state cstate, enum node_state *rstate,
                   struct node_info *info, struct part_info *part);
void broadcast_error(const bool persistent, const char *format, ...);
void check_error(void);
void wait_for_master(struct node_info *info, struct part_info *part);
void wake_core_local(const int apicid, const int vector);
void wake_core_global(const int apicid, const int vector);
void enable_probefilter(const ht_t max_ht);
void selftest_late_msrs(void);
void selftest_late_apiclvt(void);
void parse_cmdline(const int argc, const char *argv[]);
void dnc_dimmtest(const int testmask, const struct dimm_config dimms[2]);

extern bool dnc_asic_mode;
extern uint32_t dnc_chip_rev;
extern char dnc_card_type[16];

extern const char *config_file_name;
extern const char *next_label;
extern const char *microcode_path;
extern bool disable_smm;
extern bool disable_c1e;
extern int renumber_bsp;
extern bool mem_offline;
extern uint64_t trace_buf_size;
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
extern uint64_t pf_maxmem;
extern uint32_t max_mem_per_node;
extern int force_probefilteroff;
extern int force_probefilteron;
extern uint64_t mem_gap;
extern bool workaround_locks;
extern int enable_nbwdt;
extern bool pf_cstate6;
extern int disable_kvm;

extern const char *node_state_name[];

#endif
