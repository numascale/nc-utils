/*
 * Copyright (C) 2008-2015 Numascale AS, support@numascale.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

/*
 * Error codes are defined here.
 * - NUMACHIP_ERR_BUSY:
 * The performance counter has been selected by someone else.
 * The performance counter has non-zero value.
 * To solve: Clear the performance counter to claim ownership
 *
 * - NUMACHIP_ERR_INVALID_PARAMETER
 * An invalid performance counter has been selected.
 */
typedef enum {
	NUMACHIP_ERR_OK                = 0x000,
	NUMACHIP_ERR_INVALID_PARAMETER = 0x001,
	NUMACHIP_ERR_BUSY              = 0x002
} nc_error_t;

struct numachip_device;
struct numachip_context;

/**
 * numachip_get_device_list - Get list of NumaChip devices currently available
 * @num_devices: optional. If non-NULL, set to the number of devices
 * returned in the array.
 *
 * Return a NULL-terminated array of NumaChip devices.  The array can be
 * released with numachip_free_device_list().
 */
struct numachip_device **numachip_get_device_list(int32_t *num_devices);

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
			   uint16_t offset);
/**
 * numachip_write_csr - Write CSR
 */
void numachip_write_csr(struct numachip_context *context,
			uint16_t offset,
			uint32_t value);

/**
 * numachip_clear_pcounter
 * Clear the counter selection register
 * (H2S_CSR_G3_SELECT_COUNTER),
 * mask-register
 * ( H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_X)
 * and the corresponding performance register
 * (H2S_CSR_G3_PERFORMANCE_COUNTER_X_40_BIT_UPPER_BITS
 * and
 * (H2S_CSR_G3_PERFORMANCE_COUNTER_X_40_BIT_LOWER_BITS)
 * for this performance counter.
 */

void numachip_clear_pcounter(struct numachip_context *cntxt,
			     uint32_t counterno,
			     nc_error_t *error);

/**
 * numachip_restart_pcounter
 * Clear only the performance register for this counter:
 * H2S_CSR_G3_PERFORMANCE_COUNTER_X_40_BIT_UPPER_BITS
 * and
 * H2S_CSR_G3_PERFORMANCE_COUNTER_X_40_BIT_LOWER_BITS
 */
void numachip_restart_pcounter(struct numachip_context *cntxt,
			     uint32_t counterno,
			     nc_error_t *error);


/**
 * numachip_mask_pcounter
 * Write to the counter mask register
 * H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_X
 *
 */
void numachip_mask_pcounter(struct numachip_context *cntxt,
			    uint32_t counterno,
			    uint32_t mask,
			    nc_error_t *error);

/**
 * numachip_stop_pcounter
 * Clear both the mask register
 * H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_X
 * and the corresponding performance register
 * for this counter:
 * H2S_CSR_G3_PERFORMANCE_COUNTER_X_40_BIT_UPPER_BITS
 * and
 * H2S_CSR_G3_PERFORMANCE_COUNTER_X_40_BIT_LOWER_BITS
 */
void numachip_stop_pcounter(struct numachip_context *cntxt,
			    uint32_t counterno,
			    nc_error_t *error);

/**
 * numachip_get_pcounter_mask
 * Read the mask from the counter mask register
 * H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_X
 *
 */
uint32_t numachip_get_pcounter_mask(struct numachip_context *cntxt,
				    uint32_t counterno,
				    nc_error_t *error) ;

/**
 * numachip_get_pcounter - Read Performance Counter Register
 * H2S_CSR_G3_PERFORMANCE_COUNTER_X_40_BIT_UPPER_BITS
 * and
 * H2S_CSR_G3_PERFORMANCE_COUNTER_X_40_BIT_LOWER_BITS
 */
uint64_t numachip_get_pcounter(struct numachip_context *cntxt,
			       uint32_t counterno,
			       nc_error_t *error);

/**
 * numachip_get_pcounter
 * Reads the counter selection register
 * H2S_CSR_G3_SELECT_COUNTER
 * for this counter.
 */
uint32_t numachip_get_pcounter_select(struct numachip_context *cntxt,
				      uint32_t counterno,
				      nc_error_t *error);

/**
 * numachip_select_pcounter
 * Write the  the counter selection register
 * H2S_CSR_G3_SELECT_COUNTER
 * for this counter to select the source.
 *
 * There are eight different sources to choose from:
 * Select = 0, REM/SPrb
 * Select = 1, REM/HReq
 * Select = 2, LOC/SReq
 * Select = 3, LOC/HPrb
 * Select = 4, CData
 * Select = 5, FTag
 * Select = 6, MCTag
 * Select = 7, cHT-Cave
 */
void numachip_select_pcounter(struct numachip_context *cntxt,
			      uint32_t counterno,
			      uint32_t eventreg,
			      nc_error_t *error);

/**
 * numachip_fullstart_pcounter
 *
 * In this order do all these operations:
 * - numachip_clear_pcounter
 * - numachip_select_pcounter
 * - numachip_mask_pcounter
 *
 */
void numachip_fullstart_pcounter(struct numachip_context *cntxt,
				 uint32_t counterno,
				 uint32_t event,
				 uint32_t mask,
				 nc_error_t *error);

/**
 * For an array of numachips do
 * numachip_fullstart_pcounter
 */
void numachip_all_start_pcounter(struct numachip_context **cntxt,
				 uint32_t num_nodes,
				 uint32_t counterno,
				 uint32_t event,
				 uint32_t mask,
				 nc_error_t *error);

char *numachip_strerror(nc_error_t errorcode);

END_C_DECLS

#endif
