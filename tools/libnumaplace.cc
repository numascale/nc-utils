// libbnumaplace
// acquire a set of cores as needed from UNIX-domain locks
// TODO: round-robin thread allocations within this set
// TODO: implement lockless rand()

#include "utils.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sched.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <pthread.h>
#include <numa.h>
#include <numaif.h>
#include <strings.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/socket.h>

#define MAX_CORES 8192
#define MAX_NUMA 1024
#define THP_ALIGN_THRESH (1 << 20)
#define PTHREAD_STACK_SIZE (8 << 20)
#define BITS_PER_LONG (sizeof(long) * 8)
#define BITOP_WORD(nr)      ((nr) / BITS_PER_LONG)

static uint16_t cores, nodes;
static unsigned flags;
static unsigned long available[MAX_CORES / BITS_PER_LONG];

typedef const unsigned long __attribute__((__may_alias__)) long_alias_t;

static inline void set_bit(const unsigned nr, unsigned long *addr)
{
    addr[nr / BITS_PER_LONG] |= 1UL << (nr % BITS_PER_LONG);
}

static inline void clear_bit(const unsigned nr, unsigned long *addr)
{
    addr[nr / BITS_PER_LONG] &= ~(1UL << (nr % BITS_PER_LONG));
}

static __always_inline int test_bit(const unsigned nr, const unsigned long *addr)
{
    return ((1UL << (nr % BITS_PER_LONG)) &
        (((unsigned long *)addr)[nr / BITS_PER_LONG])) != 0;
}

static void print_mask(const unsigned long *addr, const unsigned long bits)
{
	printf("mask: %lu ", bits);
	for (unsigned bit = 0; bit < bits; bit++)
		printf("%u", test_bit(bit, addr));
	printf("\n");
}

static inline unsigned long find_first_bit(const unsigned long *addr, unsigned long size)
{
    long_alias_t *p = (long_alias_t *) addr;
    unsigned long result = 0;
    unsigned long tmp;

    while (size & ~(BITS_PER_LONG-1)) {
        if ((tmp = *(p++)))
            goto found;
        result += BITS_PER_LONG;
        size -= BITS_PER_LONG;
    }
    if (!size)
        return result;

    tmp = (*p) & (~0UL >> (BITS_PER_LONG - size));
    if (tmp == 0UL)     /* Are any bits set? */
        return result + size;   /* Nope. */
found:
    return result + ffsl(tmp);
}

static inline unsigned long find_next_bit(const unsigned long *addr, unsigned long size, unsigned long offset)
{
    const unsigned long *p = addr + BITOP_WORD(offset);
    unsigned long result = offset & ~(BITS_PER_LONG-1);
    unsigned long tmp;

    if (offset >= size)
        return size;
    size -= result;
    offset %= BITS_PER_LONG;
    if (offset) {
        tmp = *(p++);
        tmp &= (~0UL << offset);
        if (size < BITS_PER_LONG)
            goto found_first;
        if (tmp)
            goto found_middle;
        size -= BITS_PER_LONG;
        result += BITS_PER_LONG;
    }
    while (size & ~(BITS_PER_LONG-1)) {
        if ((tmp = *(p++)))
            goto found_middle;
        result += BITS_PER_LONG;
        size -= BITS_PER_LONG;
    }
    if (!size)
        return result;
    tmp = *p;

found_first:
    tmp &= (~0UL >> (BITS_PER_LONG - size));
    if (tmp == 0UL)     /* Are any bits set? */
        return result + size;   /* Nope. */
found_middle:
    return result + ffsl(tmp);
}

#ifdef OLD
static void *malloc_spatial(const uint16_t core, const size_t size)
{
	const int node = numa_node_of_cpu(core);
	assert(node != -1);

	void *addr = numa_alloc_onnode(size, node);
	if (addr)
		assert(!mbind(addr, size, MPOL_BIND, &nodemask, sizeof(nodemask) * 8, MPOL_MF_MOVE));
	return ret;
}
#endif

static void *malloc_spatial(const uint16_t core, const size_t size)
{
	const size_t gran = min(max(sizeof(long), roundup_nextpow2(size)), THP_ALIGN_THRESH);
	void *addr;
	assert(!posix_memalign(&addr, gran, size));

	int node = numa_node_of_cpu(core);
	assert(node != -1);

	size_t masksize = roundup(node / 8 + 1, sizeof(long));
	unsigned long *nodemask = (unsigned long *)alloca(masksize);
	memset(nodemask, 0, sizeof(masksize));
	set_bit(node, nodemask);
//	print_mask(nodemask, masksize);

	int f = mbind(addr, size, MPOL_PREFERRED, nodemask, masksize * 8, MPOL_MF_MOVE);
	assertf(f == 0, "mbind errno %d\n", errno);

	if (flags & FLAGS_DEBUG)
		fprintf(stderr, "allocated %zuMB on core %u, node %d\n", size >> 20, core, node);

	return addr;
}

// find next system-wide unallocated core, using UNIX domain sockets
// returns core number allocated
uint16_t allocate_core(void)
{
	int sfd = socket(AF_UNIX, SOCK_STREAM, 0);
	assert(sfd > -1);

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;

	unsigned long core = find_first_bit(available, cores);
	do {
		snprintf(&addr.sun_path[1], sizeof(addr.sun_path) - 1, "numaplace-%lu", core - 1);

		// if bind succeeds, core wasn't already taken
		if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
			clear_bit(core - 1, available);
			assertf(core <= cores, "core=%lu cores=%u", core, cores);
			return core - 1;
		}

		assert(errno == EADDRINUSE);
		core = find_next_bit(available, cores, core);
	} while (core < cores);

	warn_once("All cores already allocated");
	return MAX_CORES;
}

__attribute__((constructor)) void init(void)
{
	char *flagstr = getenv("NUMAPLACE_FLAGS");
	if (flagstr)
		flags = atoi(flagstr);

	nodes = numa_max_node() + 1;
	cores = sysconf(_SC_NPROCESSORS_ONLN);
	for (unsigned core = 0; core < cores; core++)
		set_bit(core, available);

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

	size_t stacksize = PTHREAD_STACK_SIZE;
	pthread_attr_t attr2;
	if (attr) {
		assert(!pthread_attr_getstacksize(attr, &stacksize));
		memcpy(&attr2, attr, sizeof(attr2));
	} else
		assert(!pthread_attr_init(&attr2));

	if (core < MAX_CORES) {
		const size_t setsize = CPU_ALLOC_SIZE(core + 1);
		cpu_set_t *cpuset = (cpu_set_t *)alloca(setsize);
		CPU_ZERO_S(setsize, cpuset);
		CPU_SET_S(core, setsize, cpuset);

		int r = pthread_attr_setaffinity_np(&attr2, setsize, cpuset);
		printf("core=%u attr=%p attr2=%p setsize=%zu cpuset=%p r=%d errno=%d\n", core, attr, &attr2, setsize, cpuset, r, errno);

		void *stack = malloc_spatial(core, stacksize);
		assert(stack);
//		assert(!pthread_attr_setstack(&attr2, stack, stacksize));

		ret = xpthread_create(thread, &attr2, start_routine, arg);
	} else
		ret = xpthread_create(thread, NULL, start_routine, arg);

	return ret;
}

#ifdef DEV
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
