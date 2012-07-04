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
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "numachip_lib.h"
#include "numachip_user.h"
#define DEBUG_STATEMENT(x) 


void numachip_fullstart_pcounter(struct numachip_context *cntxt,
			    uint32_t counterno,
			    uint32_t event,
			    uint32_t mask,
			    nc_error_t *error) {

    /** CLEAR CNT **/
    numachip_clear_pcounter(cntxt,counterno,error);
    if ((*error) != NUMACHIP_ERR_OK) {
	fprintf(stderr,"Numachip user API failed for counterno %d retval = 0x%x", counterno, *error);
	return;
    }

    /** SELECT COUNTER **/
    numachip_select_pcounter(cntxt,counterno,event, error);
    if ((*error) != NUMACHIP_ERR_OK) {
	fprintf(stderr,"Numachip user API failed for counterno %d event %d retval = 0x%x", counterno,event,*error);
	return;
    }
    /** Apply Mask **/
    numachip_mask_pcounter(cntxt,counterno, mask, error);
    if ((*error) != NUMACHIP_ERR_OK) {
	fprintf(stderr,"Numachip user API failed for counterno %d mask %d retval = 0x%x", counterno, mask,*error);
	return;
    }
       
}

void numachip_all_start_pcounter(struct numachip_context **cntxt,
				 uint32_t num_nodes,
				 uint32_t counterno,
				 uint32_t event,
				 uint32_t mask,
				 nc_error_t *error) {

    nc_error_t retval = NUMACHIP_ERR_OK;
    uint32_t node=0;
    
    for(node=0; node<num_nodes; node++) {
	numachip_fullstart_pcounter(cntxt[node],counterno,event,mask, error);
	if (retval != NUMACHIP_ERR_OK) {
	    fprintf(stderr,"Numachip user API failed retval = 0x%x", *error);
	    return;
	}
    }
    
	DEBUG_STATEMENT(
		printf("%d: select is 0x%x mask 0x%x value 0x%llx (%lld) \n",
		       counterno,
		       numachip_get_pcounter_select(cntxt[node],counterno,error),
		       numachip_get_pcounter_mask(cntxt[node],counterno,error),
		       numachip_get_pcounter(cntxt[node],counterno,error),
		       numachip_get_pcounter(cntxt[node],counterno,error));
		if (retval != NUMACHIP_ERR_OK) fprintf(stderr,"Numachip user API failed retval = 0x%x",*error);)
		
	    

}
	



/**
 * numachip_select_counter - Select Performance Counter
 */
void numachip_select_pcounter(struct numachip_context *cntxt,
			      uint32_t counterno,
			      uint32_t eventreg,
			      nc_error_t *error) { 

    uint32_t current_counter_val=0;
    *error = NUMACHIP_ERR_OK;
    
    if (counterno > 7) {
	*error=NUMACHIP_ERR_INVALID_PARAMETER;
	return;
    }
    
    if (eventreg > 7) {
	*error=NUMACHIP_ERR_INVALID_PARAMETER;
	return;
    }
    
    
    current_counter_val=numachip_read_csr(cntxt,H2S_CSR_G3_SELECT_COUNTER);
    DEBUG_STATEMENT(printf("Current counterval read 0x%x eventreg 0x%x\n", current_counter_val, eventreg));
    DEBUG_STATEMENT(printf("Current (eventreg << (counterno*4))  0x%x\n", (eventreg << (counterno*4))));
    if (current_counter_val & ((eventreg << (counterno*4)))) {
	DEBUG_STATEMENT(printf("Current (eventreg << (counterno*4))  0x%x equals current_counterval 0x%x\n", (eventreg << (counterno*4)), current_counter_val));
	*error=NUMACHIP_ERR_BUSY;
	return;
    }
    
    current_counter_val=current_counter_val | ((eventreg << (counterno*4)));
    numachip_write_csr(cntxt,H2S_CSR_G3_SELECT_COUNTER,current_counter_val);
    DEBUG_STATEMENT(printf("Current counterval written 0x%x readback 0x%x\n",current_counter_val,numachip_read_csr(cntxt,H2S_CSR_G3_SELECT_COUNTER)));

}

uint32_t numachip_get_pcounter_select(struct numachip_context *cntxt,
				      uint32_t counterno,
				      nc_error_t *error) { 

    *error = NUMACHIP_ERR_OK;
    
    if (counterno > 7) {
	*error=NUMACHIP_ERR_INVALID_PARAMETER;
	return 0;
    }
    
    
    return numachip_read_csr(cntxt,H2S_CSR_G3_SELECT_COUNTER);
    
}


/**
 * numachip_mask_counter - Apply counter mask
 *
 * Select = 0, REM/SPrb :
 *
 *       7 - SCC-Request Invalidate          (shared   => invalid)
 *       6 - SCC-Request Read                (modified => shared)
 *       5 - SCC-Request Read and Invalidate (modified => invalid)
 *       4 - SCC-Request Aliased Invalidate  (shared   => invalid)
 *       3 - SCC-Request Aliased Read and Invalidate (modified => invalid)
 *       2 - SCC-Request with SPrb conflict
 *       1 - SCC-Request with HReq conflict
 *       0 - Cache data access
 *
 * Select = 1, REM/HReq :
 *
 *       7 - HT-Request start processing
 *       6 - HT-Request with ctag miss
 *       5 - HT-Request with ctag hit
 *       4 - HT-Request with HReq conflict
 *       3 - HT-Request with SPrb conflict
 *       2 - HT-command unknown
 *       1 - Broadcast messages
 *       0 - Direct interrupt (no broadcast)
 *
 *
 * Select = 2, LOC/SReq :
 *
 *       7 - Interrupt request
 *       6 - Config Space request
 *       5 - VictimBlk request
 *       4 - VictimBlk conflict
 *       3 - SCC conflict
 *       2 - SCC discard
 *       1 - SCC request (all)
 *       0 - Error in interrupt
 *
 *
 * Select = 3, LOC/HPrb :
 *
 *       7 - HT lock pending
 *       6 - VictimBlk conflict
 *       5 - HT-probe with next-state=invalidate
 *       4 - SCC retries
 *       3 - SCC requests
 *       2 - HT-probe on own request
 *       1 - HT-probe with next-state=shared
 *       0 - HT-probe to non-shared memory
 *
 *
 * Select = 4, CData :
 *
 *       7 - CData write request from REM/HReq
 *       6 - CData write request from REM/HReq accepted
 *       5 - CData read request from REM/HReq
 *       4 - CData read request from REM/HReq accepted
 *       3 - CData write request from REM/SPrb
 *       2 - CData write request from REM/SPrb accepted
 *       1 - CData read request from REM/SPrb
 *       0 - CData read request from REM/SPrb accepted
 *
 *
 * Select = 5, FTag :
 *
 *       7 - Tag update valid from MCTag
 *       6 - Tag read valid from MCTag
 *       5 - MCTag request
 *       4 - Tag response valid from MCTag to LOC/HPrb
 *       3 - Unused
 *       2 - Tag response valid from prefetch to LOC/HPrb
 *       1 - Unused
 *       0 - Tag request from LOC/HPrb
 *
 *
 * Select = 6, MCTag :
 *
 *       7 - Unused
 *       6 - Prefetch buffer address hit
 *       5 - Prefetch buffer full hit
 *       4 - Tag request from REM/HReq
 *       3 - CTag cache hit
 *       2 - CTag cache miss
 *       1 - DRAM read request
 *       0 - DRAM read request delayed
 *
 *
 * Select = 7, cHT-Cave :
 *
 *       7 - Outgoing HT-Probe
 *       6 - Outgoing HT-Response
 *       5 - Outgoing posted HT-Request
 *       4 - Outgoing non-posted HT-Request
 *       3 - Incoming HT-Probe
 *       2 - Incoming HT-Response
 *       1 - Incoming posted HT-Request
 *       0 - Incoming non-posted HT-Request
 *
 *
 *
 *
 * Documentation is in hdl:
 * assign prfMask0  = CSR_H2S_G3xFA0[`prfMask0Range];
 * assign prfCom0   = CSR_H2S_G3xFA0[`prfCom0Range];
 * assign pc0Event = |(prfMask0 & (prfCom0 ~^ Sel0Event));
 * With the hdl code above (which is the documentation we got) we get the following if only we want look at event 6:
 * prfCom0       10 0000 (0x40)
 * ~^ Sel0Event  10 1010 (Event on some channels among them "6 - HT-Request with ctag miss")
 * =             11 0101
 * & prfMask0    10 0000
 * =             10 0000
 * 
 */


void numachip_mask_pcounter(struct numachip_context *cntxt,
			    uint32_t counterno,
			    uint32_t mask,
			    nc_error_t *error) { 
    
    uint32_t mask_register, mask_value;
    
    *error = NUMACHIP_ERR_OK;
    if (counterno > 7) {
	*error=NUMACHIP_ERR_INVALID_PARAMETER;
	return;
    }
    
    /*
     * We will accept values from 0-7. And translate them in here
     */
    
    if (mask>7) {
	*error=NUMACHIP_ERR_INVALID_PARAMETER;
	return;
    }
    //6,5,3,2
    
    if (counterno==7) {
	DEBUG_STATEMENT(printf("Need to consider H2S_CSR_G3_TIMER_FOR_ECC_COUNTER_7: 0x%x\n",numachip_read_csr(cntxt,H2S_CSR_G3_TIMER_FOR_ECC_COUNTER_7)));
	numachip_write_csr(cntxt,H2S_CSR_G3_TIMER_FOR_ECC_COUNTER_7,0x0);
    }
    
    mask_register=H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_0 + (0x4*counterno);
    
    //TBD: Check that the counter has been cleared?
    
    mask_value = 1 << mask ;
    DEBUG_STATEMENT(printf("1.We program the mask_register to mask value 0x%x\n", mask_value));
    //2.We program the mask_register to count 6 - HT-Request with ctag miss  (and the counting starts).
    //We use the same value for compare and mask to only count this event.
    mask_value= (mask_value << 8) | mask_value;
    DEBUG_STATEMENT(printf("2.We program the mask_register to count mask value 0x%x was 0x%x\n", mask_value,numachip_get_pcounter_mask(cntxt,counterno,error)));
    
    if (numachip_get_pcounter_mask(cntxt,counterno,error) & mask_value) {
	*error=NUMACHIP_ERR_BUSY;
	return;
    }
    
    numachip_write_csr(cntxt,mask_register,mask_value);
    DEBUG_STATEMENT(printf("Mask register at 0x%x is set to 0x%x READBACK: 0x%x\n",mask_register, mask_value,numachip_read_csr(cntxt,mask_register)));
    
}

uint32_t numachip_get_pcounter_mask(struct numachip_context *cntxt,
				    uint32_t counterno,
				    nc_error_t *error) { 
    
    uint32_t mask_register;
    
    *error = NUMACHIP_ERR_OK;
    if (counterno > 7) {
	*error=NUMACHIP_ERR_INVALID_PARAMETER;
	return 0;
    }
    
    mask_register=H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_0 + (0x4*counterno);
    return numachip_read_csr(cntxt,mask_register);
    
}

/*Stop also clears the mask. Not obvius*/
void numachip_stop_pcounter(struct numachip_context *cntxt, uint32_t counterno, nc_error_t *error) {
    
    uint32_t current_counter_val, mask_register;

    *error = NUMACHIP_ERR_OK;
    if (counterno > 7) {
	*error=NUMACHIP_ERR_INVALID_PARAMETER;
	return;
    }
    
    current_counter_val=numachip_read_csr(cntxt,H2S_CSR_G3_SELECT_COUNTER);
    current_counter_val=0x77777777 & (~(0x7 << (counterno*4)));
    DEBUG_STATEMENT(printf("Current counterval to be stopped 0x%x\n",current_counter_val));
    numachip_write_csr(cntxt,H2S_CSR_G3_SELECT_COUNTER,current_counter_val);
    DEBUG_STATEMENT(printf("Select register at 0x%x is set to 0x%x READBACK: 0x%x\n",H2S_CSR_G3_SELECT_COUNTER,current_counter_val,numachip_read_csr(cntxt,H2S_CSR_G3_SELECT_COUNTER)));
    mask_register=H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_0 + (0x4*counterno);
    numachip_write_csr(cntxt,mask_register,0);
    DEBUG_STATEMENT(printf("Mask register at 0x%x is set to 0x%x READBACK: 0x%x\n",mask_register, 0,numachip_read_csr(cntxt,mask_register)));
}

/**
 * numachip_clear_counter - Clear Performance Counter Register
 */
void numachip_clear_pcounter(struct numachip_context *cntxt,
			     uint32_t counterno,
			     nc_error_t *error) { 
    uint32_t mask_register,  perf_reg, current_counter_val;

    *error = NUMACHIP_ERR_OK;
    if (counterno > 7) {
	*error=NUMACHIP_ERR_INVALID_PARAMETER;
	return;
    } 
    /*
     * 1. Clear: First we clear the counter selection, mask-register and the
     * corresponding performance register.
     *
     * Note: We do not care about the previous values in the mask and perf reg,
     * since we are going to pollute these anyway (in the first version).
     */
    
    DEBUG_STATEMENT(printf("1. Clear: First we clear the counter select register for counter %d .\n", counterno));
    current_counter_val=numachip_read_csr(cntxt,H2S_CSR_G3_SELECT_COUNTER);
    DEBUG_STATEMENT(printf("Current counter value before to be cleared 0x%x\n",current_counter_val));
    DEBUG_STATEMENT(printf("Mask to be used for calculating value 0x%x\n",0x7 << (counterno*4)));
    current_counter_val=current_counter_val & (~(0x7 << (counterno*4)));
    DEBUG_STATEMENT(printf("New Current counter value to be cleared 0x%x\n",current_counter_val));
    numachip_write_csr(cntxt,H2S_CSR_G3_SELECT_COUNTER,current_counter_val);
    /* We need to clear the corresponing comare and mask register we are planning to use:
     * H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_0 (0xFA0 + 0x4*counterno)
     */
    mask_register=H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_0 + (0x4*counterno);
    numachip_write_csr(cntxt,mask_register,0x0);

    /* We need to clear the corresponding Performance counter register we plan to use
     * H2S_CSR_G3_PERFORMANCE_COUNTER_0_40_BIT_UPPER_BITS (0xFC0 + (0x8
     */
    perf_reg=H2S_CSR_G3_PERFORMANCE_COUNTER_0_40_BIT_UPPER_BITS + (0x8*counterno);
    numachip_write_csr(cntxt, perf_reg,0x0);
    perf_reg=H2S_CSR_G3_PERFORMANCE_COUNTER_0_40_BIT_LOWER_BITS + (0x8*counterno);
    numachip_write_csr(cntxt, perf_reg ,0x0);
        
}

   
/**
 * numachip_get_counter - Read Performance Counter Register
 */

uint64_t numachip_get_pcounter(struct numachip_context *cntxt,
					 uint32_t counterno, nc_error_t *error)
{
    uint32_t perfreg=0;
    uint64_t counterval=0;

    *error = NUMACHIP_ERR_OK;
    if (counterno > 7) {
	*error=NUMACHIP_ERR_INVALID_PARAMETER;
	return 0;
    }
    
    perfreg=H2S_CSR_G3_PERFORMANCE_COUNTER_0_40_BIT_UPPER_BITS + (0x8*counterno);
    counterval= (uint64_t) numachip_read_csr(cntxt,perfreg) << 32;
    DEBUG_STATEMENT(printf("PERF_REF=0x%x value=0x%Lx \n",perfreg, counterval));
    
    perfreg=H2S_CSR_G3_PERFORMANCE_COUNTER_0_40_BIT_LOWER_BITS + (0x8*counterno);
    counterval= (uint64_t) counterval | numachip_read_csr(cntxt,perfreg);
    DEBUG_STATEMENT(printf("PERF_REF=0x%x value=0x%Lx \n", perfreg, counterval));
    
    return counterval;
    
}

