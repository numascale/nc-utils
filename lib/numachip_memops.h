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

#ifndef __NUMACHIP_MEMOPS_H
#define __NUMACHIP_MEMOPS_H

#include <stdint.h>

#ifdef __cplusplus
#  define BEGIN_C_DECLS extern "C" {
#  define END_C_DECLS   }
#else /* !__cplusplus */
#  define BEGIN_C_DECLS
#  define END_C_DECLS
#endif /* __cplusplus */

BEGIN_C_DECLS

struct numachip_sge {
	uint64_t from;
	uint64_t to;
	uint32_t length;
} __attribute__((aligned(16)));

/**
 * numachip_sge_copy - Optimized SG Copy
 */
void (*numachip_sge_copy)(struct numachip_sge *sg_list, size_t num_sge);

END_C_DECLS

#endif
