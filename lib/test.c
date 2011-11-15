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
#include <pthread.h>

#include "numachip_user.h"

int main(int argc, char **argv)
{
    struct numachip_device **devices;
    struct numachip_context *cntxt;
    int num_devices;

    devices = numachip_get_device_list(&num_devices);

    printf("Found %d NumaChip devices\n", num_devices);

    if (!devices)
	return -1;

    cntxt = numachip_open_device(devices[0]);
    numachip_free_device_list(devices);
    
    if (!cntxt)
	return -1;

    printf("NodeID=%03x\n", numachip_read_csr(cntxt, 0x0A00));
    (void)numachip_close_device(cntxt);
    
    return 0;
}
