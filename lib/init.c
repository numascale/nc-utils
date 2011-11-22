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
// Copyright © 2008-2011
// Numascale AS Oslo, Norway.
// All Rights Reserved.
//

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "numachip_lib.h"

#define NUMASCALE_VENDOR_ID   0x1B47
#define NUMACHIP_DEVICE_ID    0x0601

static void add_device(char *dev_path,
		       struct numachip_device ***dev_list,
		       int *ndevices,
		       int *list_size)
{
    struct numachip_device **new_list;
    struct numachip_device *dev;

    dev = malloc(sizeof(struct numachip_device));
    if (!dev)
	return;

    strncpy(dev->dev_path, dev_path, NUMACHIP_SYSFS_PATH_MAX);

    if (*list_size <= *ndevices) {
	*list_size = *list_size ? *list_size * 2 : 1;
	new_list = realloc(*dev_list, *list_size * sizeof (struct numachip_device *));
	if (!new_list) {
	    free(dev);
	    return;
	}
	*dev_list = new_list;
    }

    (*dev_list)[(*ndevices)++] = dev;
}

HIDDEN int numachip_init(struct numachip_device ***list)
{
    int ndevices = 0;
    int list_size = 0;

    add_device("/sys/devices/pci0000:00/0000:00:1a", list, &ndevices, &list_size);

    return ndevices;
}
