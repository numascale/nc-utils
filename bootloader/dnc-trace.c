/*
 * Copyright (C) 2008-2012 Numascale AS, support@numascale.com
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
    udelay(100);
    stop_ht_trace(0);
    dump_ht_trace(0);
}

#else

void system_trace(void)
{
}

#endif
