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

#ifndef __NUMACHIP_USER_H
#define __NUMACHIP_USER_H

#include <stdint.h>

#ifdef __cplusplus
#  define BEGIN_C_DECLS extern "C" {
#  define END_C_DECLS   }
#else /* !__cplusplus */
#  define BEGIN_C_DECLS
#  define END_C_DECLS
#endif /* __cplusplus */

BEGIN_C_DECLS

struct numachip_device;
struct numachip_context;

struct numachip_device_ops {
    struct numachip_context * (*alloc_context)(struct numachip_device *device);
    void                      (*free_context)(struct numachip_context *context);
};

struct numachip_device {
    struct numachip_device_ops  ops;
    off_t                       csr_base;
};

struct numachip_context_ops {
    uint32_t (*read_csr)(struct numachip_context *context, uint16_t offset);
    void     (*write_csr)(struct numachip_context *context, uint16_t offset, uint32_t value);
};

struct numachip_context {
    struct numachip_device      *device;
    struct numachip_context_ops  ops;
    int                          memfd;
    uint32_t                    *csr_space;
};

/**
 * numachip_get_device_list - Get list of NumaChip devices currently available
 * @num_devices: optional. If non-NULL, set to the number of devices
 * returned in the array.
 *
 * Return a NULL-terminated array of NumaChip devices.  The array can be
 * released with numachip_free_device_list().
 */
struct numachip_device **numachip_get_device_list(int *num_devices);

/**
 * numachip_free_device_list - Free list from numachip_get_device_list()
 *
 * Free an array of devices returned from numachip_get_device_list().  Once
 * the array is freed, pointers to devices that were not opened with
 * numachip_open_device() are no longer valid.  Client code must open all
 * devices it intends to use before calling numachip_free_device_list().
 */
void numachip_free_device_list(struct numachip_device **list);

/**
 * numachip_open_device - Initialize device for use
 */
struct numachip_context *numachip_open_device(struct numachip_device *device);

/**
 * numachip_close_device - Release device
 */
int numachip_close_device(struct numachip_context *context);

/**
 * numachip_read_csr - Read CSR
 */
static inline uint32_t numachip_read_csr(struct numachip_context *context, uint16_t offset)
{
    return context->ops.read_csr(context, offset);
}

/**
 * numachip_write_csr - Write CSR
 */
static inline void numachip_write_csr(struct numachip_context *context, uint16_t offset, uint32_t value)
{
    context->ops.write_csr(context, offset, value);
}

END_C_DECLS

#endif
