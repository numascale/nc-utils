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
//    int64_t total[4];
};
uint64_t *totalhit, *totalmiss;

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
    printf("************************************************\n");
    
    for(node=0; node<num_nodes; node++) {
	double missrate = 0;
	double hitrate=0;
	uint64_t total;
	count_api_read_rcache2( cntxt[node], 0, 1, &missrate, &hitrate, &total, &miss_cnt, &hit_cnt, &retval);
	countstat[node].hit=hit_cnt;
	countstat[node].miss=miss_cnt;
	totalhit[node]=hit_cnt + totalhit[node];
	totalmiss[node]=miss_cnt + totalmiss[node];
	countstat[node].totalhit=totalhit[node];
	countstat[node].totalmiss=totalmiss[node];
	printf("Node %d: Miss rate %0.2f  Hit rate %0.2f transactions %llu\n",  node,missrate,hitrate, (unsigned long long) total);
	printf("Node %d: Avrage Miss rate %0.2f  Hit rate %0.2f transactions %llu\n",  node,100 - avghitrate (node),avghitrate (node), (unsigned long long) totalhit[node] + totalmiss[node]);
    }
    printf("************************************************\n");
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

    count_api_stop(cntxt, num_devices);
    count_api_start(cntxt, num_devices);
    count_rate(cntxt, num_devices,countstat);
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

	printf("Size of %lu \n", sizeof(int));
	
	n = write(newsockfd, buf, sizeof(int));

	if( n < sizeof(int) ) {
	    fprintf(stderr, "error writing to socket\n");
	    sleep(0);
	    socklisten(sockfd, &cli_addr, &newsockfd);
	    continue;
	}

	count_api_stop(cntxt, num_devices);
	count_api_start(cntxt, num_devices);
	count_rate(cntxt, num_devices,countstat);
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


