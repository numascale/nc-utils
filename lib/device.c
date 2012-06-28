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
//#define DEBUG(...) printf(__VA_ARGS__)
#define DEBUG(...) do { } while (0)

static inline uint32_t u32bswap(uint32_t val)
{
    asm volatile("bswap %0" : "+r"(val));
    return val;
}

const char *numachip_device_str(numachip_device_type_t str) {

    switch (str) {
	case SCC: return "SCC";
	case LCXA: return "LCXA";
	case LCXB: return "LCXB";
	case LCYA: return "LCYA";
	case LCYB: return "LCYB";
	case LCZA: return "LCZA";
	case LCZB: return "LCZB";
	default: return "UNKNOWN_ENUM_VALUE";	    
    }
}

/**
 * numachip_get_device_list - Get list of NumaChip devices currently available
 * @num_devices: optional. If non-NULL, set to the number of devices
 * returned in the array.
 *
 * Return a NULL-terminated array of NumaChip devices.  The array can be
 * released with numachip_free_device_list().
 */
struct numachip_device **numachip_get_device_list(int32_t *num, const char *filename)
{
    struct numachip_device **l = NULL;
    int32_t i;

    if (num)
	*num = 0;

    pthread_mutex_lock(&device_list_lock);

    if (!num_devices)
	num_devices = numachip_init(&device_list, filename);

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
    off_t csr_base;
    int32_t memfd = -1;
    uint64_t val=NUMACHIP_CSR_BASE;
    int32_t i=0;

    context = malloc(sizeof(struct numachip_context));
    if (!context) {
      fprintf(stderr,"malloc failed errno %d\n", errno);
      return NULL;
    }

    /*
     * Global CSR is designed in a way that allows access to CSR 
     * on all nodes from any CPU in the system at (nodeid is 0x0,0x1, 0x10 etc)
     * global_csr_offset + nodeid <<16
     *
     * The ideal situation would be if struct numachip device ** numachip_get_device_list(int32_t*num)
     * returned a list with all the numachips in the coherent system.
     * 
     * When the PCI tables gets sorted out we will have a list of PCI devices like this: 
     * 00:1a.0 Host bridge: Device 1b47:0601 (rev 01)
     * 00:1a.1 Host bridge: Device 1b47:0602 (rev 01)
     * 
     * Typically for a 2 node system: 
     * 0000:00:1a.0 Host bridge: Device 1b47:0601 (rev 01)
     * 0000:00:1a.1 Host bridge: Device 1b47:0602 (rev 01)
     * 0001:00:1a.0 Host bridge: Device 1b47:0601 (rev 01)
     * 0001:00:1a.1 Host bridge: Device 1b47:0602 (rev 01)
     * As you can see two different domains where the pci domain number 
     * specifies the SCI nodeid. 
     *
     * The domain number gives reveals the SCI nodeid. 
     * 0010:00:1a.0 Host bridge: Device 1b47:0601 (rev 01)
     * 0010:00:1a.1 Host bridge: Device 1b47:0602 (rev 01)
     * refer to SCI nodeid = 0x010
     *
     * Until this functionality is available we are reading the nodids from the json file (val)
     *
     */
    
    csr_base = (((uint64_t) val)  | (1ULL<<15) | (device->nodeid<<16));

    memfd = open("/dev/mem", O_RDWR);
    if (memfd < 0) {
      fprintf(stderr, "Unable to open </dev/mem>\n");
      goto err;
    }
    
    context->csr_space.csr = MAP_FAILED;

    context->csr_space.csr = mmap(NULL, CSR_SPACE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, memfd, csr_base | i<<16);  	
    if (context->csr_space.csr == MAP_FAILED) 	goto err;
    DEBUG("Mapped CSR base @ %012llx\n", (unsigned long long)csr_base);
    DEBUG("CSR  @ %012llx\n", (unsigned long long)context->csr_space.csr);

    context->device = device;
    context->memfd = memfd;
    return context;

err:
    if (context->csr_space.csr != MAP_FAILED) munmap(context->csr_space.csr, CSR_SPACE_SIZE);
      
    free(context);
    if (memfd >= 0) close(memfd);

    return NULL;
}

/**
 * numachip_close_device - Release device
 */
int32_t numachip_close_device(struct numachip_context *context)
{
 
    int32_t memfd = context->memfd;
    if (memfd >= 0) close(memfd);
    
    if (context->csr_space.csr != MAP_FAILED) {
	DEBUG("UnMapped CSR base @ %012llx\n", (unsigned long long)context->csr_space.csr);
	munmap(context->csr_space.csr, CSR_SPACE_SIZE);
    }
    
    free(context);

    return 0;
}

/**
 * numachip_read_csr - Read CSR
 */
uint32_t numachip_read_csr(struct numachip_context *context,
			   uint16_t offset
			   )
{
  DEBUG("csr = 0x%x", context->csr_space.csr[offset/4]);
  DEBUG("\n");
  return u32bswap(context->csr_space.csr[offset/4]);
}

/**
 * numachip_write_csr - Write CSR
 */
void numachip_write_csr(struct numachip_context *context,
			uint16_t offset, 
			uint32_t value
			)
{
    context->csr_space.csr[offset/4] = u32bswap(value);
}

