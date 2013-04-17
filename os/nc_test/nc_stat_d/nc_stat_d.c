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

#ifdef WITH_PAPI
#include <termios.h>
#include <sched.h>
#include <pthread.h>
#include "papi.h"
#include <sched.h>
#include <assert.h>

#define NUM_EVENTS 2
#define NUM_CORES 48
#define ERROR_RETURN(retval) { fprintf(stderr, "Error %d (%s) %s:line %d: \n", retval,PAPI_strerror(retval),__FILE__,__LINE__);  exit(retval); }
struct papi_stats_t {
    int64_t papi_event_0; //PAPI_L1_DCM
    int64_t papi_event_1; //PAPI_L1_DCA
};
struct papi_registers {
    int papi_reg_0; 
    int papi_reg_1;
};
struct papi_stats_t *papistat;
unsigned int* new_counters;
struct papi_registers papi_select_registers = {PAPI_L1_DCM,PAPI_L1_DCA};

#endif

#define DEBUG_STATEMENT(x)
#define DEBUG_STATEMENT_SOCKET(x) 
#define DEBUG_STATEMENT_PAPI(x) 


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

static void socklisten();

#ifdef WITH_PAPI

void stick_this_thread_to_core(int core_id) {
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (core_id >= num_cores) {
	fprintf(stderr,"PAPI Error. Invalid core_id %d. We only have %d cores\n", core_id, num_cores);
	ERROR_RETURN(-1);
	return;
    }
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    
    pthread_t current_thread = pthread_self();    
    pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
}

int get_event_code(char *EventName) 
{
    int EventCode;
    char EventName2[20];
    DEBUG_STATEMENT_PAPI(printf("strlen EventName %d\n", strlen(EventName)));
    snprintf(EventName2,strlen(EventName),"%s",EventName);
    PAPI_event_name_to_code( EventName, &EventCode ); 
    DEBUG_STATEMENT_PAPI(printf("Deducted code 0x%x\n", EventCode));
    return EventCode;

    
}

int get_event_name(int EventCode, char *EventName) 
{

    PAPI_event_code_to_name(EventCode, EventName);
    DEBUG_STATEMENT_PAPI(printf("Deducted name %s\n", EventName));
    return 0;
	
}

void *Thread(void *arg)
{
    int retval;
    int EventSet1 = PAPI_NULL, t;
    long long values[NUM_EVENTS];
    struct papi_registers papi_select_reg_local_copy;
    int number=NUM_EVENTS, Events[NUM_EVENTS];
    t=*(int *)arg;
    DEBUG_STATEMENT_PAPI(fprintf(stderr,"Thread %lx running PAPI (t=%d)\n",PAPI_thread_id(), t));
    stick_this_thread_to_core(t) ;

    
    if ( (retval = PAPI_create_eventset(&EventSet1)) != PAPI_OK)
	ERROR_RETURN(retval);

    //First time.
    
    papi_select_reg_local_copy.papi_reg_0=papi_select_registers.papi_reg_0;
    papi_select_reg_local_copy.papi_reg_1=papi_select_registers.papi_reg_1;

    
    if ( (retval = PAPI_add_event(EventSet1, papi_select_registers.papi_reg_0)) != PAPI_OK)
	ERROR_RETURN(retval);
    
    if ( (retval = PAPI_add_event(EventSet1, papi_select_registers.papi_reg_1)) != PAPI_OK)
	ERROR_RETURN(retval);
   
    /* Start counting */
    if ( (retval = PAPI_start(EventSet1)) != PAPI_OK)
	ERROR_RETURN(retval);

    new_counters[t]=0;

    while (1) {

	if (new_counters[t]) {
	    papistat[t].papi_event_0=0;
	    papistat[t].papi_event_1=0;

	    /* Stop counting */
	    if ( (retval = PAPI_stop(EventSet1,values)) != PAPI_OK)
		ERROR_RETURN(retval);


			
            //remove event
	    if ( (retval = PAPI_remove_event(EventSet1, papi_select_reg_local_copy.papi_reg_0)) != PAPI_OK)
		ERROR_RETURN(retval);
	    
	    if ( (retval = PAPI_remove_event(EventSet1, papi_select_reg_local_copy.papi_reg_1)) != PAPI_OK)
		ERROR_RETURN(retval);


	    papi_select_reg_local_copy.papi_reg_0=papi_select_registers.papi_reg_0;
	    papi_select_reg_local_copy.papi_reg_1=papi_select_registers.papi_reg_1;
	    
	    if ( (retval = PAPI_add_event(EventSet1, papi_select_registers.papi_reg_0)) != PAPI_OK)
		ERROR_RETURN(retval);

	    
	    if ( (retval = PAPI_add_event(EventSet1, papi_select_registers.papi_reg_1)) != PAPI_OK)
		ERROR_RETURN(retval);
	    
	    /* get the number of events in the event set */
	    if ( (retval = PAPI_list_events(EventSet1, Events, &number)) != PAPI_OK) 
	    	ERROR_RETURN(retval);
	    
		
	    /* Start counting */
	    if ( (retval = PAPI_start(EventSet1)) != PAPI_OK)
		ERROR_RETURN(retval);
	    
     	    new_counters[t]=0;
	} else {

	    /*
	     * Read the counter values
	     * and store them in the
	     * values array
	     */

	    if ( (retval=PAPI_read(EventSet1, values)) != PAPI_OK)
		ERROR_RETURN(retval);
	    
	    papistat[t].papi_event_0=values[0];
	    papistat[t].papi_event_1=values[1];
	    DEBUG_STATEMENT_PAPI(
		if (t==0) {
		    
		    fprintf(stderr,"Thread %lx running PAPI (t=%d)\n",PAPI_thread_id(), t);
		    printf("Thread %d: The total event 0x%x %lld \n", t,papi_select_registers.papi_reg_0, values[0]);	
		    printf("Thread %d: The total event 0x%x %lld \n", t, papi_select_registers.papi_reg_0,values[1]);
		}
		)
	    
	     sleep(1);
	}
    }
    /* Stop counting and store the values into the array */
    if ( (retval = PAPI_stop(EventSet1, values)) != PAPI_OK)
	ERROR_RETURN(retval);
    
    printf("Thread %d: The total PAPI_L1_DCH are %lld \n", t,values[0]);
    printf("Thread %d: The total PAPI_L1_DCA are %lld \n",t, values[1]);
    
    
    PAPI_unregister_thread(  );
    if ( retval != PAPI_OK )
	ERROR_RETURN(retval);
    
    pthread_exit(NULL);
}
#endif
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

int init_numachip_counters(int num_devices, struct msgstats_t *countstat) {
    int node=0;
    
    totalhit = malloc (num_devices * sizeof(uint64_t));
    totalmiss = malloc (num_devices * sizeof(uint64_t));
    DEBUG_STATEMENT(printf("sizeof countstat %ld\n", sizeof(countstat)));   
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


int init_papi_counters(int num_cores) {
#ifdef WITH_PAPI    
    pthread_attr_t attr;
    int retval, i=0;    
    int *argument;
    pthread_t *thread_one;
    
    retval =
	PAPI_thread_init( ( unsigned long ( * )( void ) ) ( pthread_self ) );
    if ( retval != PAPI_OK ) 
	ERROR_RETURN(retval);

    /* pthread attribution init */
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
    
    argument =  malloc (num_cores * sizeof(int));
    thread_one = malloc (num_cores * sizeof(pthread_t));
    papistat = malloc (num_cores * sizeof(struct papi_stats_t));
    new_counters=malloc (num_cores * sizeof(int));
    
    for ( i = 0; i < num_cores ; i++ ) {
	argument[i]=i;
	papistat[i].papi_event_0=0;
	papistat[i].papi_event_1=0;
	retval = pthread_create(&thread_one[i],&attr, Thread,  (void *)&(argument[i]));
	//printf("\n Thread init %d \n", i);
	if (retval)
	    ERROR_RETURN(retval);
		
    }
/*
    pthread_attr_destroy(&attr);
    for ( i = 1; i < num_cores; i++ ) {
	pthread_join(thread_one[i], NULL);
	//Thread( i, 10000000 * i );
	
    }
*/
#endif 
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
    /*count_api_stop(cntxt, num_devices);*/
    counter_start_all(cntxt, num_devices, cachehit, cache, cachehit_mask);
    counter_start_all(cntxt, num_devices, cachemiss, cache, cachemiss_mask);
    /*count_api_start(cntxt, num_devices);*/
    count_rate(cntxt, num_devices,countstat);
    get_cave(cntxt, num_devices,countstat);
    DEBUG_STATEMENT(printf("Lets print the first hit %lld\n",countstat[0].hit));
    return 0;
}

int main(int argc, char* argv[]) {
    int sockfd, newsockfd, portno;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    int msgsize = sizeof(struct msgstats_t);
    struct msgstats_t *countstat = NULL;
    struct numachip_device **nc_devices;
    struct numachip_context **cntxt;
    int i=0;
    int num_devices;

    struct cmd_packet {
	unsigned int num_devices;
	unsigned int num_cores;
	char PAPI_EVENT_0[20];
	char PAPI_EVENT_1[20];
	
    } devices; 

    if( argc < 2 ) {
        fprintf(stderr,"error, no port provided\n");
        exit(1);
    }

   
    nc_devices = numachip_get_device_list_oem(&num_devices);
    msgsize = num_devices*sizeof(struct msgstats_t);
    countstat = malloc (num_devices * sizeof(struct msgstats_t));
    init_numachip_counters(num_devices, countstat);
    devices.num_cores=0;

    snprintf(devices.PAPI_EVENT_0,20,"PAPI_L1_NAV");
    snprintf(devices.PAPI_EVENT_1,20,"PAPI_L1_NAV");
#ifdef WITH_PAPI
    int retval;
    if((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT )
	ERROR_RETURN(retval);
    snprintf(devices.PAPI_EVENT_0,20,"PAPI_L1_DCM");
    snprintf(devices.PAPI_EVENT_1,20,"PAPI_L1_DCA");
    devices.num_cores=sysconf(_SC_NPROCESSORS_ONLN);
    init_papi_counters(devices.num_cores);
#endif
    
    if (!nc_devices)
	return -1;
    
    cntxt = malloc(num_devices * sizeof(struct numachip_context *));
    
    for(i=0; i<num_devices; i++) {
	cntxt[i] = numachip_open_device(nc_devices[i]);
    }
    
    numachip_free_device_list(nc_devices);
    
    
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
    start_numachip_counters(cntxt, num_devices, countstat);
    
    socklisten(sockfd, &cli_addr, &newsockfd);
    
    char buf[4096];
    devices.num_devices=num_devices;
    
    for( ; ; ) {
	struct cmd_packet {
	    unsigned int num_devices;
	    unsigned int num_cores;
	    char PAPI_EVENT_0[20];
	    char PAPI_EVENT_1[20];
	    
	} recv_devices; 

	DEBUG_STATEMENT_SOCKET(printf("Reading sizeof(struct cmd_packet) %d bytes\n",sizeof(struct cmd_packet) ));
	n = read(newsockfd, &recv_devices, sizeof(struct cmd_packet));
	DEBUG_STATEMENT_SOCKET(printf("Reading %d bytes\n", n));
	
	if( n <= 0 ) {
	    sleep(0);
	    socklisten(sockfd, &cli_addr, &newsockfd);
	    continue;
	}
	
	if( n <  sizeof(struct cmd_packet) ) {
	    sleep(0);
	    socklisten(sockfd, &cli_addr, &newsockfd);
	    continue;
	}
#ifdef WITH_PAPI
	/*
	 * So how does the GUI know if we are supplying only
	 * NumaChip stats, or also PAPI stats?
	 * We need to handshake something smarter then an int.
	 * Let us create a handshake struct where the
	 * the server nc_pstats_d tells the gui what to show. 
	 *
	 * Then we exchange which statisics to
	 * show and handshake the other way as well. 
	 */
	if (strlen(recv_devices.PAPI_EVENT_0) >10) 
	{
	   
	    DEBUG_STATEMENT_PAPI(
		printf("PAPI_0 %s (recv)\n", recv_devices.PAPI_EVENT_0);
		printf("PAPI_1 %s (recv)\n", recv_devices.PAPI_EVENT_1);
		printf("PAPI_0 %s\n", devices.PAPI_EVENT_0);
		printf("PAPI_1 %s\n", devices.PAPI_EVENT_1));
	    if (
		( strncmp(recv_devices.PAPI_EVENT_0,devices.PAPI_EVENT_0,20)  
		 || strncmp(recv_devices.PAPI_EVENT_1,devices.PAPI_EVENT_1,20)) != 0) {
		strncpy(devices.PAPI_EVENT_0, recv_devices.PAPI_EVENT_0,20);
		strncpy(devices.PAPI_EVENT_1, recv_devices.PAPI_EVENT_1,20);


		/*
		 * Option 1: We need to tell all the threads that we have a new paradigm.
		 * should we completely tear them down and set them up again?
		 *
		 * Option 2: We access their global variables.
		 * We then need to watch out for race conditions. 
		 */
		 for ( i = 0; i < devices.num_cores ; i++ ) {
		     new_counters[i]=1;

		 }
		 DEBUG_STATEMENT_PAPI(printf(" Get event code is 0x%x shouldbe 0x%x \n", PAPI_L1_DCA, get_event_code(devices.PAPI_EVENT_1)));
		 papi_select_registers.papi_reg_0=get_event_code(devices.PAPI_EVENT_0);
		 papi_select_registers.papi_reg_1=get_event_code(devices.PAPI_EVENT_1);

		 DEBUG_STATEMENT_PAPI(
		     printf("The  papi_select_registers.papi_reg_0 0x%x \n", papi_select_registers.papi_reg_0);
		     printf("The  papi_select_registers.papi_reg_1 0x%x \n", papi_select_registers.papi_reg_1);
		     printf("DO THE MAGIC strlen %zu\n", strlen(devices.PAPI_EVENT_0));
		     printf("PAPI_0 %s\n", devices.PAPI_EVENT_0);
		     printf("PAPI_1 %s\n", devices.PAPI_EVENT_1));
		     
	    }
	    
	} else {
	    DEBUG_STATEMENT_PAPI(
		printf("PAPI_0 %s (recv) strncmp %d strlen %d\n",
		   recv_devices.PAPI_EVENT_0,
		   strncmp(recv_devices.PAPI_EVENT_0,devices.PAPI_EVENT_0,20),
		   strlen(recv_devices.PAPI_EVENT_0)
		);
		
		printf("PAPI_0 %s\n", devices.PAPI_EVENT_0);
		printf("PAPI_1 %s\n", devices.PAPI_EVENT_1));
		

	}
#endif	
	memcpy(buf, &devices,  sizeof(struct cmd_packet));
	
	DEBUG_STATEMENT_SOCKET(printf("Size of cmd_packet %lu \n",  sizeof(struct cmd_packet)));
	
	n = write(newsockfd, buf,  sizeof(struct cmd_packet));
	
	if( n <  sizeof(struct cmd_packet) ) {
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
	counter_start_all(cntxt, num_devices, cachehit, cache, cachehit_mask);
	counter_start_all(cntxt, num_devices, cachemiss, cache, cachemiss_mask);
	count_rate(cntxt, num_devices,countstat);
	get_cave(cntxt, num_devices,countstat);
	
	memcpy(buf, countstat, devices.num_devices*sizeof(struct msgstats_t));
	DEBUG_STATEMENT_SOCKET(printf("Size of msgstat %lu \n", devices.num_devices*sizeof(struct msgstats_t)));
	n = write(newsockfd, buf, msgsize);
	if( n < msgsize ) {
	    fprintf(stderr, "error writing to socket\n");
	    sleep(0);
	    socklisten(sockfd, &cli_addr, &newsockfd);	    
	    continue;
	}
	DEBUG_STATEMENT_PAPI(printf("devices.num_cores %d\n", devices.num_cores));
#ifdef WITH_PAPI
	if (devices.num_cores>0) {
	    /* read the counter values and store them in the values array */
	    DEBUG_STATEMENT_PAPI(
	    for ( i = 0; i < devices.num_cores ; i=i+6 ) {
		printf("Core #%d: The total PAPI_L1_DCM  %lld \n",i, papistat[i].papi_event_0);
		printf("Core #%d: The total PAPI_L1_DCA  %lld \n",i, papistat[i].papi_event_1);
	    }
		)
	    
		DEBUG_STATEMENT_SOCKET(printf("cores %d size papi %d\n",devices.num_cores,devices.num_cores*sizeof(struct papi_stats_t) ));
	    
	    memcpy(buf, papistat, devices.num_cores*sizeof(struct papi_stats_t));
	    n = write(newsockfd, buf,devices.num_cores*sizeof(struct papi_stats_t));
	    DEBUG_STATEMENT_SOCKET(printf("size written %d\n",n));
	    if( n < devices.num_cores*sizeof(struct papi_stats_t)) {
		fprintf(stderr, "error writing to socket\n");
		sleep(0);
		socklisten(sockfd, &cli_addr, &newsockfd);
		continue;
	    }
	}
#endif
	
	
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


