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
// Copyright © 2008-2012
// Numascale AS Oslo, Norway.
// All Rights Reserved.
//

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "numachip_lib.h"

#define NUMASCALE_VENDOR_ID   0x1B47
#define NUMACHIP_DEVICE_ID    0x0601
#define DEBUG_STATEMENT(x)

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

struct ent_global {
	unsigned int numachip_rev : 4;
	unsigned int size_x : 4;
	unsigned int size_y : 4;
	unsigned int size_z : 4;
	unsigned int northbridges : 3;
	unsigned int neigh_ht : 3;
	unsigned int neigh_link : 2;
	unsigned int symmetric : 1;
	unsigned int renumbering : 1;
	unsigned int remote_io : 1;
} __attribute__((packed));

struct ent_external {
	uint8_t id;
	uint8_t flags;
	uint64_t base : 48;
	uint16_t reserved;
	uint64_t limit : 48;
} __attribute__((packed));

union escrow_ent {
	uint8_t id;
	struct ent_global global;
	struct ent_external external;
} __attribute__((packed));

static void add_device(int32_t nodeid,
		       struct numachip_device ***dev_list,
		       int32_t *ndevices,
		       int32_t *list_size)
{
	struct numachip_device **new_list;
	struct numachip_device *dev;

	dev = malloc(sizeof(struct numachip_device));
	if (!dev)
		return;

	dev->nodeid=nodeid;

	if (*list_size <= *ndevices) {
		*list_size = *list_size ? *list_size * 2 : 1;
		new_list = realloc(*dev_list, *list_size * sizeof (struct numachip_device *));
		if (!new_list) {
			free(dev);
			return;
		}
		*dev_list = new_list;
	}

	(*dev_list)[(*ndevices)++] = dev;
}

HIDDEN int32_t numachip_init(struct numachip_device ***list)
{
	int32_t ndevices = 0;
	int32_t list_size = 0;
	int32_t sciid=0;
	int32_t x=0,y=0,z=0,i=0;
	int32_t fd;
	size_t len;
	struct acpi_sdt *oemn;
	union escrow_ent *cur;

	oemn = malloc(sizeof(struct acpi_sdt) + sizeof(union escrow_ent) + 1);
	if (!oemn) {
		fprintf(stderr, "Unable to allocate %ld bytes of memory for OEMN table\n",
			sizeof(struct acpi_sdt) + sizeof(union escrow_ent) + 1);
		return -1;
	}

	fd = open("/sys/firmware/acpi/tables/OEMN", O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Unable to open OEMN table: %d\n", fd);
		return -1;
	}

	len = read(fd, oemn, sizeof(struct acpi_sdt) + sizeof(union escrow_ent));
	if (len < 0) {
		fprintf(stderr, "Bad OEMN length: %ld\n", len);
		return -1;
	}

	cur = (union escrow_ent *)oemn->data;
	DEBUG_STATEMENT(printf("Escrow: numachip_rev=%d size=%d,%d,%d northbridges=%d neigh=%d,%d symmetric=%d renumbering=%d\n",
			       cur->global.numachip_rev, cur->global.size_x, cur->global.size_y, cur->global.size_z,
			       cur->global.northbridges, cur->global.neigh_ht, cur->global.neigh_link,
			       cur->global.symmetric, cur->global.renumbering));

	if (!cur->global.size_z) cur->global.size_z=1;
	if (!cur->global.size_y) cur->global.size_y=1;
	if (!cur->global.size_x) cur->global.size_x=1;

	for (z = 0; z < (cur->global.size_z); z++) {
		for (y = 0; y < (cur->global.size_y); y++) {
			for (x = 0; x < (cur->global.size_x); x++) {

				sciid= (z<<8) + (y << 4) + x;
				DEBUG_STATEMENT(printf("Node %d: sciid: 0x%03x\n",
						       i, sciid));
				i++;
				add_device(sciid, list, &ndevices, &list_size);
			}
		}
	}
	return ndevices;
}
