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

unsigned int dump_scc_csr(struct numachip_context *cntxt) {
    
    
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
    
    return 0;
}


unsigned int phy_regs(struct numachip_context *cntxt) {
    
    /*PHY*/
    printf("H2S_CSR_G0_PHYXA_LINK_STAT =%03x\n", numachip_read_csr(cntxt, H2S_CSR_G0_PHYXA_LINK_STAT ));
    printf("H2S_CSR_G0_PHYXA_ELOG  =%03x\n", numachip_read_csr(cntxt,H2S_CSR_G0_PHYXA_ELOG));
    printf("H2S_CSR_G0_PHYXB_LINK_STAT =%03x\n", numachip_read_csr(cntxt, H2S_CSR_G0_PHYXB_LINK_STAT ));
    printf("H2S_CSR_G0_PHYXB_ELOG  =%03x\n", numachip_read_csr(cntxt,H2S_CSR_G0_PHYXB_ELOG));
    
    printf("H2S_CSR_G0_PHYYA_LINK_STAT =%03x\n", numachip_read_csr(cntxt, H2S_CSR_G0_PHYYA_LINK_STAT ));
    printf("H2S_CSR_G0_PHYYA_ELOG  =%03x\n", numachip_read_csr(cntxt,H2S_CSR_G0_PHYYA_ELOG));
    printf("H2S_CSR_G0_PHYYB_LINK_STAT =%03x\n", numachip_read_csr(cntxt, H2S_CSR_G0_PHYYB_LINK_STAT ));
    printf("H2S_CSR_G0_PHYYB_ELOG  =%03x\n", numachip_read_csr(cntxt,H2S_CSR_G0_PHYYB_ELOG));
    
    printf("H2S_CSR_G0_PHYZA_LINK_STAT =%03x\n", numachip_read_csr(cntxt, H2S_CSR_G0_PHYZA_LINK_STAT ));
    printf("H2S_CSR_G0_PHYZA_ELOG  =%03x\n", numachip_read_csr(cntxt,H2S_CSR_G0_PHYZA_ELOG));
    printf("H2S_CSR_G0_PHYZB_LINK_STAT =%03x\n", numachip_read_csr(cntxt, H2S_CSR_G0_PHYZB_LINK_STAT ));
    printf("H2S_CSR_G0_PHYZB_ELOG  =%03x\n", numachip_read_csr(cntxt,H2S_CSR_G0_PHYZB_ELOG));
    
    return 0;
}

unsigned int tracer_setup(struct numachip_context *cntxt) {
    

    // TRACER SETUP
    unsigned int value=0x3;
    
    
    // Stop Tracer - so that the CSR accesses don' show up when ALL is selected    
    //`Node0.local_write_csr(CSR_H2S_TRACER_CTRL, 32'h0000_0003);
    printf("INFO: H2S_CSR_G3_TRACERSTAT(0x%x)=%x\n", H2S_CSR_G3_TRACERSTAT,numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT));
    printf("INFO: H2S_CSR_G3_TRACERCTRL(0x%x)=%x\n", H2S_CSR_G3_TRACERCTRL, numachip_read_csr(cntxt,H2S_CSR_G3_TRACERCTRL));
      //numachip_write_csr(cntxt,H2S_CSR_G3_TRACERCTRL,0x1);
    numachip_write_csr(cntxt,H2S_CSR_G3_TRACERCTRL,value);
    printf("WRITE H2S_CSR_G3_TRACERCTRL (0x%x)=0x%x  \n", H2S_CSR_G3_TRACERCTRL, value);
    printf("READBACK: H2S_CSR_G3_TRACERCTRL(0x%x)=%x\n", H2S_CSR_G3_TRACERCTRL, numachip_read_csr(cntxt,H2S_CSR_G3_TRACERCTRL));
    printf("READBACK: H2S_CSR_G3_TRACERSTAT(0x%x)=%x\n", H2S_CSR_G3_TRACERSTAT, numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT));
/*
    if (value & 0x1) {
	printf("INFO: H2S_CSR_G3_TRACERCTRL=%03x\n", numachip_read_csr(cntxt,H2S_CSR_G3_TRACERCTRL));
	numachip_write_csr(cntxt,H2S_CSR_G3_TRACERCTRL,0x3);
	value=numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT);
    }
*/
    value=numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT);
    // Check if tracer stopped
    if (value & 0x1) {
	printf("H2S_CSR_G3_TRACERCTRL(0x%x)=%x. Tracer did not stop\n", H2S_CSR_G3_TRACERCTRL, numachip_read_csr(cntxt,H2S_CSR_G3_TRACERCTRL));
	printf("H2S_CSR_G3_TRACERSTAT(0x%x)=%x. Tracer did not stop\n", H2S_CSR_G3_TRACERSTAT, numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT));
	
	return 1;
    }
    
    
    value=numachip_read_csr(cntxt,H2S_CSR_G3_HREQ_CTRL);
    printf("INFO: Tracer has stopped H2S_CSR_G3_HREQ_CTRL (at 0x%x)=%x\n", H2S_CSR_G3_HREQ_CTRL, value);
    
    // Check if registers were setup correctly
    //`Node0.local_read_csr(CSR_H2S_HREQ_CTRL, data[31:0]);
    
    //`Node0.local_read_csr(CSR_H2S_TRACER_CTRL, data[31:0]);
    printf("INFO: H2S_CSR_G3_TRACERCTRL(at 0x%x)=%x\n",H2S_CSR_G3_TRACERCTRL, numachip_read_csr(cntxt,H2S_CSR_G3_TRACERCTRL));
    
    // `Node0.local_read_csr(CSR_H2S_TRACER_STATUS, data[31:0]);
    printf("INFO: H2S_CSR_G3_TRACERSTAT(at 0x%x)=%x\n",H2S_CSR_G3_TRACERSTAT, numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT));
    
    //`Node0.local_read_csr(CSR_MTAG_MAINT_UTIL, data[31:0]);
    printf("INFO: H2S_CSR_G4_MCTAG_MAINTR(at 0x%x)=%x\n", H2S_CSR_G4_MCTAG_MAINTR,numachip_read_csr(cntxt,H2S_CSR_G4_MCTAG_MAINTR));
    
    //`Node0.local_read_csr(CSR_H2S_WATCH_BUS_SEL, data[31:0]);
    printf("INFO: H2S_CSR_G3_WATCH_BUS_SELECT(at 0x%x)=%x\n", H2S_CSR_G3_WATCH_BUS_SELECT,numachip_read_csr(cntxt,H2S_CSR_G3_WATCH_BUS_SELECT));
    
    printf("************************SET UP THE REGS******************************* \n");
    // G3xC00 Watch Bus Select, Limit
    // Select Tracer as input instead of REM
    // 5 RW Watch Bus Select - H2S: 0: REM module 1: Tracer module
    //`Node0.local_write_csr(CSR_H2S_WATCH_BUS_SEL, 32'h0000_0020);
    printf("READBEFORE(at 0x%x):=%x\n",H2S_CSR_G3_WATCH_BUS_SELECT, numachip_read_csr(cntxt,H2S_CSR_G3_WATCH_BUS_SELECT));
    printf("WRITE: H2S_CSR_G3_WATCH_BUS_SELECT(at 0x%x)=%x\n",H2S_CSR_G3_WATCH_BUS_SELECT, 0x20);
    numachip_write_csr(cntxt,H2S_CSR_G3_WATCH_BUS_SELECT,0x20);
    printf("READBACK(at 0x%x):=%x\n", H2S_CSR_G3_WATCH_BUS_SELECT,numachip_read_csr(cntxt,H2S_CSR_G3_WATCH_BUS_SELECT));
    {
	//unsigned int mpu1_addr_plus200[63:0] = 64'h1001_0000_0000 + 64'h0200;     
	// G3xC0C Tracer Event Address (Upper Bits)
	// Atle:mpu1_addr_plus200[63:0] = 64'h1001_0000_0000 + 64'h0200;     
	//`Node0.local_write_csr(CSR_H2S_TRACER_ADDR0, mpu1_addr_plus200[47:38]);
	//numachip_write_csr(cntxt,H2S_CSR_G3_TRACER_EVENT_ADDRESS_UPPER_BITS,mpu1_addr_plus200[47:38]);
	printf("READBEFORE:=%x\n", numachip_read_csr(cntxt,H2S_CSR_G3_TRACER_EVENT_ADDRESS_UPPER_BITS));
	numachip_write_csr(cntxt,H2S_CSR_G3_TRACER_EVENT_ADDRESS_UPPER_BITS,0x40);
	printf("WRITE:H2S_CSR_G3_TRACER_EVENT_ADDRESS_UPPER_BITS(at 0x%x) =%0x\n",H2S_CSR_G3_TRACER_EVENT_ADDRESS_UPPER_BITS, 0x40);
	printf("READBACK:=%x\n", numachip_read_csr(cntxt,H2S_CSR_G3_TRACER_EVENT_ADDRESS_UPPER_BITS));
	// G3xC10 Tracer Event Address (Lower Bits)
	// `Node0.local_write_csr(CSR_H2S_TRACER_ADDR1, mpu1_addr_plus200[37:6]);
	//numachip_write_csr(cntxt,H2S_CSR_G3_TRACER_EVENT_ADDRESS_LOWER_BITS,mpu1_addr_plus200[37:6]);
	printf("READBEFORE:=%x\n", numachip_read_csr(cntxt,H2S_CSR_G3_TRACER_EVENT_ADDRESS_LOWER_BITS));
	numachip_write_csr(cntxt,H2S_CSR_G3_TRACER_EVENT_ADDRESS_LOWER_BITS,0x4000008);
	printf("WRITE:H2S_CSR_G3_TRACER_EVENT_ADDRESS_LOWER_BITS(at 0x%x) =0x%x\n",H2S_CSR_G3_TRACER_EVENT_ADDRESS_LOWER_BITS, 0x4000008);
	printf("READBACK:=%x\n", numachip_read_csr(cntxt,H2S_CSR_G3_TRACER_EVENT_ADDRESS_LOWER_BITS));
	// G3xC14 Tracer Select, Compare and Mask
	//Node0.local_write_csr(CSR_H2S_TRACER_SCM, 32'h0001_3F3F);
	printf("READBEFORE:=%x\n", numachip_read_csr(cntxt,H2S_CSR_G3_TRACER_SELECT_COMPARE_AND_MASK));
	numachip_write_csr(cntxt,H2S_CSR_G3_TRACER_SELECT_COMPARE_AND_MASK,0x13F3F);
	printf("WRITE:H2S_CSR_G3_TRACER_SELECT_COMPARE_AND_MASK (at 0x%x)0x%x\n",H2S_CSR_G3_TRACER_SELECT_COMPARE_AND_MASK,0x13F3F);
	printf("READBACK:=%x\n", numachip_read_csr(cntxt,H2S_CSR_G3_TRACER_SELECT_COMPARE_AND_MASK));
	// G4x700 MCTAG MAINTR (Maintenance Register)
	// 8: RW Diagnose: Enable Tracer
	//`Node0.local_write_csr(CSR_MTAG_MAINT_UTIL, 32'h0000_0107);
	printf("READBEFORE:=%x\n", numachip_read_csr(cntxt,H2S_CSR_G4_MCTAG_MAINTR));
	numachip_write_csr(cntxt,H2S_CSR_G4_MCTAG_MAINTR,0x107);
	printf("WRITE:H2S_CSR_G4_MCTAG_MAINTR (at 0x%x)0x%x\n",H2S_CSR_G4_MCTAG_MAINTR,0x107);
	printf("READBACK:=%x\n", numachip_read_csr(cntxt,H2S_CSR_G4_MCTAG_MAINTR));
	// G4xF00 MCTAG MAINTR (Maintenance Register)
	// 8: RW Diagnose: Enable Tracer
	// 7: RW Diagnose: Select Port1 at watchbus and tracer (only CDATA)
	// `Node0.local_write_csr(CSR_CDAT_MAINT_UTIL, 32'h0000_0107);
	printf("READBEFORE:%x\n", numachip_read_csr(cntxt,H2S_CSR_G4_CDATA_MAINTR));
	numachip_write_csr(cntxt,H2S_CSR_G4_CDATA_MAINTR,0x107);
	printf("WRITE:H2S_CSR_G4_CDATA_MAINTR (at 0x%x)0x%x\n",H2S_CSR_G4_CDATA_MAINTR,0x107);
	printf("READBACK:%x\n", numachip_read_csr(cntxt,H2S_CSR_G4_CDATA_MAINTR));

	// G3x400 HReq Ctrl (REM)
	// 31:30 RW Tracer address mode: 0= block, 1= ctag entry, 2= mtag sector, 3= all
	// 29:28 RW Tracer event mode: 0 - requests and no conflicts, 
	//                             1 - requests and conflicts, 
	//                             2 - requests, state changes and no conflicts, 
	//                             3 - requests, state changes and conflicts
	//`Node0.local_write_csr(CSR_H2S_HREQ_CTRL, 32'hB002_0000);
	value=numachip_read_csr(cntxt,H2S_CSR_G3_HREQ_CTRL);
	printf("READBEFORE:H2S_CSR_G3_HREQ_CTRL=%x read %x\n", value, numachip_read_csr(cntxt,H2S_CSR_G3_HREQ_CTRL));
	
	numachip_write_csr(cntxt,H2S_CSR_G3_HREQ_CTRL,0xF0000000|value);
	printf("WRITE:H2S_CSR_G3_HREQ_CTRL (at 0x%x) 0x%x\n",H2S_CSR_G3_HREQ_CTRL,0xF0000000|value);
	// G3xC00 Watch Bus Select, Limit
	// Check if registers were setup correctly
	// `Node0.local_read_csr(CSR_H2S_HREQ_CTRL, data[31:0]);
	value=numachip_read_csr(cntxt,H2S_CSR_G3_HREQ_CTRL);
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
	printf("INFO: H2S_CSR_G3_TRACERCTRL(at 0x%x)=%x\n", H2S_CSR_G3_TRACERCTRL,numachip_read_csr(cntxt,H2S_CSR_G3_TRACERCTRL));
	// `Node0.local_read_csr(CSR_H2S_TRACER_STATUS, data[31:0]); // G3xC08 TracerStat
	printf("INFO: H2S_CSR_G3_TRACERSTAT(at 0x%x)=%x\n",H2S_CSR_G3_TRACERSTAT, numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT));
	//`Node0.local_read_csr(CSR_H2S_TRACER_ADDR0, data[31:0]);
	printf("INFO: H2S_CSR_G3_TRACER_EVENT_ADDRESS_UPPER_BITS(at 0x%x)=%x\n", H2S_CSR_G3_TRACER_EVENT_ADDRESS_UPPER_BITS, numachip_read_csr(cntxt,H2S_CSR_G3_TRACER_EVENT_ADDRESS_UPPER_BITS));
	//`Node0.local_read_csr(CSR_H2S_TRACER_ADDR1, data[31:0]);
	printf("INFO: H2S_CSR_G3_TRACER_EVENT_ADDRESS_LOWER_BITS(at 0x%x)=%x\n",H2S_CSR_G3_TRACER_EVENT_ADDRESS_LOWER_BITS, numachip_read_csr(cntxt,H2S_CSR_G3_TRACER_EVENT_ADDRESS_LOWER_BITS));
	//`Node0.local_read_csr(CSR_H2S_TRACER_SCM, data[31:0]);
	printf("INFO: H2S_CSR_G3_TRACER_SELECT_COMPARE_AND_MASK(at 0x%x)=%x\n", H2S_CSR_G3_TRACER_SELECT_COMPARE_AND_MASK,numachip_read_csr(cntxt,H2S_CSR_G3_TRACER_SELECT_COMPARE_AND_MASK));
	//`Node0.local_read_csr(CSR_MTAG_MAINT_UTIL, data[31:0]);
	printf("INFO: H2S_CSR_G4_MCTAG_MAINTR(at 0x%x)=%x\n", H2S_CSR_G4_MCTAG_MAINTR,numachip_read_csr(cntxt,H2S_CSR_G4_MCTAG_MAINTR));
	//`Node0.local_read_csr(CSR_CDAT_MAINT_UTIL, data[31:0]);
	printf("INFO: H2S_CSR_G4_CDATA_MAINTR(at 0x%x)=%x\n",H2S_CSR_G4_CDATA_MAINTR, numachip_read_csr(cntxt,H2S_CSR_G4_CDATA_MAINTR));
	//`Node0.local_read_csr(CSR_H2S_HREQ_CTRL, data[31:0]);
	printf("H2S_CSR_G3_HREQ_CTRL(at 0x%x)=%x\n", H2S_CSR_G3_HREQ_CTRL,numachip_read_csr(cntxt,H2S_CSR_G3_HREQ_CTRL));
	//`Node0.local_read_csr(CSR_H2S_WATCH_BUS_SEL, data[31:0]);
	printf("INFO: H2S_CSR_G3_WATCH_BUS_SELECT(at 0x%x)=%x\n",H2S_CSR_G3_WATCH_BUS_SELECT, numachip_read_csr(cntxt,H2S_CSR_G3_WATCH_BUS_SELECT));
	
	// Start Tracer
	//`Node0.local_write_csr(CSR_H2S_TRACER_CTRL, 32'h0000_00002);
	printf("READBEFORE: CSR_H2S_TRACER_CTRL(at 0x%x)=%x\n", H2S_CSR_G3_TRACERCTRL,numachip_read_csr(cntxt,H2S_CSR_G3_TRACERCTRL));
	printf("READBEFORE: CSR_H2S_TRACERSTAT (at 0x%x)=%x\n", H2S_CSR_G3_TRACERCTRL,numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT));
	numachip_write_csr(cntxt,H2S_CSR_G3_TRACERCTRL,0x2);
	//numachip_write_csr(cntxt,H2S_CSR_G3_TRACERCTRL,0x3);
	printf("READBACK AFTER WRITE 0x2: CSR_H2S_TRACER_CTRL(at 0x%x)=%x\n", H2S_CSR_G3_TRACERCTRL,numachip_read_csr(cntxt,H2S_CSR_G3_TRACERCTRL));
	printf("READBACK: CSR_H2S_TRACERSTAT(at 0x%x) =%x\n", H2S_CSR_G3_TRACERSTAT,numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT));
	
	// Check if tracer stopped
	//`Node0.local_read_csr(CSR_H2S_TRACER_STATUS, data[31:0]);
	{
	    unsigned int stopped=numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT);
	    /*printf("INFO: H2S_CSR_G3_TRACERSTAT=%x\n", stopped);*/
	    
	    if (!(stopped & 0x1)) {
		
		printf("ERROR: Tracer not running!!! CSR_H2S_TRACER_STATUS = 0x%x\n", numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT));
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

    printf("READBEFORE: CSR_H2S_TRACER_CTRL(at 0x%x) =%x\n", H2S_CSR_G3_TRACERCTRL, numachip_read_csr(cntxt,H2S_CSR_G3_TRACERCTRL));
    printf("READBEFORE: CSR_H2S_TRACERSTAT (at 0x%x) =%x\n", H2S_CSR_G3_TRACERSTAT, numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT));
//    numachip_write_csr(cntxt,H2S_CSR_G3_TRACERCTRL,0x1);
    numachip_write_csr(cntxt,H2S_CSR_G3_TRACERCTRL,value);
    printf("READBACK AFTER WRITE 0x%x: CSR_H2S_TRACER_CTRL(at 0x%x) =%x\n", value,H2S_CSR_G3_TRACERCTRL,numachip_read_csr(cntxt,H2S_CSR_G3_TRACERCTRL));
    sleep(5);
    printf("READBACK: CSR_H2S_TRACERSTAT(at 0x%x)  =%x\n", H2S_CSR_G3_TRACERSTAT, numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT));
    // Check if tracer stopped
    //`Node0.local_read_csr(CSR_H2S_TRACER_CTRL, data[31:0]);
    printf("INFO: H2S_CSR_G3_TRACERCTRL(at 0x%x) =%x\n",H2S_CSR_G3_TRACERCTRL, numachip_read_csr(cntxt,H2S_CSR_G3_TRACERCTRL));
    
    // Check if registers were setup correctly
    //`Node0.local_read_csr(CSR_H2S_HREQ_CTRL, data[31:0]);
    printf("H2S_CSR_G3_HREQ_CTRL(at 0x%x) =%x\n", H2S_CSR_G3_HREQ_CTRL, numachip_read_csr(cntxt,H2S_CSR_G3_HREQ_CTRL));
    
    //`Node0.local_read_csr(CSR_H2S_TRACER_CTRL, data[31:0]);
    printf("INFO: H2S_CSR_G3_TRACERCTRL(at 0x%x) =%x\n", H2S_CSR_G3_TRACERCTRL, numachip_read_csr(cntxt,H2S_CSR_G3_TRACERCTRL));
    
    // `Node0.local_read_csr(CSR_H2S_TRACER_SCM, data[31:0]);
    printf("INFO: H2S_CSR_G3_TRACER_SELECT_COMPARE_AND_MASK(at 0x%x) =%x\n", H2S_CSR_G3_TRACER_SELECT_COMPARE_AND_MASK, numachip_read_csr(cntxt,H2S_CSR_G3_TRACER_SELECT_COMPARE_AND_MASK));
    // `Node0.local_read_csr(CSR_MTAG_MAINT_UTIL, data[31:0]);
    printf("INFO: H2S_CSR_G4_MCTAG_MAINTR(at 0x%x) =%x\n", H2S_CSR_G4_MCTAG_MAINTR, numachip_read_csr(cntxt,H2S_CSR_G4_MCTAG_MAINTR));
    
    // `Node0.local_read_csr(CSR_CDAT_MAINT_UTIL, data[31:0]);
    printf("INFO: H2S_CSR_G4_CDATA_MAINTR(at 0x%x) =%x\n", H2S_CSR_G4_CDATA_MAINTR, numachip_read_csr(cntxt,H2S_CSR_G4_CDATA_MAINTR));
    // `Node0.local_read_csr(CSR_H2S_HREQ_CTRL, data[31:0]);
    printf("H2S_CSR_G3_HREQ_CTRL(at 0x%x) =%x\n", H2S_CSR_G3_HREQ_CTRL, numachip_read_csr(cntxt,H2S_CSR_G3_HREQ_CTRL));
    
    // `Node0.local_read_csr(CSR_H2S_WATCH_BUS_SEL, data[31:0]);
    printf("INFO: H2S_CSR_G3_WATCH_BUS_SELECT(at 0x%x) =%x\n", H2S_CSR_G3_WATCH_BUS_SELECT, numachip_read_csr(cntxt,H2S_CSR_G3_WATCH_BUS_SELECT));
        
    // Find the last valid tracer address (obtained from CSR_H2S_TRACER_STATUS bit 10:1)
    // `Node0.local_read_csr(CSR_H2S_TRACER_STATUS, data[31:0]); // G3xC08 TracerStat
    {
	unsigned int zeroCnt = 0;
	unsigned int tracerEndAddr=0, tracerLoop=0; 
	data=numachip_read_csr(cntxt,H2S_CSR_G3_TRACERSTAT);
	
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
		data=numachip_read_csr(cntxt,H2S_CSR_G5_TRACER_RAM_1024X_32BIT_0+tracerLoop);
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
    printf("./test <json_file> [-dump-scc-csr] [-dump-phy-regs] [-dump-lc-csr] [-setup-tracer] [-tracer-result] [-lc3-perftest] [-count-api] [-count-api2] [-count-api-compare] [-parse-json]\n");
}

void count_api_test(struct numachip_context *cntxt) {
    unsigned int counter=0;
    unsigned int mask=6;
    nc_error_t retval = NUMACHIP_ERR_OK;

    
    printf("Timer or counter 0x%x\n", numachip_read_csr(cntxt, H2S_CSR_G3_TIMER_FOR_ECC_COUNTER_7));
    //Counter mode
    numachip_write_csr(cntxt,
		       H2S_CSR_G3_TIMER_FOR_ECC_COUNTER_7,
		       numachip_read_csr(cntxt, H2S_CSR_G3_TIMER_FOR_ECC_COUNTER_7) & 0xFFFFFFFE);
    printf("Timer or counter 0x%x\n", numachip_read_csr(cntxt, H2S_CSR_G3_TIMER_FOR_ECC_COUNTER_7));

    
    printf("/** CLEAR CNT **/\n");
    //for (counter=0; counter<8; counter++)
    numachip_clear_pcounter(cntxt,counter, &retval);
    if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);

    printf("/** SELECT COUNTER **/\n");
    //for (counter=0; counter<8; counter++)
    
    numachip_select_pcounter(cntxt,counter,mask, &retval);;
    if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
    
    printf("/** Apply Mask **/\n");
    //for (counter=0; counter<8; counter++) 
    numachip_mask_pcounter(cntxt,counter,mask, &retval);
    if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
    //for (counter=0; counter<8; counter++)
    printf("%d: 0x%llx (%lld) \n", counter,numachip_get_pcounter(cntxt,counter,&retval), numachip_get_pcounter(cntxt,counter,&retval));

    sleep(10);
    
    printf("/*AFTER 10s*/\n");
//    for (counter=0; counter<8; counter++) 
    numachip_stop_pcounter(cntxt,counter, &retval);
    if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
    
    printf("*STOP COUNTER* \n");
//   for (counter=0; counter<8; counter++)
    printf("%d: 0x%llx (%lld) \n", counter,numachip_get_pcounter(cntxt,counter,&retval), numachip_get_pcounter(cntxt,counter,&retval));
    if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
    
    sleep(10);
    printf("*CONFIRM STOPPED COUNTER AFTER 10s*/\n");

    //for (counter=0; counter<8; counter++)
    printf("%d: 0x%llx (%lld) \n", counter,numachip_get_pcounter(cntxt,counter,&retval), numachip_get_pcounter(cntxt,counter,&retval));

    if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
}

void count_api_test2(struct numachip_context *cntxt) {
    nc_error_t retval = NUMACHIP_ERR_OK;
    unsigned int counter=0;
    printf("/** CLEAR CNT **/\n");
    for (counter=0; counter<8; counter++) {
	numachip_clear_pcounter(cntxt,counter,&retval);
	if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
    }
    
    printf("/** SELECT COUNTER **/\n");
    //for (counter=0; counter<8; counter++) 
    numachip_select_pcounter(cntxt,0,0x1, &retval);
    if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
    numachip_select_pcounter(cntxt,1,0x1, &retval);
    if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);    
    numachip_select_pcounter(cntxt,2,0x6, &retval);
    if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
    numachip_select_pcounter(cntxt,3,0x6, &retval);
    if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
    
    
    printf("/** Apply Mask **/\n");
    
    numachip_mask_pcounter(cntxt,0,6, &retval);
    if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
    numachip_mask_pcounter(cntxt,1,5, &retval);
    if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
    numachip_mask_pcounter(cntxt,2,3, &retval);
    if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
    numachip_mask_pcounter(cntxt,3,2, &retval);
    if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);    
    
    for (counter=0; counter<4; counter++) {
	
	printf("%d: select is 0x%x mask 0x%x value 0x%llx (%lld) \n",
	       counter,
	       numachip_get_pcounter_select(cntxt,counter,&retval),
	       numachip_get_pcounter_mask(cntxt,counter,&retval),
	       numachip_get_pcounter(cntxt,counter,&retval),
	       numachip_get_pcounter(cntxt,counter,&retval));
	if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
    }
    
    sleep(10);
    
    printf("/*AFTER 10s*/\n");
    for (counter=0; counter<4; counter++) {
	numachip_stop_pcounter(cntxt,counter, &retval);
	if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
    }
    
    printf("*STOP COUNTER* \n");
    for (counter=0; counter<4; counter++) {
	printf("%d: 0x%llx (%lld) \n", counter,numachip_get_pcounter(cntxt,counter,&retval), numachip_get_pcounter(cntxt,counter,&retval));
	if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
    }
    
    sleep(10);
    printf("*CONFIRM STOPPED COUNTER AFTER 10s*/\n");


    for (counter=0; counter<4; counter++) {
	printf("%d: 0x%llx (%lld) \n", counter,numachip_get_pcounter(cntxt,counter,&retval), numachip_get_pcounter(cntxt,counter,&retval));
	if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
    }

    printf("Summary: \n");
    printf("REM CAHCE HIT: %lld MISS %lld \n",
	   numachip_get_pcounter(cntxt,1,&retval),
	   numachip_get_pcounter(cntxt,0,&retval));

    
    printf("REM CAHCE HIT + MISS %lld \n",  numachip_get_pcounter(cntxt,0,&retval) + numachip_get_pcounter(cntxt,1,&retval));

    {
	unsigned long long total=numachip_get_pcounter(cntxt,0,&retval) + numachip_get_pcounter(cntxt,1,&retval);
	double long  missrate = (double long) 100*numachip_get_pcounter(cntxt,0,&retval)/total;
	double long hitrate=(double long)100*numachip_get_pcounter(cntxt,1,&retval)/total;
	printf("REM CAHCE HIT + MISS %lld Miss rate %0.2Lf \n",  total,missrate);
	printf("REM CAHCE HIT + MISS %lld Hit rate %0.2Lf \n",  total,hitrate);
	
    }
        
}

void count_api_test4(struct numachip_context **cntxt, unsigned int num_nodes) {

    nc_error_t retval = NUMACHIP_ERR_OK;
    unsigned int node=0, counter=0;
    printf("/** CLEAR CNT **/\n");
    for(node=0; node<num_nodes; node++) {
	for (counter=0; counter<8; counter++) {
	    numachip_clear_pcounter(cntxt[node],counter,&retval);
	    if (retval != NUMACHIP_ERR_OK) printf("API failed for node %d retval = 0x%x", node, retval);
	}
    }
    
    printf("/** SELECT COUNTER **/\n");
    for(node=0; node<num_nodes; node++) {
    //for (counter=0; counter<8; counter++) 
	numachip_select_pcounter(cntxt[node],0,0x1, &retval);
	if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
	numachip_select_pcounter(cntxt[node],1,0x1, &retval);
	if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);    
	numachip_select_pcounter(cntxt[node],2,0x6, &retval);
	if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
	numachip_select_pcounter(cntxt[node],3,0x6, &retval);
	if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
    }
    
    
    printf("/** Apply Mask **/\n");
    for(node=0; node<num_nodes; node++) {
	numachip_mask_pcounter(cntxt[node],0,6, &retval);
	if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
	numachip_mask_pcounter(cntxt[node],1,5, &retval);
	if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
	numachip_mask_pcounter(cntxt[node],2,3, &retval);
	if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
	numachip_mask_pcounter(cntxt[node],3,2, &retval);
	if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);    
	
	for (counter=0; counter<4; counter++) {
	    
	    printf("%d: select is 0x%x mask 0x%x value 0x%llx (%lld) \n",
		   counter,
		   numachip_get_pcounter_select(cntxt[node],counter,&retval),
		   numachip_get_pcounter_mask(cntxt[node],counter,&retval),
		   numachip_get_pcounter(cntxt[node],counter,&retval),
		   numachip_get_pcounter(cntxt[node],counter,&retval));
	    if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
	}

    }
    sleep(10);
	
    printf("/*AFTER 10s*/\n");
    printf("*STOP COUNTER* \n");
    for(node=0; node<num_nodes; node++) {
	for (counter=0; counter<4; counter++) {
	    numachip_stop_pcounter(cntxt[node],counter, &retval);
	    if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
	}

	
	
	for (counter=0; counter<4; counter++) {
	    printf("node %d counter %d: 0x%llx (%lld) \n",node, counter,numachip_get_pcounter(cntxt[node],counter,&retval), numachip_get_pcounter(cntxt[node],counter,&retval));
	    if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
	}
    }
    
    sleep(10);

    
    printf("*CONFIRM STOPPED COUNTER AFTER 10s*/\n");
    for(node=0; node<num_nodes; node++) {

	for (counter=0; counter<4; counter++) {
	    printf("node %d counter %d: 0x%llx (%lld) \n", node, counter,numachip_get_pcounter(cntxt[node],counter,&retval), numachip_get_pcounter(cntxt[node],counter,&retval));
	    if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
	}
    }

    printf("Summary: \n");
    for(node=0; node<num_nodes; node++) {
	unsigned long long hit=numachip_get_pcounter(cntxt[node],1,&retval);
	unsigned long long miss=numachip_get_pcounter(cntxt[node],0,&retval);
	unsigned long long total=numachip_get_pcounter(cntxt[node],0,&retval) + numachip_get_pcounter(cntxt[node],1,&retval);
	double long  missrate = (double long) 100*miss/total;
	double long hitrate=(double long)100*hit/total;
	printf("Node %d: REM CAHCE HIT: %lld MISS %lld total %lld\n",node,
	       hit,miss, total);
	
        if (total==0) {
	    printf("Node %d: NO hits or miss in remote cache\n", node);
	} else if (miss==0) {
	    printf("Node %d: Hit rate 100 percent \n", node);
	} else if (hit==0) {
	    printf("Node %d: Miss rate 100 percent \n", node);
	} else {
	    printf("Node %d: Miss rate %0.2Lf \n", node,missrate);
	    printf("Node %d: Hit rate %0.2Lf \n",  node,hitrate);
	    
	}
    }
}


void count_api_start4(struct numachip_context **cntxt, unsigned int num_nodes) {

    nc_error_t retval = NUMACHIP_ERR_OK;
    unsigned int node=0, counter=0;
    printf("/** CLEAR CNT **/\n");
    for(node=0; node<num_nodes; node++) {
	for (counter=0; counter<8; counter++) {
	    numachip_clear_pcounter(cntxt[node],counter,&retval);
	    if (retval != NUMACHIP_ERR_OK) printf("API failed for node %d retval = 0x%x", node, retval);
	}
    }
    
    printf("/** SELECT COUNTER **/\n");
    for(node=0; node<num_nodes; node++) {
    //for (counter=0; counter<8; counter++) 
	numachip_select_pcounter(cntxt[node],0,0x1, &retval);
	if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
	numachip_select_pcounter(cntxt[node],1,0x1, &retval);
	if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);    
	numachip_select_pcounter(cntxt[node],2,0x6, &retval);
	if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
	numachip_select_pcounter(cntxt[node],3,0x6, &retval);
	if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
    }
    
    
    printf("/** Apply Mask **/\n");
    for(node=0; node<num_nodes; node++) {
	numachip_mask_pcounter(cntxt[node],0,6, &retval);
	if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
	numachip_mask_pcounter(cntxt[node],1,5, &retval);
	if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
	numachip_mask_pcounter(cntxt[node],2,3, &retval);
	if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
	numachip_mask_pcounter(cntxt[node],3,2, &retval);
	if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);    
	
	for (counter=0; counter<4; counter++) {
	    
	    printf("%d: select is 0x%x mask 0x%x value 0x%llx (%lld) \n",
		   counter,
		   numachip_get_pcounter_select(cntxt[node],counter,&retval),
		   numachip_get_pcounter_mask(cntxt[node],counter,&retval),
		   numachip_get_pcounter(cntxt[node],counter,&retval),
		   numachip_get_pcounter(cntxt[node],counter,&retval));
	    if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
	}

    }
    
	
}
void count_api_stop4(struct numachip_context **cntxt, unsigned int num_nodes) {

    nc_error_t retval = NUMACHIP_ERR_OK;
    unsigned int node=0, counter=0;
    printf("*STOP COUNTER* \n");
    for(node=0; node<num_nodes; node++) {
	for (counter=0; counter<4; counter++) {
	    numachip_stop_pcounter(cntxt[node],counter, &retval);
	    if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
	}

	
	
	for (counter=0; counter<4; counter++) {
	    printf("node %d counter %d: 0x%llx (%lld) \n",node, counter,numachip_get_pcounter(cntxt[node],counter,&retval), numachip_get_pcounter(cntxt[node],counter,&retval));
	    if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
	}
    }
    
}

void count_api_read4(struct numachip_context **cntxt, unsigned int num_nodes) {
    nc_error_t retval = NUMACHIP_ERR_OK;
    unsigned int node=0;
    printf("************************************************\n");
    for(node=0; node<num_nodes; node++) {
	unsigned long long hit=numachip_get_pcounter(cntxt[node],1,&retval);
	unsigned long long miss=numachip_get_pcounter(cntxt[node],0,&retval);
	unsigned long long total=numachip_get_pcounter(cntxt[node],0,&retval) + numachip_get_pcounter(cntxt[node],1,&retval);
	double long  missrate = (double long) 100*miss/total;
	double long hitrate=(double long)100*hit/total;


	//printf("Node %d: REM CAHCE HIT: %lld MISS %lld total %lld\n",node, hit,miss, total);
        if (total==0) {
	    printf("Node %d: NO hits or miss in remote cache\n", node);
	} else if (miss==0) {
	    printf("Node %d: Hit rate 100 percent \n", node);
	} else if (hit==0) {
	    printf("Node %d: Miss rate 100 percent \n", node);
	} else {
	    printf("Node %d: Miss rate %0.2Lf  Hit rate %0.2Lf \n",  node,missrate,hitrate);
	    
	}

    }
    printf("************************************************\n");
	
}
void count_api_compare_test(struct numachip_context *cntxt) {
    unsigned int counter=0;
    unsigned int mask=6;
    nc_error_t retval = NUMACHIP_ERR_OK;
    printf("/**************/\n");
    printf(" * CLEAR CNT **/\n");
    printf("/**************/\n");
    for (counter=0; counter<8; counter++) {
	numachip_clear_pcounter(cntxt,counter, &retval);
	if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
    }
    counter=0;
    printf("/**************/\n");
    printf(" ** SELE CNT **/\n");
    printf("/**************/\n");

    for (counter=0; counter<8; counter++) {
	numachip_mask_pcounter(cntxt,counter,mask, &retval);
	if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
    }

/*    for (counter=0; counter<8; counter++)
      numachip_select_pcounter(cntxt,counter);*/
    numachip_write_csr(cntxt,H2S_CSR_G3_SELECT_COUNTER,0x77777777);


    sleep(10);
    printf("/**************/\n");
    printf("*AFTER 10s*/\n");
    printf("/**************/\n");
    
    /*for (counter=0; counter<8; counter++) 
	numachip_stop_pcounter(cntxt,counter);
    */
    numachip_write_csr(cntxt,H2S_CSR_G3_SELECT_COUNTER,0x0);
    
    printf("/**************/\n");
    printf("*STOP COUNTER* \n");
    printf("/**************/\n");
    
    for (counter=0; counter<8; counter++) {
	printf("%d: 0x%llx (%lld) \n", counter,numachip_get_pcounter(cntxt,counter,&retval), numachip_get_pcounter(cntxt,counter,&retval));
    }


    sleep(10);
    printf("/**************/\n");
    printf("*CONFIRM STOPPED COUNTER AFTER 10s*/\n");
    printf("/**************/\n");
    for (counter=0; counter<8; counter++) {
	printf("%d: 0x%llx (%lld) \n", counter,numachip_get_pcounter(cntxt,counter,&retval), numachip_get_pcounter(cntxt,counter,&retval));
	if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
    }
        
}

void close_device(struct numachip_context *cntxt) {
    (void)numachip_close_device(cntxt);
}


int main(int argc, char **argv)
{
    struct numachip_device **devices;
    //struct numachip_context *cntxt;
    struct numachip_context **cntxt;
    int counter=0, i=0;
    int num_devices; 
    const char *filename = "fabric-loop-05.json";
    
    if (argc<2) {
        usage();
        return(-1);
    }


    
    devices = numachip_get_device_list(&num_devices, filename);
    DEBUG_STATEMENT(printf("Found %d NumaChip devices\n", num_devices));
    
    if (!devices)
	return -1;

    DEBUG_STATEMENT(printf("sizeof(struct numachip_context *) %ld\n", sizeof(struct numachip_context *)));
    cntxt = malloc(num_devices * sizeof(struct numachip_context *));

    for(i=0; i<num_devices; i++) {
	cntxt[i] = numachip_open_device(devices[i]);
    }
    
    numachip_free_device_list(devices);
	
    
    if (!cntxt[0])
	return -1;

    
    
     /* Get the parameters */
    for (counter=1; (int)counter<argc; counter++) {
	if (!strcmp("-dump-scc-csr",argv[counter])) {
	    dump_scc_csr(cntxt[0]);
	    dump_scc_csr(cntxt[1]);
	    continue;
	}
	
	if (!strcmp("-dump-phy-regs",argv[counter])) {
	    phy_regs(cntxt[0]);
	    phy_regs(cntxt[1]);
	    continue;
	}

	if (!strcmp("-setup-tracer",argv[counter])) {	    
	    tracer_setup(*cntxt);
	    continue;
	}

	if (!strcmp("-tracer-result",argv[counter])) {	    
	    tracer_result(*cntxt);	    
	    continue;
	}
	

	if (!strcmp("-count-start",argv[counter])) {	    
	    count_api_start4(cntxt, num_devices);
	    continue;
	}

	if (!strcmp("-count-stop",argv[counter])) {	    
	    count_api_stop4(cntxt, num_devices);
	    continue;
	}
	
	if (!strcmp("-count-read",argv[counter])) {	    
	    count_api_read4(cntxt, num_devices);
	    continue;
	}


    }

    
    for(i=0; i<num_devices; i++) {
	close_device(cntxt[i]);
    }
    free(cntxt);
    
    return 0;
}
