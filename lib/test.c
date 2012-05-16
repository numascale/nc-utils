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

/* check for link instability */
//static int cht_error(int node, int link)
//{
//    u32 status = cht_read_config(node, NB_FUNC_HT, 0x84 + link * 0x20);
//    return status & ((3 << 8) | (1 << 4)); /* CrcErr, LinkFail */
//}

int main(int argc, char **argv)
{
    struct numachip_device **devices;
    struct numachip_context *cntxt;
    int num_devices;

#if 1
    devices = numachip_get_device_list(&num_devices);

    printf("Found %d NumaChip devices\n", num_devices);

    if (!devices)
	return -1;

    cntxt = numachip_open_device(devices[0],SCC);
    numachip_free_device_list(devices);
    
    if (!cntxt)
	return -1;

    /*SCC Configuraton Registers*/
    printf("Read config DeviceID/VendorID %03x\n", numachip_read_config(cntxt,0,0x0));
    printf("Read config BADR#0 %03x\n", numachip_read_config(cntxt,0,H2S_CSR_F0_BASE_ADDRESS_REGISTER_0));
    printf("Read config BADR#1 %03x\n", numachip_read_config(cntxt,0,H2S_CSR_F0_BASE_ADDRESS_REGISTER_1));
    printf("Read config BADR#2 %03x\n", numachip_read_config(cntxt,0,H2S_CSR_F0_BASE_ADDRESS_REGISTER_2));
    printf("Read config BADR#3 %03x\n", numachip_read_config(cntxt,0,H2S_CSR_F0_BASE_ADDRESS_REGISTER_3));
    printf("Read config BADR#4 %03x\n", numachip_read_config(cntxt,0,H2S_CSR_F0_BASE_ADDRESS_REGISTER_4));
    printf("Read config BADR#5 %03x\n", numachip_read_config(cntxt,0,H2S_CSR_F0_BASE_ADDRESS_REGISTER_5));
    printf("Read config Link controller register %03x\n", numachip_read_config(cntxt,0,H2S_CSR_F0_LINK_CONTROL_REGISTER));
    printf("Read config Base Channel Buffer Count Register %03x\n", numachip_read_config(cntxt,0,H2S_CSR_F0_LINK_BASE_CHANNEL_BUFFER_COUNT_REGISTER ));
    printf("H2S_CSR_F0_CHTX_NODE_ID =%03x\n", numachip_read_config(cntxt,0,H2S_CSR_F0_CHTX_NODE_ID));


    printf("FN1 Read config BADR#0 %03x\n", numachip_read_config(cntxt,1,H2S_CSR_F1_BASE_ADDRESS_REGISTER_0));
    printf("FN1 Read config BADR#1 %03x\n", numachip_read_config(cntxt,1,H2S_CSR_F1_BASE_ADDRESS_REGISTER_1));
    printf("FN1 Read config BADR#2 %03x\n", numachip_read_config(cntxt,1,H2S_CSR_F1_BASE_ADDRESS_REGISTER_2));
    printf("FN1 Read config BADR#3 %03x\n", numachip_read_config(cntxt,1,H2S_CSR_F1_BASE_ADDRESS_REGISTER_3));
    printf("FN1 Read config BADR#4 %03x\n", numachip_read_config(cntxt,1,H2S_CSR_F1_BASE_ADDRESS_REGISTER_4));
    printf("FN1 Read config BADR#5 %03x\n", numachip_read_config(cntxt,1,H2S_CSR_F1_BASE_ADDRESS_REGISTER_5));

    printf("FN1 Read config DRAM BADR %03x\n", numachip_read_config(cntxt,1,H2S_CSR_F1_DRAM_BASE_ADDRESS_REGISTERS));

    /*PHY*/
    printf("H2S_CSR_G0_PHYXA_LINK_STAT =%03x\n", numachip_read_csr(cntxt, H2S_CSR_G0_PHYXA_LINK_STAT ));
    printf("H2S_CSR_G0_PHYXA_ELOG  =%03x\n", numachip_read_csr(cntxt,H2S_CSR_G0_PHYXA_ELOG));
    printf("H2S_CSR_G0_PHYXB_LINK_STAT =%03x\n", numachip_read_csr(cntxt, H2S_CSR_G0_PHYXB_LINK_STAT ));
    printf("H2S_CSR_G0_PHYXB_ELOG  =%03x\n", numachip_read_csr(cntxt,H2S_CSR_G0_PHYXB_ELOG));
    printf("H2S_CSR_G0_PHYYA_LINK_STAT =%03x\n", numachip_read_csr(cntxt, H2S_CSR_G0_PHYXA_LINK_STAT ));
    printf("H2S_CSR_G0_PHYYA_ELOG  =%03x\n", numachip_read_csr(cntxt,H2S_CSR_G0_PHYXA_ELOG));
    printf("H2S_CSR_G0_PHYYB_LINK_STAT =%03x\n", numachip_read_csr(cntxt, H2S_CSR_G0_PHYXB_LINK_STAT ));
    printf("H2S_CSR_G0_PHYYB_ELOG  =%03x\n", numachip_read_csr(cntxt,H2S_CSR_G0_PHYXB_ELOG));
    printf("H2S_CSR_G0_PHYZA_LINK_STAT =%03x\n", numachip_read_csr(cntxt, H2S_CSR_G0_PHYXA_LINK_STAT ));
    printf("H2S_CSR_G0_PHYZA_ELOG  =%03x\n", numachip_read_csr(cntxt,H2S_CSR_G0_PHYXA_ELOG));
    printf("H2S_CSR_G0_PHYZB_LINK_STAT =%03x\n", numachip_read_csr(cntxt, H2S_CSR_G0_PHYXB_LINK_STAT ));
    printf("H2S_CSR_G0_PHYZB_ELOG  =%03x\n", numachip_read_csr(cntxt,H2S_CSR_G0_PHYXB_ELOG));

    /*H2S Configuration and Status Registers - Group 2:*/
    printf("H2S_CSR_G2_FTAG_STATUS  =%03x\n", numachip_read_csr(cntxt,H2S_CSR_G2_FTAG_STATUS));
    printf("H2S_CSR_G2_DDL_STATUS   =%03x\n", numachip_read_csr(cntxt,H2S_CSR_G2_DDL_STATUS));
    
    /*H2S Configuration and Status Registers - Group 3:*/
    printf("H2S_CSR_G3_HT_NODEID (at %09x) =%03x\n",H2S_CSR_G3_HT_NODEID, numachip_read_csr(cntxt, H2S_CSR_G3_HT_NODEID));        
    printf("H2S_CSR_G3_CSR_BASE_ADDRESS=%03x\n", numachip_read_csr(cntxt, H2S_CSR_G3_CSR_BASE_ADDRESS));   
    
    /*H2S Configuration and Status Registers - Group 3: Tracer*/
    printf("H2S_CSR_G3_HREQ_CTRL=%03x\n", numachip_read_csr(cntxt, H2S_CSR_G3_HREQ_CTRL));   
    printf("H2S_CSR_G3_ERROR_STATUS=%03x\n", numachip_read_csr(cntxt, H2S_CSR_G3_ERROR_STATUS));   
 
    printf("H2S_CSR_G3_WATCH_BUS_SELECT =%03x\n", numachip_read_csr(cntxt,H2S_CSR_G3_WATCH_BUS_SELECT));
    printf("H2S_CSR_G3_TRACERCTRL=%03x\n", numachip_read_csr(cntxt,H2S_CSR_G3_TRACERCTRL));
    printf("H2S_CSR_G3_TRACERSTAT=%03x\n", numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT));
    printf("H2S_CSR_G3_TRACER_EVENT_ADDRESS_UPPER_BITS=%03x\n", numachip_read_csr(cntxt,H2S_CSR_G3_TRACER_EVENT_ADDRESS_UPPER_BITS));
    printf("H2S_CSR_G3_TRACER_EVENT_ADDRESS_LOWER_BITS=%03x\n", numachip_read_csr(cntxt, H2S_CSR_G3_TRACER_EVENT_ADDRESS_LOWER_BITS));
    printf("H2S_CSR_G3_TRACER_SELECT_COMPARE_AND_MASK=%03x\n", numachip_read_csr(cntxt,H2S_CSR_G3_TRACER_SELECT_COMPARE_AND_MASK));
    printf("H2S_CSR_G3_SELECT_COUNTER=%03x\n", numachip_read_csr(cntxt, H2S_CSR_G3_SELECT_COUNTER));
    printf("H2S_CSR_G3_OVERFLOW_COUNTER_0_7=%03x\n", numachip_read_csr(cntxt,H2S_CSR_G3_OVERFLOW_COUNTER_0_7));
    printf("H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_0=%03x\n", numachip_read_csr(cntxt,H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_0));
    printf("H2S_CSR_G3_PERFORMANCE_COUNTER_0_40_BIT_UPPER_BITS=%03x\n", numachip_read_csr(cntxt,H2S_CSR_G3_PERFORMANCE_COUNTER_0_40_BIT_UPPER_BITS));
    printf("H2S_CSR_G3_PERFORMANCE_COUNTER_0_40_BIT_LOWER_BITS=%03x\n", numachip_read_csr(cntxt,H2S_CSR_G3_PERFORMANCE_COUNTER_0_40_BIT_LOWER_BITS));

    /*    printf("=%03x\n", numachip_read_csr(cntxt,));
    printf("=%03x\n", numachip_read_csr(cntxt,));
    printf("=%03x\n", numachip_read_csr(cntxt,));*/

#endif
    (void)numachip_close_device(cntxt);

    devices = numachip_get_device_list(&num_devices);

    printf("Found %d NumaChip devices\n", num_devices);

    if (!devices)
	return -1;

    //    cntxt = numachip_open_device(devices[0]);
    cntxt = numachip_open_device(devices[0],LCXA);
    numachip_free_device_list(devices);
    
    if (!cntxt)
	return -1;

    //We have to change bit [18-16] in order to read the LC3s
    printf("LC3 NodeID=%03x\n", numachip_read_csr(cntxt, LC3_CSR_NODE_IDS));
    printf("State Clear=%03x\n", numachip_read_csr(cntxt,LC3_CSR_STATE_CLEAR ));
    printf("INTR MASK=%03x\n", numachip_read_csr(cntxt, LC3_CSR_ROUT_MASK));   

    (void)numachip_close_device(cntxt);

    devices = numachip_get_device_list(&num_devices);

    printf("Found %d NumaChip devices\n", num_devices);

    if (!devices)
	return -1;

    //    cntxt = numachip_open_device(devices[0]);
    cntxt = numachip_open_device(devices[0],LCXB);
    numachip_free_device_list(devices);
    
    if (!cntxt)
	return -1;

    //We have to change bit [18-16] in order to read the LC3s
    printf("LC3 NodeID=%03x\n", numachip_read_csr(cntxt, LC3_CSR_NODE_IDS));
    printf("State Clear=%03x\n", numachip_read_csr(cntxt,LC3_CSR_STATE_CLEAR ));
    printf("INTR MASK=%03x\n", numachip_read_csr(cntxt, LC3_CSR_ROUT_MASK));   

    (void)numachip_close_device(cntxt);

    return 0;
}
