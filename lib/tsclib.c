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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <inttypes.h>

#include "ncutils_atomic.h"
#include "tsclib.h"

static double _cycletime = 0.0;
static int _initialized = 0;

#define PROC_CPUINFO "/proc/cpuinfo"
#define PROC_LINE_SZ 1024

static char *_getprocline(const char *fname, long *offset, const char *token, char *value)
{
	char *ret = NULL;
	char *gsrv;
	char line[PROC_LINE_SZ];
	FILE *f;

	f = fopen(fname, "r");
	if (!f) {
		return NULL;
	}

	if (fseek(f, *offset, SEEK_SET)) {
		fclose(f);
		return NULL;
	}

	for (; (gsrv = fgets(line, PROC_LINE_SZ, f)); ) {
		if (!strncasecmp(line, token, strlen(token))) {
			int pos;
			/* We assume the format being <token> : <value> */
			pos = strcspn(line+strlen(token), ":") + 1;
			ret = strcpy(value, line+strlen(token)+pos);
			break;
		}
	}
	/* close and return after file has been opened */

	*offset = ftell(f);
	fclose(f);
	return ret;
}

static double
_getcpufreq(int cpu)
{
	char line[PROC_LINE_SZ];
	long offset=0;

	while (_getprocline(PROC_CPUINFO, &offset, "processor", line)) {
		if (atoi(line) == cpu) {
			if (_getprocline(PROC_CPUINFO, &offset, "cpu MHz", line)) {
				return atof(line);
			}
		}
	}
	return 0.0;
}

static void _tsc_init(void)
{
/*
	double t;
	uint64_t s1, s2;
	struct timeval tp;
	struct timezone tzp;
	int i;

	i = gettimeofday(&tp,&tzp);
	t = ((double)tp.tv_sec + (double)tp.tv_usec * 1.e-6);
	s1 = tsc_getsample();
	usleep(100000);
	s2 = tsc_getsample();
	i = gettimeofday(&tp,&tzp);
	t = ((double)tp.tv_sec + (double)tp.tv_usec * 1.e-6) - t;
	_cycletime = (t / (double)(s2 - s1));
*/
	_cycletime = 1 / (_getcpufreq(0) * 1e6);
	_initialized = 1;
}

double tsc_getsecs(void)
{
	uint64_t s;
	if (atomic_compare_and_exchange(&_initialized, 0, 1) == 0)
		_tsc_init();
	s = tsc_getsample();
	return (double)s * _cycletime;
}

double tsc_getresolution(void)
{
	if (atomic_compare_and_exchange(&_initialized, 0, 1) == 0)
		_tsc_init();
	return _cycletime;
}

double tsc_sample2secs(uint64_t s)
{
	if (atomic_compare_and_exchange(&_initialized, 0, 1) == 0)
		_tsc_init();
	return (double)s * _cycletime;
}
