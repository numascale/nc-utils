#include "utils.h"

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>

static void *workload(void *arg)
{
	sleep(1);
	return (void *)0;
}

int main(void)
{
	unsigned long cores = sysconf(_SC_NPROCESSORS_ONLN);
	pthread_t threads[cores];

	for (unsigned long n = 0; n < (cores - 1); n++)
		pthread_create(&threads[n], NULL, workload, (void *)&n);

	unsigned long rc;

	for (unsigned long n = 0; n < (cores - 1); n++) {
		sysassertf(pthread_join(threads[n], (void **)&rc) == 0, "pthread_join failed");
		assert(rc == 0);
	}

	return 0;
}
