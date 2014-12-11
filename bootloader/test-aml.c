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

uint16_t dnc_node_count = 0;
int verbose = 2;
bool remote_io = 1;
bool test_manufacture = 0;
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

static void launch(const char *cmdline)
{
	printf("[%s]\n", cmdline);
	int rc = system(cmdline);
	if (rc)
		 printf("warning: command returned rc %d\n", rc);
}

static void gen(const int nnodes)
{
	char filename[32];

	snprintf(filename, sizeof(filename), "SSDT-%dnodes.amx", nnodes);

	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	assert(fd != -1);

	if (sizeof(table)) {
		assert(write(fd, table, sizeof(table)) == sizeof(table));
		printf("wrote %s (%zd bytes, %d nodes)\n", filename, sizeof(table), nnodes);
	} else {
		uint32_t extra_len;
		dnc_node_count = nnodes;
		unsigned char *extra = remote_aml(&extra_len);

		acpi_sdt_p ssdt = (acpi_sdt_p)malloc(sizeof(struct acpi_sdt) + extra_len);
		memcpy(ssdt->sig.s, "SSDT", 4);
		ssdt->revision = ACPI_REV;
		memcpy(ssdt->oemid, "NUMASC", 6);
		memcpy(ssdt->oemtableid, "NCONNECT", 8);
		ssdt->oemrev = 0;
		memcpy(ssdt->creatorid, "INTL", 4);
		ssdt->creatorrev = 0x20100528;
		memcpy(ssdt->data, extra, extra_len);
		ssdt->len = offsetof(struct acpi_sdt, data) + extra_len;
		ssdt->checksum = 0;
		ssdt->checksum = checksum(ssdt, ssdt->len);

		assert(write(fd, ssdt, ssdt->len) == ssdt->len);
		printf("wrote %s (%d bytes, %d nodes)\n", filename, ssdt->len, nnodes);
		free(ssdt);
	}

	assert(close(fd) == 0);

	char cmdline[64], filename2[32];

	/* Disassemble generated AML to .dsl file */
	snprintf(cmdline, sizeof(cmdline), "iasl -vs -w3 -d %s 2>/dev/null", filename);
	launch(cmdline);
	strncpy(filename2, filename, sizeof(filename2));
	strcpy(strrchr(filename2, '.'), ".dsl");

	/* Reassemble to .aml file */
	snprintf(cmdline, sizeof(cmdline), "iasl -vs -w3 %s >/dev/null", filename2);
	launch(cmdline);
	strcpy(strrchr(filename2, '.'), ".aml");

	/* Diff output */
	snprintf(cmdline, sizeof(cmdline), "diff %s %s", filename, filename2);
	launch(cmdline);
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
	nodes[0].bsp_ht       = 0;
	nodes[0].ht[nodes[0].bsp_ht].pdom = 0;

	for (node_info_t *node = &nodes[1]; node < &nodes[AML_MAXNODES]; node++) {
		node_info_t *last = node - 1;

		node->io_base      = last->io_limit + 1;
		node->io_limit     = node->io_base + (last->io_limit - last->io_base);
		node->mmio32_base  = last->mmio32_limit + 1;
		node->mmio32_limit = node->mmio32_base + (last->mmio32_limit - last->mmio32_base);
		node->mmio64_base  = last->mmio64_limit + 1;
		node->mmio64_limit = node->mmio64_base + (last->mmio64_limit - last->mmio64_base);
		node->bsp_ht       = last->bsp_ht;
		node->ht[node->bsp_ht].pdom = last->ht[last->bsp_ht].pdom + 4;
	}

	gen(AML_MAXNODES);

	free(nodes);
	return 0;
}

