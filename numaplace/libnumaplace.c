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
#include <malloc.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>

#define MAX_CORES 8192
#define MAX_NUMA 1024
#define THP_SIZE (2 << 20)
#define SHM_LEN THP_SIZE
#define BITS_PER_LONG (sizeof(long) * 8)
#define BITOP_WORD(nr)      ((nr) / BITS_PER_LONG)
#define STACKNAME "[stack]"

typedef struct {
	__cpu_mask __bits[MAX_CORES / __NCPUBITS];
} cpu_xset_t;

struct context {
	pid_t pid;
	cpu_xset_t available;
};

#define CONTEXTS (SHM_LEN / (sizeof(struct context) + 8))

struct table {
	pid_t lock;
	unsigned allocated;
	struct context contexts[CONTEXTS];
};

#ifdef LEGACY
static uint8_t stride = 1;
static uint8_t stride_alloc; // threshold for allocation in group of 'stride' bits
#endif
static uint16_t threads;
static unsigned lastcore;
static unsigned flags;
static cpu_xset_t occupied;
static pthread_key_t thread_key;
static uint64_t stacksize = 8 << 20;
static struct table *table;
static struct context *context;
static int (*xsched_getaffinity)(pid_t, size_t, cpu_set_t *);

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
		assert(xmmap);
	}

	if (flags & FLAGS_DEBUG)
		fprintf(stderr, "mmap(addr=%p length=%zu prot=%d flags=%u fd=%d offset=%ld)\n",
			addr, length, prot, flags, fd, offset);

	void *map = xmmap(addr, length, prot, mflags, fd, offset);

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

	cpu_set_t nodemask;
	CPU_ZERO(&nodemask);
	CPU_SET(node, &nodemask);

	int ret = mbind(addr, size, MPOL_BIND, (unsigned long *)&nodemask.__bits, node, MPOL_MF_MOVE | MPOL_MF_STRICT);
	if (ret && (flags & FLAGS_VERBOSE))
		printf("failed to bind memory with errno %d\n", errno);
}

static bool lock_core_outer(const unsigned core, struct thread_info *info)
{
	if (!CPU_ISSET_S(core, sizeof(occupied), &occupied)) {
		int fd = lock_core(core);
		if (fd > -1) {
			CPU_SET_S(core, sizeof(occupied), &occupied);
			info->core = core;
			info->fd = fd;
			return 1;
		}
	}

	return 0;
}

static bool core_allocate(struct thread_info *info)
{
	// search NUMA nodes in sequence
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
					printf("core %u, lastcore %u, lastnode %d, i %u, dist %u\n", info->core, lastcore, node, i, dist[i].dist);
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
				used += CPU_ISSET_S(core, sizeof(occupied), &occupied);

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
			if (!CPU_ISSET_S(core2, sizeof(occupied), &occupied)) {
				int fd = lock_core(core2);
				if (fd > -1) {
					CPU_SET_S(core2, sizeoof(occupied), &occupied);
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
static void core_deallocate(struct thread_info *const info)
{
	if (flags & FLAGS_ALLOCATOR)
		exit_heap();

	// FIXME: can fail
	close(info->fd); // unbind lock
	CPU_CLR_S(info->core, sizeof(occupied), &occupied);

	if (unlikely(flags & FLAGS_VERBOSE))
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
		if (unlikely(flags & FLAGS_VERBOSE))
			printf("init core %u\n", info.core);

		cpu_xset_t current;
		CPU_ZERO_S(sizeof(current), &current);
		CPU_SET_S(info.core, sizeof(current), &current);
		assert(!sched_setaffinity(0, sizeof(current), (cpu_set_t *)&current));

		// pin stack
		struct rlimit limit;
		assert(!getrlimit(RLIMIT_STACK, &limit));

		int node = numa_node_of_cpu(info.core);
		assert(node != -1);

		cpu_set_t nodemask;
		CPU_ZERO(&nodemask);
		CPU_SET(node, &nodemask);

		unsigned long *start, *end;
		find_stack(&start, &end);
		int f = mbind(start, end - start, MPOL_PREFERRED, (unsigned long *)&nodemask.__bits, node + 1, MPOL_MF_MOVE | MPOL_MF_STRICT);
		sysassertf(!f, "mbind failed");
	}
}

// block on lock, testing pid
static void table_lock(void)
{
	const pid_t pid = getpid();

	while (1) {
		uint32_t lock = __sync_val_compare_and_swap(&table->lock, 0, pid);
		if (!lock) // lock acquired
			return;

		// FIXME: delay longer to avoid pressure?
		cpu_relax();

		// lock occupied; if PID doesn't exist, clear lock
		if (kill(lock, 0)) {
			assert(errno == ESRCH);
			table->lock = 0;
		}
	}
}

static void table_unlock(void)
{
	table->lock = 0;
}

static void context_deallocate(struct context *c)
{
	table_lock();
	context->pid = 0;
	table->allocated--;
	table_unlock();
}

static void context_allocate(void)
{
	table_lock();
	unsigned left = table->allocated, i = 0;

	// deallocate any dead contexts
	while (left) {
		// skip unuallocated
		if (!table->contexts[i].pid) {
			i++;
			assert(i < CONTEXTS);
			continue;
		}

		if (kill(table->contexts[i].pid, 0)) {
			assert(errno == ESRCH);
			context_deallocate(&table->contexts[i]);
		}

		left--;
	}

	// use first available
	for (i = 0; i < CONTEXTS; i++) {
		if (!table->contexts[i].pid) {
			context = &table->contexts[i];
			context->pid = getpid();

			// FIXME: unify
			if (unlikely(!xsched_getaffinity)) {
				xsched_getaffinity = (int (*)(pid_t, size_t, cpu_set_t *))dlsym(RTLD_NEXT, "sched_getaffinity");
				assert(xsched_getaffinity);
			}

			assert(!xsched_getaffinity(0, sizeof(context->available), (cpu_set_t *)&context->available));
			table->allocated++;
			table_unlock();
			return;
		}
	}

	table_unlock();
	fprintf(stderr, "numaplace: out of contexts\n");
	exit(1);
}

#ifndef PARENT
__attribute__((destructor))
static void finit(void)
{
	assert(context);
	context_deallocate(context);
}

// warning: the constructor gets called after eg pthread_getaffinity_np
__attribute__((constructor))
static void init(void)
{
	char *envstr = getenv("NUMAPLACE_FLAGS");
	if (envstr)
		flags = atoi(envstr);

#ifdef LEGACY
	envstr = getenv("NUMAPLACE_STRIDE");
	if (envstr)
		stride = atoi(envstr);
#endif

	envstr = getenv("OMP_STACKSIZE");
	if (envstr)
		stacksize = parseint(envstr);

	int fd = shm_open("/numaplace", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	assert(fd != -1);
	assert(ftruncate(fd, SHM_LEN) != -1);
	table = mmap(NULL, SHM_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	assert(table != MAP_FAILED);
	assert(!table->allocated); // documented to be zeroed for the first consumer
	context_allocate();
}
#endif

static void init_threads(void)
{
	char *envstr = getenv("OMP_NUM_THREADS");
	if (envstr)
		threads = atoi(envstr);
	else
		// FIXME: check how many cores free
		threads = get_nprocs_conf();
}

int sched_getaffinity(pid_t pid, size_t cpusetsize, cpu_set_t *cpuset)
{
	if (unlikely(!threads))
		init_threads();

	if (unlikely(flags & FLAGS_DEBUG))
		fprintf(stderr, "sched_getaffinity: returning %u threads\n", threads);

	if (pid && pid != getpid()) {
		fprintf(stderr, "warning: returning affinity for non-local process\n");
		return xsched_getaffinity(pid, cpusetsize, cpuset);
	}

	// FIXME: unset cores which will be allocated
	table_lock();
	memcpy(cpuset, &context->available, min(cpusetsize, sizeof(context->available)));
	table_unlock();

	return 0;
}

int pthread_getaffinity_np(pthread_t thread, size_t cpusetsize, cpu_set_t *cpuset)
{
	static int (*xpthread_getaffinity_np)(pthread_t, size_t, cpu_set_t *);
	if (unlikely(!xpthread_getaffinity_np)) {
		xpthread_getaffinity_np = (int (*)(pthread_t, size_t, cpu_set_t *))dlsym(RTLD_NEXT, "pthread_getaffinity_np");
		assert(xpthread_getaffinity_np);
	}

	if (unlikely(!threads))
		init_threads();

	if (unlikely(flags & FLAGS_DEBUG))
		fprintf(stderr, "pthread_getaffinity_np: returning %u threads\n", threads);

	if (thread != pthread_self()) {
		fprintf(stderr, "warning: returning affinity for non-local thread\n");
		return xpthread_getaffinity_np(thread, cpusetsize, cpuset);
	}

	if (!cpuset)
		return EFAULT;

	if (cpusetsize < threads / 8)
		return EINVAL;

	CPU_ZERO_S(cpusetsize, cpuset);
	for (unsigned i = 0; i < threads; i++)
		CPU_SET_S(i, cpusetsize, cpuset);

	return 0;
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
	if (unlikely(!xpthread_create)) {
		xpthread_create = (int (*)(pthread_t*, const pthread_attr_t*, void *(*)(void *), void *))dlsym(RTLD_NEXT, "pthread_create");
		assert(xpthread_create);
	}

	if (!key_created) {
		assert(!pthread_key_create(&thread_key, (void(*)(void *))core_deallocate));
		key_created = 1;
	}

	if (unlikely(flags & FLAGS_DEBUG))
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
	} else {
		// hugepage redzone
		assert(!posix_memalign(&stack, THP_SIZE, stacksize + THP_SIZE));

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
