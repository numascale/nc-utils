// g++ test-router.c -g -o test-router -O3 -Wall && ./test-router

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define XS 3
#define YS 3
#define ZS 0
#define min(x,y) ((x) < (y) ? (x) : (y))
#define max(x,y) ((x) > (y) ? (x) : (y))
#define MAXPATH (16 + 16 + 16)

#define XBAR_COST 1
#define XBAR_USAGE_COST 1
#define LC_COST 1
#define LC_USAGE_COST 1
#define ROUTER_DEBUG 2
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
	uint8_t possible_lcs;
	unsigned bestcost;
	sci_t src, dst;

	uint8_t lcs(const sci_t pos, const uint8_t available_lcs, const uint8_t lastlc) const;
	sci_t neigh(sci_t pos, const uint8_t lc) const;
	void find(const sci_t pos, const unsigned cost, const unsigned offset, const uint8_t available_lcs, const uint8_t lastlc);
	void dump();
	void update();
	void router(const sci_t _src, const sci_t _dst);
public:
	Router();
	void show_usage() const;
};

void Router::dump()
{
	sci_t pos = src;

	for (unsigned offset = 0; route[offset]; offset++) {
		uint8_t lc = route[offset];
		debugf(3, " %u", lc);
		pos = neigh(pos, lc);
	}

	debugf(3, "\n");
}

// increment path usage
void Router::update()
{
	if (bestcost == ~0U) {
		debugf(1, "route %03x > %03x failed to route\n", src, dst);
		return;
	}

	debugf(2, "route %03x > %03x has cost %u:", src, dst, bestcost);
	sci_t pos = src;
	uint8_t last_lc = 0;

	for (unsigned offset = 0; bestroute[offset]; offset++) {
		uint8_t lc = bestroute[offset];
		debugf(2, " %u", lc);
		lc_usage[pos][lc - 1]++;

		if (last_lc != lc) // ring change, Xbar involved
			xbar_usage[pos]++;

		pos = neigh(pos, lc);
	}

	debugf(2, "\n");

	assert(pos == dst);
}

// return bitmask of possible LCs, restricting to any adjacent axes
uint8_t Router::lcs(const sci_t pos, const uint8_t available_lcs, const uint8_t lastlc) const
{
#ifdef CONVEX
	// if already on a ring, stay in ring until adjacent in another axis
	if (lastlc) {
		const uint8_t axis = (lastlc - 1) / 2; // 0-2
		if (((pos >> (axis * 4)) & 0xf) != ((dst >> (axis * 4)) & 0xf))
			return 1 << lastlc;
	}
#else
	// if adjacent on any ring
	for (unsigned i = 0; i < 3; i++)
		if (((pos >> (i * 4)) & 0xf) == ((dst >> (i * 4)) & 0xf))
			available_lcs &= ~(3 << (i * 2 + 1));
#endif
	return available_lcs;
}

sci_t Router::neigh(sci_t pos, const uint8_t lc) const
{
	const uint8_t axis = (lc - 1) / 2; // 0-2
	int ring_pos = (pos >> (axis * 4)) & 0xf;
	const bool dir = (lc - 1) & 1;

	debugf(4, "<neigh: pos=0x%03x lc=%u axis=%u", pos, lc, axis);
	if (dir) {
		if (--ring_pos == -1)
			ring_pos = sizes[axis] - 1;
	} else
		ring_pos = (ring_pos + 1) % sizes[axis];

	pos &= ~(0xf << (axis * 4));
	pos |= ring_pos << (axis * 4);

	debugf(4, " pos=%03x>", pos);
	return pos;
}

// calculate optimal route
 void Router::find(const sci_t pos, const unsigned cost, const unsigned offset, const uint8_t available_lcs, const uint8_t lastlc)
{
	debugf(3, "<find: pos=%03x cost=%u offset=%u available=0x%x>", pos, cost, offset, available_lcs);

	// long route
	if (cost >= bestcost) {
		debugf(3, " high cost %u:", cost);
		dump();
		return;
	}
	
	if (offset > dnc_node_count) {
		debugf(3, " too distant %u:", offset);
		dump();
		return;
	}

	// if reached goal, update best
	if (pos == dst) {
		memcpy(bestroute, route, offset * sizeof(route[0]));
		bestroute[offset] = 0;
		bestcost = cost;
		debugf(4, " goal, cost=%u\n", cost);
		return;
	}

	const uint8_t llcs = lcs(pos, available_lcs, lastlc);
	for (uint8_t lc = 1; lc <= 7; lc++) {
		// skip unavailable LCs 
		if (!(llcs & (1 << lc)))
			continue;

		const sci_t next = neigh(pos, lc);
		route[offset] = lc;

		unsigned newcost = cost + LC_COST + lc_usage[pos][lc - 1] * LC_USAGE_COST;
		if (lc != lastlc)
			newcost += XBAR_COST + xbar_usage[pos] * XBAR_USAGE_COST;

		find(next, newcost, offset + 1, available_lcs & ~(1 << lc), lc);
	}

	debugf(4, "\n");
}

void Router::router(const sci_t _src, const sci_t _dst)
{
	src = _src;
	dst = _dst;

	uint8_t available_lcs = 0;

	for (uint8_t i = 0; i < 3; i++)
		if ((sizes[i]) && (src ^ dst) & (0xf << (i * 4)))
			available_lcs |= 3 << (i * 2 + 1);

	bestcost = ~0U;
	debugf(3, "%03x > %03x\n", src, dst);
	find(src, 0, 0, available_lcs, 0);
	update();

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
