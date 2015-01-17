#include "utils.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sched.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <pthread.h>
#include <numa.h>
#include <numaif.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define SHM_NAME "numaplace"
#define MAX_CORES 8192
#define MAX_NUMA 1024
#define THP_ALIGN_THRESH (1 << 20)
#define PTHREAD_STACK_SIZE (8 << 20)

// TODO: implement lockless rand()

struct shared_s {
	pthread_mutex_t lock;
	unsigned attached;
	bool initialised;
	union {
		cpu_set_t allocated;
		char u[CPU_ALLOC_SIZE(MAX_CORES)];
	};
	unsigned char dist[MAX_NUMA][MAX_NUMA];
};

static struct shared_s *shared;
static cpu_set_t *allowed;
static cpu_set_t allocated;
static uint16_t cores;
static size_t masksize;
static unsigned flags;

static void *malloc_spatial(const uint16_t core, const size_t size)
{
	const size_t gran = min(max(sizeof(long), roundup_nextpow2(size)), THP_ALIGN_THRESH);
	void *addr;
	assert(!posix_memalign(&addr, gran, size));

	int node = numa_node_of_cpu(core);
	assert(node != -1);
	unsigned long nodemask = 1 << node;

	assert(!mbind(addr, size, MPOL_BIND, &nodemask, sizeof(nodemask) * 8, MPOL_MF_MOVE));

	if (flags & FLAGS_DEBUG)
		fprintf(stderr, "allocated %zuMB on core %u, node %d\n", size >> 20, core, node);

	return addr;
}

static void populate(void)
{
	if (flags & FLAGS_DEBUG)
		fprintf(stderr, "populate\n");

	// check /proc/cmdline to check isolcpus param
	int fd = open("/proc/cmdline", O_RDONLY);
	assert(fd != -1);
	char buf[4096];
	assert(read(fd, buf, sizeof(buf)) > 0);
	assert(!close(fd));
	if (!strstr(buf, "isolcpus="))
		fprintf(stderr, "Please boot with 'maxcpus=1-8191' for best parallel performance\n");
	shared->initialised = 1;
}

// returns core number
static uint16_t allocate_core(void)
{
	uint16_t core = MAX_CORES;

	pthread_mutex_lock(&shared->lock);

	for (unsigned n = 0; n < cores; n++) {
		if (!CPU_ISSET_S(n, masksize, &shared->allocated)) {
			CPU_SET_S(n, masksize, &shared->allocated);
			CPU_SET_S(n, masksize, &allocated);
			core = n;
			break;
		}
	}

	pthread_mutex_unlock(&shared->lock);
	if (core == MAX_CORES)
		warn_once("All cores already allocated");

	return core;
}

// called with lock held
static void state(void)
{
	fprintf(stderr, "cores used:");
	for (unsigned n = 0; n < cores; n++)
		if (CPU_ISSET_S(n, masksize, &shared->allocated))
			fprintf(stderr, " %u", n);
	fprintf(stderr, "\n");
}

void _fini(void)
{
	if (flags & FLAGS_DEBUG)
		fprintf(stderr, "cleanup\n");

	pthread_mutex_lock(&shared->lock);

	// cleanup allocated cores
	for (unsigned n = 0; n < cores; n++)
		if (CPU_ISSET_S(n, masksize, &allocated))
			CPU_CLR_S(n, masksize, &shared->allocated);
	shared->attached--;

	pthread_mutex_unlock(&shared->lock);

	sysassertf(!munmap(shared, sizeof(struct shared_s)), "munmap failed");
	sysassertf(shm_unlink(SHM_NAME) == 0, "shm_unlink failed");

	CPU_FREE(allowed);
}

__attribute__((constructor)) void init(void)
{
	char *flagstr = getenv("NUMAPLACE_FLAGS");
	if (flagstr)
		flags = atoi(flagstr);

	cores = sysconf(_SC_NPROCESSORS_CONF);

	allowed = CPU_ALLOC(cores);
	sysassertf(allowed, "CPU_ALLOC failed");
	masksize = CPU_ALLOC_SIZE(cores);

	int shared_fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0666);
	sysassertf(shared_fd != -1, "shm_open failed");

	sysassertf(ftruncate(shared_fd, sizeof(struct shared_s)) == 0, "ftruncate failed");
	shared = (struct shared_s *)mmap(NULL, sizeof(struct shared_s), PROT_READ | PROT_WRITE,
	  MAP_SHARED | MAP_POPULATE, shared_fd, 0);
	sysassertf(shared != MAP_FAILED, "mmap failed");
	sysassertf(close(shared_fd) != -1, "close failed");

	pthread_mutex_lock(&shared->lock);

	if (!shared->initialised == 0)
		populate();

	if (flags & FLAGS_DEBUG)
		state();

	shared->attached++;
	if (flags & FLAGS_DEBUG)
		printf("attached %u\n", shared->attached);
	pthread_mutex_unlock(&shared->lock);

	// use batch scheduling priority to minimise preemption
	const struct sched_param params = {0};
	sysassertf(sched_setscheduler(0, SCHED_BATCH, &params) == 0, "sched_setscheduler failed");
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
  void *(*start_routine) (void *), void *arg)
{
	int ret;
	static int (*xpthread_create)(pthread_t *, const pthread_attr_t *,
	  void *(*) (void *), void *);

	if (!xpthread_create) {
		xpthread_create = (int (*)(pthread_t*, const pthread_attr_t*, void* (*)(void*), void*))dlsym(RTLD_NEXT, "pthread_create");
		assertf(xpthread_create, "dlsym failed");
	}

	if (flags & FLAGS_DEBUG)
		fprintf(stderr, "pthread_create(thread=%p attr=%p start_routine=%p arg=%p\n",
			thread, attr, start_routine, arg);

	const uint16_t core = allocate_core();
	if (core < MAX_CORES) {
		cpu_set_t cpuset;
		CPU_ZERO(&cpuset);
		CPU_SET(core, &cpuset);

		pthread_attr_t attr2;
		assert(!pthread_attr_init(&attr2));
		assert(!pthread_attr_setaffinity_np(&attr2, sizeof(cpuset), &cpuset));

		void *stack = malloc_spatial(core, PTHREAD_STACK_SIZE);
		assert(!pthread_attr_setstack(&attr2, stack, PTHREAD_STACK_SIZE));

		ret = xpthread_create(thread, &attr2, start_routine, arg);
		assert(!pthread_attr_destroy(&attr2));
	} else
		ret = xpthread_create(thread, NULL, start_routine, arg);

	return ret;
}

void *malloc(size_t size)
{
	static void *(*xmalloc)(size_t);
	if (!xmalloc) {
		xmalloc = (void *(*)(size_t))dlsym(RTLD_NEXT, "malloc");
		assertf(xmalloc, "dlsym failed");
	}
	if (flags & FLAGS_DEBUG)
		fprintf(stderr, "malloc(size=%lu)\n", size);

	const size_t gran = min(max(sizeof(long), roundup_nextpow2(size)), THP_ALIGN_THRESH);
	void *ret;
	assert(!posix_memalign(&ret, gran, size));
	return ret;
}

#ifdef DEV
void pthread_exit(void *retval)
{
	static void (*xpthread_exit)(void *);
	if (!xpthread_exit) {
		xpthread_exit = (void (*)(void *))dlsym(RTLD_NEXT, "pthread_exit");
		assertf(xpthread_exit, "dlsym failed");
	}

	if (flags & FLAGS_DEBUG)
		fprintf(stderr, "pthread_exit(retval=%p)\n", retval);

	xpthread_exit(retval);
}

pid_t fork(void)
{
	static pid_t (*xfork)(void);
	if (!xfork) {
		xfork = (pid_t (*)(void))dlsym(RTLD_NEXT, "fork");
		assertf(xfork, "dlsym failed");
	}

	allocate();

	if (flags & FLAGS_DEBUG)
		fprintf(stderr, "fork()\n");

	return xfork();
}

pid_t vfork(void)
{
	static pid_t (*xvfork)(void);
	if (!xvfork) {
		xvfork = (pid_t (*)(void))dlsym(RTLD_NEXT, "vfork");
		assertf(xvfork, "dlsym failed");
	}

	allocate();

	if (flags & FLAGS_DEBUG)
		fprintf(stderr, "vfork()\n");

	return xvfork();
}

int clone(int (*fn)(void *), void *child_stack, int flags, void *arg, ...)
{
	allocate();

	if (flags & FLAGS_DEBUG)
		fprintf(stderr, "clone(fn=%p child_stack=%p flags=%d arg=%p)\n",
			fn, child_stack, flags, arg);

	return xclone(fn, child_stack, flags, arg, ...);
}

void *memcpy(void *dest, const void *src, size_t n)
{
	static void *(*xmemcpy)(void *, const void *, size_t);
	if (!xmemcpy) {
		xmemcpy = (void *(*)(void *, const void *, size_t))dlsym(RTLD_NEXT, "memcpy");
		assertf(xmemcpy, "dlsym failed");
	}

	if (flags & FLAGS_DEBUG)
		fprintf(stderr, "memcpy(dest=%p src=%p size=%lu)\n", dest, src, n);

	return xmemcpy(dest, src, n);
}

void *memmove(void *dest, const void *src, size_t n)
{
	static void *(*xmemmove)(void *, const void *, size_t);
	if (!xmemmove) {
		xmemmove = (void *(*)(void *, const void *, size_t))dlsym(RTLD_NEXT, "memmove");
		assertf(xmemmove, "dlsym failed");
	}

	if (flags & FLAGS_DEBUG)
		fprintf(stderr, "memmove(dest=%p src=%p size=%lu)\n", dest, src, n);

	return xmemmove(dest, src, n);
}

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
	static void *(*xmmap)(void *, size_t, int, int, int, off_t);
	if (!xmmap) {
		xmmap = (void *(*)(void *, size_t, int, int, int, off_t))dlsym(RTLD_NEXT, "mmap");
		assertf(xmmap, "dlsym failed");
	}

	if (flags & FLAGS_DEBUG)
		fprintf(stderr, "mmap(addr=%p length=%lu prot=%d flags=%d fd=%d offset=%ld)\n",
			addr, length, prot, flags, fd, offset);

	void *map = xmmap(addr, length, prot, flags | MAP_HUGETLB, fd, offset);
	// MAP_NONBLOCK | MAP_POPULATE
	if (map) {
		int rc = madvise(map, length, MADV_HUGEPAGE);
		if (rc != 0)
			fprintf(stderr, "madvise(map=%p length=%zu MADV_HUGEPAGE) failed with %d\n",
			  map, length, errno);
	}

	return map;
}

void *calloc(size_t nmemb, size_t size)
{
	static void *(*xcalloc)(size_t, size_t);
	if (!xcalloc) {
		xcalloc = (void *(*)(size_t, size_t))dlsym(RTLD_NEXT, "calloc");
		assertf(xcalloc, "dlsym failed");
	}

	if (flags & FLAGS_DEBUG)
		fprintf(stderr, "calloc(nmemb=%lu size=%lu)\n", nmemb, size);

	return xcalloc(nmemb, size);
}
#endif
