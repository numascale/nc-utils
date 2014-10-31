// g++ test-router.c -g -o test-router -O3 -Wall && ./test-router

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define XS 2
#define YS 0
#define ZS 0
#define min(x,y) ((x) < (y) ? (x) : (y))
#define max(x,y) ((x) > (y) ? (x) : (y))
#define MAXPATH (16 + 16 + 16)

#define XBAR_COST 1
#define XBAR_USAGE_COST 1
#define LC_COST 1
#define LC_USAGE_COST 1
#define ROUTER_DEBUG 1
#define debugf(l, ...) do { if (l <= ROUTER_DEBUG) fprintf(stderr, __VA_ARGS__); } while (0)
#define SCI(x,y,z) ((x) | ((y) << 4) | ((z) << 8))
#define CONVEX 1

typedef uint16_t sci_t;

static unsigned dnc_node_count = 0;
static uint8_t sizes[3] = {XS, YS, ZS};

class Router {
	bool finished[4096][4096];
	unsigned xbar_usage[4096]; // only used when changing rings
	unsigned lc_usage[4096][6]; // send buffering
	uint8_t route[MAXPATH], bestroute[MAXPATH];
	unsigned bestcost;
	uint8_t possible_lcs;

	uint8_t lcs(const sci_t src, const sci_t dst, const uint8_t lastlc) const;
	void find(const sci_t src, const sci_t dst, const unsigned cost, const unsigned offset, const uint8_t lastlc);
	sci_t neigh(sci_t src, const uint8_t lc) const;
	void update(const sci_t src, const sci_t dst);
	void router(const sci_t src, const sci_t dst);
public:
	Router(void);
	void show_usage(void) const;
};

// increment path usage
void Router::update(const sci_t src, const sci_t dst)
{
	debugf(1, "route %03x > %03x has cost %u:", src, dst, bestcost);
	sci_t sci = src;

	uint8_t last_lc = 0;

	for (unsigned offset = 0; bestroute[offset]; offset++) {
		uint8_t lc = bestroute[offset];
		debugf(1, " %u", lc);
		lc_usage[sci][lc - 1]++;

		if (last_lc != lc) // ring change, Xbar involved
			xbar_usage[sci]++;

		sci = neigh(sci, lc);
	}

	debugf(1, "\n");

	assert(sci == dst);
}

// return bitmask of possible LCs, restricting to any adjacent axes
uint8_t Router::lcs(const sci_t src, const sci_t dst, const uint8_t lastlc) const
{
	uint8_t ret = possible_lcs;
#ifdef CONVEX
	// if already on a ring, stay in ring until adjacent in another axis
	if (lastlc) {
		const uint8_t axis = (lastlc - 1) / 2; // 0-2
		if (((src >> (axis * 4)) & 0xf) != ((dst >> (axis * 4)) & 0xf))
			return 1 << lastlc;
	}
#else
	// if adjacent on any ring
	for (unsigned i = 0; i < 3; i++)
		if (((src >> (i * 4)) & 0xf) == ((dst >> (i * 4)) & 0xf))
			ret &= ~(3 << (i * 2 + 1));
#endif
	return ret;
}

// calculate optimal route
 void Router::find(const sci_t src, const sci_t dst, const unsigned cost, const unsigned offset, const uint8_t lastlc)
{
	debugf(2, "<pos=%03x cost=%u offset=%u>", src, cost, offset);

	// long route
	if (cost >= bestcost) {
		debugf(3, " high cost %u\n", cost);
		return;
	}
	
	if (offset > dnc_node_count) {
		debugf(3, " too distant %u\n", offset);
		return;
	}

	// if reached goal, update best
	if (src == dst) {
		memcpy(bestroute, route, offset * sizeof(route[0]));
		bestroute[offset] = 0;
		bestcost = cost;
		debugf(3, " goal, cost=%u\n", cost);
		return;
	}

	const uint8_t llcs = lcs(src, dst, lastlc);
	debugf(3, " lcs=0x%x\n", llcs);
	for (uint8_t lc = 1; lc <= 7; lc++) {
		// skip unavailable LCs 
		if (!(llcs & (1 << lc)))
			continue;

		const sci_t next = neigh(src, lc);
		route[offset] = lc;

		unsigned newcost = cost + LC_COST + lc_usage[src][lc - 1] * LC_USAGE_COST;
		if (lc != lastlc)
			newcost += XBAR_COST + xbar_usage[src] * XBAR_USAGE_COST;

		find(next, dst, newcost, offset + 1, lc);
	}
}

sci_t Router::neigh(sci_t src, const uint8_t lc) const
{
	const uint8_t axis = (lc - 1) / 2; // 0-2
	int pos = (src >> (axis * 4)) & 0xf;
	const bool dir = (lc - 1) & 1;

	if (dir) {
		if (--pos == -1)
			pos = sizes[axis];
	} else
		pos = (pos + 1) % sizes[axis];

	src &= ~(0xf << (axis * 4));
	src |= pos << (axis * 4);
	return src;
}

void Router::router(const sci_t src, const sci_t dst)
{
	if (finished[src][dst])
		return;

	bestcost = ~0U;
	debugf(2, "%03x > %03x\n", src, dst);
	find(src, dst, 0, 0, 0);
	update(src, dst);

	finished[src][dst] = 1;
	finished[dst][src] = 1;
}

Router::Router(void): possible_lcs(0)
{
	memset(finished, 0, sizeof(finished));
	memset(bestroute, 0, sizeof(bestroute));
	memset(route, 0, sizeof(route));
	memset(xbar_usage, 0, sizeof(xbar_usage));
	memset(lc_usage, 0, sizeof(lc_usage));

	for (unsigned i = 0; i < 3; i++)
		if (sizes[i])
			possible_lcs |= 3 << (i * 2 + 1);

	for (unsigned sz = 0; sz < max(ZS, 1); sz++)
		for (unsigned sy = 0; sy < max(YS, 1); sy++)
			for (unsigned sx = 0; sx < max(XS, 1); sx++)
				for (unsigned dz = 0; dz < max(ZS, 1); dz++)
					for (unsigned dy = 0; dy < max(YS, 1); dy++)
						for (unsigned dx = 0; dx < max(XS, 1); dx++)
							router(SCI(sx, sy, sz), SCI(dx, dy, dz));
}

void Router::show_usage(void) const
{
	printf("\nUsage:\n");
	for (unsigned z = 0; z < max(ZS, 1); z++) {
		for (unsigned y = 0; y < max(YS, 1); y++) {
			for (unsigned x = 0; x < max(XS, 1); x++) {
				printf("%03x xbar %5u, lcs:", SCI(x, y, z), xbar_usage[SCI(x, y, z)]);
				for (uint8_t i = 1; i <= 6; i++)
					if (possible_lcs & (1 << i))
						printf(" %3u", lc_usage[SCI(x, y, z)][i - 1]);
				printf("\n");
			}
		}
	}
}

int main(void)
{
	dnc_node_count = max(XS, 1) * max(YS, 1) * max(ZS, 1);
	Router *r = new Router();
	r->show_usage();
	delete r;

	return 0;
}
