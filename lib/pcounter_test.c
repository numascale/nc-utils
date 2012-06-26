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
#include "pcounter_test.h"
#define DEBUG_STATEMENT(x) x


void count_api_start(struct numachip_context **cntxt, unsigned int num_nodes) {

    nc_error_t retval = NUMACHIP_ERR_OK;
    unsigned int node=0, counter=0;
  
    for(node=0; node<num_nodes; node++) {
	numachip_fullstart_pcounter(cntxt[node],0,0x1,6, &retval);
	if (retval != NUMACHIP_ERR_OK) fprintf(stderr,"Numachip user API failed retval = 0x%x", retval);
	numachip_fullstart_pcounter(cntxt[node],1,0x1,5, &retval);
	if (retval != NUMACHIP_ERR_OK) fprintf(stderr,"Numachip user API failed retval = 0x%x", retval);    
	numachip_fullstart_pcounter(cntxt[node],2,0x6,3, &retval);
	if (retval != NUMACHIP_ERR_OK) fprintf(stderr,"Numachip user API failed retval = 0x%x", retval);
	numachip_fullstart_pcounter(cntxt[node],3,0x6,2, &retval);
	if (retval != NUMACHIP_ERR_OK) fprintf(stderr,"Numachip user API failed retval = 0x%x", retval);

	DEBUG_STATEMENT(
	    for (counter=0; counter<4; counter++) {
	    
		printf("%d: select is 0x%x mask 0x%x value 0x%llx (%lld) \n",
		       counter,
		       numachip_get_pcounter_select(cntxt[node],counter,&retval),
		       numachip_get_pcounter_mask(cntxt[node],counter,&retval),
		       numachip_get_pcounter(cntxt[node],counter,&retval),
		       numachip_get_pcounter(cntxt[node],counter,&retval));
		if (retval != NUMACHIP_ERR_OK) fprintf(stderr,"Numachip user API failed retval = 0x%x", retval);
	    }
	)
    }
    
	
}


void count_api_stop(struct numachip_context **cntxt, unsigned int num_nodes) {

    nc_error_t retval = NUMACHIP_ERR_OK;
    unsigned int node=0, counter=0;

    /*STOP COUNTER*/
    for(node=0; node<num_nodes; node++) {
	for (counter=0; counter<4; counter++) {
	    numachip_stop_pcounter(cntxt[node],counter, &retval);
	    if (retval != NUMACHIP_ERR_OK) fprintf(stderr,"Numachip user API failed retval = 0x%x", retval);
	}

	
	
	DEBUG_STATEMENT(
	for (counter=0; counter<4; counter++) {
	    printf("node %d counter %d: 0x%llx (%lld) \n",node, counter,numachip_get_pcounter(cntxt[node],counter,&retval), numachip_get_pcounter(cntxt[node],counter,&retval));
	    if (retval != NUMACHIP_ERR_OK) fprintf(stderr,"Numachip user API failed retval = 0x%x", retval);
	})
	
    }
    
}

#if 0
void count_api_start(struct numachip_context **cntxt, unsigned int num_nodes) {

    nc_error_t retval = NUMACHIP_ERR_OK;
    unsigned int node=0, counter=0;
    /** CLEAR CNT **/
    for(node=0; node<num_nodes; node++) {
	for (counter=0; counter<4; counter++) {
	    numachip_clear_pcounter(cntxt[node],counter,&retval);
	    if (retval != NUMACHIP_ERR_OK) fprintf(stderr,"Numachip user API failed for node %d retval = 0x%x", node, retval);
	}
    }
    
    /** SELECT COUNTER **/
    for(node=0; node<num_nodes; node++) {
	numachip_select_pcounter(cntxt[node],0,0x1, &retval);
	if (retval != NUMACHIP_ERR_OK) fprintf(stderr,"Numachip user API failed retval = 0x%x", retval);
	numachip_select_pcounter(cntxt[node],1,0x1, &retval);
	if (retval != NUMACHIP_ERR_OK) fprintf(stderr,"Numachip user API failed retval = 0x%x", retval);    
	numachip_select_pcounter(cntxt[node],2,0x6, &retval);
	if (retval != NUMACHIP_ERR_OK) fprintf(stderr,"Numachip user API failed retval = 0x%x", retval);
	numachip_select_pcounter(cntxt[node],3,0x6, &retval);
	if (retval != NUMACHIP_ERR_OK) fprintf(stderr,"Numachip user API failed retval = 0x%x", retval);
    }
    
    
    /** Apply Mask **/
    for(node=0; node<num_nodes; node++) {
	numachip_mask_pcounter(cntxt[node],0,6, &retval);
	if (retval != NUMACHIP_ERR_OK) fprintf(stderr,"Numachip user API failed retval = 0x%x", retval);
	numachip_mask_pcounter(cntxt[node],1,5, &retval);
	if (retval != NUMACHIP_ERR_OK) fprintf(stderr,"Numachip user API failed retval = 0x%x", retval);
	numachip_mask_pcounter(cntxt[node],2,3, &retval);
	if (retval != NUMACHIP_ERR_OK) fprintf(stderr,"Numachip user API failed retval = 0x%x", retval);
	numachip_mask_pcounter(cntxt[node],3,2, &retval);
	if (retval != NUMACHIP_ERR_OK) fprintf(stderr,"Numachip user API failed retval = 0x%x", retval);    

	DEBUG_STATEMENT(
	for (counter=0; counter<4; counter++) {
	    
	    printf("%d: select is 0x%x mask 0x%x value 0x%llx (%lld) \n",
		   counter,
		   numachip_get_pcounter_select(cntxt[node],counter,&retval),
		   numachip_get_pcounter_mask(cntxt[node],counter,&retval),
		   numachip_get_pcounter(cntxt[node],counter,&retval),
		   numachip_get_pcounter(cntxt[node],counter,&retval));
	    if (retval != NUMACHIP_ERR_OK) fprintf(stderr,"Numachip user API failed retval = 0x%x", retval);
	}
	)
    }
    
	
}

#endif
void count_api_read_rate(struct numachip_context *cntxt, double *missrate, double *hitrate, unsigned long long *total, nc_error_t *error) {
    unsigned long long hit, miss;
    *missrate=0;
    *hitrate=0;
    *total=0;
    hit=numachip_get_pcounter(cntxt,1,error);
    if (*error != NUMACHIP_ERR_OK) return;
    miss=numachip_get_pcounter(cntxt,0,error);
    if (*error != NUMACHIP_ERR_OK) return;
    *total=numachip_get_pcounter(cntxt,0,error) + numachip_get_pcounter(cntxt,1,error);
    if (*error != NUMACHIP_ERR_OK) return;

    if (*total==0) {
	*missrate=0;
	*hitrate=100;	
	DEBUG_STATEMENT(printf("NO hits or miss in remote cache\n"));
    } else if (miss==0) {
	*missrate=0;
	*hitrate=100;
	DEBUG_STATEMENT(printf("Hit rate 100 percent \n"));
    } else if (hit==0) {
	*missrate=100;
	*hitrate=0;
	DEBUG_STATEMENT(printf("Miss rate 100 percent \n"));
    } else {
	*missrate=(double)100*miss/(*total);
	*hitrate=(double)100*hit/(*total);
	DEBUG_STATEMENT(printf("Miss rate %0.2f  Hit rate %0.2f \n", *missrate,*hitrate));
    }
}
