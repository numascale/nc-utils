#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "dnc-aml.h"
#include "dnc-acpi.h"

#define MAXLEN 65536

uint16_t dnc_node_count = 0;
int verbose = 2;
node_info_t *nodes;

/* Insert SSDT dumped from booting with verbose=2 into array */
static uint32_t table[] = {
};

void broadcast_error(const bool persistent, const char *format, ...)
{
}

uint8_t checksum(const acpi_sdt_p addr, const int len)
{
	uint8_t sum = 0;

	for (int i = 0; i < len; i++)
		sum -= *((uint8_t *)addr + i);

	return sum;
}

void gen(const int nnodes)
{
	char filename[32];

	snprintf(filename, sizeof(filename), "SSDT-%dnodes.aml", nnodes);

	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	assert(fd != -1);

	if (sizeof(table)) {
		assert(write(fd, table, sizeof(table)) == sizeof(table));
		printf("wrote %s (%zd bytes, %d nodes)\n", filename, sizeof(table), nnodes);
	} else {
		acpi_sdt_p ssdt = (acpi_sdt_p)malloc(MAXLEN);
		uint32_t extra_len;
		dnc_node_count = nnodes;
		unsigned char *extra = remote_aml(&extra_len);

		memcpy(ssdt->sig.s, "SSDT", 4);
		ssdt->revision = ACPI_REV;
		memcpy(ssdt->oemid, "NUMASC", 6);
		memcpy(ssdt->oemtableid, "N313NUMA", 8);
		ssdt->oemrev = 0;
		memcpy(ssdt->creatorid, "1B47", 4);
		ssdt->creatorrev = 1;
		memcpy(ssdt->data, extra, extra_len);
		ssdt->len = offsetof(struct acpi_sdt, data) + extra_len;
		ssdt->checksum = 0;
		ssdt->checksum = checksum(ssdt, ssdt->len);

		assert(write(fd, ssdt, ssdt->len) == ssdt->len);
		printf("wrote %s (%d bytes, %d nodes)\n", filename, ssdt->len, nnodes);
		free(ssdt);
	}

	assert(close(fd) == 0);
}

int main(void)
{
	nodes = (node_info_t *)malloc(sizeof(*nodes) * AML_MAXNODES);
	assert(nodes);

	nodes[0].io_base      = 0x000000;
	nodes[0].io_limit     = 0x00ffff;
	nodes[0].mmio32_base  = 0x80000000;
	nodes[0].mmio32_limit = 0x823fffff;
	nodes[0].mmio64_base  = 0x1200000000;
	nodes[0].mmio64_limit = 0x12ffffffff;

	for (node_info_t *node = &nodes[1]; node < &nodes[AML_MAXNODES]; node++) {
		node_info_t *last = node - 1;

		node->io_base      = last->io_limit + 1;
		node->io_limit     = node->io_base + (last->io_limit - last->io_base);
		node->mmio32_base  = last->mmio32_base + 1;
		node->mmio32_limit = node->mmio32_base + (last->mmio32_limit - last->mmio32_base);
		node->mmio64_base  = last->mmio64_base + 1;
		node->mmio64_limit = node->mmio64_base + (last->mmio64_limit - last->mmio64_base);
	}

	gen(1);
	gen(8);
	gen(AML_MAXNODES);

	free(nodes);
	printf("use 'iasl -d file.aml to extract\n");
	return 0;
}

