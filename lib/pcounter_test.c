/*
 * Copyright (C) 2008-2015 Numascale AS, support@numascale.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
#define DEBUG_STATEMENT(x)


nc_error_t count_api_start(struct numachip_context **cntxt, uint32_t num_nodes) {

    nc_error_t retval = NUMACHIP_ERR_OK;
    uint32_t node=0;
    DEBUG_STATEMENT(uint32_t counter=0);

    for(node=0; node<num_nodes; node++) {
	numachip_fullstart_pcounter(cntxt[node],0,0x1,6, &retval);
	if (retval != NUMACHIP_ERR_OK) return retval;
	numachip_fullstart_pcounter(cntxt[node],1,0x1,5, &retval);
	if (retval != NUMACHIP_ERR_OK) return retval;
	numachip_fullstart_pcounter(cntxt[node],2,0x6,3, &retval);
	if (retval != NUMACHIP_ERR_OK) return retval;
	numachip_fullstart_pcounter(cntxt[node],3,0x6,2, &retval);
	if (retval != NUMACHIP_ERR_OK) return retval;

	DEBUG_STATEMENT(
	    for (counter=0; counter<4; counter++) {

		printf("%d: select is 0x%x mask 0x%x value 0x%llx (%lld) \n",
		       counter,
		       numachip_get_pcounter_select(cntxt[node],counter,&retval),
		       numachip_get_pcounter_mask(cntxt[node],counter,&retval),
		       numachip_get_pcounter(cntxt[node],counter,&retval),
		       numachip_get_pcounter(cntxt[node],counter,&retval));
		if (retval != NUMACHIP_ERR_OK) return retval;
	    }
	)
    }

    return retval;
}
nc_error_t counter_select(struct numachip_context *cntxt, uint32_t counterno,uint32_t val) {

    /*Has counter already been selected?*/
    /*Are parameters valid?*/
    nc_error_t retval = NUMACHIP_ERR_OK;

    numachip_select_pcounter(cntxt,counterno,val, &retval);
    if (retval != NUMACHIP_ERR_OK) fprintf(stderr,"Numachip select pcounter failed: %s\n", numachip_strerror(retval));
    return retval;
}
nc_error_t counter_mask(struct numachip_context *cntxt, uint32_t counterno, uint32_t val) {
    nc_error_t retval = NUMACHIP_ERR_OK;

    /*Has counter already been masked?*/
    DEBUG_STATEMENT(printf("Masking counter counterno %d mask 0x%x\n",
			   counterno, val));
    numachip_mask_pcounter(cntxt,counterno,val, &retval);
    if (retval != NUMACHIP_ERR_OK) fprintf(stderr,"Numachip mask pcounter failed  %s\n", numachip_strerror(retval));
    return retval;

}

nc_error_t counter_clear(struct numachip_context *cntxt, uint32_t counterno) {
    nc_error_t retval = NUMACHIP_ERR_OK;

    numachip_clear_pcounter(cntxt,counterno,&retval);
    if (retval != NUMACHIP_ERR_OK) fprintf(stderr,"Numachip clear pcounter failed  %s\n", numachip_strerror(retval));
    return retval;

}

nc_error_t counter_restart(struct numachip_context *cntxt, uint32_t counterno) {
    nc_error_t retval = NUMACHIP_ERR_OK;

    numachip_restart_pcounter(cntxt,counterno,&retval);
    if (retval != NUMACHIP_ERR_OK) fprintf(stderr,"Numachip restart pcounter failed  %s\n", numachip_strerror(retval));
    return retval;

}

nc_error_t counter_stop(struct numachip_context *cntxt, uint32_t counterno) {
    nc_error_t retval = NUMACHIP_ERR_OK;

    numachip_stop_pcounter(cntxt,counterno, &retval);
    if (retval != NUMACHIP_ERR_OK) fprintf(stderr,"Numachip stop pcounter failed  %s\n", numachip_strerror(retval));
    return retval;



}

nc_error_t counter_start(struct numachip_context *cntxt, uint32_t counterno, uint32_t event, uint32_t mask) {
    nc_error_t retval = NUMACHIP_ERR_OK;

    numachip_fullstart_pcounter(cntxt,counterno,event,mask,&retval);
    if (retval != NUMACHIP_ERR_OK) fprintf(stderr,"Numachip start pcounter failed  %s\n", numachip_strerror(retval));
    return retval;



}
uint64_t counter_read(struct numachip_context *cntxt,uint32_t counterno) {
    nc_error_t retval = NUMACHIP_ERR_OK;

    uint64_t counterval= numachip_get_pcounter(cntxt,counterno,&retval);
    if (retval != NUMACHIP_ERR_OK) fprintf(stderr,"Numachip read pcounter failed  %s\n", numachip_strerror(retval));

    return counterval;

}

nc_error_t counter_start_all(struct numachip_context **cntxt, uint32_t num_nodes, uint32_t counterno, uint32_t event, uint32_t mask) {
    nc_error_t retval = NUMACHIP_ERR_OK;

    numachip_all_start_pcounter(cntxt,num_nodes,counterno,event,mask,&retval);
    if (retval != NUMACHIP_ERR_OK) fprintf(stderr,"Numachip all start pcounter failed  %s\n", numachip_strerror(retval));
    return retval;



}
nc_error_t counter_select_all(struct numachip_context **cntxt, uint32_t num_nodes,uint32_t counterno,uint32_t val) {
    uint32_t node=0;
    nc_error_t retval = NUMACHIP_ERR_OK;

    for(node=0; node<num_nodes; node++) {
	retval=counter_select(cntxt[node],counterno,val);
	if (retval != NUMACHIP_ERR_OK)  return retval;
    }
    return retval;
}
nc_error_t counter_mask_all(struct numachip_context **cntxt, uint32_t num_nodes,uint32_t counterno,uint32_t val) {
    uint32_t node=0;
    nc_error_t retval = NUMACHIP_ERR_OK;
    for(node=0; node<num_nodes; node++) {
	retval=counter_mask(cntxt[node], counterno,val);
	if (retval != NUMACHIP_ERR_OK)  return retval;
    }
    return retval;
}

nc_error_t counter_clear_all(struct numachip_context **cntxt, uint32_t num_nodes,uint32_t counterno) {
    uint32_t node=0;
    nc_error_t retval = NUMACHIP_ERR_OK;
    for(node=0; node<num_nodes; node++) {
	    DEBUG_STATEMENT(
	    printf("node %d counter %d: \n",node, counterno));

	retval=counter_clear(cntxt[node],counterno);
	if (retval != NUMACHIP_ERR_OK)  return retval;
    }
    return retval;
}

nc_error_t counter_restart_all(struct numachip_context **cntxt, uint32_t num_nodes,uint32_t counterno) {
    uint32_t node=0;
    nc_error_t retval = NUMACHIP_ERR_OK;
    for(node=0; node<num_nodes; node++) {
	retval=counter_restart(cntxt[node],counterno);
	if (retval != NUMACHIP_ERR_OK)  return retval;
    }
    return retval;
}

nc_error_t counter_stop_all(struct numachip_context **cntxt, uint32_t num_nodes,uint32_t counterno) {
    nc_error_t retval = NUMACHIP_ERR_OK;
    uint32_t node=0;
    for(node=0; node<num_nodes; node++) {
	retval=counter_stop(cntxt[node],counterno);
	if (retval != NUMACHIP_ERR_OK)  return retval;
    }
    return retval;
}
void counter_print_all(struct numachip_context **cntxt, uint32_t num_nodes,uint32_t counterno) {

    uint32_t node=0;
    for(node=0; node<num_nodes; node++) {
	printf("Reading counter node %d counterno %d = %lld \n",
		       node,counterno, (unsigned long long)
		       counter_read(cntxt[node],counterno));
    }
}


nc_error_t count_api_stop(struct numachip_context **cntxt, uint32_t num_nodes) {

    nc_error_t retval = NUMACHIP_ERR_OK;
    uint32_t node=0, counter=0;

    /*STOP COUNTER*/
    for(node=0; node<num_nodes; node++) {
	for (counter=0; counter<4; counter++) {
	    numachip_stop_pcounter(cntxt[node],counter, &retval);
	    if (retval != NUMACHIP_ERR_OK) fprintf(stderr,"Numachip stop pcounter failed  %s\n", numachip_strerror(retval));
	    return retval;
	}



	DEBUG_STATEMENT(
	for (counter=0; counter<4; counter++) {
	    printf("node %d counter %d: 0x%llx (%lld) \n",node, counter,numachip_get_pcounter(cntxt[node],counter,&retval), numachip_get_pcounter(cntxt[node],counter,&retval));
	    if (retval != NUMACHIP_ERR_OK) fprintf(stderr,"Numachip user API failed retval = 0x%x", retval);
	})

    }
    return retval;
}

#if 0
void count_api_start(struct numachip_context **cntxt, uint32_t num_nodes) {

    nc_error_t retval = NUMACHIP_ERR_OK;
    uint32_t node=0, counter=0;
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
void count_api_read_rcache(struct numachip_context *cntxt,
			   uint32_t misscounter,
			   uint32_t hitcounter,
			   double *missrate,
			   double *hitrate,
			   uint64_t *total,
			   nc_error_t *error) {
    uint64_t hit, miss;
    *missrate=0;
    *hitrate=0;
    *total=0;
    hit=numachip_get_pcounter(cntxt,hitcounter,error);
    if (*error != NUMACHIP_ERR_OK) return;
    miss=numachip_get_pcounter(cntxt,misscounter,error);
    if (*error != NUMACHIP_ERR_OK) return;
    *total=numachip_get_pcounter(cntxt,hitcounter,error) + numachip_get_pcounter(cntxt,misscounter,error);
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
void count_api_read_rcache2(struct numachip_context *cntxt,
			   uint32_t misscounter,
			   uint32_t hitcounter,
			   double *missrate,
			   double *hitrate,
			   uint64_t *total,
			   uint64_t *miss,
			   uint64_t *hit,
			   nc_error_t *error) {

    *missrate=0;
    *hitrate=0;
    *total=0;
    *hit=numachip_get_pcounter(cntxt,hitcounter,error);
    if (*error != NUMACHIP_ERR_OK) return;
    *miss=numachip_get_pcounter(cntxt,misscounter,error);
    if (*error != NUMACHIP_ERR_OK) return;
    *total=numachip_get_pcounter(cntxt,hitcounter,error) + numachip_get_pcounter(cntxt,misscounter,error);
    if (*error != NUMACHIP_ERR_OK) return;

    if (*total==0) {
	*missrate=0;
	*hitrate=100;
	DEBUG_STATEMENT(printf("NO hits or miss in remote cache\n"));
    } else if (*miss==0) {
	*missrate=0;
	*hitrate=100;
	DEBUG_STATEMENT(printf("Hit rate 100 percent \n"));
    } else if (*hit==0) {
	*missrate=100;
	*hitrate=0;
	DEBUG_STATEMENT(printf("Miss rate 100 percent \n"));
    } else {
	*missrate=(double)100*(*miss)/(*total);
	*hitrate=(double)100*(*hit)/(*total);
	DEBUG_STATEMENT(printf("Miss rate %0.2f  Hit rate %0.2f \n", *missrate,*hitrate));
    }
}
