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
// Copyright © 2008-2011
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
static int num_devices;
static struct numachip_device **device_list;

#define CSR_SPACE_SIZE 32*1024

static inline uint32_t u32bswap(uint32_t val)
{
    asm volatile("bswap %0" : "+r"(val));
    return val;
}

/**
 * numachip_get_device_list - Get list of NumaChip devices currently available
 * @num_devices: optional. If non-NULL, set to the number of devices
 * returned in the array.
 *
 * Return a NULL-terminated array of NumaChip devices.  The array can be
 * released with numachip_free_device_list().
 */
struct numachip_device **numachip_get_device_list(int *num)
{
    struct numachip_device **l = NULL;
    int i;

    if (num)
	*num = 0;

    pthread_mutex_lock(&device_list_lock);

    if (!num_devices)
	num_devices = numachip_init(&device_list);

    if (num_devices < 0) {
	errno = -num_devices;
	goto out;
    }

    l = calloc(num_devices + 1, sizeof (struct ibv_device *));
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
    void *csr = MAP_FAILED;
    off_t csr_base;
    int memfd = -1;
    char cfgpath[NUMACHIP_SYSFS_PATH_MAX];
    int cfg_space_fn0 = -1;
    int cfg_space_fn1 = -1;
    uint32_t val;

    context = malloc(sizeof(struct numachip_context));
    if (!context)
	return NULL;

    snprintf(cfgpath, NUMACHIP_SYSFS_PATH_MAX,
	     "%s.%d/config", device->dev_path, 0);
    cfgpath[255] = '\0';
    cfg_space_fn0 = open(cfgpath, O_RDWR);
    if (cfg_space_fn0 < 0)
	goto err;

    snprintf(cfgpath, NUMACHIP_SYSFS_PATH_MAX,
	     "%s.%d/config", device->dev_path, 1);
    cfgpath[255] = '\0';
    cfg_space_fn1 = open(cfgpath, O_RDWR);
    if (cfg_space_fn1 < 0)
	goto err;

    if (pread(cfg_space_fn0, &val, 4, 0x04) != 4)
	goto err;

    if (val & (1 << 1)) {
        // If BAR0 is set, use it
	if (pread(cfg_space_fn0, &val, 4, 0x10) != 4)
	    goto err;
        if (!val) {
            fprintf(stderr, "BAR0 not set, unable to determine CSR base\n");
            goto err;
        }
        csr_base = (off_t)val | (1ULL<<15);
    } else {
        // Device not responding to BAR0, check if we've fixed the expansion rom
        // address register to hold the global csr base.
	if (pread(cfg_space_fn0, &val, 4, 0x30) != 4)
	    goto err;
        if (!val || (val & 1)) {
            fprintf(stderr, "Unable to determine CSR base\n");
	    goto err;
        }
        csr_base = (((off_t)val & 0xffff0000ULL) << 16) | 0x0000fff00000ULL | (1ULL<<15);
    }

    memfd = open("/dev/mem", O_RDWR);
    if (memfd < 0) {
        fprintf(stderr, "Unable to open </dev/mem>\n");
        goto err;
    }

    csr = mmap(NULL, CSR_SPACE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, memfd, csr_base);
    if (csr == MAP_FAILED)
	goto err;

    printf("Mapped CSR base @ %012llx\n", (unsigned long long)csr_base);

    context->device = device;
    context->cfg_space_fd[0] = cfg_space_fn0;
    context->cfg_space_fd[1] = cfg_space_fn1;
    context->memfd = memfd;
    context->csr_space = (uint32_t *)csr;

    return context;

err:
    free(context);

    if (csr != MAP_FAILED) munmap(csr, CSR_SPACE_SIZE);
    if (memfd >= 0) close(memfd);
    if (cfg_space_fn1 >= 0) close(cfg_space_fn1);
    if (cfg_space_fn0 >= 0) close(cfg_space_fn0);

    return NULL;
}

/**
 * numachip_close_device - Release device
 */
int numachip_close_device(struct numachip_context *context)
{
    int memfd = context->memfd;
    int cfg_space_fn0 = context->cfg_space_fd[0];
    int cfg_space_fn1 = context->cfg_space_fd[1];
    void *csr = context->csr_space;

    free(context);

    if (csr != MAP_FAILED) munmap(csr, CSR_SPACE_SIZE);
    if (memfd >= 0) close(memfd);
    if (cfg_space_fn1 >= 0) close(cfg_space_fn1);
    if (cfg_space_fn0 >= 0) close(cfg_space_fn0);

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
			uint16_t offset, uint32_t value)
{
    context->csr_space[offset/4] = u32bswap(value);
}

/**
 * numachip_read_config - Read Config Space
 */
uint32_t numachip_read_config(struct numachip_context *context,
			      uint8_t fn, uint16_t offset)
{
    uint32_t val;
    if (fn > 1)
	return 0xffffffff;
    if (pread64(context->cfg_space_fd[fn], &val, 4, offset) != 4)
	return 0xffffffff;
    return val;
}

/**
 * numachip_write_csr - Write ConfigSpace
 */
void numachip_write_config(struct numachip_context *context,
			   uint8_t fn, uint16_t offset, uint32_t value)
{
    if (fn > 1)
	return;
    (void)pwrite(context->cfg_space_fd[fn], &value, 4, offset);
}
