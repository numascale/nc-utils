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

// TODO: implement lockless rand()

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

static void populate(void)
{
	fprintf(stderr, "populate\n");
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
	fprintf(stderr, "core=%u\n", core);

	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(core, &cpuset);
	int rc = pthread_setaffinity_np(sizeof(cpuset), &cpuset);
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

__attribute__((constructor)) void init(void)
{
	fprintf(stderr, "init\n");

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

__attribute__((destructor)) void fini(void)
{
	fprintf(stderr, "fini\n");

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
	static int (*xpthread_create)(pthread_t *, const pthread_attr_t *,
	  void *(*) (void *), void *);
	if (!xpthread_create) {
		xpthread_create = (int (*)(pthread_t*, const pthread_attr_t*, void* (*)(void*), void*))dlsym(RTLD_NEXT, "pthread_create");
		assertf(xpthread_create, "dlsym failed");
	}

	fprintf(stderr, "pthread_create(thread=%p attr=%p start_routine=%p arg=%p\n",
		thread, attr, start_routine, arg);

	allocate();
//	pthread_cleanup_push(release, core);
//	pthread_cleanup_pop();
	return xpthread_create(thread, attr, start_routine, arg);
}

void pthread_exit(void *retval)
{
	static void (*xpthread_exit)(void *);
	if (!xpthread_exit) {
		xpthread_exit = (void (*)(void *))dlsym(RTLD_NEXT, "pthread_exit");
		assertf(xpthread_exit, "dlsym failed");
	}

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
	fprintf(stderr, "vfork()\n");
	return xvfork();
}

#ifdef DEV
int clone(int (*fn)(void *), void *child_stack, int flags, void *arg, ...)
{
	allocate();
	fprintf(stderr, "clone(fn=%p child_stack=%p flags=%d arg=%p)\n",
		fn, child_stack, flags, arg);
	return xclone(fn, child_stack, flags, arg, ...);
}
#endif

void *memcpy(void *dest, const void *src, size_t n)
{
	static void *(*xmemcpy)(void *, const void *, size_t);
	if (!xmemcpy) {
		xmemcpy = (void *(*)(void *, const void *, size_t))dlsym(RTLD_NEXT, "memcpy");
		assertf(xmemcpy, "dlsym failed");
	}

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

	fprintf(stderr, "mmap(addr=%p length=%lu prot=%d flags=%d fd=%d offset=%ld)\n",
		addr, length, prot, flags, fd, offset);
	void *map = xmmap(addr, length, prot | MAP_HUGETLB, flags, fd, offset);
	// MAP_NONBLOCK | MAP_POPULATE
	if (map) {
		int rc = madvise(map, length, MADV_HUGEPAGE);
		if (rc != 0)
			fprintf(stderr, "madvise(map=%p length=%lu MADV_HUGEPAGE) failed with %d\n",
			  map, length, errno);
	}

	return map;
}

void *malloc(size_t size)
{
	static void *(*xmalloc)(size_t);
	if (!xmalloc) {
		xmalloc = (void *(*)(size_t))dlsym(RTLD_NEXT, "malloc");
		assertf(xmalloc, "dlsym failed");
	}

	fprintf(stderr, "malloc(size=%lu)\n", size);
	return xmalloc(size);
}

void *calloc(size_t nmemb, size_t size)
{
	static void *(*xcalloc)(size_t, size_t);
	if (!xcalloc) {
		xcalloc = (void *(*)(size_t, size_t))dlsym(RTLD_NEXT, "calloc");
		assertf(xcalloc, "dlsym failed");
	}

	fprintf(stderr, "calloc(nmemb=%lu size=%lu)\n", nmemb, size);
	return xcalloc(nmemb, size);
}
