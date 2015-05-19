// libbnumaplace
// acquire a set of cores as needed from UNIX-domain locks

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
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/socket.h>

#define MAX_CORES 8192
#define MAX_NUMA 1024
#define THP_SIZE (1 << 20)
#define BITS_PER_LONG (sizeof(long) * 8)
#define BITOP_WORD(nr)      ((nr) / BITS_PER_LONG)
#define STACKNAME "[stack]"

#ifdef LEGACY
static uint8_t stride = 1;
static uint16_t cores;
static uint8_t stride_alloc; // threshold for allocation in group of 'stride' bits
#endif
static unsigned lastcore;
static unsigned flags;
static unsigned long occupied[MAX_CORES / BITS_PER_LONG];
static pthread_key_t thread_key;
static uint64_t stacksize = 8 << 20;

typedef const unsigned long __attribute__((__may_alias__)) long_alias_t;

struct thread_info {
	void *(* start_routine)(void *);
	void *arg;
	int fd;
	uint16_t core;
};

struct distance {
	uint16_t node;
	uint8_t dist;
};

static inline void set_bit(const unsigned nr, unsigned long *addr)
{
	addr[nr / BITS_PER_LONG] |= 1UL << (nr % BITS_PER_LONG);
}

static inline void clear_bit(const unsigned nr, unsigned long *addr)
{
	addr[nr / BITS_PER_LONG] &= ~(1UL << (nr % BITS_PER_LONG));
}

static inline int test_bit(const unsigned nr, const unsigned long *addr)
{
	assert(nr < MAX_CORES);

	return ((1UL << (nr % BITS_PER_LONG)) &
	  (((unsigned long *)addr)[nr / BITS_PER_LONG])) != 0;
}
#if UNUSED
static void print_mask(const unsigned long *addr, const unsigned long bits)
{
	printf("mask: %lu ", bits);
	for (unsigned bit = 0; bit < bits; bit++)
		printf("%u", test_bit(bit, addr));
	printf("\n");
}

static unsigned long find_first_bit(const unsigned long *addr, unsigned long size)
{
	long_alias_t *p = (long_alias_t *) addr;
	unsigned long result = 0;
	unsigned long tmp;

	while (size & ~(BITS_PER_LONG - 1)) {
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

static unsigned long find_next_bit(const unsigned long *addr, unsigned long size, unsigned long offset)
{
	const unsigned long *p = addr + BITOP_WORD(offset);
	unsigned long result = offset & ~(BITS_PER_LONG - 1);
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
	while (size & ~(BITS_PER_LONG - 1)) {
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

static void *malloc_spatial(const uint16_t core, const size_t size)
{
	void *addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	assert(addr != MAP_FAILED);

	assert(!madvise(addr, size, (flags & FLAGS_NOTHP) ? MADV_NOHUGEPAGE : MADV_HUGEPAGE));
	membind(addr, size, core);

	if (flags & FLAGS_DEBUG)
		fprintf(stderr, "allocated %zuMB on core %u\n", size >> 20, core);

	return addr;
}

void *mmap(void *addr, size_t length, int prot, int mflags, int fd, off_t offset)
{
	static void *(*xmmap)(void *, size_t, int, int, int, off_t);

	if (!xmmap) {
		xmmap = (void *(*)(void *, size_t, int, int, int, off_t))dlsym(RTLD_NEXT, "mmap");
		assertf(xmmap, "dlsym failed");
	}

	if (flags & FLAGS_DEBUG)
		fprintf(stderr, "mmap(addr=%p length=%zu prot=%d flags=%u fd=%d offset=%ld)\n",
			addr, length, prot, flags, fd, offset);

	void *map = xmmap(addr, length, prot, mflags, fd, offset);
	if (map) {
		int rc = madvise(map, length, (flags & FLAGS_NOTHP) ? MADV_NOHUGEPAGE : MADV_HUGEPAGE);
		if (rc != 0)
			fprintf(stderr, "madvise(map=%p length=%zu MADV_HUGEPAGE) failed with %d\n",
			  map, length, errno);
	}

	return map;
}
#endif
static void find_stack(unsigned long **start, unsigned long **end)
{
	FILE *maps = fopen("/proc/self/maps", "r");
	assert(maps);

	char line[128];

	while (fgets(line, sizeof(line), maps)) {
		int m = -1;

		if (sscanf(line, "%16p-%16p rw-p %*x %*x:%*x %*u %n", start, end, &m) != 2 || m < 0)
			continue;

		if (!strncmp(&line[m], STACKNAME, sizeof(STACKNAME) - 1)) {
			fclose(maps);
			return;
		}
	}

	abort();
}

static int lock_core(const unsigned core)
{
	int sfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sfd == -1 && errno == EMFILE) {
		fprintf(stderr, "Error: open file descriptor too low; please increase with 'ulimit -n 8192'\n");
		exit(1);
	}
	assert(sfd > -1);

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	snprintf(&addr.sun_path[1], sizeof(addr.sun_path) - 1, "numaplace-%u", core);

	// if bind succeeds, core wasn't already taken
	int ret = bind(sfd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret) {
		assert(errno == EADDRINUSE);
		assert(!close(sfd));
		return -1;
	}

	return sfd;
}

static void membind(void *const addr, const unsigned long size, const uint16_t core)
{
	int node = numa_node_of_cpu(core);
	assert(node != -1);

	size_t masksize = roundup(node / 8 + 1, sizeof(long));
	unsigned long nodemask[MAX_CORES / 8 / sizeof(long)];
	memset(nodemask, 0, masksize);
	set_bit(node, nodemask);

	int ret = mbind(addr, size, MPOL_BIND, nodemask, node + 2, MPOL_MF_MOVE | MPOL_MF_STRICT);
	if (ret && (flags & FLAGS_VERBOSE))
		printf("failed to bind memory with errno %d\n", errno);
}

static bool lock_core_outer(const unsigned core, struct thread_info *info)
{
	if (!test_bit(core, occupied)) {
		int fd = lock_core(core);
		if (fd > -1) {
			set_bit(core, occupied);
			info->core = core;
			info->fd = fd;
			return 1;
		}
	}

	return 0;
}

static bool core_allocate(struct thread_info *info)
{
#ifdef TEST
	// search local NUMA node for available cores
	static struct bitmask *local = NULL;
	if (!local) {
		local = numa_allocate_cpumask();
		assert(local);
	}

	const int node = numa_node_of_cpu(lastcore);
	assert(!numa_node_to_cpus(node, local));

	for (unsigned i = 0; i < local->size; i++)
		if (numa_bitmask_isbitset(local, i) && lock_core_outer(i, info))
			return 1;
#endif
	// next, search NUMA nodes in sequence
	char buf[MAX_NUMA * 4]; // each NUMA distance is max 3 chars plus space each

	const int node = numa_node_of_cpu(lastcore);
	assert(node != -1);
	snprintf(buf, sizeof(buf), "/sys/devices/system/node/node%d/distance", node);

	int fd = open(buf, O_RDONLY);
	assert(fd != -1);
	int len = read(fd, buf, sizeof(buf));
	assert(len > 0);
	buf[len - 1] = '\0';
	assert(!close(fd));

	struct distance dist[MAX_NUMA];
	unsigned dists = 0;
	char *p = buf;

	// parse string into array
	while (1) {
		dist[dists].node = dists;
		dist[dists].dist = atoi(p);
		dists++;

		p = strchr(p, ' ');
		if (!p)
			break;
		p++;
	}

	// sort
	for (unsigned i = 1; i < dists; i++) {
		unsigned j;
		struct distance tmp = dist[i];

		for (j = i; j >= 1 && tmp.dist < dist[j - 1].dist; j--)
			dist[j] = dist[j - 1];

		dist[j] = tmp;
	}

	struct bitmask *local = numa_allocate_cpumask();
	assert(local);

	for (unsigned i = 0; i < dists; i++) {
		assert(!numa_node_to_cpus(dist[i].node, local));
		for (unsigned j = 0; j < local->size; j++)
			if (numa_bitmask_isbitset(local, j) && lock_core_outer(j, info)) {
				if (flags & FLAGS_VERBOSE)
					printf("core %u, lastcore %u, lastnode %u, i %u, dist %u\n", info->core, lastcore, node, i, dist[i].dist);
				lastcore = j;
				return 1;
			}
	}

	return 0;
}
#ifdef LEGACY
// find next system-wide unallocated core, using UNIX domain sockets
// returns core number allocated
static bool core_allocate2(struct thread_info *info)
{
	while (1) {
		// find group of 'stride' bits with sufficiently low occupancy
		unsigned core, used;

		do {
			used = 0;

			// find weight of 'stride' group of bits
			for (core = lastcore; core < (lastcore + stride); core++)
				used += test_bit(core, occupied);

			lastcore += stride;

			if (lastcore > cores) {
				lastcore = 0;
				stride_alloc++;
			}

			if (stride_alloc >= stride) {
				warn_once("Oversubscribing cores");
				return 0;
			}
		} while (used > stride_alloc);

		// use first unset occupied bit
		for (unsigned core2 = core - 1; core2 >= core - stride; core2--) {
			if (!test_bit(core2, occupied)) {
				int fd = lock_core(core2);
				if (fd > -1) {
					set_bit(core2, occupied);
					info->core = core2;
					info->fd = fd;
					return 1;
				} else
					// skip
					break;
			}
		}
	}
}
#endif
static void core_deallocate(struct thread_info *info)
{
	assert(!close(info->fd)); // unbind lock
	clear_bit(info->core, occupied);

	if (flags & FLAGS_VERBOSE)
		printf("core %u deallocate\n", info->core);

	free(info);
}

void bind_current(void)
{
	// use batch scheduling priority to reduce preemption
	const struct sched_param params = {0};
	sysassertf(sched_setscheduler(0, SCHED_BATCH, &params) == 0, "sched_setscheduler failed");

	struct thread_info info;
	if (core_allocate(&info)) {
		if (flags & FLAGS_VERBOSE)
			printf("init core %u\n", info.core);

		const size_t setsize = CPU_ALLOC_SIZE(info.core + 1);
		cpu_set_t cpuset[MAX_CORES / 8 / sizeof(cpu_set_t)];
		CPU_ZERO_S(setsize, cpuset);
		CPU_SET_S(info.core, setsize, cpuset);

		assert(!sched_setaffinity(0, setsize, cpuset));

		// pin stack
		struct rlimit limit;
		assert(!getrlimit(RLIMIT_STACK, &limit));

		int node = numa_node_of_cpu(info.core);
		assert(node != -1);

		size_t masksize = roundup(node / 8 + 1, sizeof(long));
		unsigned long nodemask[MAX_CORES / 8 / sizeof(long)];
		memset(nodemask, 0, masksize);
		set_bit(node, nodemask);

		unsigned long *start, *end;
		find_stack(&start, &end);
		int f = mbind(start, end - start, MPOL_BIND, nodemask, node + 2, MPOL_MF_MOVE | MPOL_MF_STRICT);
		assertf(!f, "mbind errno %d\n", errno);
	}
}

static __attribute__((constructor)) void init(void)
{
	char *envstr = getenv("NUMAPLACE_FLAGS");
	if (envstr)
		flags = atoi(envstr);

#ifdef LEGACY
	envstr = getenv("NUMAPLACE_STRIDE");
	if (envstr)
		stride = atoi(envstr);

	cores = sysconf(_SC_NPROCESSORS_ONLN);
#endif

	envstr = getenv("OMP_STACKSIZE");
	if (envstr)
		stacksize = parseint(envstr);
}

static void *pthread_create_inner(struct thread_info *info)
{
	assert(!pthread_setspecific(thread_key, info));
	return info->start_routine(info->arg);
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
  void *(*start_routine) (void *), void *arg)
{
	static bool key_created;
	static int (*xpthread_create)(pthread_t *, const pthread_attr_t *, void *(*) (void *), void *);

	if (!key_created) {
		assert(!pthread_key_create(&thread_key, (void(*)(void *))core_deallocate));
		key_created = 1;
	}

	if (!xpthread_create) {
		xpthread_create = (int (*)(pthread_t*, const pthread_attr_t*, void *(*)(void *), void *))dlsym(RTLD_NEXT, "pthread_create");
		assertf(xpthread_create, "dlsym failed");
	}

	if (flags & FLAGS_DEBUG)
		fprintf(stderr, "pthread_create(thread=%p attr=%p start_routine=%p arg=%p\n",
			thread, attr, start_routine, arg);

	struct thread_info *info = malloc(sizeof(struct thread_info));
	assert(info);
	info->start_routine = start_routine;
	info->arg = arg;

	size_t stacksize2 = 0;
	void *stack = NULL;
	pthread_attr_t attr2;
	if (attr) {
		assert(!pthread_attr_getstack(attr, &stack, &stacksize2));
		memcpy(&attr2, attr, sizeof(attr2));
	} else
		assert(!pthread_attr_init(&attr2));

	bool guard;

	// allocate stack if not already
	if (stacksize2) {
		guard = 0;
		// ignore return code, allowing failure due to size and/or misalignment
		int ret = madvise(stack, stacksize2, MADV_HUGEPAGE);
		if (ret && (flags & FLAGS_VERBOSE))
			printf("madvise failed with errno %d\n", errno);
	} else {
		// hugepage redzone
		assert(!posix_memalign(&stack, THP_SIZE, stacksize + THP_SIZE));
		assert(!madvise(stack, stacksize + THP_SIZE, MADV_HUGEPAGE));

		// protect redzone
		assert(!mprotect(stack, THP_SIZE, PROT_NONE));
		assert(!pthread_attr_setstack(&attr2, stack, stacksize));
	}

	if (core_allocate(info)) {
		const size_t setsize = CPU_ALLOC_SIZE(info->core + 1);
		cpu_set_t cpuset[MAX_CORES / 8 / sizeof(cpu_set_t)];
		CPU_ZERO_S(setsize, cpuset);
		CPU_SET_S(info->core, setsize, cpuset);

		assert(!pthread_attr_setaffinity_np(&attr2, setsize, cpuset));

		membind(stack, stacksize + guard * THP_SIZE, info->core);
	} else
		if (flags & FLAGS_VERBOSE)
			printf("cores overallocated\n");

	return xpthread_create(thread, &attr2, (void *(*)(void *))pthread_create_inner, info);
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
		fprintf(stderr, "malloc(size=%zu)\n", size);

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
		fprintf(stderr, "memcpy(dest=%p src=%p size=%zu)\n", dest, src, n);

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
		fprintf(stderr, "memmove(dest=%p src=%p size=%zu)\n", dest, src, n);

	return xmemmove(dest, src, n);
}

void *calloc(size_t nmemb, size_t size)
{
	static void *(*xcalloc)(size_t, size_t);
	if (!xcalloc) {
		xcalloc = (void *(*)(size_t, size_t))dlsym(RTLD_NEXT, "calloc");
		assertf(xcalloc, "dlsym failed");
	}

	if (flags & FLAGS_DEBUG)
		fprintf(stderr, "calloc(nmemb=%zu size=%zu)\n", nmemb, size);

	return xcalloc(nmemb, size);
}
#endif
