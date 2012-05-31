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
#include <unistd.h>

#include "numachip_user.h"

#define DEBUG_STATEMENT(x) 

unsigned int link_up(struct numachip_context *cntxt, unsigned int lc);
/* check for link instability */
//static int cht_error(int node, int link)
//{
//    u32 status = cht_read_config(node, NB_FUNC_HT, 0x84 + link * 0x20);
//    return status & ((3 << 8) | (1 << 4)); /* CrcErr, LinkFail */
//}

int lfb_to_count(unsigned int val)
{
    unsigned long cMSB = 14;
    unsigned long lsb;
    unsigned long seq = 0;
    unsigned long start = 1 << cMSB;
    unsigned long lfsr = start;

    DEBUG_STATEMENT(printf ("cMSB %lx start/lfsr %lx val %x\n",cMSB, start, val));
    
    while ((lfsr != val) || (lfsr == start)) {
        // printf("linear feedback sr 0x%lx val 0x%x\n", lfsr,val);
	DEBUG_STATEMENT(printf("seq %5d, countervalue %5d (0x%04x)\n",seq,lfsr,lfsr));

        lsb = lfsr;
        lfsr >>= 1;

/*
	printf("lsb 0x%lx lfsr 0x%lx seq %lx \n",lsb, lfsr,seq);
	printf("Hatt: 0x%lx\n",lsb ^ lfsr);
	printf("Hatt &1: 0x%lx\n",(lsb ^ lfsr) & 1);
	printf("Hatt &1 << 0x%lx : 0x%lx\n",cMSB,(lsb ^ lfsr) & 1 << cMSB  );
	printf("lfsr |= ((lsb ^ lfsr) & 1) << cMSB 0x%lx\n",lfsr | ((lsb ^ lfsr) & 1) << cMSB  );		
*/     
	lfsr |= ((lsb ^ lfsr) & 1) << cMSB;

        seq++;
    }
    DEBUG_STATEMENT(printf("linear feedback sr 0x%lx val 0x%x\n", lfsr,val));
    sleep(1);
    
    /* Subtract fixed offset */
    //return seq - 2;
    return seq;
} 

unsigned int lc3_perf(unsigned int lc) {
    
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
    
    //We have to change bit [18-16] in order to read the LC3s
    printf("LC3 NodeID=%03x\n", numachip_read_csr(cntxt, LC3_CSR_NODE_IDS,LCXA));
    printf("State Clear=%03x\n", numachip_read_csr(cntxt,LC3_CSR_STATE_CLEAR,LCXA ));
    printf("INTR MASK=%03x\n", numachip_read_csr(cntxt, LC3_CSR_ROUT_MASK, LCXA));   
    
    
    if (!cntxt)
	return -1;
    
    /* Count LC3 B-Link grant, packet reject and accept events */
    printf("Count LC3 B-Link grant, packet reject and accept events \n");
    for (lc = 1; lc <= 2; lc++) {
	numachip_write_csr(cntxt, LC3_CSR_PCSEL,lc,0x13);
	//printf("WRITE PCSEL Readback LC %d:@ 0x%x =%03x\n", lc,LC3_CSR_PCSEL, numachip_read_csr(cntxt, LC3_CSR_PCSEL,lc));
    }
    
    /* Clear count */
    for (lc = 1; lc <= 2; lc++) {
	numachip_write_csr(cntxt, LC3_CSR_PCCNT,lc, 0);
	//printf("WRITE PCCNT (CLEAR) Readback LC %d:@0x%x =%03x\n",lc, LC3_CSR_PCCNT, numachip_read_csr(cntxt, LC3_CSR_PCCNT,lc));
    }
    
    sleep(1);
    DEBUG_STATEMENT(printf("I cant get no SLEEEEP\n"));
    
    for (lc = 1; lc <= 2; lc++) {
	unsigned int val = numachip_read_csr(cntxt, LC3_CSR_PCCNT, lc);
	DEBUG_STATEMENT(printf("READ PCCNT LC %d:@0x%x =%03x\n",lc, LC3_CSR_PCCNT,val));
	printf("READ PCCNT (after translate)- %d(0x%x) events on LC#%x\n", lfb_to_count(val),lfb_to_count(val), lc);
	DEBUG_STATEMENT(printf("AFTER READ PCCNT LC %d:@0x%x =%03x\n",lc, LC3_CSR_PCCNT,val);
			sleep(5));

    }
    
    /* Disable counting */
    printf("Disable counting\n");
    for (lc = 1; lc < 2; lc++)
	numachip_write_csr(cntxt, LC3_CSR_PCSEL,lc, 0);
    
    //(note lfb_to_count is used to translate the count [1]) 
    
    return 0;
}

unsigned int read_lcregs(struct numachip_context *cntxt) {
    
    numachip_device_type_t ncd=LCXA;
    for(ncd=LCXA; ncd<=LCZB;ncd++) {
	if (link_up(cntxt,ncd)) {
	    printf("Numachip device %s is UP (%d)\n",
		   numachip_device_str(ncd),
		   link_up(cntxt,ncd));
	    //We have to change bit [18-16] in order to read the LC3s
	    printf("LC3 NodeID=%03x\n", numachip_read_csr(cntxt, LC3_CSR_NODE_IDS,ncd));
	    printf("State Clear=%03x\n", numachip_read_csr(cntxt,LC3_CSR_STATE_CLEAR,ncd ));
	    printf("INTR MASK=%03x\n", numachip_read_csr(cntxt, LC3_CSR_ROUT_MASK,ncd));   
	    
	} else printf("Numachip device %s is DOWN (%d)\n", numachip_device_str(ncd),link_up(cntxt,ncd));
    }
    
    
    return 0;
}

unsigned int dump_scc_csr(struct numachip_context *cntxt) {
    

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
    
    
    /*H2S Configuration and Status Registers - Group 2:*/
    printf("H2S_CSR_G2_FTAG_STATUS  =%03x\n", numachip_read_csr(cntxt,H2S_CSR_G2_FTAG_STATUS,SCC));
    printf("H2S_CSR_G2_DDL_STATUS   =%03x\n", numachip_read_csr(cntxt,H2S_CSR_G2_DDL_STATUS,SCC));
    
    /*H2S Configuration and Status Registers - Group 3:*/
    printf("H2S_CSR_G3_HT_NODEID (at %09x) =%03x\n",H2S_CSR_G3_HT_NODEID, numachip_read_csr(cntxt, H2S_CSR_G3_HT_NODEID,SCC));        
    printf("H2S_CSR_G3_CSR_BASE_ADDRESS=%03x\n", numachip_read_csr(cntxt, H2S_CSR_G3_CSR_BASE_ADDRESS,SCC));   
    
    /*H2S Configuration and Status Registers - Group 3: Tracer*/
    printf("H2S_CSR_G3_HREQ_CTRL=%03x\n", numachip_read_csr(cntxt, H2S_CSR_G3_HREQ_CTRL,SCC));   
    printf("H2S_CSR_G3_ERROR_STATUS=%03x\n", numachip_read_csr(cntxt, H2S_CSR_G3_ERROR_STATUS,SCC));   
 
    printf("H2S_CSR_G3_WATCH_BUS_SELECT =%03x\n", numachip_read_csr(cntxt,H2S_CSR_G3_WATCH_BUS_SELECT,SCC));
    printf("H2S_CSR_G3_TRACERCTRL=%03x\n", numachip_read_csr(cntxt,H2S_CSR_G3_TRACERCTRL,SCC));
    printf("H2S_CSR_G3_TRACERSTAT=%03x\n", numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT,SCC));
    printf("H2S_CSR_G3_TRACER_EVENT_ADDRESS_UPPER_BITS=%03x\n", numachip_read_csr(cntxt,H2S_CSR_G3_TRACER_EVENT_ADDRESS_UPPER_BITS,SCC));
    printf("H2S_CSR_G3_TRACER_EVENT_ADDRESS_LOWER_BITS=%03x\n", numachip_read_csr(cntxt, H2S_CSR_G3_TRACER_EVENT_ADDRESS_LOWER_BITS,SCC));
    printf("H2S_CSR_G3_TRACER_SELECT_COMPARE_AND_MASK=%03x\n", numachip_read_csr(cntxt,H2S_CSR_G3_TRACER_SELECT_COMPARE_AND_MASK,SCC));
    printf("H2S_CSR_G3_SELECT_COUNTER=%03x\n", numachip_read_csr(cntxt, H2S_CSR_G3_SELECT_COUNTER,SCC));
    printf("H2S_CSR_G3_OVERFLOW_COUNTER_0_7=%03x\n", numachip_read_csr(cntxt,H2S_CSR_G3_OVERFLOW_COUNTER_0_7,SCC));
    printf("H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_0=%03x\n", numachip_read_csr(cntxt,H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_0,SCC));
    printf("H2S_CSR_G3_PERFORMANCE_COUNTER_0_40_BIT_UPPER_BITS=%03x\n", numachip_read_csr(cntxt,H2S_CSR_G3_PERFORMANCE_COUNTER_0_40_BIT_UPPER_BITS,SCC));
    printf("H2S_CSR_G3_PERFORMANCE_COUNTER_0_40_BIT_LOWER_BITS=%03x\n", numachip_read_csr(cntxt,H2S_CSR_G3_PERFORMANCE_COUNTER_0_40_BIT_LOWER_BITS,SCC));
    
    return 0;
}


unsigned int phy_regs(struct numachip_context *cntxt) {
    
    /*PHY*/
    printf("H2S_CSR_G0_PHYXA_LINK_STAT =%03x\n", numachip_read_csr(cntxt, H2S_CSR_G0_PHYXA_LINK_STAT ,SCC));
    printf("H2S_CSR_G0_PHYXA_ELOG  =%03x\n", numachip_read_csr(cntxt,H2S_CSR_G0_PHYXA_ELOG,SCC));
    printf("H2S_CSR_G0_PHYXB_LINK_STAT =%03x\n", numachip_read_csr(cntxt, H2S_CSR_G0_PHYXB_LINK_STAT ,SCC));
    printf("H2S_CSR_G0_PHYXB_ELOG  =%03x\n", numachip_read_csr(cntxt,H2S_CSR_G0_PHYXB_ELOG,SCC));
    
    printf("H2S_CSR_G0_PHYYA_LINK_STAT =%03x\n", numachip_read_csr(cntxt, H2S_CSR_G0_PHYYA_LINK_STAT ,SCC));
    printf("H2S_CSR_G0_PHYYA_ELOG  =%03x\n", numachip_read_csr(cntxt,H2S_CSR_G0_PHYYA_ELOG,SCC));
    printf("H2S_CSR_G0_PHYYB_LINK_STAT =%03x\n", numachip_read_csr(cntxt, H2S_CSR_G0_PHYYB_LINK_STAT ,SCC));
    printf("H2S_CSR_G0_PHYYB_ELOG  =%03x\n", numachip_read_csr(cntxt,H2S_CSR_G0_PHYYB_ELOG,SCC));
    
    printf("H2S_CSR_G0_PHYZA_LINK_STAT =%03x\n", numachip_read_csr(cntxt, H2S_CSR_G0_PHYZA_LINK_STAT ,SCC));
    printf("H2S_CSR_G0_PHYZA_ELOG  =%03x\n", numachip_read_csr(cntxt,H2S_CSR_G0_PHYZA_ELOG,SCC));
    printf("H2S_CSR_G0_PHYZB_LINK_STAT =%03x\n", numachip_read_csr(cntxt, H2S_CSR_G0_PHYZB_LINK_STAT ,SCC));
    printf("H2S_CSR_G0_PHYZB_ELOG  =%03x\n", numachip_read_csr(cntxt,H2S_CSR_G0_PHYZB_ELOG,SCC));
    
    return 0;
}

/*
 * Note that the LC3 registers will *not* be
 * accessible if the PHY is not up yet.
 * This is why any access to the LC3 registers
 * should be preceeded by a check to the respective 
 * PHYxx_LINK_STAT register and check if the "linkup" bit is set.
 * Also, note that since the PHYs are connected to the LC3s in a
 * crossing fashion, *both* PHYs must be checked before either of
 * the LC3XA or LC3XB registers are accessed. 
 */
unsigned int link_up(struct numachip_context *cntxt, unsigned int lc) {
    
    switch (lc) {
	case LCXA:
	case LCXB:
	    return (numachip_read_csr(cntxt, H2S_CSR_G0_PHYXA_LINK_STAT ,SCC) & numachip_read_csr(cntxt, H2S_CSR_G0_PHYXB_LINK_STAT ,SCC) & 1);
	case LCYA:	    
	case LCYB:
	    return (numachip_read_csr(cntxt, H2S_CSR_G0_PHYYA_LINK_STAT ,SCC) & numachip_read_csr(cntxt, H2S_CSR_G0_PHYYB_LINK_STAT ,SCC) & 1);
	case LCZA:	    
	case LCZB:
	    return (numachip_read_csr(cntxt, H2S_CSR_G0_PHYZA_LINK_STAT ,SCC) & numachip_read_csr(cntxt, H2S_CSR_G0_PHYZB_LINK_STAT ,SCC) & 1);
	default:
	    printf("%d is not a valid lc entry. Valid entries are (LCXA %d - LCZB %d)\n", lc, LCXA,LCZB);
	    
	    return 0;
    }
}

unsigned int perf_cnt(struct numachip_context *cntxt, unsigned int lc) {
    //Performance counter - LOC (HPrb) ??
    return 0;
}

unsigned int tracer_setup(struct numachip_context *cntxt) {
    

    // TRACER SETUP
    unsigned int value=0x3;
    
    
    // Stop Tracer - so that the CSR accesses don' show up when ALL is selected    
    //`Node0.local_write_csr(CSR_H2S_TRACER_CTRL, 32'h0000_0003);
    printf("INFO: H2S_CSR_G3_TRACERSTAT(0x%x)=%x\n", H2S_CSR_G3_TRACERSTAT,numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT,SCC));
    printf("INFO: H2S_CSR_G3_TRACERCTRL(0x%x)=%x\n", H2S_CSR_G3_TRACERCTRL, numachip_read_csr(cntxt,H2S_CSR_G3_TRACERCTRL,SCC));
      //numachip_write_csr(cntxt,H2S_CSR_G3_TRACERCTRL,SCC,0x1);
    numachip_write_csr(cntxt,H2S_CSR_G3_TRACERCTRL,SCC,value);
    printf("WRITE H2S_CSR_G3_TRACERCTRL (0x%x)=0x%x  \n", H2S_CSR_G3_TRACERCTRL, value);
    printf("READBACK: H2S_CSR_G3_TRACERCTRL(0x%x)=%x\n", H2S_CSR_G3_TRACERCTRL, numachip_read_csr(cntxt,H2S_CSR_G3_TRACERCTRL,SCC));
    printf("READBACK: H2S_CSR_G3_TRACERSTAT(0x%x)=%x\n", H2S_CSR_G3_TRACERSTAT, numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT,SCC));
/*
    if (value & 0x1) {
	printf("INFO: H2S_CSR_G3_TRACERCTRL=%03x\n", numachip_read_csr(cntxt,H2S_CSR_G3_TRACERCTRL,SCC));
	numachip_write_csr(cntxt,H2S_CSR_G3_TRACERCTRL,SCC,0x3);
	value=numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT,SCC);
    }
*/
    value=numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT,SCC);
    // Check if tracer stopped
    if (value & 0x1) {
	printf("H2S_CSR_G3_TRACERCTRL(0x%x)=%x. Tracer did not stop\n", H2S_CSR_G3_TRACERCTRL, numachip_read_csr(cntxt,H2S_CSR_G3_TRACERCTRL,SCC));
	printf("H2S_CSR_G3_TRACERSTAT(0x%x)=%x. Tracer did not stop\n", H2S_CSR_G3_TRACERSTAT, numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT,SCC));
	
	return 1;
    }
    
    
    value=numachip_read_csr(cntxt,H2S_CSR_G3_HREQ_CTRL,SCC);
    printf("INFO: Tracer has stopped H2S_CSR_G3_HREQ_CTRL (at 0x%x)=%x\n", H2S_CSR_G3_HREQ_CTRL, value);
    
    // Check if registers were setup correctly
    //`Node0.local_read_csr(CSR_H2S_HREQ_CTRL, data[31:0]);
    
    //`Node0.local_read_csr(CSR_H2S_TRACER_CTRL, data[31:0]);
    printf("INFO: H2S_CSR_G3_TRACERCTRL(at 0x%x)=%x\n",H2S_CSR_G3_TRACERCTRL, numachip_read_csr(cntxt,H2S_CSR_G3_TRACERCTRL,SCC));
    
    // `Node0.local_read_csr(CSR_H2S_TRACER_STATUS, data[31:0]);
    printf("INFO: H2S_CSR_G3_TRACERSTAT(at 0x%x)=%x\n",H2S_CSR_G3_TRACERSTAT, numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT,SCC));
    
    //`Node0.local_read_csr(CSR_MTAG_MAINT_UTIL, data[31:0]);
    printf("INFO: H2S_CSR_G4_MCTAG_MAINTR(at 0x%x)=%x\n", H2S_CSR_G4_MCTAG_MAINTR,numachip_read_csr(cntxt,H2S_CSR_G4_MCTAG_MAINTR,SCC));
    
    //`Node0.local_read_csr(CSR_H2S_WATCH_BUS_SEL, data[31:0]);
    printf("INFO: H2S_CSR_G3_WATCH_BUS_SELECT(at 0x%x)=%x\n", H2S_CSR_G3_WATCH_BUS_SELECT,numachip_read_csr(cntxt,H2S_CSR_G3_WATCH_BUS_SELECT,SCC));
    
    printf("************************SET UP THE REGS******************************* \n");
    // G3xC00 Watch Bus Select, Limit
    // Select Tracer as input instead of REM
    // 5 RW Watch Bus Select - H2S: 0: REM module 1: Tracer module
    //`Node0.local_write_csr(CSR_H2S_WATCH_BUS_SEL, 32'h0000_0020);
    printf("READBEFORE(at 0x%x):=%x\n",H2S_CSR_G3_WATCH_BUS_SELECT, numachip_read_csr(cntxt,H2S_CSR_G3_WATCH_BUS_SELECT,SCC));
    printf("WRITE: H2S_CSR_G3_WATCH_BUS_SELECT(at 0x%x)=%x\n",H2S_CSR_G3_WATCH_BUS_SELECT, 0x20);
    numachip_write_csr(cntxt,H2S_CSR_G3_WATCH_BUS_SELECT,SCC,0x20);
    printf("READBACK(at 0x%x):=%x\n", H2S_CSR_G3_WATCH_BUS_SELECT,numachip_read_csr(cntxt,H2S_CSR_G3_WATCH_BUS_SELECT,SCC));
    {
	//unsigned int mpu1_addr_plus200[63:0] = 64'h1001_0000_0000 + 64'h0200;     
	// G3xC0C Tracer Event Address (Upper Bits)
	// Atle:mpu1_addr_plus200[63:0] = 64'h1001_0000_0000 + 64'h0200;     
	//`Node0.local_write_csr(CSR_H2S_TRACER_ADDR0, mpu1_addr_plus200[47:38]);
	//numachip_write_csr(cntxt,H2S_CSR_G3_TRACER_EVENT_ADDRESS_UPPER_BITS,SCC,mpu1_addr_plus200[47:38]);
	printf("READBEFORE:=%x\n", numachip_read_csr(cntxt,H2S_CSR_G3_TRACER_EVENT_ADDRESS_UPPER_BITS,SCC));
	numachip_write_csr(cntxt,H2S_CSR_G3_TRACER_EVENT_ADDRESS_UPPER_BITS,SCC,0x40);
	printf("WRITE:H2S_CSR_G3_TRACER_EVENT_ADDRESS_UPPER_BITS(at 0x%x) =%0x\n",H2S_CSR_G3_TRACER_EVENT_ADDRESS_UPPER_BITS, 0x40);
	printf("READBACK:=%x\n", numachip_read_csr(cntxt,H2S_CSR_G3_TRACER_EVENT_ADDRESS_UPPER_BITS,SCC));
	// G3xC10 Tracer Event Address (Lower Bits)
	// `Node0.local_write_csr(CSR_H2S_TRACER_ADDR1, mpu1_addr_plus200[37:6]);
	//numachip_write_csr(cntxt,H2S_CSR_G3_TRACER_EVENT_ADDRESS_LOWER_BITS,mpu1_addr_plus200[37:6]);
	printf("READBEFORE:=%x\n", numachip_read_csr(cntxt,H2S_CSR_G3_TRACER_EVENT_ADDRESS_LOWER_BITS,SCC));
	numachip_write_csr(cntxt,H2S_CSR_G3_TRACER_EVENT_ADDRESS_LOWER_BITS,SCC,0x4000008);
	printf("WRITE:H2S_CSR_G3_TRACER_EVENT_ADDRESS_LOWER_BITS(at 0x%x) =0x%x\n",H2S_CSR_G3_TRACER_EVENT_ADDRESS_LOWER_BITS, 0x4000008);
	printf("READBACK:=%x\n", numachip_read_csr(cntxt,H2S_CSR_G3_TRACER_EVENT_ADDRESS_LOWER_BITS,SCC));
	// G3xC14 Tracer Select, Compare and Mask
	//Node0.local_write_csr(CSR_H2S_TRACER_SCM, 32'h0001_3F3F);
	printf("READBEFORE:=%x\n", numachip_read_csr(cntxt,H2S_CSR_G3_TRACER_SELECT_COMPARE_AND_MASK,SCC));
	numachip_write_csr(cntxt,H2S_CSR_G3_TRACER_SELECT_COMPARE_AND_MASK,SCC,0x13F3F);
	printf("WRITE:H2S_CSR_G3_TRACER_SELECT_COMPARE_AND_MASK (at 0x%x)0x%x\n",H2S_CSR_G3_TRACER_SELECT_COMPARE_AND_MASK,0x13F3F);
	printf("READBACK:=%x\n", numachip_read_csr(cntxt,H2S_CSR_G3_TRACER_SELECT_COMPARE_AND_MASK,SCC));
	// G4x700 MCTAG MAINTR (Maintenance Register)
	// 8: RW Diagnose: Enable Tracer
	//`Node0.local_write_csr(CSR_MTAG_MAINT_UTIL, 32'h0000_0107);
	printf("READBEFORE:=%x\n", numachip_read_csr(cntxt,H2S_CSR_G4_MCTAG_MAINTR,SCC));
	numachip_write_csr(cntxt,H2S_CSR_G4_MCTAG_MAINTR,SCC,0x107);
	printf("WRITE:H2S_CSR_G4_MCTAG_MAINTR (at 0x%x)0x%x\n",H2S_CSR_G4_MCTAG_MAINTR,0x107);
	printf("READBACK:=%x\n", numachip_read_csr(cntxt,H2S_CSR_G4_MCTAG_MAINTR,SCC));
	// G4xF00 MCTAG MAINTR (Maintenance Register)
	// 8: RW Diagnose: Enable Tracer
	// 7: RW Diagnose: Select Port1 at watchbus and tracer (only CDATA)
	// `Node0.local_write_csr(CSR_CDAT_MAINT_UTIL, 32'h0000_0107);
	printf("READBEFORE:%x\n", numachip_read_csr(cntxt,H2S_CSR_G4_CDATA_MAINTR,SCC));
	numachip_write_csr(cntxt,H2S_CSR_G4_CDATA_MAINTR,SCC,0x107);
	printf("WRITE:H2S_CSR_G4_CDATA_MAINTR (at 0x%x)0x%x\n",H2S_CSR_G4_CDATA_MAINTR,0x107);
	printf("READBACK:%x\n", numachip_read_csr(cntxt,H2S_CSR_G4_CDATA_MAINTR,SCC));

	// G3x400 HReq Ctrl (REM)
	// 31:30 RW Tracer address mode: 0= block, 1= ctag entry, 2= mtag sector, 3= all
	// 29:28 RW Tracer event mode: 0 - requests and no conflicts, 
	//                             1 - requests and conflicts, 
	//                             2 - requests, state changes and no conflicts, 
	//                             3 - requests, state changes and conflicts
	//`Node0.local_write_csr(CSR_H2S_HREQ_CTRL, 32'hB002_0000);
	value=numachip_read_csr(cntxt,H2S_CSR_G3_HREQ_CTRL,SCC);
	printf("READBEFORE:H2S_CSR_G3_HREQ_CTRL=%x read %x\n", value, numachip_read_csr(cntxt,H2S_CSR_G3_HREQ_CTRL,SCC));
	
	numachip_write_csr(cntxt,H2S_CSR_G3_HREQ_CTRL,SCC,0xF0000000|value);
	printf("WRITE:H2S_CSR_G3_HREQ_CTRL (at 0x%x) 0x%x\n",H2S_CSR_G3_HREQ_CTRL,0xF0000000|value);
	// G3xC00 Watch Bus Select, Limit
	// Check if registers were setup correctly
	// `Node0.local_read_csr(CSR_H2S_HREQ_CTRL, data[31:0]);
	value=numachip_read_csr(cntxt,H2S_CSR_G3_HREQ_CTRL,SCC);
	printf("READBACK:H2S_CSR_G3_HREQ_CTRL=%x \n", value);
	

	if (value & 0x1000) {
	    printf("H2S_CSR_G3_HREQ_CTRL (0x%x) H2S initialized after firmware initialization.(value 0x%x ) bit 12 is 1\n", H2S_CSR_G3_HREQ_CTRL, value);
	} else {
	    printf("H2S_CSR_G3_HREQ_CTRL (0x%x) H2S initialized after reset (value 0x%x ) bit 12 is 0\n", H2S_CSR_G3_HREQ_CTRL, value);
	}
	
	if ((value&0x8000) && (value&0x4000) &&  (value&0x2000) )
	    printf("H2S_CSR_G3_HREQ_CTRL (0x%x): Delay for lock assertion: 30 * 1024 cycles(value 0x%x )\n", H2S_CSR_G3_HREQ_CTRL, value);
	else if ((value&0x8000) && (value&0x4000) )
	    printf("H2S_CSR_G3_HREQ_CTRL (0x%x): Delay for lock assertion: 16 * 1024 cycles(value 0x%x )\n", H2S_CSR_G3_HREQ_CTRL, value);
	else if ((value&0x8000) && (value&0x2000) )
	    printf("H2S_CSR_G3_HREQ_CTRL (0x%x): Delay for lock assertion: 8 * 1024 cycles(value 0x%x )\n", H2S_CSR_G3_HREQ_CTRL, value);
	else if (value&0x8000) 
	    printf("H2S_CSR_G3_HREQ_CTRL (0x%x): Delay for lock assertion: 4 * 1024 cycles(value 0x%x )\n", H2S_CSR_G3_HREQ_CTRL, value);
	else if ((value&0x4000) &&  (value&0x2000) )
	    printf("H2S_CSR_G3_HREQ_CTRL (0x%x): Delay for lock assertion: 2 * 1024 cycles(value 0x%x )\n", H2S_CSR_G3_HREQ_CTRL, value);
	else if (value&0x4000) 
	    printf("H2S_CSR_G3_HREQ_CTRL (0x%x): Delay for lock assertion: 1 * 1024 cycles(value 0x%x )\n", H2S_CSR_G3_HREQ_CTRL, value);
	else if (value&0x2000) 
	    printf("H2S_CSR_G3_HREQ_CTRL (0x%x): Delay for lock assertion: 0,5 * 1024 cycles(value 0x%x )\n", H2S_CSR_G3_HREQ_CTRL, value);
	else
	    printf("H2S_CSR_G3_HREQ_CTRL (0x%x): Delay for lock assertion: 0 * 1024 cycles (value 0x%x )\n", H2S_CSR_G3_HREQ_CTRL, value);		
	
	if (value & 0x10000)
	    printf("H2S_CSR_G3_HREQ_CTRL (0x%x): Enable SMI Shutdown(value 0x%x )\n", H2S_CSR_G3_HREQ_CTRL, value);
	
	if (value & 0x20000)
	    printf("H2S_CSR_G3_HREQ_CTRL (0x%x): Value of Error bit in coherent error responses(value 0x%x )\n", H2S_CSR_G3_HREQ_CTRL, value);
	
	if (value & 0x40000)
	    printf("H2S_CSR_G3_HREQ_CTRL (0x%x): Value of NXA bit in coherent error responses(value 0x%x )\n", H2S_CSR_G3_HREQ_CTRL, value);
	
	if (value & 0x80000)
	    printf("H2S_CSR_G3_HREQ_CTRL (0x%x): Enable weak ordering on coherent Posted WrSized(value 0x%x )\n", H2S_CSR_G3_HREQ_CTRL, value);
	
	
	if ((value&0x8000000) && (value&0x4000000)  )
	    printf("H2S_CSR_G3_HREQ_CTRL (0x%x):Cache size 16 GB (value 0x%x )\n", H2S_CSR_G3_HREQ_CTRL, value);
	
	else if (value&0x8000000)
	    printf("H2S_CSR_G3_HREQ_CTRL (0x%x):Cache size 8 GB (value 0x%x )\n", H2S_CSR_G3_HREQ_CTRL, value);

	else if (value&0x4000000)
	    printf("H2S_CSR_G3_HREQ_CTRL (0x%x):Cache size 4 GB (value 0x%x )\n", H2S_CSR_G3_HREQ_CTRL, value);
	
	else 
	    printf("H2S_CSR_G3_HREQ_CTRL (0x%x):Cache size 2 GB (value 0x%x )\n", H2S_CSR_G3_HREQ_CTRL, value);
	
	
	if ((value&0x20000000) && (value&0x10000000)  )
	    printf("H2S_CSR_G3_HREQ_CTRL (0x%x):Tracer event mode:3 - requests, state changes and conflicts(value 0x%x )\n", H2S_CSR_G3_HREQ_CTRL, value);
	
	else if (value&0x20000000)
	    printf("H2S_CSR_G3_HREQ_CTRL (0x%x):Tracer event mode: 2 - requests, state changes and no conflicts(value 0x%x )\n", H2S_CSR_G3_HREQ_CTRL, value);
		   
	else if (value&0x10000000) printf("H2S_CSR_G3_HREQ_CTRL (0x%x):Tracer event mode: 1 - requests and conflicts(value 0x%x )\n", H2S_CSR_G3_HREQ_CTRL, value);
	
	else 
	    printf("H2S_CSR_G3_HREQ_CTRL (0x%x):Tracer event mode: 0 - requests and no conflicts,(value 0x%x )\n", H2S_CSR_G3_HREQ_CTRL, value);

		
	if ((value&0x80000000) && (value&0x40000000)  )
	    printf("H2S_CSR_G3_HREQ_CTRL (0x%x):Tracer address mode: block, ctag entry, mtag sector, 3 - all(value 0x%x )\n", H2S_CSR_G3_HREQ_CTRL, value);
	
	else if (value&0x80000000)
	    printf("H2S_CSR_G3_HREQ_CTRL (0x%x):Tracer address mode: 2 - mtag sector(value 0x%x )\n", H2S_CSR_G3_HREQ_CTRL, value);

	else if (value&0x40000000)
	    printf("H2S_CSR_G3_HREQ_CTRL (0x%x):Tracer address mode: 1 - ctag entry(value 0x%x )\n", H2S_CSR_G3_HREQ_CTRL, value);
	
	else 
	    printf("H2S_CSR_G3_HREQ_CTRL (0x%x):Tracer address mode: 0 - block(value 0x%x )\n", H2S_CSR_G3_HREQ_CTRL, value);
	
	//`Node0.local_read_csr(CSR_H2S_TRACER_CTRL, data[31:0]);
	printf("INFO: H2S_CSR_G3_TRACERCTRL(at 0x%x)=%x\n", H2S_CSR_G3_TRACERCTRL,numachip_read_csr(cntxt,H2S_CSR_G3_TRACERCTRL,SCC));
	// `Node0.local_read_csr(CSR_H2S_TRACER_STATUS, data[31:0]); // G3xC08 TracerStat
	printf("INFO: H2S_CSR_G3_TRACERSTAT(at 0x%x)=%x\n",H2S_CSR_G3_TRACERSTAT, numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT,SCC));
	//`Node0.local_read_csr(CSR_H2S_TRACER_ADDR0, data[31:0]);
	printf("INFO: H2S_CSR_G3_TRACER_EVENT_ADDRESS_UPPER_BITS(at 0x%x)=%x\n", H2S_CSR_G3_TRACER_EVENT_ADDRESS_UPPER_BITS, numachip_read_csr(cntxt,H2S_CSR_G3_TRACER_EVENT_ADDRESS_UPPER_BITS,SCC));
	//`Node0.local_read_csr(CSR_H2S_TRACER_ADDR1, data[31:0]);
	printf("INFO: H2S_CSR_G3_TRACER_EVENT_ADDRESS_LOWER_BITS(at 0x%x)=%x\n",H2S_CSR_G3_TRACER_EVENT_ADDRESS_LOWER_BITS, numachip_read_csr(cntxt,H2S_CSR_G3_TRACER_EVENT_ADDRESS_LOWER_BITS,SCC));
	//`Node0.local_read_csr(CSR_H2S_TRACER_SCM, data[31:0]);
	printf("INFO: H2S_CSR_G3_TRACER_SELECT_COMPARE_AND_MASK(at 0x%x)=%x\n", H2S_CSR_G3_TRACER_SELECT_COMPARE_AND_MASK,numachip_read_csr(cntxt,H2S_CSR_G3_TRACER_SELECT_COMPARE_AND_MASK,SCC));
	//`Node0.local_read_csr(CSR_MTAG_MAINT_UTIL, data[31:0]);
	printf("INFO: H2S_CSR_G4_MCTAG_MAINTR(at 0x%x)=%x\n", H2S_CSR_G4_MCTAG_MAINTR,numachip_read_csr(cntxt,H2S_CSR_G4_MCTAG_MAINTR,SCC));
	//`Node0.local_read_csr(CSR_CDAT_MAINT_UTIL, data[31:0]);
	printf("INFO: H2S_CSR_G4_CDATA_MAINTR(at 0x%x)=%x\n",H2S_CSR_G4_CDATA_MAINTR, numachip_read_csr(cntxt,H2S_CSR_G4_CDATA_MAINTR,SCC));
	//`Node0.local_read_csr(CSR_H2S_HREQ_CTRL, data[31:0]);
	printf("H2S_CSR_G3_HREQ_CTRL(at 0x%x)=%x\n", H2S_CSR_G3_HREQ_CTRL,numachip_read_csr(cntxt,H2S_CSR_G3_HREQ_CTRL,SCC));
	//`Node0.local_read_csr(CSR_H2S_WATCH_BUS_SEL, data[31:0]);
	printf("INFO: H2S_CSR_G3_WATCH_BUS_SELECT(at 0x%x)=%x\n",H2S_CSR_G3_WATCH_BUS_SELECT, numachip_read_csr(cntxt,H2S_CSR_G3_WATCH_BUS_SELECT,SCC));
	
	// Start Tracer
	//`Node0.local_write_csr(CSR_H2S_TRACER_CTRL, 32'h0000_00002);
	printf("READBEFORE: CSR_H2S_TRACER_CTRL(at 0x%x)=%x\n", H2S_CSR_G3_TRACERCTRL,numachip_read_csr(cntxt,H2S_CSR_G3_TRACERCTRL,SCC));
	printf("READBEFORE: CSR_H2S_TRACERSTAT (at 0x%x)=%x\n", H2S_CSR_G3_TRACERCTRL,numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT,SCC));
	numachip_write_csr(cntxt,H2S_CSR_G3_TRACERCTRL,SCC,0x2);
	//numachip_write_csr(cntxt,H2S_CSR_G3_TRACERCTRL,SCC,0x3);
	printf("READBACK AFTER WRITE 0x2: CSR_H2S_TRACER_CTRL(at 0x%x)=%x\n", H2S_CSR_G3_TRACERCTRL,numachip_read_csr(cntxt,H2S_CSR_G3_TRACERCTRL,SCC));
	printf("READBACK: CSR_H2S_TRACERSTAT(at 0x%x) =%x\n", H2S_CSR_G3_TRACERSTAT,numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT,SCC));
	
	// Check if tracer stopped
	//`Node0.local_read_csr(CSR_H2S_TRACER_STATUS, data[31:0]);
	{
	    unsigned int stopped=numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT,SCC);
	    /*printf("INFO: H2S_CSR_G3_TRACERSTAT=%x\n", stopped);*/
	    
	    if (!(stopped & 0x1)) {
		
		printf("ERROR: Tracer not running!!! CSR_H2S_TRACER_STATUS = 0x%x\n", numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT,SCC));
	    }
	}
    }
    
    
    return 0;
}

unsigned int tracer_runtest(struct numachip_context *cntxt) {
    // Start coherent test
    sleep(60);
    return 0;
}
    
unsigned int tracer_result(struct numachip_context *cntxt) {
    
    

    // TRACER SETUP
    unsigned int  data=0x0, result=1, value=0x3;    
    
    result = 0x1; // Anticipate success, it will be set to 0 if tests fail.
    
    // Atle: Fill in??? coherence_testsi.cachestate_test(mpu0_addr,mpu1_addr,result);
        
    // ============================================
    // TRACER END STATUS
    printf("INFO: TRACER END STATUS\n");
    
    // Stop Tracer
    //`Node0.local_write_csr(CSR_H2S_TRACER_CTRL, 32'h0000_0003);

    printf("READBEFORE: CSR_H2S_TRACER_CTRL(at 0x%x) =%x\n", H2S_CSR_G3_TRACERCTRL, numachip_read_csr(cntxt,H2S_CSR_G3_TRACERCTRL,SCC));
    printf("READBEFORE: CSR_H2S_TRACERSTAT (at 0x%x) =%x\n", H2S_CSR_G3_TRACERSTAT, numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT,SCC));
//    numachip_write_csr(cntxt,H2S_CSR_G3_TRACERCTRL,SCC,0x1);
    numachip_write_csr(cntxt,H2S_CSR_G3_TRACERCTRL,SCC,value);
    printf("READBACK AFTER WRITE 0x%x: CSR_H2S_TRACER_CTRL(at 0x%x) =%x\n", value,H2S_CSR_G3_TRACERCTRL,numachip_read_csr(cntxt,H2S_CSR_G3_TRACERCTRL,SCC));
    sleep(5);
    printf("READBACK: CSR_H2S_TRACERSTAT(at 0x%x)  =%x\n", H2S_CSR_G3_TRACERSTAT, numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT,SCC));
    // Check if tracer stopped
    //`Node0.local_read_csr(CSR_H2S_TRACER_CTRL, data[31:0]);
    printf("INFO: H2S_CSR_G3_TRACERCTRL(at 0x%x) =%x\n",H2S_CSR_G3_TRACERCTRL, numachip_read_csr(cntxt,H2S_CSR_G3_TRACERCTRL,SCC));
    
    // Check if registers were setup correctly
    //`Node0.local_read_csr(CSR_H2S_HREQ_CTRL, data[31:0]);
    printf("H2S_CSR_G3_HREQ_CTRL(at 0x%x) =%x\n", H2S_CSR_G3_HREQ_CTRL, numachip_read_csr(cntxt,H2S_CSR_G3_HREQ_CTRL,SCC));
    
    //`Node0.local_read_csr(CSR_H2S_TRACER_CTRL, data[31:0]);
    printf("INFO: H2S_CSR_G3_TRACERCTRL(at 0x%x) =%x\n", H2S_CSR_G3_TRACERCTRL, numachip_read_csr(cntxt,H2S_CSR_G3_TRACERCTRL,SCC));
    
    // `Node0.local_read_csr(CSR_H2S_TRACER_SCM, data[31:0]);
    printf("INFO: H2S_CSR_G3_TRACER_SELECT_COMPARE_AND_MASK(at 0x%x) =%x\n", H2S_CSR_G3_TRACER_SELECT_COMPARE_AND_MASK, numachip_read_csr(cntxt,H2S_CSR_G3_TRACER_SELECT_COMPARE_AND_MASK,SCC));
    // `Node0.local_read_csr(CSR_MTAG_MAINT_UTIL, data[31:0]);
    printf("INFO: H2S_CSR_G4_MCTAG_MAINTR(at 0x%x) =%x\n", H2S_CSR_G4_MCTAG_MAINTR, numachip_read_csr(cntxt,H2S_CSR_G4_MCTAG_MAINTR,SCC));
    
    // `Node0.local_read_csr(CSR_CDAT_MAINT_UTIL, data[31:0]);
    printf("INFO: H2S_CSR_G4_CDATA_MAINTR(at 0x%x) =%x\n", H2S_CSR_G4_CDATA_MAINTR, numachip_read_csr(cntxt,H2S_CSR_G4_CDATA_MAINTR,SCC));
    // `Node0.local_read_csr(CSR_H2S_HREQ_CTRL, data[31:0]);
    printf("H2S_CSR_G3_HREQ_CTRL(at 0x%x) =%x\n", H2S_CSR_G3_HREQ_CTRL, numachip_read_csr(cntxt,H2S_CSR_G3_HREQ_CTRL,SCC));
    
    // `Node0.local_read_csr(CSR_H2S_WATCH_BUS_SEL, data[31:0]);
    printf("INFO: H2S_CSR_G3_WATCH_BUS_SELECT(at 0x%x) =%x\n", H2S_CSR_G3_WATCH_BUS_SELECT, numachip_read_csr(cntxt,H2S_CSR_G3_WATCH_BUS_SELECT,SCC));
        
    // Find the last valid tracer address (obtained from CSR_H2S_TRACER_STATUS bit 10:1)
    // `Node0.local_read_csr(CSR_H2S_TRACER_STATUS, data[31:0]); // G3xC08 TracerStat
    {
	unsigned int zeroCnt = 0;
	unsigned int tracerEndAddr=0, tracerLoop=0; 
	data=numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT,SCC);
	
	printf("INFO: H2S_CSR_G3_TRACERSTAT(at 0x%x) =0x%x\n", H2S_CSR_G3_TRACERSTAT, data);
	
	//Tracer address is bit 10:1 bit 0 is Running (1) / Stopped (0)
	//tracerEndAddr[11:0] = {data[10:1], 2'b00};
	tracerEndAddr=data & 0xFFE;
	
	// Dump tracer RAM contents
	// G5x000 - G5xFFC Tracer RAM (1024x 32Bit)
	//Starting at H2S_CSR_G5_TRACER_RAM_1024X_32BIT_0
	// Read the tracer only up to the last valid address
	if (tracerEndAddr == 0) printf("ERROR: Tracer RAM is EMPTY - nothing was captured!\n");		  
	else {
	    for (tracerLoop = 0; tracerLoop <= tracerEndAddr; tracerLoop = tracerLoop + 4)
	    {
		
		//`Node0.local_read_csr((CSR_TRC_000 + tracerLoop), data[31:0]);
		data=numachip_read_csr(cntxt,H2S_CSR_G5_TRACER_RAM_1024X_32BIT_0+tracerLoop,SCC);
		printf("INFO: H2S_CSR_G5_TRACER_RAM_1024X_32BIT_0+tracerLoop (0x%x)=%x\n",tracerLoop, data);
		if (data != 0) {
		    printf("INFO: Tracer RAM data at address: 0x%x = 0x%x",
			   (H2S_CSR_G5_TRACER_RAM_1024X_32BIT_0 + tracerLoop),
			   data);
		    zeroCnt = 0;
		}
	    }
	}
	
    }	 
    
    printf("=====================================================================\n");

    return 0;
}
void usage () {

    printf("./test [-dump-scc-csr] [-dump-phy-regs] [-dump-lc-csr] [-setup-tracer] [-tracer-result] [-lc3-perftest]\n");
}
void close_device(struct numachip_context *cntxt) {
    (void)numachip_close_device(cntxt);
}
int main(int argc, char **argv)
{
    struct numachip_device **devices;
    struct numachip_context *cntxt;
    int counter=0;
    int num_devices; 

    if (argc<2) {
        usage();
        return(-1);
    }

   
    
    devices = numachip_get_device_list(&num_devices);
    printf("Found %d NumaChip devices\n", num_devices);
    
    if (!devices)
	return -1;
    
    cntxt = numachip_open_device(devices[0]);
    numachip_free_device_list(devices);
    
    if (!cntxt)
	return -1;

     /* Get the parameters */
    for (counter=1; (int)counter<argc; counter++) {
	if (!strcmp("-dump-scc-csr",argv[counter])) {
	    dump_scc_csr(cntxt);
	    continue;
	}
	
	if (!strcmp("-dump-phy-regs",argv[counter])) {
	    phy_regs(cntxt);
	    continue;
	}

	if (!strcmp("-dump-lc-csr",argv[counter])) {	    
	    read_lcregs(cntxt);
	    continue;
	}

	if (!strcmp("-setup-tracer",argv[counter])) {	    
	    tracer_setup(cntxt);
	    continue;
	}

	if (!strcmp("-tracer-result",argv[counter])) {	    
	    tracer_result(cntxt);	    
	    continue;
	}
	
	if (!strcmp("-lc3-perftest",argv[counter])) {	    
	    lc3_perf(0);
	    continue;
	}

	
	
    }


    //
    close_device(cntxt);
 
    return 0;
}
