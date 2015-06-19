#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/sysinfo.h>

// workaround for older glibc
#ifndef PR_SET_THP_DISABLE
#define PR_SET_THP_DISABLE 41
#endif

// FIXME: parse /proc/cmdline to check isolcpus param

static void usage(const int code)
{
	fprintf(stderr, "usage: numaplace [-tvd] [-c <cores>] cmd [args ...]\n");
	fprintf(stderr, "\t-a, --no-allocator\tdon't use NUMA aware memory allocation\n");
	fprintf(stderr, "\t-c, --cores\t\tset number of cores advertised\n");
	fprintf(stderr, "\t-t, --no-thp\t\tdisable Transparent Huge Pages\n");
	fprintf(stderr, "\t-v, --verbose\t\tshow cores allocated\n");
	fprintf(stderr, "\t-d, --debug\t\tshow internal information\n");
	fprintf(stderr, "\t-p, --parent\t\tdon't pin parent task\n");
	exit(code);
}

int main(int argc, char *argv[])
{
	unsigned flagval = FLAGS_ALLOCATOR;

	while (1) {
		int option_index = 0;

		static const struct option long_options[] = {
			{"no-allocator", no_argument,  0, 'a'},
			{"cores",   required_argument, 0, 'c'},
#ifdef FIXME // needs implementing in library
			{"stride",  required_argument, 0, 's'},
#endif
			{"no-thp",  no_argument,       0, 't'},
			{"verbose", no_argument,       0, 'v'},
			{"debug",   no_argument,       0, 'd'},
			{"parent",  no_argument,       0, 'p'},
			{0,         0,                 0, 0},
		};

		int c = getopt_long(argc, argv, "+c:tvdp", long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 'a':
			flagval &= ~FLAGS_ALLOCATOR;
			break;
		case 'c': {
			// validate
			char *end;
			long val = strtol(optarg, &end, 10);
			if (*end || val < 1) {
				fprintf(stderr, "error: invalid number of cores '%s'\n", optarg);
				usage(1);
			}
			assert(!setenv("OMP_NUM_THREADS", optarg, 1));
			break;
		}
#ifdef FIXME
		case 's':
			assert(!setenv("NUMAPLACE_STRIDE", optarg, 1));
			break;
#endif
		case 't':
			assert(!prctl(PR_SET_THP_DISABLE, 1, 0, 0, 0));
			break;
		case 'v':
			flagval |= FLAGS_VERBOSE;
			break;
		case 'd':
			flagval |= FLAGS_DEBUG;
			break;
		case 'p':
			flagval |= FLAGS_PARENT;
			break;
		default:
			usage(1);
		}
	}

	if (argc - optind < 1)
		usage(1);

	char flags[16];
	snprintf(flags, sizeof(flags), "%u", flagval);
	assert(!setenv("NUMAPLACE_FLAGS", flags, 1));

	char path[PATH_MAX];
	ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
	sysassertf(len > 1, "readlink failed");

	// readlink doesn't terminate string
	path[len] = '\0';

	// drop leaf name
	char *last = strrchr(path, '/');
	if (last)
		*last = 0;

	char path2[PATH_MAX];
	snprintf(path2, PATH_MAX, "%s/libnumaplace.so", path);

	assertf(access(path2, R_OK) == 0, "%s nonexisting or unreadable", path2);

	assert(!unsetenv("OMP_PROC_BIND"));
	assert(!unsetenv("GOMP_CPU_AFFINITY"));
	assert(!unsetenv("KMP_AFFINITY"));

	assert(!setenv("LD_PRELOAD", path2, 1));
	// don't overwrite any existing variable
	assert(!setenv("OMP_WAIT_POLICY", "ACTIVE", 0));
	// prevent malloc reusing heap pages already faulted onto a non-local NUMA node
	assert(!setenv("MALLOC_TRIM_THRESHOLD_", "134217728", 0));
	assert(!setenv("MALLOC_TOP_PAD_", "33554432", 0));
	assert(!setenv("MALLOC_MMAP_THRESHOLD_", "0", 0));

	if (!(flagval & FLAGS_PARENT))
		bind_current();

	// warn when transparent huge pages are disabled
	int fd = open("/sys/kernel/mm/transparent_hugepage/enabled", O_RDONLY);
	assert(fd > -1);
	char buf[16];
	assert(read(fd, buf, sizeof(buf)) == 16);
	assert(!close(fd));

	if (strncmp(buf, "[always]", 8))
		fprintf(stderr, "warning: transparent hugepages are disabled; performance may be suboptimal\n");

	// enable low-latency socket behaviour
	assert(!prctl(PR_SET_TIMERSLACK, 1, 0, 0, 0));

	execvp(argv[optind], &argv[optind]);
	syserror("Launching %s failed", argv[optind]);
}
