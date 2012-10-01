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

    acpi_sdt_p ssdt = malloc(16384);
    assert(ssdt);

    remote_aml(ssdt);

    assert(write(fd, ssdt, ssdt->len) == (ssize_t)ssdt->len);
    assert(close(fd) == 0);

    printf("wrote " FNAME " (%d bytes)\n", ssdt->len);
    printf("use 'iasl -d " FNAME "' to extract\n");

    free(ssdt);
    return 0;
}

