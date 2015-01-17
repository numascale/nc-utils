#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include <getopt.h>

static void usage(const int code)
{
	fprintf(stderr, "Usage: numaplace [-v] [-s <stride>] cmd [args..]\n");
	exit(code);
}

int main(int argc, char *argv[])
{
	int c, digit_optind = 0, flagval = 0;

	while (1) {
		int this_option_optind = optind ? optind : 1;
		int option_index = 0;

		static const struct option long_options[] = {
			{"stride",  required_argument, 0, 0},
			{"verbose", no_argument,       0, 1},
			{"debug",   no_argument,       0, 1},
			{0,         0,                 0, 0},
		};

		c = getopt_long(argc, argv, "s:vd", long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 's':
			assert(!setenv("NUMAPLACE_STRIDE", optarg, 1));
			break;
		case 'v':
			flagval |= FLAGS_VERBOSE;
			break;
		case 'd':
			flagval |= FLAGS_DEBUG;
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
	sysassertf(realpath(argv[0], path) != NULL, "realpath failed");

	// drop leaf name
	char *last = strrchr(path, '/');
	if (last)
		*last = 0;

	char path2[PATH_MAX];
	snprintf(path2, PATH_MAX, "%s/libnumaplace.so", path);

	assertf(access(path2, R_OK) == 0, "%s nonexisting or unreadable", path2);
	assert(!setenv("LD_PRELOAD", path2, 1));

	long cores = sysconf(_SC_NPROCESSORS_CONF);
	// FIXME: parse /proc/cmdline to check isolcpus param

	char num[32];
	snprintf(num, sizeof(num), "%ld", cores);

	// don't overwrite any existing variable
	assert(!setenv("OMP_NUM_THREADS", num, 0));
	assert(!setenv("OMP_WAIT_POLICY", "active", 0));

	execvp(argv[optind], &argv[optind]);
	syserror("Launching %s failed", argv[optind]);
}
