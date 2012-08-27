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
  
    
//    int64_t total[4];
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


static void socklisten();

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
void count_rate(struct numachip_context **cntxt, uint32_t num_nodes, struct msgstats_t* countstat) {
    nc_error_t retval = NUMACHIP_ERR_OK;
    uint64_t hit_cnt=0, miss_cnt=0;
    uint32_t node=0;
    DEBUG_STATEMENT(printf("************************************************\n"));



//    for(node=num_nodes; node>0; node--) {
    
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

		
	DEBUG_STATEMENT(printf("Node %d: Miss rate %0.2f  Hit rate %0.2f transactions %llu\n",  node,missrate,hitrate, (unsigned long long) total);
			printf("Node %d: Avrage Miss rate %0.2f  Hit rate %0.2f transactions %llu\n",  node,100 - avghitrate (node),avghitrate (node), (unsigned long long) totalhit[node] + totalmiss[node]));
    }


}
void get_cave(struct numachip_context **cntxt, uint32_t num_nodes, struct msgstats_t* countstat) {
    uint32_t node=0;




//    for(node=num_nodes; node>0; node--) {
    
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
	//countstat[node].tot_cave_in=countstat[node].cave_in;
	//countstat[node].tot_cave_out=countstat[node].cave_out;

	
	DEBUG_STATEMENT(printf("Reading counter node %d counterno %d (cave_in) = %lld tot_cave_in %lld \n",
	       node,cave_in, (unsigned long long)countstat[node].cave_in,(unsigned long long)countstat[node].tot_cave_in );
	printf("Reading counter node %d counterno %d (cave_out) = %lld  tot_cave_out %lld \n",
	       node,cave_out, (unsigned long long)countstat[node].cave_out,(unsigned long long)countstat[node].tot_cave_out);
	printf("Reading counter node %d counterno %d tot_probe_in %lld \n",
	       node,cave_in, (unsigned long long)countstat[node].tot_probe_in );
	printf("Reading counter node %d counterno %d tot_probe_out %lld \n",
	       node,cave_out, (unsigned long long)countstat[node].tot_probe_out));
    

    }

}

void close_device(struct numachip_context *cntxt) {
    (void)numachip_close_device(cntxt);
}

int main(int argc, char* argv[]) {

    int sockfd, newsockfd, portno;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    int msgsize = sizeof(struct msgstats_t);
    struct msgstats_t *countstat;
    
    struct numachip_device **devices;
    struct numachip_context **cntxt;
    int i=0, node=0, num_nodes=4;
    int num_devices;
    //Real files are found in /var/lib/tftpboot/nc-config
//    const char *filename = "/net/numastore/storage/home/av/handy/nc-config/fabric-loop-05.json";
    //const char *filename = "fabric-narya-2d.json";
    const char *filename = "fabric-loop-05.json";

    if( argc < 2 ) {
        fprintf(stderr,"error, no port provided\n");
        exit(1);
    }

   
    devices = numachip_get_device_list(&num_devices, filename);
    DEBUG_STATEMENT(printf("Found %d NumaChip devices\n", num_devices));
    msgsize = num_devices*sizeof(struct msgstats_t);
    
    
    totalhit = malloc (num_devices * sizeof(uint64_t));
    totalmiss = malloc (num_devices * sizeof(uint64_t));
    countstat = malloc (num_devices * sizeof(struct msgstats_t));
    
    for(node=0; node<num_nodes; node++) {
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
    }

    

		       
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
   
    portno = atoi(argv[1]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if( sockfd < 0 ) {
   	    fprintf(stderr, "error opening socket\n");
		exit(-1);
	}
    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if( bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0 ) {
        fprintf(stderr, "error on binding %s\n", strerror(errno));
	exit(-1);
    }


    /*
     * Cave should use counter 2 and 3 
     */

    /*
     * Cave should use counter 2 and 3 
     */
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
    /*count_api_stop(cntxt, num_devices);*/
    counter_start_all(cntxt, num_devices, cachehit, cache, cachehit_mask);
    counter_start_all(cntxt, num_devices, cachemiss, cache, cachemiss_mask);
    /*count_api_start(cntxt, num_devices);*/
    count_rate(cntxt, num_devices,countstat);
    get_cave(cntxt, num_devices,countstat);
    DEBUG_STATEMENT(printf("Lets print the first hit %lld\n",countstat[0].hit));
    socklisten(sockfd, &cli_addr, &newsockfd);
    
    char buf[4096];


    for( ; ; ) {
	
	n = read(newsockfd, buf, 4);
	
	if( n <= 0 ) {
	    sleep(0);
	    socklisten(sockfd, &cli_addr, &newsockfd);
	    continue;
	}
	
	if( n < 4 ) {
	    sleep(0);
	    socklisten(sockfd, &cli_addr, &newsockfd);
	    continue;
	}
	    
	memcpy(buf, &num_devices, sizeof(int));

	DEBUG_STATEMENT(printf("Size of %lu \n", sizeof(int)));
	
	n = write(newsockfd, buf, sizeof(int));

	if( n < sizeof(int) ) {
	    fprintf(stderr, "error writing to socket\n");
	    sleep(0);
	    socklisten(sockfd, &cli_addr, &newsockfd);
	    continue;
	}

	/*
	 * We are currently not stopping/starting (restarting) the cave count
	 */
	counter_restart_all(cntxt,num_devices,cave_in);
	counter_restart_all(cntxt,num_devices,cave_out);

	counter_stop_all(cntxt,num_devices,cachehit);
	counter_stop_all(cntxt,num_devices,cachemiss);
	/*count_api_stop(cntxt, num_devices);*/
	counter_start_all(cntxt, num_devices, cachehit, cache, cachehit_mask);
	counter_start_all(cntxt, num_devices, cachemiss, cache, cachemiss_mask);
	/*count_api_start(cntxt, num_devices);*/
	count_rate(cntxt, num_devices,countstat);
	get_cave(cntxt, num_devices,countstat);
	memcpy(buf, countstat, num_devices*sizeof(struct msgstats_t));

	DEBUG_STATEMENT(printf("Size of %lu \n", num_devices*sizeof(struct msgstats_t)));
	
	n = write(newsockfd, buf, msgsize);
	if( n < msgsize ) {
	    fprintf(stderr, "error writing to socket\n");
	    sleep(0);
	    socklisten(sockfd, &cli_addr, &newsockfd);
	    continue;
	}
    }
    
    for(i=0; i<num_devices; i++) {
	close_device(cntxt[i]);
    }
    free(cntxt);
    
    close(newsockfd);
    close(sockfd);
    
    return 0;
}


static void socklisten(int sockfd, struct sockaddr_in* cli_addr, int* newsockfd) {
    socklen_t clilen;
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    *newsockfd = accept(sockfd, (struct sockaddr*)cli_addr, &clilen);
    if( newsockfd < 0 ) {
       	fprintf(stderr, "error on accept\n");
		exit(-1);
	}
}


