#include "utils.h"

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>

#define THREADS 6

void *workload(void *arg)
{
	unsigned long tid = (unsigned long)arg;
	sleep(3);
	return (void *)0;
}

int main(void)
{
	pthread_t threads[THREADS];

	for (unsigned long n = 0; n < THREADS; n++)
		pthread_create(&threads[n], NULL, workload, (void *)&n);

	unsigned long rc;

	for (unsigned long n = 0; n < THREADS; n++) {
		sysassertf(pthread_join(threads[n], (void **)&rc) == 0, "pthread_join failed");
		assert(rc == 0);
	}

	return 0;
}
