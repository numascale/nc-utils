#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	if (argc < 2)
		error("Usage: place [-v] cmd [args..]");

	unsigned n = 1;
	if (!strcmp(argv[1], "-v")) {
		char flags[16];
		snprintf(flags, sizeof(flags), "%d", PLACE_VERBOSE);
		sysassertf(setenv("PLACE_FLAGS", flags, 1) == 0, "setenv failed");
		n++;
	}

	char path[PATH_MAX];
	sysassertf(realpath(argv[0], path) != NULL, "realpath failed");

	// drop leaf name
	char *last = strrchr(path, '/');
	if (last)
		*last = 0;

	char path2[PATH_MAX];
	snprintf(path2, PATH_MAX, "%s/libplace.so", path);

	assertf(access(path2, R_OK) == 0, "%s nonexisting or unreadable", path2);
	sysassertf(setenv("LD_PRELOAD", path2, 1) == 0, "setenv failed");

	execvp(argv[n], &argv[n]);
	syserror("Launching %s failed", argv[n]);
}
