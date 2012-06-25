// $Id:$
// This source code including any derived information including but
// not limited by net-lists, fpga bit-streams, and object files is the
// confidential and proprietary property of
//
// Numascale AS
// Enebakkveien 302A
// NO-1188 Oslo
// Norway
//
// Any unauthorized use, reproduction or transfer of the information
// provided herein is strictly prohibited.
//
// Copyright © 2008-2012
// Numascale AS Oslo, Norway.
// All Rights Reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "numachip_lib.h"

static char *sysfs_path;

const char *numachip_get_sysfs_path(void)
{
    char *env = NULL;

    if (sysfs_path)
	return sysfs_path;

    /*
     * Only follow use path passed in through the calling user's
     * environment if we're not running SUID.
     */
    if (getuid() == geteuid())
	env = getenv("SYSFS_PATH");

    if (env) {
	int len;

	sysfs_path = strndup(env, NUMACHIP_SYSFS_PATH_MAX);
	len = strlen(sysfs_path);
	while (len > 0 && sysfs_path[len - 1] == '/') {
	    --len;
	    sysfs_path[len] = '\0';
	}
    } else
	sysfs_path = "/sys";

    return sysfs_path;
}

int numachip_read_sysfs_file(const char *dir, const char *file,
			     char *buf, size_t size)
{
    char *path;
    int fd;
    int len;

    if (asprintf(&path, "%s/%s", dir, file) < 0)
	return -1;

    fd = open(path, O_RDONLY);
    if (fd < 0) {
	free(path);
	return -1;
    }

    len = read(fd, buf, size);

    close(fd);
    free(path);

    if (len > 0 && buf[len - 1] == '\n')
	buf[--len] = '\0';

    return len;
}
