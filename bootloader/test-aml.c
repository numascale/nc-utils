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

#define FNAME "SSDT.dat"

uint16_t dnc_node_count = 3;
int verbose = 2;

/* Insert SSDT dumped from booting with verbose=2 into array */
static uint32_t table[] = {
};

void wait_key(void)
{
}

uint8_t checksum(void *addr, int len)
{
	uint8_t sum = 0;
	int i;

	for (i = 0; i < len; i++)
		sum += *(uint8_t *)(addr + i);

	return sum;
}

int main(void)
{
	int fd = open(FNAME, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	if (fd == -1) {
		perror("failed to write");
		return 1;
	}

	if (sizeof(table)) {
		assert(write(fd, table, sizeof(table)) == sizeof(table));
		printf("wrote " FNAME " (%zd bytes)\n", sizeof(table));
	} else {
		struct acpi_sdt ssdt;
		uint32_t len;
		unsigned char *data = remote_aml(&len);

		memcpy(&ssdt.sig.s, "SSDT", 4);
		ssdt.len = len + sizeof(ssdt);
		ssdt.revision = ACPI_REV;
		memcpy(&ssdt.oemid, "NUMASC", 6);
		memcpy(&ssdt.oemtableid, "N313NUMA", 8);
		ssdt.oemrev = 0;
		memcpy(&ssdt.creatorid, "1B47", 4);
		ssdt.creatorrev = 1;

		assert(write(fd, &ssdt, sizeof(ssdt)) == sizeof(ssdt));
		assert(write(fd, data, len) == len);
		printf("wrote " FNAME " (%d bytes)\n", ssdt.len);
		free(data);
	}

	printf("use 'iasl -d " FNAME "' to extract\n");
	assert(close(fd) == 0);
	return 0;
}

