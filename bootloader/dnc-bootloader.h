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

#include <stdint.h>
#include "dnc-types.h"

#define IMPORT_RELOCATED(sym) extern volatile u8 sym ## _relocate
#define REL8(sym) ((u8 *)((volatile u8 *)asm_relocated + ((volatile u8 *)&sym ## _relocate - (volatile u8 *)&asm_relocate_start)))
#define REL16(sym) ((u16 *)((volatile u8 *)asm_relocated + ((volatile u8 *)&sym ## _relocate - (volatile u8 *)&asm_relocate_start)))
#define REL32(sym) ((u32 *)((volatile u8 *)asm_relocated + ((volatile u8 *)&sym ## _relocate - (volatile u8 *)&asm_relocate_start)))
#define REL64(sym) ((u64 *)((volatile u8 *)asm_relocated + ((volatile u8 *)&sym ## _relocate - (volatile u8 *)&asm_relocate_start)))
#define BOOTSTRAP_DELAY 1000

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
    u64 base;
    u64 length;
    u32 type;
} __attribute__((packed));

typedef struct ht_node_info {
    u32 base;		/* Start of DRAM at individual HT nodes, in 16MB chunks */
    u32 size;		/* Amount of DRAM at individual HT nodes, in 16MB chunks */
    u16 pdom;		/* Proximity domain of individual HT nodes */
    u16 cores;		/* Number of cores at individual HT nodes */
    u16 apic_base;
    u32 cpuid;
} ht_node_info_t;

typedef struct nc_node_info {
    u32 node_mem;	/* Amount of DRAM at dnc nodes, in 16MB chunks */
    u32 addr_base;
    u32 addr_end;
    u32 mmio_base;	/* Start of local MMIO mapping, in 16MB chunks */
    u32 mmio_end;
    ht_node_info_t ht[8];
    u16 sci_id;		/* Maps logical DNC node ids to physical (SCI) ids */
    u16 nc_ht_id;	/* HT id of dnc node dnc controller on local system */
    u16 apic_offset;	/* Offset to shift APIC ids by when unifying */
    u8 nc_neigh;	/* Our nearest neighbour HT node on local system */
} nc_node_info_t;

/* Traversal info per node.  Bit 7: seen, bits 5:0 rings walked */
extern u8 nodedata[4096];
extern u8 post_apic_mapping[256];
extern u16 dnc_node_count;
extern int dnc_master_ht_id;     /* HT id of NC on master node, equivalent to dnc_node_ht_id[0] */
extern nc_node_info_t nc_node[128];
extern u16 ht_pdom_count;
extern u16 apic_per_node;
extern u16 ht_next_apic;
extern u32 dnc_top_of_dram;
extern u32 dnc_top_of_mem;
extern char *asm_relocated;
extern unsigned char asm_relocate_start;
extern unsigned char asm_relocate_end;

int read_config_file(char *file_name);
int udp_open(void);
void udp_broadcast_state(int handle, void *buf, int len);
int udp_read_state(int handle, void *buf, int len);

#endif

