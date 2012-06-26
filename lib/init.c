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
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "numachip_lib.h"
#include "numachip_config.h"

#define NUMASCALE_VENDOR_ID   0x1B47
#define NUMACHIP_DEVICE_ID    0x0601
#define DEBUG_STATEMENT(x)
static void add_device(int nodeid,
		       struct numachip_device ***dev_list,
		       int *ndevices,
		       int *list_size)
{
    struct numachip_device **new_list;
    struct numachip_device *dev;

    dev = malloc(sizeof(struct numachip_device));
    if (!dev)
	return;

    dev->nodeid=nodeid;

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


HIDDEN int numachip_init(struct numachip_device ***list, const char *filename)
{
    int ndevices = 0;
    int list_size = 0;
    int cfg_nodes;
    struct node_info *cfg_nodelist;
    int i = 0;

    //parse_json_file
    if (parse_config_file(filename,&cfg_nodelist,&cfg_nodes)) {
	for (i = 0; i < cfg_nodes; i++) {
	    DEBUG_STATEMENT(printf("Node %d: <%s> uuid: %d, sciid: 0x%03x, partition: %d, osc: %d, sync-only: %d\n",
				   i, cfg_nodelist[i].desc, cfg_nodelist[i].uuid,
				   cfg_nodelist[i].sciid, cfg_nodelist[i].partition,
				   cfg_nodelist[i].osc, cfg_nodelist[i].sync_only));
	    
	    add_device(cfg_nodelist[i].sciid, list, &ndevices, &list_size);
	}
    }
    return ndevices;
}
