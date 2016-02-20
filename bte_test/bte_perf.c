/*
 * Copyright (C) 2008-2014 Numascale AS, support@numascale.com
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

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <numa.h>
#include <sys/time.h>

#include "libncbte.h"
#include "numachip_memops.h"

static struct ncbte_context *context = NULL;
static struct ncbte_completion *comp = NULL;
static struct ncbte_region *local_region = NULL;
static struct ncbte_region *remote_region = NULL;
static void *local_buf = NULL, *remote_buf = NULL;

static const int min_iter=2;
static int reverse=0;
static double tgoal=0.1;
static double tres;

#define DO_MEMCPY    1
#define DO_SGE_COPY  2
#define DO_BTE_WRITE 4
#define DO_BTE_READ  8
#define DO_MEMSET    16
#define DO_MEMSET_NT 32
#define DO_BTE_CLEAR 64

typedef void (*bench_rout_t)(int, int);
typedef struct {
	bench_rout_t bench;
	char *mnem;
	int mask;
} bench_desc_t;

static inline double gtod(void)
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return (double)t.tv_sec+(1.e-6)*t.tv_usec;
}

static void bind_node(long node)
{
	struct bitmask *nodemask;

	nodemask = numa_allocate_nodemask();
	assert(nodemask);
	numa_bitmask_clearall(nodemask);
	numa_bitmask_setbit(nodemask, node);
	numa_bind(nodemask);
	numa_bitmask_free(nodemask);
}

static void usage(const int code)
{
	printf("Usage: bte_perf [-h] [-b m|g|w|r] [-m min] [-M max] [-t tgoal] <local node> <remote node>\n");
	printf("      -h           : Prints this message\n"
	       "      -b <m|g|w|r> : Benchmark to run, meMcpy, sGe_copy, bte_Write or bte_Read.\n"
	       "                     Default all.\n"
	       "      -m <int>     : Minimum message length. Default 0.\n"
	       "      -M <int>     : Maximum message length. Default 16M.\n"
	       "      -r           : Reverse message lengths.\n"
	       "      -t <float>   : Specifies the goal for the elapsed time used per test.\n"
	       "                     Default %.1f\n"
	       , tgoal);
	exit(code);
}

static void print_header(char *str)
{
	int i;
	const char leadtxt[] = "Benchmark ";
	printf("%s%s\n", leadtxt, str);
	for (i=0; i < strlen(str)+strlen(leadtxt); ++i) printf("=");
	printf("\n");

	printf("%14s %14s %14s %14s %14s \n",
	       "length", "iterations", "elapsed", "rate", "latency");
	printf("%14s %14s %14s %14s %14s\n",
	       "(bytes)", "(count)", "(seconds)", "(Mbytes/s)", "(usec)");
	printf("--------------------------------------------------------------------------\n");
	fflush(stdout);
}

static void measure(bench_rout_t rout, int sz)
{
	int i, iter=0;
	static int          prev_iter = 0;
	static bench_rout_t prev_rout = NULL;
	double              t, tmin;

	/*
	 * Only spend time calculating a new iteration count if
	 *    1) we have a new test function
	 *       or
	 *    2) we are not down to min_iter number of iterations yet
	 *       or
	 *    3) we are running with reverse lengths
	 */
	if (prev_rout != rout || prev_iter > min_iter || reverse) {
		/* quick evaluation to find the correct no of iterations */
		for (iter=1; 1; iter+= iter) {
			tmin = 9999.;
			for (i=0; i < 3; ++i) {         /* minimum of 3 times seems good */
				double ts;
				rout(1, sz);
				/* Avoid distortion due to Nagel's algorithm */
				t = gtod();
				rout(iter, sz);
				ts = gtod() - t;
				t = 0.5*ts;
				tmin =  (t < tmin) ? t : tmin;
			}
			if (tmin > 10.0*tres) break;
		}

		/* compute estimated no iterations to reach tgoal */
		iter = (int)(iter * (tgoal / tmin));
	}

	if (iter < min_iter) iter = min_iter; /* at least min_iter iterations */

	/* measure with one dry run first to get things settled in cache */
	rout(1 , sz);
	t = gtod();
	rout(iter, sz);
	/* Normalize timing */
	t = (gtod() - t) / (double)iter;
	printf("%14d %14d %14.3f %14.3f %14.3f\n",
	       sz, iter, t*iter,
	       sz*1e-6/t,
	       t*1e6);
	fflush(stdout);

	prev_iter = iter;
	prev_rout = rout;
}

static void libc_memcpy(int iter, int len)
{
	for (int i=0; i < iter; i++)
		memcpy(remote_buf, local_buf, len);
}

static void sge_copy(int iter, int len)
{
	struct numachip_sge sge = { .from = (uint64_t)local_buf, .to = (uint64_t)remote_buf, .length = len };

	for (int i=0; i < iter; i++)
		numachip_sge_copy(&sge, 1);
}

static void bte_write(int iter, int len)
{
	for (int i=0; i < iter; i++) {
		(void)ncbte_write_region(context, local_region, 0, remote_region, 0, len, comp);
		ncbte_wait_completion(context, comp);
	}
}

static void bte_read(int iter, int len)
{
	for (int i=0; i < iter; i++) {
		(void)ncbte_read_region(context, local_region, 0, remote_region, 0, len, comp);
		ncbte_wait_completion(context, comp);
	}
}

static void libc_memset(int iter, int len)
{
	for (int i=0; i < iter; i++)
		memset(remote_buf, 0, len);
}

static void memset_nt(int iter, int len)
{
	for (int i=0; i < iter; i++) {
		for (long j = 0; j < (len / sizeof(int)); j++)
			__builtin_ia32_movnti((int *)remote_buf + j, 0);
		asm volatile ("sfence\n\t" ::: "memory");
	}
}

static void bte_clear(int iter, int len)
{
	for (int i=0; i < iter; i++) {
		(void)ncbte_clear_region(context, remote_region, 0, len, comp);
		ncbte_wait_completion(context, comp);
	}
}


bench_desc_t bench[] = {
	{ libc_memcpy, "libc memcpy",       DO_MEMCPY    },
	{ sge_copy,    "numachip_sge_copy", DO_SGE_COPY  },
	{ bte_write,   "BTE remote write",  DO_BTE_WRITE },
	{ bte_read,    "BTE remote read",   DO_BTE_READ  },
	{ libc_memset, "libc memset",       DO_MEMSET    },
	{ memset_nt,   "memset_nt",         DO_MEMSET_NT },
	{ bte_clear,   "BTE remote clear",  DO_BTE_CLEAR },
	{ NULL,        "none",              0            },
};


int main(int argc, char **argv)
{
	int j, inc, bench_mask=0;
	int sz, msg_min=128, msg_max=64*1024*1024;
	double t1, t2, extreme;

	while (1) {
		int option_index = 0;

		static const struct option long_options[] = {
			{"help",    no_argument,       0, 'h'},
			{"bench",   required_argument, 0, 'b'},
			{"min",     required_argument, 0, 'm'},
			{"max",     required_argument, 0, 'M'},
			{"reverse", no_argument,       0, 'r'},
			{"time",    required_argument, 0, 't'},
			{0,         0,                 0, 0},
		};

		int c = getopt_long(argc, argv, "+hb:m:M:rt:", long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 'b':
			switch (*optarg) {
			case 'm':
				bench_mask|= DO_MEMCPY;
				break;
			case 'g':
				bench_mask|= DO_SGE_COPY;
				break;
			case 'w':
				bench_mask|= DO_BTE_WRITE;
				break;
			case 'r':
				bench_mask|= DO_BTE_READ;
				break;
			case 's':
				bench_mask|= DO_MEMSET;
				break;
			case 'n':
				bench_mask|= DO_MEMSET_NT;
				break;
			case 'c':
				bench_mask|= DO_BTE_CLEAR;
				break;
			case '?':
				usage(1);
				break;
			}
			break;
		case 'm':
			msg_min = atoi(optarg);
			break;
		case 'M':
			msg_max = atoi(optarg);
			break;
		case 'r':
			reverse = 1;
			break;
		case 't':
			tgoal = atof(optarg);
			break;
		default:
			usage(1);
		}
	}
	argc -= optind;
	argv += optind;

	if (argc < 2)
		usage(1);

	int local_node = atoi(argv[0]);
	int remote_node = atoi(argv[1]);

	if (local_node > numa_max_node() ||
	    remote_node > numa_max_node()) {
		printf("Invalid node number specified\n");
		exit(1);
	}

	context = ncbte_open(0);
	if (!context)
		exit(-1);

	// Allocate local buffer
	bind_node(local_node);

	local_buf = ncbte_alloc_region(context, NULL, msg_max, NCBTE_ALLOCATE_HUGEPAGE, &local_region);
	if (!local_buf)
		exit(-1);

	// Allocate completion object
	comp = ncbte_alloc_completion(context, 0);
	if (!comp)
		exit(-1);

	// Allocate remote buffer
	bind_node(remote_node);

	remote_buf = ncbte_alloc_region(context, NULL, msg_max, NCBTE_ALLOCATE_HUGEPAGE, &remote_region);
	if (!remote_buf)
		exit(-1);

	// Jump back to local core
	bind_node(local_node);

	/* if no benchmarks have been specified, we default to all */
	if (!bench_mask) bench_mask = DO_BTE_WRITE|DO_BTE_READ|DO_MEMCPY|DO_SGE_COPY;

	/* find clock resolution */
	for (j=0, extreme=-1; j<10; ++j) {
		for (t1=gtod(); (t2=gtod()) == t1;);
		tres = t2 - t1;
		if (tres > extreme) extreme=tres;
	}
	tres = extreme;

	printf("Resolution (usec): %f\n", tres*1e6);

	for (j=0; bench[j].bench; ++j) {

		if (!(bench_mask & bench[j].mask)) continue;

		print_header(bench[j].mnem);
		inc = reverse ? (msg_max>>1) : (msg_min>>2);
		for (sz=reverse?msg_max:msg_min;
		     reverse ? (sz >= msg_min) : (sz <= msg_max);
		     sz+= reverse ? -inc : inc) {

			if (sz < 4) {
				inc = 1;
			} else {
				if (!(sz & inc)) inc+= reverse ? -(inc/2) : inc; /* Got this? */
			}

			measure(bench[j].bench, sz);
		}
		printf("\n\n");
	}

	ncbte_free_region(context, remote_region);
	ncbte_free_region(context, local_region);

	ncbte_close(context);

	return 0;
}
