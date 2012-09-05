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

#ifndef __NUMACHIP_PCOUNTER_TEST_H
#define __NUMACHIP_PCOUNTER_TEST_H

#include <stdint.h>
#include "numachip_user.h"

#ifdef __cplusplus
#  define BEGIN_C_DECLS extern "C" {
#  define END_C_DECLS   }
#else /* !__cplusplus */
#  define BEGIN_C_DECLS
#  define END_C_DECLS
#endif /* __cplusplus */


/**
 * Counter select
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
nc_error_t counter_select(struct numachip_context *cntxt,
			  uint32_t counterno,
			  uint32_t val);

/**
 * counter_mask
 * Write to the counter mask register
 * H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_X
 *
 */
nc_error_t counter_mask(struct numachip_context *cntxt,
			uint32_t counterno,
			uint32_t val);

/**
 * counter_clear
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
nc_error_t counter_clear(struct numachip_context *cntxt,
			 uint32_t counterno);

/**
 * counter_restart
 * Clear only the performance register for this counter:
 * H2S_CSR_G3_PERFORMANCE_COUNTER_X_40_BIT_UPPER_BITS
 * and
 * H2S_CSR_G3_PERFORMANCE_COUNTER_X_40_BIT_LOWER_BITS
 */
nc_error_t counter_restart(struct numachip_context *cntxt,
			   uint32_t counterno);
/**
 * counter_stop
 * Clear both the mask register
 * H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_X
 * and the corresponding performance register
 * for this counter:
 * H2S_CSR_G3_PERFORMANCE_COUNTER_X_40_BIT_UPPER_BITS
 * and
 * H2S_CSR_G3_PERFORMANCE_COUNTER_X_40_BIT_LOWER_BITS
 */

nc_error_t counter_stop(struct numachip_context *cntxt,
			uint32_t counterno);

/**
 * counter_read - Read Performance Counter Register
 * H2S_CSR_G3_PERFORMANCE_COUNTER_X_40_BIT_UPPER_BITS
 * and
 * H2S_CSR_G3_PERFORMANCE_COUNTER_X_40_BIT_LOWER_BITS
 */
uint64_t counter_read(struct numachip_context *cntxt,
		      uint32_t counterno);


/**
 * counter_start
 *
 * In this order do all these operations:
 * - counter_clear
 * - counter_select
 * - counter_mask
 *
 */
nc_error_t counter_start(struct numachip_context *cntxt,
			 uint32_t counterno,
			 uint32_t event,
			 uint32_t mask);

/**
 * For an array of numachips do
 * counter_select
 */
nc_error_t counter_select_all(struct numachip_context **cntxt,
			      uint32_t num_nodes,
			      uint32_t counterno,
			      uint32_t val);

/**
 * For an array of numachips do
 * counter_mask
 */
nc_error_t counter_mask_all(struct numachip_context **cntxt,
			    uint32_t num_nodes,
			    uint32_t counterno,
			    uint32_t val);

/**
 * For an array of numachips do
 * counter_clear
 * 
 */
nc_error_t counter_clear_all(struct numachip_context **cntxt,
			     uint32_t num_nodes,
			     uint32_t counterno);

/**
 * For an array of numachips do
 * counter_restart
 * 
 */
nc_error_t counter_restart_all(struct numachip_context **cntxt,
			       uint32_t num_nodes,
			       uint32_t counterno);

/**
 * For an array of numachips do
 * counter_stop
 * 
 */
nc_error_t counter_stop_all(struct numachip_context **cntxt,
			    uint32_t num_nodes,
			    uint32_t counterno);

/**
 * For an array of numachips do
 * - print the counter_read result to stdout.
 * 
 */
void counter_print_all(struct numachip_context **cntxt,
		       uint32_t num_nodes,
		       uint32_t counterno);

/**
 * For an array of numachips do
 * counter_start
 * 
 */
nc_error_t counter_start_all(struct numachip_context **cntxt,
			     uint32_t num_nodes,
			     uint32_t counterno,
			     uint32_t event,
			     uint32_t mask);


/* count_api_start
 * Do numachip_fullstart_pcounter on an array of numachips
 * these predefied values:
 * Counter 0, source 1 and mask 6:
 * Select = 1, REM/HReq, Mask: 6 - HT-Request with ctag miss
 * Counter 1, source 1 and mask 5:
 * Select = 1, REM/HReq, Mask: 5 - HT-Request with ctag hit
 * Counter 2, source 6 and mask 3
 * Select = 6, MCTag Mask: 3 - CTag cache hit
 * Counter 3, source 6 and mask 2
 * Select = 6, MCTag Mask: 2 - CTag cache miss
 * 
 */
nc_error_t count_api_start(struct numachip_context **cntxt, uint32_t num_nodes);

/* count_api_stop
 * Do numachip_stop_pcounter on an array of numachips
 * these predefied values:
 * Counter 0, source 1 and mask 6:
 * Select = 1, REM/HReq, Mask: 6 - HT-Request with ctag miss
 * Counter 1, source 1 and mask 5:
 * Select = 1, REM/HReq, Mask: 5 - HT-Request with ctag hit
 * Counter 2, source 6 and mask 3
 * Select = 6, MCTag Mask: 3 - CTag cache hit
 * Counter 3, source 6 and mask 2
 * Select = 6, MCTag Mask: 2 - CTag cache miss
 * 
 */
nc_error_t count_api_stop(struct numachip_context **cntxt, uint32_t num_nodes);


/* count_api_read_rcache
 * Do numachip_get_pcounter on an array of numachips with
 * these predefied values:
 * misscounter: source 1 and mask 6:
 * Select = 1, REM/HReq, Mask: 6 - HT-Request with ctag miss
 * hitcounter, source 1 and mask 5:
 * Select = 1, REM/HReq, Mask: 5 - HT-Request with ctag hit
 *
 * Returns hitrate and missrate.
 */
void count_api_read_rcache(struct numachip_context *cntxt,
			   uint32_t misscounter,
			   uint32_t hitcounter, 
			   double *missrate,
			   double *hitrate,
			   uint64_t *total,
			   nc_error_t *error);


/* count_api_read_rcache2
 * Do numachip_get_pcounter on an array of numachips with
 * these predefied values:
 * misscounter: source 1 and mask 6:
 * Select = 1, REM/HReq, Mask: 6 - HT-Request with ctag miss
 * hitcounter, source 1 and mask 5:
 * Select = 1, REM/HReq, Mask: 5 - HT-Request with ctag hit
 *
 * Returns hitrate, missrate,total number of cache
 * transactions, number of cache hits and number of
 * cache misses.
 */
void count_api_read_rcache2(struct numachip_context *cntxt,
			   uint32_t misscounter,
			   uint32_t hitcounter, 
			   double *missrate,
			   double *hitrate,
			   uint64_t *total,
			   uint64_t *miss,
			   uint64_t *hit,
			    nc_error_t *error);
#endif
