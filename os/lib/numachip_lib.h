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

#ifndef __NUMACHIP_LIB_H
#define __NUMACHIP_LIB_H

#include "numachip_user.h"

#define HIDDEN		__attribute__((visibility ("hidden")))

enum {
    NUMACHIP_SYSFS_PATH_MAX = 256
};


struct numachip_device {
    //char dev_path[NUMACHIP_SYSFS_PATH_MAX];
    int nodeid;
};

struct numachip_csr_space {
    uint32_t                    *csr;
};

struct numachip_context {
    struct numachip_device      *device;
    int                         memfd;
    struct numachip_csr_space   csr_space;
    
};


HIDDEN int numachip_init(struct numachip_device ***list, const char *filename);

/*
 * sysfs helper functions
 */
const char *numachip_get_sysfs_path(void);

int numachip_read_sysfs_file(const char *dir, const char *file,
			     char *buf, size_t size);

#endif


