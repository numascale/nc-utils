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

#define IMPORT_RELOCATED(sym) extern volatile uint8_t sym ## _relocate
#define REL8(sym) ((uint8_t *)((volatile uint8_t *)asm_relocated + ((volatile uint8_t *)&sym ## _relocate - (volatile uint8_t *)&asm_relocate_start)))
#define REL16(sym) ((uint16_t *)((volatile uint8_t *)asm_relocated + ((volatile uint8_t *)&sym ## _relocate - (volatile uint8_t *)&asm_relocate_start)))
#define REL32(sym) ((uint32_t *)((volatile uint8_t *)asm_relocated + ((volatile uint8_t *)&sym ## _relocate - (volatile uint8_t *)&asm_relocate_start)))
#define REL64(sym) ((uint64_t *)((volatile uint8_t *)asm_relocated + ((volatile uint8_t *)&sym ## _relocate - (volatile uint8_t *)&asm_relocate_start)))

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
	uint32_t cpuid;
	uint32_t scrub;
} ht_node_info_t;

typedef struct nc_node_info {
	uint32_t node_mem;	/* Amount of DRAM at dnc nodes, in 16MB chunks */
	uint32_t dram_base;
	uint32_t dram_limit;
	uint32_t mmio_base;	/* Start of local MMIO mapping, in 16MB chunks */
	uint32_t mmio_end;
	ht_node_info_t ht[8];
	uint16_t sci_id;		/* Maps logical DNC node ids to physical (SCI) ids */
	uint16_t nc_ht_id;	/* HT id of dnc node dnc controller on local system */
	uint16_t apic_offset;	/* Offset to shift APIC ids by when unifying */
	uint8_t nc_neigh;	/* Our nearest neighbour HT node on local system */
} nc_node_info_t;

/* Traversal info per node.  Bit 7: seen, bits 5:0 rings walked */
extern uint8_t nodedata[4096];
extern uint8_t post_apic_mapping[256];
extern uint16_t dnc_node_count;
extern int dnc_master_ht_id;     /* HT id of NC on master node, equivalent to dnc_node_ht_id[0] */
extern nc_node_info_t nc_node[128];
extern uint16_t ht_pdom_count;
extern uint16_t apic_per_node;
extern uint16_t ht_next_apic;
extern uint32_t dnc_top_of_dram;
extern uint32_t dnc_top_of_mem;
extern char *asm_relocated;
extern unsigned char asm_relocate_start;
extern unsigned char asm_relocate_end;

void set_cf8extcfg_enable(const int ht);
int read_config_file(char *file_name);
int udp_open(void);
void udp_broadcast_state(int handle, void *buf, int len);
int udp_read_state(int handle, void *buf, int len);

#endif

