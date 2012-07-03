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
#include "pcounter_test.h"
#include "../../../interface/numachip-defines.h"

#define DEBUG_STATEMENT(x) 
void print_involved() {
    printf("------------------------------------------\n");
    printf("INVOLVED REGISTRIES:----------------------\n");
}
void print_involved_api() {
    printf("------------------------------------------\n");
    printf("INVOLVED API FUNCTION:--------------------\n");
}
void print_operation() {
    printf("------------------------------------------\n");
    printf("OPERERATION:------------------------------\n");    
}
void print_example() {
    printf("------------------------------------------\n");
    printf("EXAMPLE:----------------------------------\n");
}
void print_comp_n_mask() {
    printf("G3xFA0 Compare and Mask of counter 0\n");
    printf("G3xFA4 Compare and Mask of counter 1\n");
    printf("G3xFA8 Compare and Mask of counter 2\n");
    printf("G3xFAC Compare and Mask of counter 3\n");
    printf("G3xFB0 Compare and Mask of counter 4\n");
    printf("G3xFB4 Compare and Mask of counter 5\n");
    printf("G3xFB8 Compare and Mask of counter 6\n");
    printf("G3xFBC Compare and Mask of counter 7\n");
}
void print_perf_cnt() {
    printf("G3xFC0 Performance counter 0 40-Bit (Upper Bits) \n");
    printf("G3xFC4 Performance counter 0 40-Bit (Lower Bits)\n");
    printf("G3xFC8 Performance counter 1 40-Bit (Upper Bits) \n");
    printf("G3xFCC Performance counter 1 40-Bit (Lower Bits)\n");
    printf("G3xFD0 Performance counter 2 40-Bit (Upper Bits) \n");
    printf("G3xFD4 Performance counter 2 40-Bit (Lower Bits)\n");
    printf("G3xFD8 Performance counter 3 40-Bit (Upper Bits) \n");
    printf("G3xFDC Performance counter 3 40-Bit (Lower Bits)\n");
    printf("G3xFE0 Performance counter 4 40-Bit (Upper Bits) \n");
    printf("G3xFE4 Performance counter 4 40-Bit (Lower Bits)\n");
    printf("G3xFE8 Performance counter 5 40-Bit (Upper Bits) \n");
    printf("G3xFEC Performance counter 5 40-Bit (Lower Bits)\n");
    printf("G3xFF0 Performance counter 6 40-Bit (Upper Bits) \n");
    printf("G3xFF4 Performance counter 6 40-Bit (Lower Bits)\n");
    printf("G3xFF8 Performance counter 7 40-Bit (Upper Bits) \n");
    printf("G3xFFC Performance counter 7 40-Bit (Lower Bits)\n");
}
void counter_select_help() {

    print_involved();
    printf("G3xF78 Select Counter\n");
    printf("------------------------------------------\n");
    printf("Reset: 0000 0000h\n");
    printf("------------------------------------------\n");
    printf("Bits \tDescription\n");
    printf("------------------------------------------\n");
    printf("31 \tRO Reserved\n");
    printf("30:28 \tRW Select Counter 7:\n");
    printf("27 \tRO Reserved\n");
    printf("26:24 \tRW Select Counter 6:\n");
    printf("23 \tRO Reserved\n");
    printf("22:20 \tRW Select Counter 5:\n");
    printf("19 \tRO Reserved\n");
    printf("18:16 \tRW Select Counter 4:\n");
    printf("15 \tRO Reserved\n");
    printf("14:12 \tRW Select Counter 3:\n");
    printf("11 \tRO Reserved\n");
    printf("10:8 \tRW Select Counter 2:\n");
    printf("7 \tRO Reserved\n");
    printf("6:4 \tRW Select Counter 1:\n");
    printf("3 \tRO Reserved\n");
    printf("2:0 \tRW Select Counter 0:\n");
    printf("------------------------------------------\n");

    printf("ALLOWED VALUES----------------------------\n");
    printf("------------------------------------------\n");
    printf("Select Counter eventreg:\n");
    printf("------------------------------------------\n");
    printf(" 7 - cHT Cave [7:0]\n");
    printf(" 6 - MCTag [7:0]\n");
    printf(" 5 - FLAG [7:0]\n");
    printf(" 4 - CDATA [7:0]\n");
    printf(" 3 - LOC (HPrb)\n");
    printf(" 2 - LOC (SPrb) [7:0] - \n");
    printf(" 1 - REM (Hreq) [7:0] - Remote (L4) cache\n");
    printf(" 0 - REM (SPrb) [7:0] - Probes from SCC\n\n");
    
    print_involved_api();
    printf("void numachip_select_pcounter(struct numachip_context *cntxt,\n");
    printf("			      uint32_t counterno,\n");
    printf("			      uint32_t eventreg,\n");
    printf(" 		              nc_error_t *error);\n");

    print_example();
    printf("Select counter 0 for mux: 1 - REM (Hreq):\n");
    print_operation();
    printf("numachip_select_pcounter(cntxt[node],0,0x1, &retval);\n");
    printf("------------------------------------------\n");
}

void counter_mask_help() {
    
    print_involved();
    printf("G3xF9C Timer for ECC / Counter 7 (if you select \ncounter 7, then we will set this register for you.) \n");
    print_comp_n_mask();
    printf("------------------------------------------\n");
    printf("Reset: 0000 0000h\n");
    printf("------------------------------------------\n");
    printf("Bits \tDescription\n");
    printf("------------------------------------------\n");
    printf("31:16 \tRO Reserved\n");
    printf("15:8 \tRW Compare of performance counter \n");
    printf("7:0 \tRW Mask of performance counter \n");
    printf("------------------------------------------\n");
    printf("------------------------------------------\n");  
    printf("ALLOWED VALUES----------------------------\n");
    printf("------------------------------------------\n");
    printf(" numachip_mask_counter - Apply counter mask\n");
    printf("\n");
    printf(" Select = 0, REM/SPrb :\n");
    printf("\n");
    printf("       7 - SCC-Request Invalidate          (shared   => invalid)\n");
    printf("       6 - SCC-Request Read                (modified => shared)\n");
    printf("       5 - SCC-Request Read and Invalidate (modified => invalid)\n");
    printf("       4 - SCC-Request Aliased Invalidate  (shared   => invalid)\n");
    printf("       3 - SCC-Request Aliased Read and Invalidate (modified => invalid)\n");
    printf("       2 - SCC-Request with SPrb conflict\n");
    printf("       1 - SCC-Request with HReq conflict\n");
    printf("       0 - Cache data access\n");
    printf("\n");
    printf(" Select = 1, REM/HReq :\n");
    printf("\n");
    printf("       7 - HT-Request start processing\n");
    printf("       6 - HT-Request with ctag miss\n");
    printf("       5 - HT-Request with ctag hit\n");
    printf("       4 - HT-Request with HReq conflict\n");
    printf("       3 - HT-Request with SPrb conflict\n");
    printf("       2 - HT-command unknown\n");
    printf("       1 - Broadcast messages\n");
    printf("       0 - Direct interrupt (no broadcast)\n");
    printf("\n");
    printf("\n");
    printf(" Select = 2, LOC/SReq :\n");
    printf("\n");
    printf("       7 - Interrupt request\n");
    printf("       6 - Config Space request\n");
    printf("       5 - VictimBlk request\n");
    printf("       4 - VictimBlk conflict\n");
    printf("       3 - SCC conflict\n");
    printf("       2 - SCC discard\n");
    printf("       1 - SCC request (all)\n");
    printf("       0 - Error in interrupt\n");
    printf("\n");
    printf("\n");
    printf(" Select = 3, LOC/HPrb :\n");
    printf("\n");
    printf("       7 - HT lock pending\n");
    printf("       6 - VictimBlk conflict\n");
    printf("       5 - HT-probe with next-state=invalidate\n");
    printf("       4 - SCC retries\n");
    printf("       3 - SCC requests\n");
    printf("       2 - HT-probe on own request\n");
    printf("       1 - HT-probe with next-state=shared\n");
    printf("       0 - HT-probe to non-shared memory\n");
    printf("\n");
    printf("\n");
    printf(" Select = 4, CData :\n");
    printf("\n");
    printf("       7 - CData write request from REM/HReq\n");
    printf("       6 - CData write request from REM/HReq accepted\n");
    printf("       5 - CData read request from REM/HReq\n");
    printf("       4 - CData read request from REM/HReq accepted\n");
    printf("       3 - CData write request from REM/SPrb\n");
    printf("       2 - CData write request from REM/SPrb accepted\n");
    printf("       1 - CData read request from REM/SPrb\n");
    printf("       0 - CData read request from REM/SPrb accepted\n");
    printf("\n");
    printf("\n");
    printf(" Select = 5, FTag :\n");
    printf("\n");
    printf("       7 - Tag update valid from MCTag\n");
    printf("       6 - Tag read valid from MCTag\n");
    printf("       5 - MCTag request\n");
    printf("       4 - Tag response valid from MCTag to LOC/HPrb\n");
    printf("       3 - Unused\n");
    printf("       2 - Tag response valid from prefetch to LOC/HPrb\n");
    printf("       1 - Unused\n");
    printf("       0 - Tag request from LOC/HPrb\n");
    printf("\n");
    printf("\n");
    printf(" Select = 6, MCTag :\n");
    printf("\n");
    printf("       7 - Unused\n");
    printf("       6 - Prefetch buffer address hit\n");
    printf("       5 - Prefetch buffer full hit\n");
    printf("       4 - Tag request from REM/HReq\n");
    printf("       3 - CTag cache hit\n");
    printf("       2 - CTag cache miss\n");
    printf("       1 - DRAM read request\n");
    printf("       0 - DRAM read request delayed\n");
    printf("\n");
    printf("\n");
    printf(" Select = 7, cHT-Cave :\n");
    printf("\n");
    printf("       7 - Outgoing HT-Probe\n");
    printf("       6 - Outgoing HT-Response\n");
    printf("       5 - Outgoing posted HT-Request\n");
    printf("       4 - Outgoing non-posted HT-Request\n");
    printf("       3 - Incoming HT-Probe\n");
    printf("       2 - Incoming HT-Response\n");
    printf("       1 - Incoming posted HT-Request\n");
    printf("       0 - Incoming non-posted HT-Request\n");
    printf("\n");
    printf("\n");
    printf("\n");
    printf("\n");
    printf(" Documentation is in hdl:\n");
    printf(" assign prfMask0  = CSR_H2S_G3xFA0[`prfMask0Range];\n");
    printf(" assign prfCom0   = CSR_H2S_G3xFA0[`prfCom0Range];\n");
    printf(" assign pc0Event = |(prfMask0 & (prfCom0 ~^ Sel0Event));\n");
    print_involved_api();    
    printf("Select mask by writing api: \n");
    printf("void numachip_mask_pcounter(struct numachip_context *cntxt,\n");
    printf("			      uint32_t counterno,\n");
    printf("			      uint32_t mask,\n");
    printf(" 		              nc_error_t *error);\n");
    print_example();
    printf("Select counter 0 for mux: 1 - REM (Hreq):\n");
    printf("       6 - HT-Request with ctag miss\n");
    print_operation();
    printf("numachip_mask_pcounter(cntxt[node],0,6, &retval);\n");
    printf("------------------------------------------\n");
}

void counter_read_help() {
    print_involved();
    print_perf_cnt();
    print_involved_api();
    printf("uint64_t numachip_get_pcounter(struct numachip_context *cntxt,\n");
    printf("					 uint32_t counterno,\n");
    printf("					 nc_error_t *error);\n");
    print_example();
    printf("Read the counters Performance counter registry values,e.g of counter 0:\n");
    print_operation();
    printf("uint64_t val=numachip_read_pcounter(cntxt[node],0 &retval);\n");
    printf("------------------------------------------\n");
}
void counter_clear_help() {
    print_involved();
    printf("G3xF78 Select Counter\n");
    print_comp_n_mask();
    print_perf_cnt();
    print_involved_api();
    printf("void numachip_clear_pcounter(struct numachip_context *cntxt,\n");
    printf("			      uint32_t counterno,\n");
    printf(" 		              nc_error_t *error);\n");
    print_example();
    printf("Clear counter by deleting Performance counter registry values,\n");
    printf("deselecting counter and clearing mask by writing api: \n");
    printf("Clear counter 0:\n");
    print_operation();
    printf("numachip_clear_pcounter(cntxt[node],0, &retval);\n");
    printf("------------------------------------------\n");
}

void counter_stop_help() {

    print_involved();
    printf("G3xF78 Select Counter\n");
    print_comp_n_mask();
    printf("G3xF9C Timer for ECC / Counter 7 (if you select \ncounter 7, then we will set this register for you.) \n");
    

    print_involved_api();
    printf("Stop counter by deselecting counter\n");
    printf("and clearing mask by writing api: \n");
    printf("void numachip_stop_pcounter(struct numachip_context *cntxt,\n");
    printf("			      uint32_t counterno,\n");
    printf(" 		              nc_error_t *error);\n");
    print_example();
    printf("Stop counter 0 by clearing the select and mask register\n");
    printf("and corresponding counter register without clearing the number of counts:\n");
    print_operation();
    printf("numachip_stop_pcounter(cntxt[node],0, &retval);\n");
    printf("------------------------------------------\n");

}
void usage () {
    printf("./nc_perf <help>/<json_file>\n");
    printf("[-counter-select <node_index>|<'all'> <counterno> <mux value>]\n");
    printf("[-counter-mask <node_index>|<'all'> <counterno> <mask value> ]\n");
    printf("[-counter-stop <node_index>|<'all'> <counterno>]\n");
    printf("[-counter-read <node_index>|<'all'> <counterno> ]\n");
    printf("[-counter-clear <node_index>|<'all'> <counterno> ] \n");
}


void close_device(struct numachip_context *cntxt) {
    (void)numachip_close_device(cntxt);
}

void count_rate(struct numachip_context **cntxt, uint32_t num_nodes) {
    nc_error_t retval = NUMACHIP_ERR_OK;
    uint32_t node=0;
    printf("************************************************\n");
    
    for(node=0; node<num_nodes; node++) {
	uint64_t total=0;
	double missrate = 0;
	double hitrate=0;
	count_api_read_rcache( cntxt[node], 1, 0, &missrate, &hitrate, &total, &retval);
	//count_api_read_rate( cntxt[node], &missrate, &hitrate, &total, &retval);

	printf("Node %d: Miss rate %0.2f  Hit rate %0.2f \n",  node,missrate,hitrate);
    }
    printf("************************************************\n");
	
}

int main(int argc, char **argv)
{
    struct numachip_device **devices;
    //struct numachip_context *cntxt;
    struct numachip_context **cntxt;
    int counter=0, i=0;
    int num_devices; 
    const char *filename = "fabric-loop-05.json";
    
    if (argc<3) {
        usage();
        return(-1);
    }

    if (!strcmp(argv[1], "help")){

	/* Get the parameters */
	for (counter=2; (int)counter<argc; counter++) {
	    printf("Argument %d: %s\n", counter, argv[counter]);
	    
	    if (!strcmp("-counter-select",argv[counter])) {	    
		counter_select_help();
		continue;
	    }
	    if (!strcmp("-counter-mask",argv[counter])) {	    
		counter_mask_help();
		continue;
	    }
	    if (!strcmp("-counter-clear",argv[counter])) {	    
		counter_clear_help();
		continue;
	    }
	    if (!strcmp("-counter-stop",argv[counter])) {	    
		counter_stop_help();
		continue;
	    }

           if (!strcmp("-counter-read",argv[counter])) {	    
		counter_read_help();
		continue;
	    }
	    

	}
	counter=0;
	return(0);
	
    }

    if (argc<3) {
	usage();
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
    for (counter=2; (int)counter<argc; counter++) {
	uint32_t nodeix = 0,counterno = 0, val= 0, val2 = 0; 
	DEBUG_STATEMENT(printf("Get the parameters counter %d argc %d argv[counter] %s\n",counter, argc, argv[counter]));

	if (argc>counter+2) {	    
	    counterno = strtol(argv[counter+2],(char **) NULL,10);
	    DEBUG_STATEMENT(printf ("Counterno %d\n", counterno));
	} else {
	    break;
	}
	DEBUG_STATEMENT(printf ("Counter print argv[counter+1]  %s\n", argv[counter+1]));    
	if (argc>counter+3) {
	    val = strtol(argv[counter+3],(char **) NULL,10);
	}
	DEBUG_STATEMENT(printf ("Counter print argv[counter+1]  %s\n", argv[counter+1]));
	if (argc>counter+4) {
	    val2 = strtol(argv[counter+4],(char **) NULL,10);
	}
	if (!strcmp("all",argv[counter+1])) {
	    
	    DEBUG_STATEMENT(printf ("Counter print all1 %d\n", num_devices));
	    if (!strcmp("-counter-select",argv[counter])) {
		counter_select_all(cntxt,num_devices,counterno,val);
		continue;
	    }
	    if (!strcmp("-counter-mask",argv[counter])) {	    
		counter_mask_all(cntxt,num_devices,counterno,val);
		continue;
	    }

	    if (!strcmp("-counter-clear",argv[counter])) {
		counter_clear_all(cntxt,num_devices,counterno);
		continue;
	    }
	
	    if (!strcmp("-counter-read",argv[counter])) {
		DEBUG_STATEMENT(printf ("Counter print all %d\n", num_devices));
		counter_print_all(cntxt,num_devices,counterno);
		continue;
	    }
	    if (!strcmp("-counter-stop",argv[counter])) {	    
		counter_stop_all(cntxt,num_devices,counterno);
		continue;
	    }

	    if (!strcmp("-counter-start",argv[counter])) {
		printf("Calling counter_start_all(cntxt,%d,%d,%d,%d);\n",num_devices, counterno, val, val2);
		counter_start_all(cntxt,num_devices,counterno,val,val2);
	    	continue;
	    }
	    
	} else {

	    if (argc>counter+1) {	    
		nodeix = strtol(argv[counter+1],(char **) NULL,10);
		DEBUG_STATEMENT(printf ("node %d\n", nodeix));
	    } else {
		break;
	    }
	    

	    if (!strcmp("-counter-select",argv[counter])) {
		DEBUG_STATEMENT(printf("Node %d counterno %d value %d\n",nodeix,counterno,val));
		counter_select(cntxt[nodeix],counterno,val);
		continue;
	    }
	    
	    if (!strcmp("-counter-mask",argv[counter])) {
		DEBUG_STATEMENT(printf("Masking counter node %d counterno %d mask 0x%x\n",
				       nodeix, counterno, val));
		counter_mask(cntxt[nodeix], counterno,val);
		continue;
	    }
	    
	    if (!strcmp("-counter-clear",argv[counter])) {
		counter_clear(cntxt[nodeix],counterno);
		continue;
	    }
	
	    if (!strcmp("-counter-read",argv[counter])) {	    
		printf("Reading counter node %d counterno %d = %lld \n",
		       nodeix, counterno, (unsigned long long)
		       counter_read(cntxt[nodeix],counterno));
		continue;
	    }
	    if (!strcmp("-counter-stop",argv[counter])) {	    
		counter_stop(cntxt[nodeix],counterno);
		continue;
	    }

	    if (!strcmp("-counter-start",argv[counter])) {	    
		counter_start(cntxt[nodeix],counterno,val,val2);
	    	continue;
	    }
		

	}
		    
		
        /*
	if (!strcmp("-count-stop",argv[counter])) {	    
	    count_api_stop(cntxt, num_devices);
	    continue;
	}
	
	if (!strcmp("-count-rate",argv[counter])) {
	    count_api_stop(cntxt, num_devices);
	    count_api_start(cntxt, num_devices);
	    count_rate(cntxt, num_devices);
	    
	    continue;
	}
	*/

    }

    
    for(i=0; i<num_devices; i++) {
	close_device(cntxt[i]);
    }
    free(cntxt);
    
    return  0;
}

