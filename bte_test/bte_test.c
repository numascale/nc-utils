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

// build with:
// g++ -Wall -Wextra -Wno-sign-compare -Wshadow -fno-rtti -Ofast -mfpmath=both -march=amdfam10 -flto -D_GNU_SOURCE -fopenmp -o timer-test timer-test.c


#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <numa.h>
#include <sys/time.h>

#include "libncbte.h"

static inline double gtod(void)
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return (double)t.tv_sec * 1e6 + (double)t.tv_usec;
}

static inline uint64_t user_to_phys(void *vaddr)
{
	static int pagemap_fd = -1;
	if (pagemap_fd == -1) {
		pagemap_fd = open("/proc/self/pagemap", O_RDONLY);
		assert(pagemap_fd != -1);
	}

	uint64_t pagemap_entry = 0ULL;
	off_t pos = ((uint64_t)vaddr) / getpagesize() * sizeof(uint64_t);
	assert(pread(pagemap_fd, &pagemap_entry, sizeof(uint64_t), pos) == sizeof(uint64_t));
	return ((pagemap_entry & 0x00ffffffffffffffULL) << 12) + ((uint64_t)vaddr & 0xfffULL);
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

void *memset_movnti(void *s, int c, size_t n)
{
	for (long i = 0; i < (n / sizeof(int)); i++)
		__builtin_ia32_movnti((int *)s + i, c);

	asm volatile ("sfence\n\t" ::: "memory");
	return s;
}

#define SIZE ((1*1024*1024)+28)
#define OFFSET 4

int main(int argc, char **argv)
{
	struct ncbte_context *context;
	struct ncbte_region *local_region;
	struct ncbte_region *remote_region;
	void *local_buf = NULL, *remote_buf = NULL;

	if (argc < 3) {
		printf("Usage %s <local node> <remote node>\n", argv[0]);
		exit(1);
	}

	int local_node = strtol(argv[1], NULL, 0);
	int remote_node = strtol(argv[2], NULL, 0);

	context = ncbte_open(0);
	if (!context)
		exit(-1);

	// Allocate local buffer
	bind_node(local_node);

	local_buf = ncbte_alloc_region(context, NULL, 2*1024*1024, NCBTE_ALLOCATE_HUGEPAGE, &local_region);
	if (!local_buf)
		exit(-1);
	memset_movnti(local_buf, 0xaa, SIZE);

//		printf("Local test buffer allocated @ %016"PRIx64"\n", user_to_phys(local_buf));

	// Allocate remote buffer
	bind_node(remote_node);

	remote_buf = ncbte_alloc_region(context, NULL, 2*1024*1024, NCBTE_ALLOCATE_HUGEPAGE, &remote_region);
	if (!remote_buf)
		exit(-1);
	memset_movnti(remote_buf, 0x55, SIZE);

//		printf("Remote test buffer allocated @ %016"PRIx64"\n", user_to_phys(remote_buf));

	// Jump back to local core
	bind_node(local_node);

	for (;;) {
		double t = gtod();
		if (ncbte_write_region(context, local_region, OFFSET, remote_region, OFFSET, SIZE, NULL) < 0)
		exit(-1);
		ncbte_wait_completion(context, NULL);
		t = gtod() - t;
		printf("write %d->%d %7.3f MByte/sec, ", local_node, remote_node, (double)SIZE/t);

		t = gtod();
		if (ncbte_read_region(context, local_region, OFFSET, remote_region, OFFSET, SIZE, NULL) < 0)
			exit(-1);
		ncbte_wait_completion(context, NULL);
		t = gtod() - t;
		printf("read %d->%d %5.3f MByte/sec\n", remote_node, local_node, (double)SIZE/t);
	}

	if (memcmp(local_buf+OFFSET, remote_buf+OFFSET, SIZE) != 0)
		printf("ERROR!\n");
	else
		printf("OK!\n");

	ncbte_free_region(context, remote_region);
	ncbte_free_region(context, local_region);

	ncbte_close(context);

	return 0;
}
