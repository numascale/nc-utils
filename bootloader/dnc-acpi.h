// $Id:$
//
// dnc-acpi.h -- Support functions for ACPI
//
// Author: Arne Georg Gleditsch <arne.gleditsch@numascale.com>
//
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

#ifndef __DNC_ACPI
#define __DNC_ACPI

#include <inttypes.h>

struct acpi_rsdp {
    unsigned char sig[8];
    uint8_t checksum;
    unsigned char oemid[6];
    uint8_t revision;
    uint32_t rsdt_addr;
    uint32_t len;
    uint64_t xsdt_addr;
    uint8_t echecksum;
} __attribute__((packed));

struct acpi_sdt {
    union {
	char s[4];
	uint32_t l;
    } sig;
    uint32_t len;
    uint8_t revision;
    uint8_t checksum;
    unsigned char oemid[6];
    unsigned char oemtableid[8];
    uint32_t oemrev;
    unsigned char creatorid[4];
    uint32_t creatorrev;
    unsigned char data[0];
} __attribute__((packed));

struct acpi_core_affinity {
    uint8_t type;
    uint8_t len;
    uint8_t prox_low;
    uint8_t apic_id;
    unsigned int enabled:1;
    unsigned int flags:31;
    uint8_t sapic_eid;
    unsigned int prox_hi:24;
    char reserved[4];
} __attribute__((packed));

struct acpi_mem_affinity {
    uint8_t type;
    uint8_t len;
    uint32_t prox_dom;
    char reserved1[2];
    uint64_t mem_base;
    uint64_t mem_size;
    char reserved2[4];
    unsigned int enabled:1;
    unsigned int hotplug:1;
    unsigned int nonvol:1;
    unsigned int reserved3:29;
    char reserved4[8];
} __attribute__((packed));

struct acpi_x2apic_affinity {
    uint8_t type;
    uint8_t len;
    char reserved1[2];
    uint32_t prox_dom;
    uint32_t x2apic_id;
    unsigned int enabled:1;
    unsigned int flags:31;
    uint32_t clock_dom;
    char reserved2[4];
} __attribute__((packed));

struct acpi_local_apic {
    uint8_t type;
    uint8_t len;
    uint8_t proc_id;
    uint8_t apic_id;
    uint32_t flags;
} __attribute__((packed));

struct acpi_local_x2apic {
    uint8_t type;
    uint8_t len;
    char reserved1[2];
    uint32_t x2apic_id;
    uint32_t flags;
    uint32_t proc_uid;
} __attribute__((packed));

struct acpi_mcfg_allocation {
    uint64_t address;		/* Base address, processor-relative */
    uint16_t pci_segment;	/* PCI segment group number */
    uint8_t start_bus_number;	/* Starting PCI Bus number */
    uint8_t end_bus_number;	/* Final PCI Bus number */
    uint32_t reserved;
} __attribute__((packed));

#define STR_DW_H(a) (uint32_t)(a[0] + (a[1]<<8) + (a[2]<<16) + (a[3]<<24))
#define STR_DW_N(a) (uint32_t)((a[0]<<24) + (a[1]<<16) + (a[2]<<8) + a[3])

typedef struct acpi_sdt *acpi_sdt_p;

void debug_acpi(void);
uint8_t checksum(void *addr, int len);
acpi_sdt_p find_sdt(char *sig);
acpi_sdt_p find_child(const char *sig,
				   acpi_sdt_p parent,
				   int ptrsize);
int replace_child(const char *sig, acpi_sdt_p new,
			 acpi_sdt_p parent,
			 int ptrsize);
acpi_sdt_p find_root(const char *sig);
int replace_root(const char *sig, acpi_sdt_p new);

#endif
