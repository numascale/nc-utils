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

#ifndef __DNC_BOOTLOADER_H
#define __DNC_BOOTLOADER_H 1

#include "dnc-types.h"

#define MSG_PORT 4711

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

struct e820entry {
	uint64_t base;
	uint64_t length;
	uint32_t type;
} __attribute__((packed));

typedef struct ht_node_info {
	uint32_t base;		/* Start of DRAM at individual HT nodes, in 16MB chunks */
	uint32_t size;		/* Amount of DRAM at individual HT nodes, in 16MB chunks */
	uint16_t pdom;		/* Proximity domain of individual HT nodes */
	uint16_t cores;		/* Number of cores at individual HT nodes */
	uint16_t apic_base;
	uint32_t scrub;
} ht_node_info_t;

typedef struct nodes_info {
	uint32_t node_mem;          /* Amount of DRAM at dnc nodes, in 16MB chunks */
	uint32_t dram_base;
	uint32_t dram_limit;
	uint32_t mmio32_base;       /* Start of local MMIO32 mapping */
	uint32_t mmio32_limit;
	uint64_t mmio64_base;       /* Start of local MMIO64 mapping */
	uint64_t mmio64_limit;
	uint32_t io_base;           /* Start of local IO mapping */
	uint32_t io_limit;
	uint32_t ioapic_addr;
	ht_node_info_t ht[8];
	sci_t sci;                  /* Maps logical DNC node ids to physical (SCI) ids */
	uint16_t apic_offset;       /* Offset to shift APIC ids by when unifying */
	ht_t bsp_ht : 3;            /* Bootstrap processor HT ID (may be renumbered) */
	ht_t nb_ht_lo : 3;          /* Lowest Northbridge HT ID */
	ht_t nb_ht_hi : 3;          /* Highest Northbridge HT ID */
	ht_t nc_ht : 3;             /* HT id of dnc node dnc controller on local system */
	ht_t nc_neigh_ht : 3;       /* Our nearest neighbour HT node on local system */
	uint8_t nc_neigh_link : 2;
} node_info_t;

#define foreach_node(x) for (node_info_t *(x) = &nodes[0]; (x) < &nodes[dnc_node_count]; (x)++)
#define foreach_nb(x, y) for (ht_t (x) = (y)->nb_ht_lo; (x) <= (y)->nb_ht_hi; (x)++)

/* Traversal info per node.  Bit 7: seen, bits 5:0 rings walked */
extern uint8_t nodedata[4096];
extern uint8_t post_apic_mapping[256];
extern uint16_t dnc_node_count, dnc_core_count;
extern node_info_t *nodes;
extern struct node_info *local_info;
extern uint16_t ht_pdom_count;
extern uint16_t apic_per_node;
extern uint16_t ht_next_apic;
extern uint32_t dnc_top_of_mem;
extern char *asm_relocated;
extern unsigned char asm_relocate_start;
extern unsigned char asm_relocate_end;
extern uint64_t ht_base;
extern struct e820entry *orig_e820_map;
extern int orig_e820_len;
extern uint64_t old_mcfg_base;
extern uint32_t old_mcfg_len;

void set_cf8extcfg_enable(const int ht);
checked int read_config_file(const char *filename);
void udp_open(void);
void udp_broadcast_state(const void *buf, const size_t len);
checked int udp_read_state(void *buf, const size_t len, uint32_t *ip);
void mtrr_range(const uint64_t base, const uint64_t limit, const int type);

#endif

