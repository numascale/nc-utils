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
#include <pci/pci.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>

#include "numachip_user.h"

#define NUMASCALE_VENDOR_ID 0x1B47
#define NUMACHIP_DEVICE_ID  0x0601

static pthread_mutex_t device_list_lock = PTHREAD_MUTEX_INITIALIZER;
static int num_devices;
static struct numachip_device **device_list;

#define CSR_SPACE_SIZE 32*1024

static inline uint32_t u32bswap(uint32_t val)
{
    asm volatile("bswap %0" : "+r"(val));
    return val;
}

static uint32_t _read_csr(struct numachip_context *context, uint16_t offset)
{
    return u32bswap(context->csr_space[offset/4]);
}

static void _write_csr(struct numachip_context *context, uint16_t offset, uint32_t val)
{
    context->csr_space[offset/4] = u32bswap(val);
}

static struct numachip_context * _alloc_context(struct numachip_device *device)
{
    struct numachip_context *context;

    context = malloc(sizeof(struct numachip_context));
    if (!context)
	return NULL;

    context->ops.read_csr = _read_csr;
    context->ops.write_csr = _write_csr;
    
    return context;
}

static void _free_context(struct numachip_context *context)
{
    free(context);
}

static void add_device(struct pci_dev *pdev,
		       struct numachip_device ***dev_list,
		       int *ndevices,
		       int *list_size)
{
    struct numachip_device **new_list;
    struct numachip_device *dev;
    off_t csr_base;
    uint32_t val;

    val = pci_read_long(pdev, 0x04);
    if (val & (1 << 1)) {
        // If BAR0 is set, use it
        val = pci_read_long(pdev, 0x10);
        if (!val) {
            //fprintf(stderr, "BAR0 not set, unable to determine CSR base\n");
            return;
        }
        csr_base = (off_t)val | (1ULL<<15);
    } else {
        // Device not responding to BAR0, check if we've fixed the expansion rom
        // address register to hold the global csr base.
        val = pci_read_long(pdev, 0x30);
        if (!val || (val & 1)) {
            //fprintf(stderr, "Unable to determine CSR base\n");
	    return;
        }
        csr_base = (((off_t)val & 0xffff0000ULL) << 16) | 0x0000fff00000ULL | (1ULL<<15);
    }

    dev = malloc(sizeof(struct numachip_device));
    if (!dev)
	return;

    dev->ops.alloc_context = _alloc_context;
    dev->ops.free_context = _free_context;
    dev->csr_base = csr_base;

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

static int numachip_init(struct numachip_device ***list)
{
    struct pci_filter our_filter = { -1, -1, -1, -1, NUMASCALE_VENDOR_ID, NUMACHIP_DEVICE_ID };
    struct pci_access *pacc;
    struct pci_dev *pdev;
    int ndevices = 0;
    int list_size = 0;

    pacc = pci_alloc();
    if (pacc == NULL)
	return -ENOSYS;

    pci_init(pacc);
    pci_scan_bus(pacc);

    for (pdev = pacc->devices; pdev; pdev = pdev->next)
        if (pci_filter_match(&our_filter, pdev))
	    add_device(pdev, list, &ndevices, &list_size);

    pci_cleanup(pacc);

    return ndevices;
}

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

void numachip_free_device_list(struct numachip_device **list)
{
    free(list);
}

struct numachip_context *numachip_open_device(struct numachip_device *device)
{
    struct numachip_context *context;
    void *csr = NULL;
    int memfd;

    memfd = open("/dev/mem", O_RDWR);
    if (memfd < 0) {
        fprintf(stderr, "Unable to open </dev/mem>\n");
        return NULL;
    }

    csr = mmap(NULL, CSR_SPACE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, memfd, device->csr_base);
    if (csr == MAP_FAILED)
	goto err;
    
    context = device->ops.alloc_context(device);
    if (!context)
	goto err;

    printf("Mapped CSR base @ %012llx\n", (unsigned long long)device->csr_base);

    context->device = device;
    context->memfd = memfd;
    context->csr_space = (uint32_t *)csr;
  
    return context;

err:
    if (csr) munmap(csr, CSR_SPACE_SIZE);
    close(memfd);
    
    return NULL;
}

int numachip_close_device(struct numachip_context *context)
{	
    int memfd = context->memfd;
    void *csr = context->csr_space;
    
    context->device->ops.free_context(context);

    munmap(csr, CSR_SPACE_SIZE);
    close(memfd);
    
    return 0;
}
