#include "utils.h"

#include <stdio.h>
#include <stdint.h>
#include <sched.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define SHM_NAME "place"
#define MAX_CORES 16384
#define MAX_NUMA 1024

struct shared_s {
	pthread_mutex_t lock;
	unsigned attached;
	union {
		cpu_set_t allocated;
		char u[CPU_ALLOC_SIZE(MAX_CORES)];
	};
	unsigned char dist[MAX_NUMA][MAX_NUMA];
};

static struct shared_s *shared = NULL;
static cpu_set_t *allowed = NULL;
static unsigned cores = 0;
static size_t masksize = 0;

static int (*xpthread_create)(pthread_t *thread, const pthread_attr_t *attr,
  void *(*start_routine) (void *), void *arg);

static void populate(void)
{
	printf("populate\n");
}

// returns core number
static unsigned allocate_core(void)
{
	unsigned core = ~0U;
	pthread_mutex_lock(&shared->lock);

	for (unsigned n = 0; n < cores; n++) {
		if (!CPU_ISSET_S(n, masksize, &shared->allocated)) {
			CPU_SET_S(n, masksize, &shared->allocated);
			core = n;
			break;
		}
	}

	pthread_mutex_unlock(&shared->lock);
	if (core == ~0U)
		error("No available cores");
	return core;
}

static void allocate(void)
{
	const unsigned core = allocate_core();
	printf("core=%u\n", core);
}

static void release_core(const unsigned core)
{
	pthread_mutex_lock(&shared->lock);
	CPU_CLR_S(core, masksize, &shared->allocated);
	pthread_mutex_unlock(&shared->lock);
}

static void release(void)
{
}

//__attribute__((constructor)) void init(void)
void _init(void)
{
	printf("init\n");

	xpthread_create = dlsym(RTLD_NEXT, "pthread_create");
	assertf(xpthread_create, "dlsym failed");

	cores = 1728; //sysconf(_SC_NPROCESSORS_CONF);
	allowed = CPU_ALLOC(cores);
	sysassertf(allowed, "CPU_ALLOC failed");
	masksize = CPU_ALLOC_SIZE(cores);

	int shared_fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0666);
	sysassertf(shared_fd != -1, "shm_open failed");

	sysassertf(ftruncate(shared_fd, sizeof(struct shared_s)) == 0, "ftruncate failed");
	// FIXME: HUGETLB?
	shared = (struct shared_s *)mmap(NULL, sizeof(struct shared_s), PROT_READ | PROT_WRITE,
	  MAP_SHARED | MAP_POPULATE, shared_fd, 0);
	sysassertf(shared != MAP_FAILED, "mmap failed");
	sysassertf(close(shared_fd) != -1, "close failed");

	pthread_mutex_lock(&shared->lock);
	if (shared->attached == 0)
		populate();

	shared->attached += 1;
	pthread_mutex_unlock(&shared->lock);
}

__attribute__((constructor)) void fini(void)
{
	printf("fini\n");

	pthread_mutex_lock(&shared->lock);
	shared->attached -= 1;
	pthread_mutex_unlock(&shared->lock);

	sysassertf(munmap(shared, sizeof(struct shared_s)) == 0, "munmap failed");
	sysassertf(shm_unlink(SHM_NAME) == 0, "shm_unlink failed");

	CPU_FREE(allowed);
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
  void *(*start_routine) (void *), void *arg)
{
	printf("pthread_create\n");
	allocate();
//	pthread_cleanup_push(release, core);
//	pthread_cleanup_pop();
	return 0;
}

#ifdef DEV
pid_t fork(void)
{
	allocate();
	printf("fork\n");
	return 0;
}

pid_t vfork(void)
{
	allocate();
	printf("vfork\n");
	return 0;
}

int clone(int (*fn)(void *), void *child_stack, int flags, void *arg, ...)
{
	allocate();
	printf("clone\n");
	return 0;
}
#endif
