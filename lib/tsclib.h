/* -*- Mode: C; c-basic-offset:8 ; indent-tabs-mode:t ; -*- */
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

#ifndef _TSCLIB_H_
#define _TSCLIB_H_ 1

#include <inttypes.h>

#ifdef __cplusplus
#  define BEGIN_C_DECLS extern "C" {
#  define END_C_DECLS   }
#else /* !__cplusplus */
#  define BEGIN_C_DECLS
#  define END_C_DECLS
#endif /* __cplusplus */

BEGIN_C_DECLS

extern double tsc_getresolution(void);
extern double tsc_getsecs(void);
extern double tsc_sample2secs(uint64_t sample);

END_C_DECLS

#ifdef __cplusplus
# define __TSC_INLINE __inline__
#else
# define __TSC_INLINE static __inline__
#endif

__TSC_INLINE uint64_t tsc_getsample(void)
{
	uint32_t low, high;

	asm volatile("rdtsc\n\t" : "=a" (low), "=d" (high));
	return ((uint64_t)low | ((uint64_t)high << 32));
}

#endif
