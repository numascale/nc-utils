/*
 * Copyright (C) 2008-2012 Numascale AS, support@numascale.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "dnc-regs.h"
#include "dnc-defs.h"
#include "dnc-access.h"
#include "dnc-commonlib.h"
#include "dnc-masterlib.h"
#include "dnc-bootloader.h"
#include "dnc-mmio.h"
#include "dnc-maps.h"
#include "dnc-devices.h"
#include "dnc-platform.h"

static uint64_t mmio_cur;
static uint64_t tmp_base[2], tmp_lim[2];

void *operator new(const size_t size)
{
    return malloc(size);
}

void *operator new[](const size_t size)
{
    return malloc(size);
}

void operator delete(void *const p)
{
    free(p);
}

void operator delete[](void *const p)
{
    free(p);
}

void dump_device(const sci_t sci, const int bus, const int dev, const int fn)
{
	printf("\nPCI device SCI%03x.%02x:%02x.%x:\n", sci, bus, dev, fn);
	for (int offset = 0; offset <= 0x3c; offset += 4)
		printf("%08x\n", dnc_read_conf(sci, bus, dev, fn, offset));
	printf("\n");
}

static bool size_bar(const uint16_t sci, const int bus, const int dev, const int fn, const int reg, uint64_t *addr, uint64_t *len, bool *pref, bool *io)
{
	uint32_t cmd = dnc_read_conf(sci, bus, dev, fn, 4);
	dnc_write_conf(sci, bus, dev, fn, 4, 0);

	uint32_t save = dnc_read_conf(sci, bus, dev, fn, reg);
	*io = save & 1;
	uint32_t mask = *io ? 1 : 15;
	bool s64 = ((save >> 1) & 3) == 2;
	*pref = (save >> 3) & 1;

	dnc_write_conf(sci, bus, dev, fn, reg, 0xffffffff);
	uint32_t val = dnc_read_conf(sci, bus, dev, fn, reg);

	/* Skip unimplemented BARs */
	if (val == 0x00000000 || val == 0xffffffff) {
		*len = 0;
		*addr = 0;
		goto out;
	}

	*len = val & ~mask;
	*addr = save & ~mask;

	if (s64) {
		uint32_t save2 = dnc_read_conf(sci, bus, dev, fn, reg + 4);
		*addr |= (uint64_t)save2 << 32;
		dnc_write_conf(sci, bus, dev, fn, reg + 4, 0xffffffff);
		*len |= (uint64_t)dnc_read_conf(sci, bus, dev, fn, reg + 4) << 32;
		dnc_write_conf(sci, bus, dev, fn, reg + 4, save2);
	}

	*len &= ~(*len - 1);
	if (*len)
		printf(" 0x%llx,%s%s", *addr, pr_size(*len), *pref ? "(P)" : "");

out:
	dnc_write_conf(sci, bus, dev, fn, reg, save);
	dnc_write_conf(sci, bus, dev, fn, 4, cmd);
	/* Return is there could be a second BAR */
	return !s64;
}

static void assign_bar(const uint16_t sci, const int bus, const int dev, const int fn, const int reg, uint64_t addr)
{
	uint32_t val = dnc_read_conf(sci, bus, dev, fn, reg);
	bool s64 = ((val >> 1) & 3) == 3;

	dnc_write_conf(sci, bus, dev, fn, reg, addr & 0xffffffff);
	if (s64)
		dnc_write_conf(sci, bus, dev, fn, reg + 4, addr >> 32);
	else
		assert(addr < 0xffffffff);
	printf(":0x%llx", addr);
}

static bool scope_bar(const uint16_t sci, const int bus, const int dev, const int fn, const int reg)
{
	uint64_t len, addr;
	bool pref, io, more = size_bar(sci, bus, dev, fn, reg, &addr, &len, &pref, &io);

	if (!io && len) {
		if (addr < tmp_base[pref])
			tmp_base[pref] = addr;

		if ((addr + len) > tmp_lim[pref])
			tmp_lim[pref] = addr + len;
	}

	return more;
}

static bool setup_bar(const uint16_t sci, const int bus, const int dev, const int fn, const int reg)
{
	uint64_t len, addr;
	bool pref, io, more = size_bar(sci, bus, dev, fn, reg, &addr, &len, &pref, &io);

	/* FIXME: Can't handle prefetchable memory ranges for now */
	assert(!pref);

	/* Disable slave IO BARs */
	if (io) {
		printf("[dis]");
		dnc_write_conf(sci, bus, dev, fn, reg, 0);
		return more;
	}

	if (len) {
		/* BARs are aligned to their size, or page */
		mmio_cur = roundup(mmio_cur, max(len, 4096));
		assign_bar(sci, bus, dev, fn, reg, mmio_cur);
		mmio_cur += len;
	}

	return more;
}

static void pci_search(const uint16_t sci, const int bus, const bool scope,
	bool (*barfn)(const uint16_t sci, const int bus, const int dev, const int fn, const int reg))
{
	for (int dev = 0; dev < (bus == 0 ? 24 : 32); dev++) {
		for (int fn = 0; fn < 8; fn++) {
			uint32_t val = dnc_read_conf(sci, bus, dev, fn, 0xc);
			/* PCI device functions are not necessarily contiguous */
			if (val == 0xffffffff)
				continue;

			uint8_t type = val >> 16;
			printf("%s @ %02x:%02x.%x:", (type & 0x7f) == 0 ? " - endpoint" : "bridge", bus, dev, fn);

			if (!scope) {
				/* Disable IO response */
				val = dnc_read_conf(sci, bus, dev, fn, 0x4);
				dnc_write_conf(sci, bus, dev, fn, 0x4, val & ~(1 | (1 << 10)));

				if ((type & 0x7f) == 0x00) {
					/* Disable device EEPROM */
					if ((type & 0x7f) == 0x00)
						dnc_write_conf(sci, bus, dev, fn, 0x30, 0);

#ifdef EXPERIMENTAL
					/* Set interrupt line to unconnected */
					val = dnc_read_conf(sci, bus, dev, fn, 0x3c);
					dnc_write_conf(sci, bus, dev, fn, 0x3c, (val & ~0xffff) | 0x00ff);
#endif

					/* If no MSI capability, disable device and continue */
					if (capability(sci, PCI_CAP_MSI, bus, dev, fn) == PCI_CAP_NONE) {
						printf(" [no MSI] ");
						disable_device(sci, bus, dev, fn);
						goto out;
					}
				}
			}

			if (barfn(sci, bus, dev, fn, 0x10))
				barfn(sci, bus, dev, fn, 0x14);

			if ((type & 0x7f) == 0) { /* Device */
				if (barfn(sci, bus, dev, fn, 0x18))
					barfn(sci, bus, dev, fn, 0x1c);
				if (barfn(sci, bus, dev, fn, 0x20))
					barfn(sci, bus, dev, fn, 0x24);
			}

			printf("\n");

			/* Recurse down bridges */
			if ((type & 0x7f) == 0x01) {
				int sec = (dnc_read_conf(sci, bus, dev, fn, 0x18) >> 8) & 0xff;

				if (scope) {
					pci_search(sci, sec, scope, barfn);

					for (int i = 0; i < 2; i++) {
						/* Round up limit */
						if (tmp_lim[i])
							tmp_lim[i] = roundup(tmp_lim[i], NC_ATT_MMIO32_GRAN) - 1;

						printf("window %d: 0x%08llx:0x%08llx\n", i, tmp_base[i], tmp_lim[i]);

						if (tmp_base[i] < nodes[0].mmio32_base)
							nodes[0].mmio32_base = tmp_base[i];
						if (tmp_lim[i] > nodes[0].mmio32_limit)
							nodes[0].mmio32_limit = tmp_lim[i];
					}
				} else {
					mmio_cur = roundup(mmio_cur, NC_ATT_MMIO32_GRAN);

					assert(mmio_cur < MMIO32_LIMIT);
					uint32_t bridge_start = mmio_cur & 0xffffffff;
					pci_search(sci, sec, scope, barfn);
					mmio_cur = roundup(mmio_cur, NC_ATT_MMIO32_GRAN);
					assert(mmio_cur < MMIO32_LIMIT);
					uint32_t bridge_end = (mmio_cur - 1) & 0xffffffff;

					if (bridge_end > bridge_start) {
						printf("window: 0x%08x:0x%08x\n", bridge_start, bridge_end);
						val = ((roundup(bridge_end, NC_ATT_MMIO32_GRAN) - 1) & ~0xfffff) | ((bridge_start >> 16) & ~0xf);
						dnc_write_conf(sci, bus, dev, fn, 0x20, val);
					} else {
						printf("window: disabled\n");
						dnc_write_conf(sci, bus, dev, fn, 0x20, 0x0000ffff);
					}
					/* FIXME: Can't handle prefetchable range for now */
					dnc_write_conf(sci, bus, dev, fn, 0x24, 0x0000ffff);
					dnc_write_conf(sci, bus, dev, fn, 0x28, 0x0);
					dnc_write_conf(sci, bus, dev, fn, 0x2c, 0x0);

					/* Disable IO base/limit and expansion ROM */
					val = dnc_read_conf(sci, bus, dev, fn, 0x1c);
					dnc_write_conf(sci, bus, dev, fn, 0x1c, (val & ~0xffff) | 0xff00);
					dnc_write_conf(sci, bus, dev, fn, 0x30, 0x0000ffff);
					dnc_write_conf(sci, bus, dev, fn, 0x38, 0);
				}
			}
out:
			/* If not multi-function, break out of function loop */
			if (!fn && !(type & 0x80))
				break;
		}
	}
}

////////////////////

#define UNITS 32

static uint64_t io_cur, mmio64_cur;

struct range {
	uint32_t start, end;
};

class Map {
public:
	struct range excluded[UNITS];
	int ranges;
	uint64_t next;

	Map(void) {
		next = rdmsr(MSR_TOPMEM);

		/* Exclude APICs and magic at 0xf0000000 */
		excluded[0].start = 0xf0000000;
		excluded[0].end = 0xffffffff;
		ranges = 1;
	}

	bool merge(void) {
		for (int range = 1; range < ranges; range++) {
			if ((excluded[range].start - excluded[range - 1].end > MMIO_MIN_GAP) &&
			  (excluded[range].end < excluded[range - 1].start || excluded[range].start > excluded[range - 1].end))
				continue;

			excluded[range - 1].start = min(excluded[range - 1].start, excluded[range].start);
			excluded[range - 1].end = max(excluded[range - 1].end, excluded[range].end);
			memmove(&excluded[range], &excluded[range + 1], (ranges - range - 1) * sizeof(excluded[0]));
			ranges--;
			return 1;
		}

		return 0;
	}

	void exclude(const uint32_t start, const uint32_t len) {
		assert(len);

		/* Find appropriate position */
		int range = 0;
		while (range < ranges && start > excluded[range].start)
			range++;

		/* Move any later elements down */
		if (ranges > range)
			memmove(&excluded[range + 1], &excluded[range], (ranges - range) * sizeof(excluded[0]));
		assert(ranges++ < UNITS);

		/* Insert */
		excluded[range].start = start;
		excluded[range].end = start + len - 1;

		while (merge());
	}

	uint32_t overlap(const uint32_t start, const uint32_t len) {
		const uint32_t end = start + len;

		/* If overlap, return amount */
		for (int i = 0; i < ranges; i++)
			if (end > excluded[i].start && start < excluded[i].end)
				return end - excluded[i].start;

		/* No overlap */
		return 0;
	}

	void dump(void) {
		printf("MMIO ranges excluded from master:\n");
		for (int i = 0; i < ranges; i++)
			printf("- 0x%x:0x%x\n", excluded[i].start, excluded[i].end);
	}
};

static Map *map32;

class BAR {
	const sci_t sci;
	const int bus, dev, fn, reg;
public:
	bool io, s64, pref;
	uint64_t assigned, len;
	BAR(const sci_t _sci, const int _bus, const int _dev, const int _fn, const int _reg)
		: sci(_sci), bus(_bus), dev(_dev), fn(_fn), reg(_reg) {
		uint32_t cmd = dnc_read_conf(sci, bus, dev, fn, 4);
		dnc_write_conf(sci, bus, dev, fn, 4, 0);

		uint32_t save = dnc_read_conf(sci, bus, dev, fn, reg);
		io = save & 1;
		uint32_t mask = io ? 1 : 15;
		assigned = save & ~mask;
		s64 = ((save >> 1) & 3) == 2;
		pref = (save >> 3) & 1;

		dnc_write_conf(sci, bus, dev, fn, reg, 0xffffffff);
		uint32_t val = dnc_read_conf(sci, bus, dev, fn, reg);

		/* Skip unimplemented BARs */
		if (val == 0x00000000 || val == 0xffffffff) {
			len = 0;
			goto out;
		}

		len = val & ~mask;

		if (s64) {
			uint32_t save2 = dnc_read_conf(sci, bus, dev, fn, reg + 4);
			assigned |= (uint64_t)save2 << 32;
			dnc_write_conf(sci, bus, dev, fn, reg + 4, 0xffffffff);
			len |= (uint64_t)dnc_read_conf(sci, bus, dev, fn, reg + 4) << 32;
			dnc_write_conf(sci, bus, dev, fn, reg + 4, save2);
		}

		len &= ~(len - 1);
		if (len)
			printf(" %s:%s%s%s @ 0x%llx", pr_size(len), io ? "IO" : "MMIO", s64 ? "64" : "", pref ? "P" : "", assigned);
out:
		dnc_write_conf(sci, bus, dev, fn, reg, save);
		dnc_write_conf(sci, bus, dev, fn, 4, cmd);
	}

	bool allocate(uint64_t *addr) {
		assert(len);

		if (!s64 && pref)
			warning("Prefetchable BAR at 0x%x on SCI%03x %02x:%02x.%d using 32-bit addressing", reg, sci, bus, dev, fn);

		if (!s64 && *addr >= MMIO32_LIMIT) {
			*addr = 0;
			warning("Unable to allocate SCI%03x %02x:%02x.%x BAR 0x%x", sci, bus, dev, fn, reg);
		} else {
			if (io)
				*addr = roundup(*addr, max(len, 16));
			else
				/* MMIO BARs are aligned from page size to their size */
				*addr = roundup(*addr, max(len, 4096));

			/* If space is allocated, move past and get parent to retry */
			uint64_t skip = map32->overlap(*addr, len);
			if (skip) {
				*addr += skip;
				return 1;
			}
		}

		assigned = *addr;
		printf("SCI%03x %02x:%02x.%x allocating %s BAR 0x%x at 0x%llx\n", sci, bus, dev, fn, pr_size(len), reg, assigned);
		dnc_write_conf(sci, bus, dev, fn, reg, assigned);
		if (s64)
			dnc_write_conf(sci, bus, dev, fn, reg + 4, *addr >> 32);

		*addr += len;
		return 0;
	}
};

class BarList {
public:
	BAR *elements[UNITS];
	int used;
	BarList(void): used(0) {};

	/* Order BARs by descending size for optimal packing */
	void insert(BAR *bar) {
		assert(bar->len);

		int i = 0;

		/* Locate next smaller BAR */
		while (i < used && elements[i]->len >= bar->len)
			i++;

		/* Ensure space, move down and insert */
		assert((used + 1) < UNITS);
		memmove(&elements[i + 1], &elements[i], (used - i) * sizeof(*elements));
		elements[i] = bar;
		used++;
	}
};

class Container {
	Container *containers[UNITS];
	Container **container;
	BarList io_bars, mmio32_bars, mmio64_bars;
	int bus;
	const int pbus, pdev, pfn;

	void device(const int dev, const int fn) {
		printf(" > dev %x:%02x.%x", bus, dev, fn);

		for (int offset = 0x10; offset <= 0x30; offset += 4) {
			/* Skip gap between last BAR and expansion ROM address */
			if (offset == 0x28)
				offset = 0x30;

			BAR *bar = new BAR(node->sci, bus, dev, fn, offset);
			if (bar->len == 0) {
				delete bar;
				continue;
			}

			if (bar->s64 && bar->pref) {
				mmio64_bars.insert(bar);

				/* Skip second register of 64-bit BAR */
				offset += 4;
			} else if (bar->io)
				io_bars.insert(bar);
			else
				mmio32_bars.insert(bar);
		}
		printf("\n");
	}

public:
	node_info_t *const node;
	uint64_t window_io, window32, window64; /* Length of MMIO window needed */

	Container(node_info_t *const _node, const int _pbus, const int _pdev, const int _pfn):
		pbus(_pbus), pdev(_pdev), pfn(_pfn), node(_node) {
		container = containers;

		bus = (dnc_read_conf(node->sci, pbus, pdev, pfn, 0x18) >> 8) & 0xff;
		printf("- bus %x\n", bus);
		const int limit = bus == 0 ? 24 : 32;

		for (int dev = 0; dev < limit; dev++) {
			for (int fn = 0; fn < 8; fn++) {
				uint32_t val = dnc_read_conf(node->sci, bus, dev, fn, 0xc);
				/* PCI device functions are not necessarily contiguous */
				if (val == 0xffffffff)
					continue;

				uint8_t type = val >> 16;
				if ((type & 0x7f) == 1) {
					*container = new Container(node, bus, dev, fn);
					container++;
				} else {
					assert((type & 0x7f) == 0);
					device(dev, fn);
				}

				/* If not multi-function, break out of function loop */
				if (!fn && !(type & 0x80))
					break;
			}
		}

		/* Disable IO on slaves; drivers will enable it if needed */
		if (node->sci) {
			uint32_t val = dnc_read_conf(node->sci, pbus, pdev, pfn, 4);
			dnc_write_conf(node->sci, pbus, pdev, pfn, 4, val & ~1);
		}
	}

	/* Exclude existing allocated ranges from map */
	void exclude(void) {
		for (BAR **bar = mmio32_bars.elements; bar < mmio32_bars.elements + mmio32_bars.used; bar++)
			map32->exclude((*bar)->assigned, (*bar)->len);

		for (BAR **bar = mmio64_bars.elements; bar < mmio64_bars.elements + mmio64_bars.used; bar++) {
			assert((*bar)->assigned < MMIO32_LIMIT);
			map32->exclude((*bar)->assigned, (*bar)->len);
		}

		for (Container **c = containers; c < container; c++)
			(*c)->exclude();
	}

	/* Set BAR addresses; return 1 if ran into an exclusion zone; caller will retry */
	bool allocate(void) {
		bool retry = 0;

		map32->next = roundup(map32->next, 1 << NC_ATT_MMIO32_GRAN);
		io_cur = roundup(io_cur, 1 << 12);

		uint32_t io_start = io_cur, mmio32_start = map32->next;
		uint64_t mmio64_start = mmio64_cur;

		/* Only reallocate 32-bit BARs on slaves */
		if (node->sci)
			for (BAR **bar = mmio32_bars.elements; bar < mmio32_bars.elements + mmio32_bars.used; bar++) {
				retry |= (*bar)->allocate(&map32->next);
				if (retry)
					return 1;
			}

		/* Allocate child containers, retrying until allocation succeeds */
		for (Container **c = containers; c < container; c++)
			while ((*c)->allocate())
				printf("retrying allocation\n");

		if (node->sci)
			for (BAR **bar = mmio64_bars.elements; bar < mmio64_bars.elements + mmio64_bars.used; bar++)
				assert(!(*bar)->allocate(&mmio64_cur));

		/* Only reallocate IO BARs on slaves */
		if (node->sci)
			for (BAR **bar = io_bars.elements; bar < io_bars.elements + io_bars.used; bar++)
				assert(!(*bar)->allocate(&io_cur));

		/* Align start of bridge windows */
		map32->next = roundup(map32->next, 1 << NC_ATT_MMIO32_GRAN);
		mmio64_cur = roundup(mmio64_cur, SCC_ATT_GRAN << DRAM_MAP_SHIFT);
		io_cur = roundup(io_cur, 1 << 12);

		uint32_t sec = (dnc_read_conf(node->sci, pbus, pdev, pfn, 0x18) >> 8) & 0xff;
		printf("SCI%03x %02x:%02x.%x bridge to %d configured for IO 0x%x:0x%llx, MMIO32 0x%x:0x%llx, MMIO64 0x%llx:0x%llx\n",
			node->sci, pbus, pdev, pfn, sec, io_start, io_cur - 1, mmio32_start, map32->next - 1, mmio64_start, mmio64_cur - 1);

		uint32_t val;

		/* Only re-allocate IO and 32-bit BARs on slaves */
		if (node->sci) {
			/* Disable IO, clear bridge BARs, expansion ROM */
			val = dnc_read_conf(node->sci, pbus, pdev, pfn, 4);
			dnc_write_conf(node->sci, pbus, pdev, pfn, 4, val & ~1);

			val = dnc_read_conf(node->sci, pbus, pdev, pfn, 0x10);
			if (val)
				warning("SCI%3x %02x:%02x.%d has reg 0x10 as %08x", node->sci, pbus, pdev, pfn, val);
			dnc_write_conf(node->sci, pbus, pdev, pfn, 0x10, 0);

			val = dnc_read_conf(node->sci, pbus, pdev, pfn, 0x14);
			if (val)
				warning("SCI%3x %02x:%02x.%d has reg 0x14 as %08x", node->sci, pbus, pdev, pfn, val);
			dnc_write_conf(node->sci, pbus, pdev, pfn, 0x14, 0);

			val = dnc_read_conf(node->sci, pbus, pdev, pfn, 0x38);
			if (val)
				warning("SCI%3x %02x:%02x.%d has reg 0x38 as %08x", node->sci, pbus, pdev, pfn, val);
			dnc_write_conf(node->sci, pbus, pdev, pfn, 0x38, 0);

			if (io_bars.used == 0 || io_cur == io_start ||
			  (pbus == 0 && pdev == 0 && pfn == 0)) {
				dnc_write_conf(node->sci, pbus, pdev, pfn, 0x1c, 0xf0);
				dnc_write_conf(node->sci, pbus, pdev, pfn, 0x30, 0);
			} else {
				val = ((io_cur - 1) & 0xf000) | ((io_start  >> 8) & 0xf0);
				dnc_write_conf(node->sci, pbus, pdev, pfn, 0x1c, val);
				val = (io_start >> 16) | ((io_cur - 1) & ~0xffff);
				dnc_write_conf(node->sci, pbus, pdev, pfn, 0x30, val);
			}

			if (mmio32_bars.used == 0 || map32->next == mmio32_start ||
			  (pbus == 0 && pdev == 0 && pfn == 0))
				dnc_write_conf(node->sci, pbus, pdev, pfn, 0x20, 0x0000fffff);
			else {
				val = (mmio32_start >> 16) | ((map32->next - 1) & 0xffff0000);
				dnc_write_conf(node->sci, pbus, pdev, pfn, 0x20, val);
			}

			if (mmio64_bars.used == 0 || mmio64_cur == mmio64_start ||
			  (pbus == 0 && pdev == 0 && pfn == 0)) {
				dnc_write_conf(node->sci, pbus, pdev, pfn, 0x24, 0x0000ffff);
				dnc_write_conf(node->sci, pbus, pdev, pfn, 0x28, 0x00000000);
				dnc_write_conf(node->sci, pbus, pdev, pfn, 0x2c, 0x00000000);
			} else {
				for (BAR **bar = mmio64_bars.elements; bar < mmio64_bars.elements + mmio64_bars.used; bar++)
					assert((*bar)->pref);

				val = (mmio64_start >> 16) | ((mmio64_cur - 1) & 0xffff0000);
				dnc_write_conf(node->sci, pbus, pdev, pfn, 0x24, val);
				dnc_write_conf(node->sci, pbus, pdev, pfn, 0x28, mmio64_start >> 32);
				dnc_write_conf(node->sci, pbus, pdev, pfn, 0x2c, (mmio64_cur - 1) >> 32);
			}
		}

		return 0;
	}
};

void setup_mmio(void) {
	Container *containers[UNITS];
	Container **container = containers;

	critical_enter();
	/* Enumerate PCI busses */
	foreach_node(node) {
		printf("SCI%03x\n", node->sci);
		*container = new Container(node, 0, 0, 0);
		container++;
		assert(container - containers < UNITS);
	}
	critical_leave();

	io_cur = IO_BASE;
	map32 = new Map();

	/* Scope master BARs and exclude from map */
	Container **c = containers;
	(*c)->exclude();
	map32->dump();

	mmio64_cur = (uint64_t)dnc_top_of_mem << DRAM_MAP_SHIFT;

	/* Skip reassigning on master */
	c++;

	/* Assign master 64-bit BARs and all slave BARs */
	while (c < container) {
		(*c)->node->mmio32_base = map32->next;
		(*c)->node->mmio64_base = mmio64_cur;
		(*c)->node->io_base = io_cur;

		/* Retry until allocation succeeds */
		while ((*c)->allocate())
			printf("retrying allocation\n");

		(*c)->node->mmio32_limit = map32->next - 1;
		(*c)->node->mmio64_limit = mmio64_cur - 1;
		io_cur = roundup(io_cur, 1 << NC_ATT_IO_GRAN);
		(*c)->node->io_limit = io_cur - 1;
		printf("SCI%03x covers IO 0x%x:0x%x, MMIO32 0x%x:0x%x, MMIO64 0x%llx:0x%llx\n",
			(*c)->node->sci,
			(*c)->node->io_base, (*c)->node->io_limit,
			(*c)->node->mmio32_base, (*c)->node->mmio32_limit,
			(*c)->node->mmio64_base, (*c)->node->mmio64_limit);
		c++;
	}
}

/* Called for the master only */
void setup_mmio_master(void)
{
	nodes[0].mmio32_base = 0xffffffff;
	nodes[0].mmio32_limit = 0x00000000;

	/* Adjusted down later */
	for (int i = 0; i < 2; i++) {
		tmp_base[i] = 0xffffffff;
		tmp_lim[i] = 0;
	}

	printf("\nScoping master PCI tree:\n");
	critical_enter();
	pci_search(0xfff0, 0, 1, scope_bar);
	critical_leave();
	printf("Master MMIO32 range 0x%x:0x%x\n\n", nodes[0].mmio32_base, nodes[0].mmio32_limit);

	/* Check if there is space for another MMIO window, else wrap */
	uint64_t len = nodes[0].mmio32_limit - nodes[0].mmio32_base + 1;
	mmio_cur = nodes[0].mmio32_limit + 1;
	if ((mmio_cur + len) >= MMIO32_LIMIT) {
		nodes[0].mmio32_limit = 0xffffffff;
		mmio_cur = rdmsr(MSR_TOPMEM);
	}

#ifdef EXPERIMENTAL
	/* Accept interrupts from slave SR56x0s */
	uint32_t val = ioh_nbmiscind_read(0xfff0, 0x75);
	ioh_nbmiscind_write(0xfff0, 0x75, (val & ~6) | 2);
#endif
}

void setup_mmio_slave(const int node)
{
	assert(node > 0);
	uint16_t sci = nodes[node].sci;

	printf("Setting up PCI routing on SCI%03x from 0x%llx\n", sci, mmio_cur);
	nodes[node].mmio32_base = mmio_cur;
	critical_enter();
	pci_search(sci, 0, 0, setup_bar);
	critical_leave();

	mmio_cur = roundup(mmio_cur, 0x1000);

#ifdef EXPERIMENTAL
	ioh_ioapicind_write(sci, 0x1, (mmio_cur & 0xffffffff) | 8);
	ioh_ioapicind_write(sci, 0x2, mmio_cur >> 32);
	printf("SCI%03x IOAPIC at 0x%08llx\n", sci, mmio_cur);

	/* Forward interrupts to primary SP5100 */
	uint32_t val = ioh_nbmiscind_read(sci, 0x75);
	ioh_nbmiscind_write(sci, 0x75, (val & ~6) | 4);
#endif
	ioh_ioapicind_write(sci, 0x0, 0);
	printf("SCI%03x IOAPIC disabled\n", sci);

	mmio_cur = roundup(mmio_cur, NC_ATT_MMIO32_GRAN) - 1;
	nodes[node].mmio32_limit = mmio_cur;
	printf("SCI%03x MMIO32 range 0x%x:0x%x\n\n", sci, nodes[node].mmio32_base, nodes[node].mmio32_limit);

	/* Check if there is space for another MMIO window, else wrap */
	uint64_t len = nodes[node].mmio32_limit - nodes[node].mmio32_base + 1;
	mmio_cur = nodes[node].mmio32_limit + 1;
	if ((mmio_cur + len) >= MMIO32_LIMIT) {
		nodes[node].mmio32_limit = 0xffffffff;
		mmio_cur = rdmsr(MSR_TOPMEM);
	}
}

void setup_mmio_late(void)
{
	printf("Setting up MMIO32 ATTs (default SCI000):\n");
	for (int dnode = 1; dnode < dnc_node_count; dnode++) {
		printf("- 0x%x:0x%x -> SCI%03x\n",
			nodes[dnode].mmio32_base, nodes[dnode].mmio32_limit, nodes[dnode].sci);

		/* Ensure alignment */
		const uint32_t mask = (1 << NC_ATT_MMIO32_GRAN) - 1;
		assert((nodes[dnode].mmio32_base & mask) == 0 && (nodes[dnode].mmio32_limit & mask) == mask);

		for (uint32_t k = nodes[dnode].mmio32_base >> NC_ATT_MMIO32_GRAN; k < ((nodes[dnode].mmio32_limit + 1) >> NC_ATT_MMIO32_GRAN); k++) {
			for (int node = 0; node < dnc_node_count; node++) {
				dnc_write_csr(nodes[node].sci, H2S_CSR_G3_NC_ATT_MAP_SELECT, NC_ATT_MMIO32 | (k / 256));
				dnc_write_csr(nodes[node].sci, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + (k % 256) * 4, nodes[dnode].sci);
			}
		}
	}

	printf("Setting up IO ATTs (default SCI000):\n");
	for (int node = 0; node < dnc_node_count; node++)
		dnc_write_csr(nodes[node].sci, H2S_CSR_G3_NC_ATT_MAP_SELECT, NC_ATT_IO);

	/* Skip range to SCI000, as it's default */
	for (int dnode = 1; dnode < dnc_node_count; dnode++) {
		printf("- 0x%x:0x%x -> SCI%03x\n", nodes[dnode].io_base, nodes[dnode].io_limit, nodes[dnode].sci);

		/* Ensure alignment */
		const uint32_t mask = (1 << NC_ATT_IO_GRAN) - 1;
		assert((nodes[dnode].io_base & mask) == 0 && (nodes[dnode].io_limit & mask) == mask);

		for (uint32_t k = nodes[dnode].io_base >> NC_ATT_IO_GRAN; k < (nodes[dnode].io_limit + 1) >> NC_ATT_IO_GRAN; k++)
			for (int node = 0; node < dnc_node_count; node++)
				dnc_write_csr(nodes[node].sci, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + (k % 256) * 4, nodes[dnode].sci);
	}

	printf("Setting up SCC ATTs for MMIO64 (default SCI000):\n");
	for (int dnode = 1; dnode < dnc_node_count; dnode++) {
		if (nodes[dnode].mmio64_limit < nodes[dnode].mmio64_base)
			continue;

		printf("- 0x%llx:0x%llx -> SCI%03x\n", nodes[dnode].mmio64_base, nodes[dnode].mmio64_limit, nodes[dnode].sci);

		for (int i = 0; i < dnc_node_count; i++) {
			uint64_t addr = nodes[dnode].mmio64_base;
			uint64_t end  = nodes[dnode].mmio64_limit;

			dnc_write_csr(nodes[i].sci, H2S_CSR_G0_ATT_INDEX, (1 << 31) |
			  (1 << (27 + SCC_ATT_INDEX_RANGE)) | (addr / (SCC_ATT_GRAN << DRAM_MAP_SHIFT)));

			while (addr < end) {
				dnc_write_csr(nodes[i].sci, H2S_CSR_G0_ATT_ENTRY, nodes[dnode].sci);
				addr += SCC_ATT_GRAN << DRAM_MAP_SHIFT;
			}
		}
	}

	uint64_t tom = rdmsr(MSR_TOPMEM);

	for (int i = 0; i < dnc_node_count; i++) {
		uint16_t sci = nodes[i].sci;
		uint8_t ioh_ht = (dnc_read_conf(sci, 0, 24 + nodes[i].bsp_ht, FUNC0_HT, 0x60) >> 8) & 7;
		int range = 0;

		printf("Setting up NC MMIO ranges on SCI%03x with IOH at %d\n", sci, ioh_ht);

		/* Local MMIO to IOH */
		if (i == 0) {
			nc_mmio_range(sci, range++, MMIO_VGA_BASE, MMIO_VGA_LIMIT, ioh_ht);

			for (int erange = 0; erange < map32->ranges; erange++)
				nc_mmio_range(sci, range++, map32->excluded[erange].start, map32->excluded[erange].end, ioh_ht);

		} else
			nc_mmio_range(sci, range++, nodes[i].mmio32_base, nodes[i].mmio32_limit, ioh_ht);

		if (nodes[i].mmio64_limit > nodes[i].mmio64_base)
			nc_mmio_range(sci, range++, nodes[i].mmio64_base, nodes[i].mmio64_limit, ioh_ht);

		while (range < 8)
			nc_mmio_range_del(sci, range++);
	}

	/* Slaves */
	for (int i = 1; i < dnc_node_count; i++) {
		uint16_t sci = nodes[i].sci;
		uint8_t ioh_ht = (dnc_read_conf(sci, 0, 24 + nodes[i].bsp_ht, FUNC0_HT, 0x60) >> 8) & 7;
		uint8_t ioh_link = (dnc_read_conf(sci, 0, 24 + nodes[i].bsp_ht, FUNC0_HT, 0x64) >> 8) & 3;

		printf("Setting up NB MMIO ranges on SCI%03x with IOH at %d.%d\n", sci, ioh_ht, ioh_link);

		for (int ht = nodes[i].nb_ht_lo; ht <= nodes[i].nb_ht_hi; ht++) {
			int range = 0;

			mmio_range(sci, ht, range++, MMIO_VGA_BASE, MMIO_VGA_LIMIT, nodes[i].nc_ht, 0, 1);

			/* 32-bit under local range */
			if (nodes[i].mmio32_base > tom)
				mmio_range(sci, ht, range++, tom, nodes[i].mmio32_base - 1, nodes[i].nc_ht, 0, 1);

			/* 32-bit local range */
			mmio_range(sci, ht, range++, nodes[i].mmio32_base, nodes[i].mmio32_limit, ioh_ht, ioh_link, 1);

			/* 32-bit above local range */
			mmio_range(sci, ht, range++, nodes[i].mmio32_limit + 1, 0xffffffff, nodes[i].nc_ht, 0, 1);

			/* 64-bit under local range */
			if (nodes[i].mmio64_base > ((uint64_t)dnc_top_of_mem << DRAM_MAP_SHIFT))
				mmio_range(sci, ht, range++, (uint64_t)dnc_top_of_mem << DRAM_MAP_SHIFT, nodes[i].mmio64_base - 1, nodes[i].nc_ht, 0, 1);

			/* 64-bit local range */
			if (nodes[i].mmio64_limit > nodes[i].mmio64_base)
				mmio_range(sci, ht, range++, nodes[i].mmio64_base, nodes[i].mmio64_limit, ioh_ht, ioh_link, 1);

			/* 64-bit above local range */
			if (nodes[dnc_node_count - 1].mmio64_limit > nodes[i].mmio64_limit && i < (dnc_node_count - 1))
				mmio_range(sci, ht, range++, nodes[i].mmio64_limit + 1, nodes[dnc_node_count - 1].mmio64_limit, nodes[i].nc_ht, 0, 1);

			while (range < 8)
				mmio_range_del(sci, ht, range++);
		}
	}

	/* Master */
	{
		uint8_t ioh_ht = (dnc_read_conf(0xfff0, 0, 24 + nodes[0].bsp_ht, FUNC0_HT, 0x60) >> 8) & 7;
		uint8_t ioh_link = (dnc_read_conf(0xfff0, 0, 24 + nodes[0].bsp_ht, FUNC0_HT, 0x64) >> 8) & 3;
		printf("Setting up NB MMIO ranges on SCI000 with IOH at %d.%d\n", ioh_ht, ioh_link);

		critical_enter();
		for (int ht = nodes[0].nb_ht_lo; ht <= nodes[0].nb_ht_hi; ht++) {
			int range = 0;

			mmio_range(0xfff0, ht, range++, MMIO_VGA_BASE, MMIO_VGA_LIMIT, ioh_ht, ioh_link, 1);

			/* Merge adjacent remote ranges */
			uint32_t start = nodes[1].mmio32_base;
			uint32_t end = nodes[1].mmio32_limit;

			for (int i = 2; i < dnc_node_count; i++) {
				if (nodes[i].mmio32_base == (end + 1)) {
					end = nodes[i].mmio32_limit;
					continue;
				}

				mmio_range(0xfff0, ht, range++, start, end, nodes[0].nc_ht, 0, 1);
				start = nodes[i].mmio32_base;
				end = nodes[i].mmio32_limit;
			}
			mmio_range(0xfff0, ht, range++, start, end, nodes[0].nc_ht, 0, 1);

			for (int erange = 0; erange < map32->ranges; erange++)
				mmio_range(0xfff0, ht, range++, map32->excluded[erange].start, map32->excluded[erange].end, ioh_ht, ioh_link, 1);

			if (nodes[0].mmio64_limit > nodes[0].mmio64_base)
				mmio_range(0xfff0, ht, range++, nodes[0].mmio64_base, nodes[0].mmio64_limit, ioh_ht, ioh_link, 1);

			if (nodes[dnc_node_count - 1].mmio64_limit > nodes[1].mmio64_base)
				mmio_range(0xfff0, ht, range++, nodes[1].mmio64_base, nodes[dnc_node_count - 1].mmio64_limit, nodes[0].nc_ht, 0, 1);

			while (range < 8)
				mmio_range_del(0xfff0, ht, range++);
		}
		critical_leave();
	}
}
