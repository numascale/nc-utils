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
#define MAXLEN 16384

uint16_t dnc_node_count = 3;
int verbose = 2;

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
		acpi_sdt_p ssdt = (acpi_sdt_p)malloc(MAXLEN);
		uint32_t extra_len;
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
		printf("wrote " FNAME " (%d bytes)\n", ssdt->len);
		free(ssdt);
	}

	printf("use 'iasl -d " FNAME "' to extract\n");
	assert(close(fd) == 0);
	return 0;
}

