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
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "numachip_lib.h"

static pthread_mutex_t device_list_lock = PTHREAD_MUTEX_INITIALIZER;
static int32_t num_devices;
static struct numachip_device **device_list;
#define CSR_SPACE_SIZE 32*1024

/**
 * numachip_get_device_list - Get list of NumaChip devices currently available
 * @num_devices: optional. If non-NULL, set to the number of devices
 * returned in the array.
 *
 * Return a NULL-terminated array of NumaChip devices.  The array can be
 * released with numachip_free_device_list().
 */
struct numachip_device **numachip_get_device_list(int32_t *num)
{
	struct numachip_device **l = NULL;
	int32_t i;

	if (num)
		*num = 0;

	pthread_mutex_lock(&device_list_lock);

	if (!num_devices)
		num_devices = numachip_init(&device_list);

	if (num_devices < 0) {
		errno = -num_devices;
		goto out;
	}

	l = calloc(num_devices + 1, sizeof (struct numachip_device *));
	if (!l) {
		errno = ENOMEM;
		goto out;
	}

	for (i = 0; i < num_devices; ++i)
		l[i] = device_list[i];
	if (num)
		*num = num_devices;

out:
	pthread_mutex_unlock(&device_list_lock);
	return l;
}

/**
 * numachip_free_device_list - Free list from numachip_get_device_list()
 *
 * Free an array of devices returned from numachip_get_device_list().  Once
 * the array is freed, pointers to devices that were not opened with
 * numachip_open_device() are no longer valid.  Client code must open all
 * devices it intends to use before calling numachip_free_device_list().
 */
void numachip_free_device_list(struct numachip_device **list)
{
	free(list);
}

/**
 * numachip_open_device - Initialize device for use
 */
struct numachip_context *numachip_open_device(struct numachip_device *device)
{
	struct numachip_context *context;
	const off_t csr_base = NUMACHIP_CSR_BASE | (device->nodeid<<16) | (1ULL<<15);

	context = malloc(sizeof(struct numachip_context));
	if (!context) {
		fprintf(stderr,"malloc failed errno %d\n", errno);
		return NULL;
	}

	context->csr_space = MAP_FAILED;
	
	context->memfd = open("/dev/mem", O_RDWR);
	if (context->memfd < 0) {
		fprintf(stderr, "Unable to open </dev/mem>\n");
		goto err;
	}

	context->csr_space = mmap(NULL, CSR_SPACE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, context->memfd, csr_base);
	if (context->csr_space == MAP_FAILED)
		goto err;

	context->device = device;
	return context;

err:
	if (context->memfd >= 0)
		close(context->memfd);

	free(context);

	return NULL;
}

/**
 * numachip_close_device - Release device
 */
int32_t numachip_close_device(struct numachip_context *context)
{
	munmap(context->csr_space, CSR_SPACE_SIZE);
	close(context->memfd);

	free(context);

	return 0;
}

/**
 * numachip_read_csr - Read CSR
 */
uint32_t numachip_read_csr(struct numachip_context *context,
			   uint16_t offset)
{
	return u32bswap(context->csr_space[offset/4]);
}

/**
 * numachip_write_csr - Write CSR
 */
void numachip_write_csr(struct numachip_context *context,
			uint16_t offset,
			uint32_t value)
{
	context->csr_space[offset/4] = u32bswap(value);
}

/**
 * numachip_error_str
 */

char *numachip_strerror(nc_error_t errorcode) {

	if (errorcode == NUMACHIP_ERR_OK) return "NUMACHIP_ERR_OK";
	if (errorcode == NUMACHIP_ERR_INVALID_PARAMETER) return "NUMACHIP_ERR_INVALID_PARAMETER";
	if (errorcode == NUMACHIP_ERR_BUSY) return "NUMACHIP_ERR_BUSY";

	return "NUMACHIP_UNKNOWN_ERROR";
};
