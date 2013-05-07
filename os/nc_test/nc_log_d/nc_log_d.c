#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include "numachip_user.h"
#include "pcounter_test.h"

#define DEBUG_STATEMENT(x) 

struct msgstats_t {
	int64_t hit;
    int64_t miss;
    int64_t totalhit;
    int64_t totalmiss;
    int64_t cave_in;
    int64_t cave_out;
    int64_t tot_cave_in;
    int64_t tot_cave_out;
    int64_t tot_probe_in;
    int64_t tot_probe_out;
};

   
uint64_t *totalhit, *totalmiss;

static int cave=7;
static int cache=1;
static const int cachehit = 0;
static const int cachemiss = 1;
static const int cave_in=2;
static const int cave_out=3;
static const int tot_cave_in=4;
static const int tot_cave_out=5;
static const int tot_probe_in=6;
static const int tot_probe_out=7;
static const int cave_in_mask=0;
static const int cave_out_mask=4;
static const int probe_in_mask=3;
static const int probe_out_mask=7;
static const int cachehit_mask = 6;
static const int cachemiss_mask = 5;

double avghitrate (uint32_t node) {
    if ((totalhit[node] + totalmiss[node])==0) {
		return 100;
    } else if (totalmiss[node]==0) {
		return 100;
    } else if (totalhit[node]==0) {
		return 0;
    } else {
		return (double)100*totalhit[node]/(totalhit[node] + totalmiss[node]);
    }
}

void get_system_string(char *cmd, int nowrap) {
	FILE *fp;
	char path[1035];

	/* Open the command for reading. */
	fp = popen(cmd, "r");
	if (fp == NULL) {
		printf("Failed to run command\n" );
		return;
	}
	
	/* Read the output a line at a time - output it. */
	while (fgets(path, sizeof(path)-1, fp) != NULL) {
		
		if (nowrap) printf("|--------%.*s-------|\n", 29, path);
		else printf("%s", path);
	}
	
	/* close */
	pclose(fp);

}

void count_rate(struct numachip_context **cntxt, uint32_t num_nodes, struct msgstats_t* countstat) {
    nc_error_t retval = NUMACHIP_ERR_OK;
    uint64_t hit_cnt=0, miss_cnt=0;
    uint32_t node=0;
    DEBUG_STATEMENT(printf("************************************************\n"));
	printf("|--------------------------------------------|\n");
	get_system_string("date", 1);
	get_system_string("ps -ef | grep demouser", 0);

	printf("Node |d_Miss |d_Hit |d_trans |T_Hit |T_miss |T_trans \n" );		
    for(node=0; node<num_nodes; node++) {
		double missrate = 0;
		double hitrate=0;
		uint64_t total;
		count_api_read_rcache2( cntxt[node],
								0,
								1,
								&missrate,
								&hitrate,
								&total,
								&miss_cnt,
								&hit_cnt,
								&retval);
		
		countstat[node].hit=hit_cnt;
		countstat[node].miss=miss_cnt;
		totalhit[node]=hit_cnt + totalhit[node];
		totalmiss[node]=miss_cnt + totalmiss[node];
		countstat[node].totalhit=totalhit[node];
		countstat[node].totalmiss=totalmiss[node];

		printf("%d %5.2f %5.2f\t%5llu %5.2f %5.2f\t%5llu\n",   node,missrate,hitrate,(unsigned long long) total,100 - avghitrate (node),avghitrate (node), (unsigned long long) totalhit[node] + totalmiss[node]);
	}
	
}
void get_cave(struct numachip_context **cntxt, uint32_t num_nodes, struct msgstats_t* countstat) {
    uint32_t node=0;
	printf("Node | d_CAVE_IN | d_CAVE_OUT| T_CAVE_IN | T_CAVE_OUT| T_PROBE_IN | T_PROBE_OUT\n");
    for(node=0; node<num_nodes; node++) {
		
		/*
		 * Cave
		 */
		
		/*Is this accurate enough. We will loose a lot of counts doing the reset stuff....*/
		countstat[node].cave_in=counter_read(*(cntxt + node),cave_in);
		countstat[node].cave_out=counter_read(*(cntxt + node),cave_out);
		countstat[node].tot_cave_in=counter_read(*(cntxt + node),tot_cave_in);
		countstat[node].tot_cave_out=counter_read(*(cntxt + node),tot_cave_out);
		countstat[node].tot_probe_in=counter_read(*(cntxt + node),tot_probe_in);
		countstat[node].tot_probe_out=counter_read(*(cntxt + node),tot_probe_out);

		printf("%d %6lld %6lld %9lld %9lld %9lld %9lld\n",
			   node, (unsigned long long)countstat[node].cave_in,
			   (unsigned long long)countstat[node].cave_out,
			   (unsigned long long)countstat[node].tot_cave_in,
			   (unsigned long long)countstat[node].tot_cave_out,
			   (unsigned long long)countstat[node].tot_probe_in,
			   (unsigned long long)countstat[node].tot_probe_out);
			   
			   
	}
	printf("\n");

}

void close_device(struct numachip_context *cntxt) {
    (void)numachip_close_device(cntxt);
}

int init_numachip_counters(int num_devices, struct msgstats_t *countstat) {
    int node=0;
    
    totalhit = malloc (num_devices * sizeof(uint64_t));
    totalmiss = malloc (num_devices * sizeof(uint64_t));
    DEBUG_STATEMENT(printf("sizeof countstat %ld\n", sizeof(countstat)));
	DEBUG_STATEMENT(printf("num_devices %d\n", num_devices));
    for(node=0; node<num_devices; node++) {
		countstat[node].hit=0;	
		countstat[node].miss=0;
		countstat[node].totalhit=0;	
		countstat[node].totalmiss=0;
		totalhit[node]=0;
		totalmiss[node]=0;
		countstat[node].cave_in=0;
		countstat[node].cave_out=0;
		countstat[node].tot_cave_in=0;
		countstat[node].tot_cave_out=0;
		countstat[node].tot_probe_in=0;
		countstat[node].tot_probe_out=0;
		DEBUG_STATEMENT(printf("countstat[%d].hit %ld\n", node,countstat[node].hit)); 
    }
	
    return 0;
    
}


int start_numachip_counters(struct numachip_context **cntxt,
							int num_devices,
							struct msgstats_t *countstat)  {
	
    counter_clear_all(cntxt,num_devices,cave_in);
    counter_clear_all(cntxt,num_devices,cave_out);
    counter_clear_all(cntxt,num_devices,tot_cave_in);
    counter_clear_all(cntxt,num_devices,tot_cave_out);
    counter_clear_all(cntxt,num_devices,tot_probe_in);
    counter_clear_all(cntxt,num_devices,tot_probe_out);
    
    if (counter_select_all(cntxt,num_devices,cave_in,cave) == NUMACHIP_ERR_BUSY)
    {
		printf("NOTE: In order to be able to modify a counter that is already in use you first have to call -counter-clear\n");
		return -1;
    }
    
    if (counter_select_all(cntxt,num_devices,cave_out,cave) == NUMACHIP_ERR_BUSY)
    {
		printf("NOTE: In order to be able to modify a counter that is already in use you first have to call -counter-clear\n");
		return -1;
    }
    
    if (counter_mask_all(cntxt,num_devices,cave_in,cave_in_mask) == NUMACHIP_ERR_BUSY)
    {
		printf("NOTE: In order to be able to modify a counter that is already in use you first have to call -counter-clear\n");
		return -1;
    }
    
    if (counter_mask_all(cntxt,num_devices,cave_out,cave_out_mask) == NUMACHIP_ERR_BUSY)
    {
		printf("NOTE: In order to be able to modify a counter that is already in use you first have to call -counter-clear\n");
		return -1;
    }
    
    /*tot*/
    if (counter_select_all(cntxt,num_devices,tot_cave_in,cave) == NUMACHIP_ERR_BUSY)
    {
		printf("NOTE: In order to be able to modify a counter that is already in use you first have to call -counter-clear\n");
		return -1;
    }
    
    if (counter_select_all(cntxt,num_devices,tot_cave_out,cave) == NUMACHIP_ERR_BUSY)
    {
		printf("NOTE: In order to be able to modify a counter that is already in use you first have to call -counter-clear\n");
		return -1;
    }
    
    if (counter_mask_all(cntxt,num_devices,tot_cave_in,cave_in_mask) == NUMACHIP_ERR_BUSY)
    {
		printf("NOTE: In order to be able to modify a counter that is already in use you first have to call -counter-clear\n");
		return -1;
    }
    
    if (counter_mask_all(cntxt,num_devices,tot_cave_out,cave_out_mask) == NUMACHIP_ERR_BUSY)
    {
		printf("NOTE: In order to be able to modify a counter that is already in use you first have to call -counter-clear\n");
		return -1;
    }
    
    /*tot probe*/
    if (counter_select_all(cntxt,num_devices,tot_probe_in,cave) == NUMACHIP_ERR_BUSY)
    {
		printf("NOTE: In order to be able to modify a counter that is already in use you first have to call -counter-clear\n");
		return -1;
    }
    
    if (counter_select_all(cntxt,num_devices,tot_probe_out,cave) == NUMACHIP_ERR_BUSY)
    {
		printf("NOTE: In order to be able to modify a counter that is already in use you first have to call -counter-clear\n");
		return -1;
    }
    
    if (counter_mask_all(cntxt,num_devices,tot_probe_in,probe_in_mask) == NUMACHIP_ERR_BUSY)
    {
		printf("NOTE: In order to be able to modify a counter that is already in use you first have to call -counter-clear\n");
		return -1;
    }
    
    if (counter_mask_all(cntxt,num_devices,tot_probe_out,probe_out_mask) == NUMACHIP_ERR_BUSY)
    {
		printf("NOTE: In order to be able to modify a counter that is already in use you first have to call -counter-clear\n");
		return -1;
    }
    counter_restart_all(cntxt,num_devices,cave_in);
    counter_restart_all(cntxt,num_devices,cave_out);
    counter_stop_all(cntxt,num_devices,cachehit);
    counter_stop_all(cntxt,num_devices,cachemiss);
    counter_start_all(cntxt, num_devices, cachehit, cache, cachehit_mask);
    counter_start_all(cntxt, num_devices, cachemiss, cache, cachemiss_mask);
    count_rate(cntxt, num_devices,countstat);
    get_cave(cntxt, num_devices,countstat);
    DEBUG_STATEMENT(printf("Lets print the first hit %lld\n",countstat[0].hit));
    return 0;
}

int main(int argc, char* argv[]) {
    struct msgstats_t *countstat = NULL;
    struct numachip_device **nc_devices;
    struct numachip_context **cntxt;
    int i=0;
    int num_devices;
	int delay = 5 ;
	
    if( argc < 2 ) {
		delay = 5;
        printf("No delay specified. Using default %d\n", delay);		
    }
   
    nc_devices = numachip_get_device_list_oem(&num_devices);
	printf("Num devices %d+n", num_devices);

    countstat = malloc (num_devices * sizeof(struct msgstats_t));
    init_numachip_counters(num_devices, countstat);
    
    if (!nc_devices)
		return -1;
    
    cntxt = malloc(num_devices * sizeof(struct numachip_context *));
    
    for(i=0; i<num_devices; i++) {
		cntxt[i] = numachip_open_device(nc_devices[i]);
    }
    
    numachip_free_device_list(nc_devices);
    
    
    if (!cntxt[0])
		return -1;
    
    start_numachip_counters(cntxt, num_devices, countstat);
	
    for( ; ; ) {

		/*
		 * We are currently not stopping/starting (restarting) the cave count
		 */		
		counter_restart_all(cntxt,num_devices,cave_in);
		counter_restart_all(cntxt,num_devices,cave_out);	
		counter_stop_all(cntxt,num_devices,cachehit);
		counter_stop_all(cntxt,num_devices,cachemiss);
		counter_start_all(cntxt, num_devices, cachehit, cache, cachehit_mask);
		counter_start_all(cntxt, num_devices, cachemiss, cache, cachemiss_mask);
		count_rate(cntxt, num_devices,countstat);
		get_cave(cntxt, num_devices,countstat);
		sleep(5);
		
	}			
	
	for(i=0; i<num_devices; i++) {
		close_device(cntxt[i]);
	}
	free(cntxt);
	
	return 0;
}
