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

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 1
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <omp.h>
#include <inttypes.h>
#include <sys/time.h>

#include "ncutils_atomic.h"
#include "ncutils_lock.h"

int testSpinLock();
int testTicketLock();
int testQTicketLock();
int testFetchAndAdd();
int testCompAndSwap();

int nmbrElems = 1024;
int nIter = (1000*1000);
int nThreads;

// Per-thread permutation table
unsigned *perm;
#pragma omp threadprivate (perm)

static void usage(const int code, const char *name)
{
	fprintf(stderr, "Usage: %s [-h] [-l elems] [-i iter]\n", name);
	fprintf(stderr, "  -h\t\tdisplay this help\n");
	fprintf(stderr, "  -l elems\tnumber of lock elements (default %d)\n", nmbrElems);
	fprintf(stderr, "  -i iter\tnumber of iterations (default %d)\n", nIter);
	exit(code);
}

int main (int argc, char **argv)
{
	int opt, sts = 0;

	while ((opt = getopt(argc, argv, "hl:i:")) != -1) {
		switch (opt) {
			case 'h':
				usage(0, argv[0]);
			case 'l':
				nmbrElems = atoi(optarg);
				break;
			case 'i':
				nIter = atoi(optarg);
				break;
			default:
				usage(1, argv[0]);
		}
	}

#pragma omp parallel
	{
#pragma omp master
		{
			nThreads = omp_get_num_threads();
		}
	}

	printf("Testing %d element(s) with %d threads:\n",
	       nmbrElems, nThreads);

	if (nThreads <= 1)
		printf("WARNING: Only %d processor online. Test is useless\n", nThreads);

	sts|= testSpinLock();
	sts|= testTicketLock();
	sts|= testQTicketLock();
	sts|= testFetchAndAdd();
	sts|= testCompAndSwap();

	return sts;
}

typedef struct { SpinLock_t l; int v; } t_spinLock;

#define TEST testSpinLock
#define NAME "testSpinLock"
#define STRUCT t_spinLock
#define INIT(s)	 { SpinInit(&((s)->l)); (s)->v = 0; }
#define VALUE(s) (s).v
#define ADD(s,i) { SpinAcquire(&((s)->l)); (s)->v+=(i); SpinRelease(&((s)->l)); }
#define SUB(s,i) ADD(s,i)

#include "test.c"

#undef TEST
#undef NAME
#undef STRUCT
#undef INIT
#undef VALUE
#undef ADD
#undef SUB
#undef N

typedef struct { TicketLock_t l; int v; } t_ticketLock;

#define TEST testTicketLock
#define NAME "testTicketLock"
#define STRUCT t_ticketLock
#define INIT(s)	 { TicketInit(&((s)->l)); (s)->v = 0; }
#define VALUE(s) (s).v
#define ADD(s,i) { TicketAcquire(&((s)->l)); (s)->v+=(i); TicketRelease(&((s)->l)); }
#define SUB(s,i) ADD(s,i)

#include "test.c"

#undef TEST
#undef NAME
#undef STRUCT
#undef INIT
#undef VALUE
#undef ADD
#undef SUB
#undef N

#define TEST testQTicketLock
#define NAME "testQTicketLock"
#define STRUCT t_ticketLock
#define INIT(s)	 { TicketInit(&((s)->l)); (s)->v = 0; }
#define VALUE(s) (s).v
#define ADD(s,i) { QTicketAcquire(&((s)->l)); (s)->v+=(i); QTicketRelease(&((s)->l)); }
#define SUB(s,i) ADD(s,i)

#include "test.c"

#undef TEST
#undef NAME
#undef STRUCT
#undef INIT
#undef VALUE
#undef ADD
#undef SUB
#undef N

typedef volatile int t_atomic;

#define TEST testAtomicIncr
#define NAME "testAtomicIncr"
#define STRUCT t_atomic
#define INIT(s)	 { *(s) = 0; }
#define VALUE(s) (s)
#define ADD(s,i) { atomic_increment((s)); }
#define SUB(s,i) { atomic_decrement((s)); }

#include "test.c"

#undef TEST
#undef NAME
#undef STRUCT
#undef INIT
#undef VALUE
#undef ADD
#undef SUB

#define TEST testFetchAndAdd
#define NAME "testFetchAndAdd"
#define STRUCT t_atomic
#define INIT(s)	 { *(s) = 0; }
#define VALUE(s) (s)
#define ADD(s,i) { atomic_exchange_and_add((s), (i)); }
#define SUB(s,i) ADD(s,i)

#include "test.c"

#undef TEST
#undef NAME
#undef STRUCT
#undef INIT
#undef VALUE
#undef ADD
#undef SUB

#define TEST testCompAndSwap
#define NAME "testCompAndSwap"
#define STRUCT t_atomic
#define INIT(s)	 { *(s) = 0; }
#define VALUE(s) (s)
#define ADD(s,i) { int tmp0=*(s); int tmp1; \
	while ((tmp1 = atomic_compare_and_exchange((s), tmp0, tmp0+(i))) != tmp0) tmp0 = tmp1; }
#define SUB(s,i) ADD(s,i)

#include "test.c"

#undef TEST
#undef NAME
#undef STRUCT
#undef INIT
#undef VALUE
#undef ADD
#undef SUB
