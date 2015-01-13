#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	if (argc < 2)
		error("Usage: numaplace [-v|-vv] cmd [args..]");

	unsigned n = 1, flagval = 0;
	if (!strcmp(argv[n], "-v")) {
		flagval = FLAGS_VERBOSE;
		n++;
	} else if (!strcmp(argv[1], "-vv")) {
		flagval = FLAGS_DEBUG;
		n++;
	}

	char flags[16];
	snprintf(flags, sizeof(flags), "%u", flagval);
	sysassertf(setenv("NUMAPLACE_FLAGS", flags, 1) == 0, "setenv failed");

	char path[PATH_MAX];
	sysassertf(realpath(argv[0], path) != NULL, "realpath failed");

	// drop leaf name
	char *last = strrchr(path, '/');
	if (last)
		*last = 0;

	char path2[PATH_MAX];
	snprintf(path2, PATH_MAX, "%s/libnumaplace.so", path);

	assertf(access(path2, R_OK) == 0, "%s nonexisting or unreadable", path2);
	sysassertf(setenv("LD_PRELOAD", path2, 1) == 0, "setenv failed");

	long cores = sysconf(_SC_NPROCESSORS_CONF);
	// FIXME: parse /proc/cmdline to check isolcpus param

	char num[32];
	snprintf(num, sizeof(num), "%ld", cores);

	// don't overwrite any existing variable
	assert(!setenv("OMP_NUM_THREADS", num, 0));

	execvp(argv[n], &argv[n]);
	syserror("Launching %s failed", argv[n]);
}
