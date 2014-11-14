// g++ test-router.c -g -o test-router -O3 -Wall && ./test-router

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define min(x,y) ((x) < (y) ? (x) : (y))
#define max(x,y) ((x) > (y) ? (x) : (y))
#define MAXPATH (16 + 16 + 16)

#define XBAR_COST 1
#define XBAR_USAGE_COST 1
#define LC_COST 1
#define LC_USAGE_COST 1
#define ROUTER_DEBUG 1
#define debugf(l, ...) do { if (l <= ROUTER_DEBUG) printf(__VA_ARGS__); } while (0)
#define SCI(x,y,z) ((x) | ((y) << 4) | ((z) << 8))

typedef uint16_t sci_t;

static unsigned dnc_node_count = 0;
static uint8_t size[3] = {3, 3, 0};

class Router {
	bool lcs_disallowed[4096][6];
	bool finished[4096][4096];
	unsigned xbar_usage[4096]; // only used when changing rings
	unsigned lc_usage[4096][6]; // send buffering
	uint8_t route[MAXPATH], bestroute[MAXPATH];
	uint8_t possible_lcs;
	unsigned bestcost;
	sci_t src, dst;

	void find(const sci_t pos, unsigned cost, const unsigned offset, const uint8_t available_lcs);
	void update(void);
	void router(const sci_t _src, const sci_t _dst);
public:
	Router(void);
	void run(void);
	void disable_node(const uint16_t sci);
	void show_usage(void) const;
};

// increment path usage
void Router::update(void)
{
	if (bestcost == ~0U) {
		debugf(1, "route %03x > %03x failed to route\n", src, dst);
		return;
	}

	debugf(2, "route %03x > %03x has cost %u:", src, dst, bestcost);
	sci_t pos = src;

	for (unsigned offset = 0; bestroute[offset]; offset++) {
		uint8_t lc = bestroute[offset];
		debugf(2, " lc=%u", lc);

		uint8_t axis = (lc - 1) / 2;

		// tally whole ring as response has to continue on same rings
		for (unsigned ring_pos = 0; ring_pos < size[axis]; ring_pos++) {
			sci_t sci = (pos & ~(0xf << (axis * 4))) | (ring_pos << (axis * 4));
			lc_usage[sci][lc - 1]++;
		}

		// move pos to next ring change
		pos &= ~(0xf << (axis * 4));
		pos |= dst & (0xf << (axis * 4));
	}

	debugf(2, " pos=%03x\n", pos);
	assert(pos == dst);
}

void Router::find(const sci_t pos, unsigned cost, const unsigned offset, const uint8_t available_lcs)
{
	if (cost >= bestcost) {
		debugf(3, "<overcost>\n");
		return;
	}

	if (pos == dst) {
		memcpy(bestroute, route, offset * sizeof(route[0]));
		bestroute[offset] = 0;
		bestcost = cost;
		debugf(3, "goal @ cost %u\n", cost);
		return;
	}

	for (unsigned lc = 1; lc <= 6; lc++) {
		if (!(available_lcs & (1 << lc)) || lcs_disallowed[pos][lc])
			continue;

		const uint8_t axis = (lc - 1) / 2;
		bool skip = 0;

		debugf(3, "<offset=%u", offset);
		// tally whole ring as response has to continue on same rings
		for (unsigned ring_pos = 0; ring_pos < size[axis]; ring_pos++) {
			sci_t sci = (pos & ~(0xf << (axis * 4))) | (ring_pos << (axis * 4));
			debugf(2, " %03x", sci);

			if (lcs_disallowed[sci][lc - 1]) {
				skip = 1;
				break;
			}

			cost += LC_COST + lc_usage[sci][lc - 1] * LC_USAGE_COST;
		}

		if (skip)
			continue;

		// tally ring change
		cost += XBAR_COST + xbar_usage[pos] * XBAR_USAGE_COST;

		// move pos to next ring change
		sci_t next = pos & ~(0xf << (axis * 4));
		next |= dst & (0xf << (axis * 4));

		route[offset] = lc;
		debugf(3, " pos=0x%03x lc=%u>", pos, lc);
		find(next, cost, offset + 1, available_lcs & ~(3 << lc));
	}
}

void Router::router(const sci_t _src, const sci_t _dst)
{
	src = _src;
	dst = _dst;

	if (finished[src][dst])
		return;

	uint8_t available_lcs = 0;

	for (uint8_t i = 0; i < 3; i++)
		if ((size[i]) && (src ^ dst) & (0xf << (i * 4)))
			available_lcs |= 3 << (i * 2 + 1);

	bestcost = ~0U;
	debugf(3, "%03x > %03x via 0x%x\n", src, dst, available_lcs);
	find(src, 0, 0, available_lcs);
	update();

	// route back already calculated
	finished[dst][src] = 0;
}

Router::Router(void): possible_lcs(0)
{
	memset(finished, 0, sizeof(finished));
	memset(bestroute, 0, sizeof(bestroute));
	memset(route, 0, sizeof(route));
	memset(xbar_usage, 0, sizeof(xbar_usage));
	memset(lc_usage, 0, sizeof(lc_usage));
	memset(lcs_disallowed, 0, sizeof(lcs_disallowed));

	for (unsigned i = 0; i < 3; i++)
		if (size[i])
			possible_lcs |= 3 << (i * 2 + 1);
}

void Router::run(void)
{
	for (unsigned sz = 0; sz < max(size[2], 1); sz++)
		for (unsigned sy = 0; sy < max(size[1], 1); sy++)
			for (unsigned sx = 0; sx < max(size[0], 1); sx++)
				for (unsigned dz = 0; dz < max(size[2], 1); dz++)
					for (unsigned dy = 0; dy < max(size[1], 1); dy++)
						for (unsigned dx = 0; dx < max(size[0], 1); dx++)
							router(SCI(sx, sy, sz), SCI(dx, dy, dz));
}

void Router::disable_node(const uint16_t sci)
{
	for (unsigned lc = 0; lc < 6; lc++)
		lcs_disallowed[sci][lc] = 1;
}

void Router::show_usage(void) const
{
	printf("\nUsage:\n");
	for (unsigned z = 0; z < max(size[2], 1); z++) {
		for (unsigned y = 0; y < max(size[1], 1); y++) {
			for (unsigned x = 0; x < max(size[0], 1); x++) {
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
	dnc_node_count = max(size[0], 1) * max(size[1], 1) * max(size[2], 1);

	Router *r = new Router();
	r->disable_node(0x011);
	r->run();
	r->show_usage();
	delete r;

	return 0;
}
