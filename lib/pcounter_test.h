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

#ifdef __cplusplus
#  define BEGIN_C_DECLS extern "C" {
#  define END_C_DECLS   }
#else /* !__cplusplus */
#  define BEGIN_C_DECLS
#  define END_C_DECLS
#endif /* __cplusplus */

BEGIN_C_DECLS

void count_api_start(struct numachip_context **cntxt, unsigned int num_nodes);
void count_api_stop(struct numachip_context **cntxt, unsigned int num_nodes);
//void count_api_read_rate(struct numachip_context *cntxt, double *missrate, double *hitrate, unsigned long long *total,nc_error_t *error);
void count_api_read_rcache(struct numachip_context *cntxt,
			   unsigned int misscounter, unsigned int hitcounter, 
			   double *missrate,
			   double *hitrate,
			   unsigned long long *total,
			   nc_error_t *error);
#endif
