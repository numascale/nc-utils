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

#ifndef __NUMACHIP_USER_H
#define __NUMACHIP_USER_H

#include <stdint.h>
#include "numachip_error.h"

#ifdef __cplusplus
#  define BEGIN_C_DECLS extern "C" {
#  define END_C_DECLS   }
#else /* !__cplusplus */
#  define BEGIN_C_DECLS
#  define END_C_DECLS
#endif /* __cplusplus */

BEGIN_C_DECLS

#define NUMACHIP_CSR_BASE 0x3fff00000000ULL;


typedef enum {
  SCC = 0,
  LCXA,
  LCXB,
  LCYA,
  LCYB,
  LCZA,
  LCZB
} numachip_device_type_t;

struct numachip_device;
struct numachip_context;

struct numachip_sge {
    uint64_t from;
    uint64_t to;
    uint32_t length;
} __attribute__((aligned(16)));


//const char *numachip_device_str(numachip_device_type_t str);

/**
 * numachip_get_device_list - Get list of NumaChip devices currently available
 * @num_devices: optional. If non-NULL, set to the number of devices
 * returned in the array.
 *
 * Return a NULL-terminated array of NumaChip devices.  The array can be
 * released with numachip_free_device_list().
 */
struct numachip_device **numachip_get_device_list(int32_t *num_devices, const char *filename);

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
int32_t numachip_close_device(struct numachip_context *context);

/**
 * numachip_read_csr - Read CSR
 */
uint32_t numachip_read_csr(struct numachip_context *context,
			   uint16_t offset
			   );
/**
 * numachip_write_csr - Write CSR
 */
void numachip_write_csr(struct numachip_context *context,
			uint16_t offset, uint32_t value
			);

/**
 * numachip_read_config - Read Config Space
 */
uint32_t numachip_read_config(struct numachip_context *context,
			      uint8_t fn, uint16_t offset);

/**
 * numachip_write_config - Write Config Space
 */
void numachip_write_config(struct numachip_context *context,
			   uint8_t fn, uint16_t offset, uint32_t value);

/**
 * numachip_select_counter - Select Performance Counter
 */
void numachip_select_pcounter(struct numachip_context *cntxt,
			      uint32_t counterno,
			      uint32_t eventreg,
			      nc_error_t *error);
void numachip_clear_pcounter(struct numachip_context *cntxt,
			     uint32_t counterno,
			     nc_error_t *error);
void numachip_mask_pcounter(struct numachip_context *cntxt,
			    uint32_t counterno,
			    uint32_t mask,
			    nc_error_t *error);
void numachip_stop_pcounter(struct numachip_context *cntxt,
			    uint32_t counterno,
			    nc_error_t *error);
uint32_t numachip_get_pcounter_mask(struct numachip_context *cntxt,
					uint32_t counterno,
					nc_error_t *error) ;
uint64_t numachip_get_pcounter(struct numachip_context *cntxt,
					 uint32_t counterno,
					 nc_error_t *error);
uint32_t numachip_get_pcounter_select(struct numachip_context *cntxt,
					  uint32_t counterno,
					  nc_error_t *error);

void numachip_fullstart_pcounter(struct numachip_context *cntxt,
				 uint32_t counterno,
				 uint32_t event,
				 uint32_t mask,
				 nc_error_t *error);
    
/**
 * numachip_sge_copy - Optimized SG Copy
 */
void numachip_sge_copy(struct numachip_sge *sg_list, int32_t num_sge);

END_C_DECLS

#endif
