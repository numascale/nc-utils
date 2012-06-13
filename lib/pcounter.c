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

#define DEBUG_STATEMENT(x) 

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

void count_api_start_silent(struct numachip_context **cntxt, unsigned int num_nodes) {

    nc_error_t retval = NUMACHIP_ERR_OK;
    unsigned int node=0, counter=0;
    //printf("/** CLEAR CNT **/\n");
    for(node=0; node<num_nodes; node++) {
	for (counter=0; counter<8; counter++) {
	    numachip_clear_pcounter(cntxt[node],counter,&retval);
	    if (retval != NUMACHIP_ERR_OK) printf("API failed for node %d retval = 0x%x", node, retval);
	}
    }
    
    //printf("/** SELECT COUNTER **/\n");
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
    
    
    //printf("/** Apply Mask **/\n");
    for(node=0; node<num_nodes; node++) {
	numachip_mask_pcounter(cntxt[node],0,6, &retval);
	if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
	numachip_mask_pcounter(cntxt[node],1,5, &retval);
	if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
	numachip_mask_pcounter(cntxt[node],2,3, &retval);
	if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
	numachip_mask_pcounter(cntxt[node],3,2, &retval);
	if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);    
	/*
	for (counter=0; counter<4; counter++) {
	    
	    printf("%d: select is 0x%x mask 0x%x value 0x%llx (%lld) \n",
		   counter,
		   numachip_get_pcounter_select(cntxt[node],counter,&retval),
		   numachip_get_pcounter_mask(cntxt[node],counter,&retval),
		   numachip_get_pcounter(cntxt[node],counter,&retval),
		   numachip_get_pcounter(cntxt[node],counter,&retval));
	    if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
	}
	*/
    }
    
	
}
void count_api_stop_silent(struct numachip_context **cntxt, unsigned int num_nodes) {

    nc_error_t retval = NUMACHIP_ERR_OK;
    unsigned int node=0, counter=0;
//    printf("*STOP COUNTER* \n");
    for(node=0; node<num_nodes; node++) {
	for (counter=0; counter<4; counter++) {
	    numachip_stop_pcounter(cntxt[node],counter, &retval);
	    if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
	}

	
	/*
	for (counter=0; counter<4; counter++) {
	    printf("node %d counter %d: 0x%llx (%lld) \n",node, counter,numachip_get_pcounter(cntxt[node],counter,&retval), numachip_get_pcounter(cntxt[node],counter,&retval));
	    if (retval != NUMACHIP_ERR_OK) printf("API failed retval = 0x%x", retval);
	}
	*/
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
void count_api_read_rate(struct numachip_context *cntxt, double long *missrate, double long *hitrate, nc_error_t *error) {
    unsigned long long hit, miss, total;
    *missrate=0;
    *hitrate=0;
    hit=numachip_get_pcounter(cntxt,1,error);
    if (*error != NUMACHIP_ERR_OK) return;
    miss=numachip_get_pcounter(cntxt,0,error);
    if (*error != NUMACHIP_ERR_OK) return;
    total=numachip_get_pcounter(cntxt,0,error) + numachip_get_pcounter(cntxt,1,error);
     if (*error != NUMACHIP_ERR_OK) return;

    if (total==0) {
	*missrate=0;
	*hitrate=0;
    } else if (miss==0) {
	*missrate=0;
	*hitrate=100;
    } else if (hit==0) {
	*missrate=100;
	*hitrate=0;
    } else {
	*missrate = (double long) 100*miss/total;
	*hitrate=(double long)100*hit/total;
    }
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

/**
 * numachip_select_counter - Select Performance Counter
 */
void numachip_select_pcounter(struct numachip_context *cntxt,
			      unsigned int counterno,
			      unsigned int eventreg,
			      nc_error_t *error) { 

    unsigned int current_counter_val=0;
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
    current_counter_val=current_counter_val | ((eventreg << (counterno*4)));
    numachip_write_csr(cntxt,H2S_CSR_G3_SELECT_COUNTER,current_counter_val);
    DEBUG_STATEMENT(printf("Current counterval written 0x%x readback 0x%x\n",current_counter_val,numachip_read_csr(cntxt,H2S_CSR_G3_SELECT_COUNTER)));

}

unsigned int numachip_get_pcounter_select(struct numachip_context *cntxt,
			      unsigned int counterno,
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
			    unsigned int counterno,
			    unsigned int mask,
			    nc_error_t *error) { 

    unsigned int mask_register, mask_value;

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
    

    
    mask_register=H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_0 + (0x4*counterno);
  
    //TBD: Check that the counter has been cleared?
    mask_value = 1 << mask ;
    DEBUG_STATEMENT(printf("1.We program the mask_register to count 6 - HT-Request with ctag miss  (and the counting starts) mask value 0x%x\n", mask_value));
    //2.We program the mask_register to count 6 - HT-Request with ctag miss  (and the counting starts).
    //We use the same value for compare and mask to only count this event.
    mask_value= (mask_value << 8) | mask_value;
    DEBUG_STATEMENT(printf("2.We program the mask_register to count 6 - HT-Request with ctag miss  (and the counting starts) mask value 0x%x\n", mask_value));
    numachip_write_csr(cntxt,mask_register,mask_value);
    DEBUG_STATEMENT(printf("Mask register at 0x%x is set to 0x%x READBACK: 0x%x\n",mask_register, mask_value,numachip_read_csr(cntxt,mask_register)));

}

unsigned int numachip_get_pcounter_mask(struct numachip_context *cntxt,
			    unsigned int counterno,
			    nc_error_t *error) { 

    unsigned int mask_register;

    *error = NUMACHIP_ERR_OK;
    if (counterno > 7) {
	*error=NUMACHIP_ERR_INVALID_PARAMETER;
	return 0;
    }

    mask_register=H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_0 + (0x4*counterno);
    return numachip_read_csr(cntxt,mask_register);

}

/*Stop also clears the mask. Not obvius*/
void numachip_stop_pcounter(struct numachip_context *cntxt, unsigned int counterno, nc_error_t *error) {

    unsigned int current_counter_val, mask_register;

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
			     unsigned int counterno,
			     nc_error_t *error) { 
    unsigned int mask_register,  perf_reg, current_counter_val;

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
    
    DEBUG_STATEMENT(printf("1. Clear: First we clear the counter selection %d .\n", counterno));
    current_counter_val=numachip_read_csr(cntxt,H2S_CSR_G3_SELECT_COUNTER);
    DEBUG_STATEMENT(printf("Current counterval before to be cleared 0x%x\n",current_counter_val));
    current_counter_val=current_counter_val & (~(0x7 << (counterno*4)));
    DEBUG_STATEMENT(printf("Current counterval to be cleared 0x%x\n",current_counter_val));
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

unsigned long long numachip_get_pcounter(struct numachip_context *cntxt,
					 unsigned int counterno, nc_error_t *error)
{
    unsigned int perfreg=0;
    unsigned long long counterval=0;

    *error = NUMACHIP_ERR_OK;
    if (counterno > 7) {
	*error=NUMACHIP_ERR_INVALID_PARAMETER;
	return 0;
    }
    
    perfreg=H2S_CSR_G3_PERFORMANCE_COUNTER_0_40_BIT_UPPER_BITS + (0x8*counterno);
    counterval= (unsigned long long) numachip_read_csr(cntxt,perfreg) << 32;
    DEBUG_STATEMENT(printf("PERF_REF=0x%x value=0x%x \n",perfreg, counterval));
    
    perfreg=H2S_CSR_G3_PERFORMANCE_COUNTER_0_40_BIT_LOWER_BITS + (0x8*counterno);
    counterval= (unsigned long long) counterval | numachip_read_csr(cntxt,perfreg);
    DEBUG_STATEMENT(printf("PERF_REF=0x%x value=0x%x \n", perfreg, counterval));
    
    return counterval;
    
}

