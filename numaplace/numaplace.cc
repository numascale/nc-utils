#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <sched.h>
#include <dirent.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>

#define CPUSET "/sys/fs/cgroup/cpuset"
#define FLAGS_VERBOSE 1
#define VER "0.9"
#define MAXNODES 1024 // limited by kernel
#define MAXCORES 8192
#define xassert(expr) ((expr) ? (void)(0) : error(__STRING(expr), __FILE__, __LINE__))
#define sysassert(expr) ((expr) ? (void)(0) : syserror(__STRING(expr), __FILE__, __LINE__))

typedef enum {op_set, op_clear} op_t;

static char base[64];
static unsigned nnodes;
static cpu_set_t *available;
static size_t masksize;
static unsigned flags;
static unsigned req_cores;
static unsigned ncores;
static unsigned stride = 1;

static void error(const char *expr, const char *file, const unsigned line)
{
	fprintf(stderr, "assertion '%s' failed at %s:%u\n", expr, file, line);
	exit(1);
}

static void syserror(const char *expr, const char *file, const unsigned line)
{
	fprintf(stderr, "assertion '%s' failed at %s:%u with %s\n", expr, file, line, strerror(errno));
	exit(1);
}

// add(n) only to be called positively monitonically
class List {
	unsigned str_off;
	unsigned range_start, range_end;
	bool first_elem;
	char _str[20480];
public:
    List(): str_off(0), range_start(~0U), range_end(~0U), first_elem(1)
    {
		_str[0] = '\0';
    }

    void add(const unsigned next)
    {
		// if first call, setup state
		if (range_start == ~0U) {
			range_start = range_end = next;
			return;
		}

		// coallesce positively
		if (next == range_end + 1) {
			range_end = next;
			return;
		}

		if (range_start == range_end)
			str_off += sprintf(&_str[str_off], "%s%u", first_elem ? "" : ",", range_start);
		else
			str_off += sprintf(&_str[str_off], "%s%u-%u", first_elem ? "" : ",", range_start, range_end);

		first_elem = 0;
		xassert(str_off < sizeof(_str));
		range_start = range_end = next;
	}

	void add(const cpu_set_t *cpuset, const unsigned lim)
	{
		for (unsigned i = 0; i < lim; i++)
			if (CPU_ISSET_S(i, masksize, cpuset))
				add(i);
	}

	char *str(void)
	{
		// close list
		add(~0U);

		return _str;
	}
};

static uint8_t dist(const unsigned src, const unsigned dst)
{
	static uint8_t _dist[MAXNODES][MAXNODES];

	// fastpath
	uint8_t d = _dist[src][dst];
	if (d)
		return d;

	char buf[4096];
	xassert(snprintf(buf, sizeof(buf), "/sys/devices/system/node/node%u/distance", src) <= (int)sizeof(buf));
	int fd = open(buf, O_RDONLY);
	xassert(fd > -1);

	int ret = read(fd, buf, sizeof(buf));
	xassert(ret > 0);
	xassert(!close(fd));
	buf[ret-1] = '\0'; // terminate

	// count number of entries for nodes
	char *p = buf;
	unsigned i = 0;

	while (1) {
		unsigned d = strtoul(p, NULL, 10);
		_dist[src][i++] = d;

		p = strchr(p, ' ');
		if (!p)
			break;
		p++; // skip space
	}

	xassert(src < i && dst < i);
	return _dist[src][dst];
}

static char *mask_string(const cpu_set_t *cpuset)
{
	static char maskstr[MAXCORES+1];

	for (unsigned c = 0; c < ncores; c++)
		maskstr[c] = CPU_ISSET_S(c, masksize, cpuset) ? '1' : '0';
	maskstr[ncores] = '\0';

	return maskstr;
}

static void string_to_mask(char *str, cpu_set_t *mask, const op_t op)
{
	do {
		unsigned a = strtoul(str, &str, 10);

		if (*str == '-') {
			str++;
			unsigned b = strtoul(str, &str, 10);
			for (unsigned i = a; i <= b; i++)
				op == op_set ? CPU_SET_S(i, masksize, mask) : CPU_CLR_S(i, masksize, mask);
		} else
			op == op_set ? CPU_SET_S(a, masksize, mask) : CPU_CLR_S(a, masksize, mask);
	} while (*str++ == ',');
}

static void scan_cores(void)
{
	available = CPU_ALLOC(ncores);
	xassert(available);

	masksize = CPU_ALLOC_SIZE(ncores);
	CPU_ZERO_S(masksize, available);

	// find max node number
	int fd = open(CPUSET "/cpuset.mems", O_RDONLY);
	xassert(fd > -1);
	char buf[64];
	ssize_t len = read(fd, buf, sizeof(buf));
	xassert(len > 0);
	xassert(!close(fd));
	buf[len-1] = '\0';

	char *dash = strchr(buf, '-');
	if (!dash)
		dash = buf;
	else
		dash++; // skip '-'
	nnodes = atoi(dash) + 1;

	// find what cores are in parent cpuset and set it bitmask
	fd = open(CPUSET "/cpuset.effective_cpus", O_RDONLY);
	xassert(fd > -1);
	xassert(read(fd, buf, sizeof(buf)) > 0);
	xassert(!close(fd));
	string_to_mask(buf, available, op_set);

	DIR *dir = opendir(CPUSET);
	xassert(dir);

	for (unsigned i = 0; i < 2; i++)
		readdir(dir); // skip '.' and '..'

	while (1) {
		struct dirent *e = readdir(dir);
		if (!e)
			break;

		if (e->d_type != DT_DIR) // skip non-directories
			continue;

		char buf[64];
		xassert(snprintf(buf, sizeof(buf), CPUSET "/%s/cpuset.cpus", e->d_name) < (int)sizeof(buf));
		int fd = open(buf, O_RDONLY);
		xassert(fd > -1);

		xassert(read(fd, buf, sizeof(buf)) > 0);
		xassert(!close(fd));

		// clear used cores from available mask
		string_to_mask(buf, available, op_clear);
	}

	xassert(!closedir(dir));
	unsigned count = CPU_COUNT_S(masksize, available);
	if (!count) {
		fprintf(stderr, "No cores available\n");
		exit(1);
	}

	// if not specified, use all available cores
	if (req_cores) {
		if (req_cores > count) {
			fprintf(stderr, "%u cores requested but %u available\n", req_cores, count);
			exit(1);
		}
	} else {
		fprintf(stderr, "Using %u available cores\n", count);
		req_cores = count;
	}

	if (flags & FLAGS_VERBOSE)
		fprintf(stderr, "%u cores over %u nodes available\n", count, nnodes);
}

static void cleanup(void)
{
	if (flags & FLAGS_VERBOSE)
		fprintf(stderr, "cleaning up\n");
	rmdir(base);
}

static void usage(const int code)
{
	fprintf(stderr, "usage: numaplace [-sSvV] -c <cores>[:<stride] cmd [args ...]\n");
	fprintf(stderr, "\t-c, --cores\t\tset number of cores advertised\n");
	fprintf(stderr, "\t-s, --stride\t\tstride of cores to allocate\n");
	fprintf(stderr, "\t-S, --spread\t\tround-robin pagecache over NUMA nodes\n");
	fprintf(stderr, "\t-v, --verbose\t\tshow cores allocated\n");
	fprintf(stderr, "\t-V, --version\t\tshow version\n");
	exit(code);
}

// FIXME: avoid formatted input and copy?
static int update(const char *path, const char *fmt, ...)
{
	char buf[40960];
	snprintf(buf, sizeof(buf), "%s/%s", base, path);

	int fd = open(buf, O_WRONLY);
	xassert(fd > -1);

	va_list args;
	va_start(args, fmt);
	int len = vsnprintf(buf, sizeof(buf), fmt, args);
	xassert(len < (int)sizeof(buf)); // check for truncation
	va_end(args);

	if (flags & FLAGS_VERBOSE)
		fprintf(stderr, "%s -> %s\n", buf, path);

	int ret = write(fd, buf, len);
	xassert(!close(fd));

	// collapse positive length to 0
	return ret == len ? 0 : ret;
}

static void strlcpy(char *dst, const char *src, size_t n)
{
	char c;

	do {
		*dst++ = c = *src++;
		xassert(n--);
	} while (c);
}

static unsigned node_of_core(const unsigned core)
{
	// node+1 stored to tell which aren't read yet
	static uint16_t _node_of_core[MAXCORES];

	// fastpath
	if (_node_of_core[core]) {
printf("F core=%u node=%u\n", core, _node_of_core[core] - 1);
		return _node_of_core[core] - 1;
	}

	char buf[64];
	xassert(snprintf(buf, sizeof(buf), "/sys/devices/system/cpu/cpu%u", core) < (int)sizeof(buf));
	DIR *dir = opendir(buf);
	xassert(dir);

	// find entry with node name eg 'node12'
	struct dirent *e;
	do {
		e = readdir(dir);
		xassert(e);
	} while (strncmp(e->d_name, "node", 4));

	unsigned node = atoi(&e->d_name[4]);
	xassert(node < MAXNODES);
	xassert(!closedir(dir)); // frees string memory

	_node_of_core[core] = node + 1;
	return node;
}

// assumption that cores on a node are contiguous
static void cores_of_node(const unsigned node, unsigned *core_start, unsigned *core_end)
{
	xassert(node < MAXNODES);
	static struct {uint16_t start, end;} _cores_of_node[MAXNODES];
	// fastpath
	if (_cores_of_node[node].end) {
		*core_start = _cores_of_node[node].start;
		*core_end = _cores_of_node[node].end;
		return;
	}

	char buf[64];
	xassert(snprintf(buf, sizeof(buf), "/sys/devices/system/node/node%u/cpulist", node) < (int)sizeof(buf));

	int fd = open(buf, O_RDONLY);
	xassert(fd > -1);

	xassert(read(fd, buf, sizeof(buf)) > 0);
	xassert(!close(fd));
	char *end = buf;

	*core_start = _cores_of_node[node].start = strtoul(end, &end, 10);
	xassert(*end == '-');
	end++; // skip '-'
	*core_end = _cores_of_node[node].end = strtoul(end, NULL, 10);
	xassert(*core_end > 0); // uniprocessor unsupported
}

static void allocate(cpu_set_t *cores, cpu_set_t *mems)
{
	unsigned c = 0; // FIXME: find first available core
	unsigned done = 0;

	// find lowest free core
	while (!CPU_ISSET_S(c, masksize, available)) {
		c++;
		xassert(c < masksize * 8);
	}

	// reserve
/*	CPU_SET_S(c, masksize, cores);
	CPU_SET_S(node_of_core(c), CPU_ALLOC_SIZE(MAXNODES), mems);
	CPU_CLR_S(c, masksize, available);*/

	while (1) {
		// find numa node of core
		unsigned numa = node_of_core(c);

		// find first nearest NUMA node
		unsigned nearest_dist = ~0U, nearest_index = ~0U;
		for (unsigned n = 0; n < nnodes; n++) {
			// if already visited, skip
			if (CPU_ISSET_S(n, CPU_ALLOC_SIZE(MAXNODES), mems))
				continue;

			uint8_t d = dist(numa, n);

			if (d < nearest_dist) {
				nearest_dist = d;
				nearest_index = n;
			}
		}
		xassert(nearest_index != ~0U);

		// allocate cores on nearest node
		unsigned a, b;
		cores_of_node(nearest_index, &a, &b);
printf("c=%u numa=%u nearest_dist=%u nearest_index=%u available=%s\n", c, numa, nearest_dist, nearest_index, mask_string(available));
		for (unsigned c_off = a; c_off <= b; c_off += stride) {
			if (!CPU_ISSET_S(c_off, masksize, available))
				continue;

			CPU_SET_S(c_off, masksize, cores);
			CPU_CLR_S(c_off, masksize, available);
			CPU_SET_S(nearest_index, CPU_ALLOC_SIZE(MAXNODES), mems);
			c = c_off; // update for nearest search
			done++;

//			printf("done=%d available=%s", done, mask_string(available));
//			printf(" cores=%s\n", mask_string(cores));

			if (done == req_cores)
				return;
		}
	}
}

static void drop_privileges(void)
{
	const gid_t newgid = getgid(), oldgid = getegid();
	const uid_t newuid = getuid(), olduid = geteuid();

	if (!olduid)
		setgroups(1, &newgid);

	if (newgid != oldgid)
		sysassert(!setregid(newgid, newgid));

	if (newuid != olduid)
		sysassert(!setreuid(newuid, newuid));
}

int main(const int argc, char *const argv[])
{
	bool spread = 0;

	while (1) {
		int option_index = 0;

		static const struct option long_options[] = {
			{"cores",   required_argument, 0, 'c'},
			{"spread",  no_argument,       0, 'S'},
			{"stride",  required_argument, 0, 's'},
			{"verbose", no_argument,       0, 'v'},
			{"verson",  no_argument,       0, 'V'},
			{0,         0,                 0, 0},
		};

		int c = getopt_long(argc, argv, "+c:svV", long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 'c': {
			// validate
			char *end;
			long val = strtol(optarg, &end, 10);
			if (*end || val < 1) {
				fprintf(stderr, "error: invalid number of cores '%s'\n", optarg);
				return 1;
			}
			req_cores = val;
			break;
		}
		case 's': {
			// validate
			char *end;
			long val = strtol(optarg, &end, 10);
			if (*end || val < 1) {
				fprintf(stderr, "error: invalid stride '%s'\n", optarg);
				return 1;
			}
			stride = val;
			break;
		}
		case 'S':
			spread = 1;
			break;
		case 'v':
			flags |= FLAGS_VERBOSE;
			break;
		case 'V':
			printf("numaplace " VER "\n");
			exit(0);
		default:
			usage(1);
		}
	}

	if (argc - optind < 1)
		usage(1);

	ncores = sysconf(_SC_NPROCESSORS_CONF);
	xassert(ncores >= 1 && ncores <= 8192);

	scan_cores();

	if (getuid() != 0) {
		fprintf(stderr, "numaplace needs to run setuid or as root\n");
		return 1;
	}

	strlcpy(base, CPUSET, sizeof(base));

#ifdef RELEASE_AGENT
	sysassert(!update("release_agent", RELEASE_AGENT));
#else
	char release_agent[PATH_MAX];

	// resolve path to local release-agent
	ssize_t done = readlink("/proc/self/exe", release_agent, sizeof(release_agent));
	sysassert(done > -1);
	release_agent[done] = '\0';
	char *end = strrchr(release_agent, '/') + 1; // skip '/'
	strcpy(end, "numaplace-release");

	// ensure notification on release
	if (access(release_agent, R_OK | X_OK)) {
		fprintf(stderr, "release agent %s not executable\n", release_agent);
		return 1;
	}

	sysassert(!update("release_agent", release_agent));
#endif

	// create parent cpuset
	pid_t pid = getpid();
	snprintf(base, sizeof(base), CPUSET "/numaplace-%d", pid);
	sysassert(!mkdir(base, 0755));
	if (flags & FLAGS_VERBOSE)
		fprintf(stderr, "base is %s\n", base);

	// remove dir if early exit
	atexit(cleanup);

	// optimise application behaviour
	sysassert(!update("cpuset.cpu_exclusive", "1"));
	sysassert(!update("notify_on_release", "1"));
	sysassert(!update("cpuset.sched_load_balance", "1"));
	sysassert(!update("cpuset.sched_relax_domain_level", "0"));

	cpu_set_t *cores = CPU_ALLOC(ncores);
	xassert(cores);
	CPU_ZERO_S(masksize, cores);
	cpu_set_t *mems = CPU_ALLOC(MAXNODES);
	xassert(mems);
	CPU_ZERO_S(CPU_ALLOC_SIZE(MAXNODES), mems);

	allocate(cores, mems);

	List cores_list;
	cores_list.add(cores, ncores);

	int ret = update("cpuset.cpus", cores_list.str());
	if (ret && errno == EINVAL) {
		fprintf(stderr, "failed to allocate cores %s\n", cores_list.str());
		return 1;
	}

	List mems_list;
	mems_list.add(mems, MAXNODES);

	sysassert(!update("cpuset.mems", mems_list.str()));

	sysassert(!update("cpuset.memory_migrate", "1"));
	if (spread)
		sysassert(!update("cpuset.memory_spread_page", "1"));

	// move launcher in
	sysassert(!update("tasks", "%d", getpid()));

	drop_privileges();

	// use batch scheduling priority to increase timeslice
	const struct sched_param params = {0};
	xassert(!sched_setscheduler(0, SCHED_BATCH, &params));

	// optimise OpenMP behaviour
	xassert(!setenv("OMP_PROC_BIND", "TRUE", 1));
	xassert(!setenv("OMP_WAIT_POLICY", "ACTIVE", 1));

	// start user application
	if (flags & FLAGS_VERBOSE) {
		fprintf(stderr, "launching");

		for (int i = optind; i < argc; i++)
			fprintf(stderr, " %s", argv[i]);
		fprintf(stderr, "\n");
	}

	execvp(argv[optind], &argv[optind]);
	perror(argv[optind]);

	return 1;
}
