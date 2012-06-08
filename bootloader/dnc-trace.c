/*
 * This source code including any derived information including but
 * not limited by net-lists, fpga bit-streams, and object files is the
 * confidential and proprietary property of
 *
 * Numascale AS
 * Enebakkveien 302A
 * NO-1188 Oslo
 * Norway
 *
 * Any unauthorized use, reproduction or transfer of the information
 * provided herein is strictly prohibited.
 *
 * Copyright (c) 2008-2012
 * Numascale AS Oslo, Norway.
 * All Rights Reserved.
 */

#ifdef TRACING

#include "dnc-trace.h"
#include "dnc-commonlib.h"
#include "dnc-bootloader.h"
#include "ht-trace.h"

void system_trace(void)
{
    if (!trace_buf_size) {
	printf("Trace buffer not requested\n");
	return;
    }

    if (!trace_buf) {
	printf("Trace buffer not allocated yet\n");
	return;
    }

    printf("Tracing HT activity...buf @ 0x%016llx for 0x%08x\n", trace_buf, trace_buf_size);
    start_ht_trace(0);
    tsc_wait(100);
    stop_ht_trace(0);
}

#else

void system_trace(void)
{
}

#endif
