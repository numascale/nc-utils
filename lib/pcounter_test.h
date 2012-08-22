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


//Single node, or all?
nc_error_t counter_select(struct numachip_context *cntxt, uint32_t counterno,uint32_t val);
nc_error_t counter_mask(struct numachip_context *cntxt, uint32_t counterno, uint32_t val);
nc_error_t counter_clear(struct numachip_context *cntxt, uint32_t counterno);
nc_error_t counter_restart(struct numachip_context *cntxt, uint32_t counterno);
nc_error_t counter_stop(struct numachip_context *cntxt, uint32_t counterno);
uint64_t counter_read(struct numachip_context *cntxt,uint32_t counterno);
nc_error_t counter_start(struct numachip_context *cntxt, uint32_t counterno, uint32_t event, uint32_t mask);
nc_error_t counter_select_all(struct numachip_context **cntxt, uint32_t num_nodes,uint32_t counterno,uint32_t val);
nc_error_t counter_mask_all(struct numachip_context **cntxt, uint32_t num_nodes, uint32_t counterno,uint32_t val);
nc_error_t counter_clear_all(struct numachip_context **cntxt, uint32_t num_nodes,uint32_t counterno);
nc_error_t counter_restart_all(struct numachip_context **cntxt, uint32_t num_nodes,uint32_t counterno);
nc_error_t counter_stop_all(struct numachip_context **cntxt, uint32_t num_nodes,uint32_t counterno);
void counter_print_all(struct numachip_context **cntxt, uint32_t num_nodes,uint32_t counterno);
nc_error_t counter_start_all(struct numachip_context **cntxt, uint32_t num_nodes, uint32_t counterno, uint32_t event, uint32_t mask);

nc_error_t count_api_start(struct numachip_context **cntxt, uint32_t num_nodes);
nc_error_t count_api_stop(struct numachip_context **cntxt, uint32_t num_nodes);
void count_api_read_rcache(struct numachip_context *cntxt,
			   uint32_t misscounter, uint32_t hitcounter, 
			   double *missrate,
			   double *hitrate,
			   uint64_t *total,
			   nc_error_t *error);
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
