#include "utils.h"

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>

static void *workload(void *arg)
{
	usleep(100000);
	return (void *)0;
}

static void print_mask(const cpu_set_t *s, const unsigned lim)
{
	for (unsigned bit = 0; bit < lim; bit++)
		printf("%u", CPU_ISSET_S(bit, CPU_ALLOC_SIZE(lim), s));
}

int main(void)
{
	unsigned long cores = sysconf(_SC_NPROCESSORS_ONLN);
	unsigned long nthreads = cores;
	pthread_t threads[nthreads];

	for (unsigned long n = 0; n < nthreads; n++)
		pthread_create(&threads[n], NULL, workload, (void *)&n);

	const size_t size = CPU_ALLOC_SIZE(cores);

	for (unsigned long n = 0; n < nthreads; n++) {
		cpu_set_t *cpuset = CPU_ALLOC(cores);
		assert(cpuset);
		assert(!pthread_getaffinity_np(threads[n], size, cpuset));
		printf("thread %lu affinity ", n);
		print_mask(cpuset, cores);
		printf("\n");
		CPU_FREE(cpuset);
	}

	unsigned long rc;

	for (unsigned long n = 0; n < nthreads; n++) {
		sysassertf(pthread_join(threads[n], (void **)&rc) == 0, "pthread_join failed");
		assert(rc == 0);
	}

	sleep(1);

	return 0;
}
