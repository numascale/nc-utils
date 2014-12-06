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
#include "dnc-devices.h"
#include "dnc-maps.h"
#include "dnc-devices.h"

uint64_t mmio64_base, mmio64_limit;
static uint64_t io_cur, mmio64_cur;
static uint64_t tmp_base[2], tmp_lim[2];
class Map;
static Map *map32;

struct range {
	uint32_t start, end;
};

void dump_device(const sci_t sci, const int bus, const int dev, const int fn)
{
	printf("\nPCI device SCI%03x.%02x:%02x.%x:\n", sci, bus, dev, fn);
	for (int offset = 0; offset <= 0x3c; offset += 4)
		printf("%08x\n", dnc_read_conf(sci, bus, dev, fn, offset));
	printf("\n");
}

static void pci_search(const uint16_t sci, const int bus,
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
				pci_search(sci, sec, barfn);

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
			}

			/* If not multi-function, break out of function loop */
			if (!fn && !(type & 0x80))
				break;
		}
	}
}

class Map {
public:
	Vector<struct range> excluded;
	uint64_t next;

	bool merge(void) {
		for (unsigned range = 1; range < excluded.used; range++) {
			if ((excluded.elements[range].start - excluded.elements[range - 1].end > MMIO_MIN_GAP) &&
			  (excluded.elements[range].end < excluded.elements[range - 1].start || excluded.elements[range].start > excluded.elements[range - 1].end))
				continue;

			excluded.elements[range - 1].start = min(excluded.elements[range - 1].start, excluded.elements[range].start);
			excluded.elements[range - 1].end = max(excluded.elements[range - 1].end, excluded.elements[range].end);
			excluded.del(range);
			return 1;
		}

		return 0;
	}

	void dump(void) {
		printf("After master MMIO32 BARs:\n");
		for (unsigned i = 0; i < excluded.used; i++)
			printf("- 0x%x:0x%x\n", excluded.elements[i].start, excluded.elements[i].end);
	}

	void exclude(const uint32_t start, const uint32_t end) {
		assertf(start >= rdmsr(MSR_TOPMEM), "BAR 0x%08x starts before MMIO32 window at 0x%08llx", start, rdmsr(MSR_TOPMEM));
		assert(end > start);

		/* Find appropriate position */
		unsigned range = 0;
		while (range < excluded.used && start > excluded.elements[range].start)
			range++;

		/* Align start and end to 1MB boundaries */
		struct range elem = {start & ~0xfffff, end | 0xfffff};
		excluded.insert(elem, range);

		while (merge());
	}

	Map(void) {
		next = rdmsr(MSR_TOPMEM);

		if (!disable_smm) {
			warning("%dMB of MMIO32 space is reserved for MCFG as SMM is enabled", old_mcfg_len >> 20);
			exclude(old_mcfg_base, old_mcfg_base + old_mcfg_len);
		}

		/* Reserve IOAPIC, HPET and LAPICs in case e820 doesn't */
		exclude(0xfec00000, 0xfeefffff);
		/* LPC FWH */
		exclude(0xffc00000, 0xffffffff);

		printf("Reserved MMIO32 ranges above TOM 0x%08llx:\n", next);
		for (unsigned i = 0; i < orig_e820_len / sizeof(*orig_e820_map); i++) {
			/* Skip excluding e820 reservation for MCFG if it is usable */
			if (disable_smm && orig_e820_map[i].base == old_mcfg_base && orig_e820_map[i].length >= old_mcfg_len)
				continue;

			if (orig_e820_map[i].type == 2 && orig_e820_map[i].base >= next && orig_e820_map[i].base <= 0xffffffff) {
				uint32_t limit = orig_e820_map[i].base + orig_e820_map[i].length - 1;
				printf("- excluding 0x%08llx:0x%08x\n", orig_e820_map[i].base, limit);
				exclude(orig_e820_map[i].base, limit);
			}
		}
	}

	uint32_t overlap(const uint32_t start, const uint32_t len) {
		const uint32_t end = start + len;

		/* If overlap, return amount */
		for (unsigned i = 0; i < excluded.used; i++)
			if (end > excluded.elements[i].start && start < excluded.elements[i].end)
				return end - excluded.elements[i].start;

		/* No overlap */
		return 0;
	}
};

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

	bool allocate(uint64_t *const addr) {
		assert(len);

		if (io) {
			if ((*addr + len) > IO_LIMIT)
				*addr = 0;
			else
				roundup(*addr, max(len, 16));
			return 0;
		}

		if ((!s64 || (s64 && !io_nonpref_high && !pref)) && io_limit && len > io_limit) {
			assigned = 0;
			warning("Not allocating large %lluMB MMIO32 range", len >> 20);
			goto out;
		}

		if (!s64 && (*addr + len) > MMIO32_LIMIT) {
			assigned = 0;
			warning("Out of MMIO32 address space when allocating SCI%03x %02x:%02x.%x BAR 0x%x with len %s", sci, bus, dev, fn, reg, pr_size(len));
			goto out;
		}

		if (io_nonpref_high && s64 && !pref)
			warning("Allocating non-prefetchable 64-bit BAR 0x%x on SCI%03x %02x:%02x.%d in 64-bit prefetchable space", reg, sci, bus, dev, fn);

		/* MMIO BARs are aligned from page size to their size */
		*addr = roundup(*addr, max(len, 4096));

		if (!s64) {
			/* If space is allocated, move past and get parent to retry */
			uint64_t skip = map32->overlap(*addr, len);
			if (skip) {
				*addr += skip;
				return 1;
			}
		}

		assigned = *addr;
		*addr += len;

	out:
		printf("SCI%03x %02x:%02x.%x allocating %d-bit %s %s BAR 0x%x at 0x%llx\n",
			sci, bus, dev, fn, s64 ? 64 : 32, pref ? "P" : "NP", pr_size(len), reg, assigned);
		dnc_write_conf(sci, bus, dev, fn, reg, assigned);
		if (s64)
			dnc_write_conf(sci, bus, dev, fn, reg + 4, assigned >> 32);

		return 0;
	}
};

class Container {
	Vector<Container *> containers;
	Vector<BAR *> io_bars, mmio32_bars, mmio64_bars;
	int bus;
	const int pbus, pdev, pfn;

	/* Order by descending length */
	void insert(Vector<BAR *> &v, BAR *bar) {
		unsigned i = 0;
		while (i < v.used && v.elements[i]->len >= bar->len)
			i++;

		v.insert(bar, i);
	}

	/* Returns offset to skip for 64-bit BAR */
	int probe(const int bus, const int dev, const int fn, const int offset) {
		BAR *bar = new BAR(node->sci, bus, dev, fn, offset);
		if (bar->len == 0) {
			delete bar;
			return 0;
		}

		/* Skip second register is a 64-bit BAR */
		int skip = bar->s64 ? 4 : 0;

		/* Allocate 64-bit non-prefetchable BARs in 64-bit prefetchable space, as it's safe on SR56x0s */
		if (bar->io)
			insert(io_bars, bar);
		else if (bar->s64 && (bar->pref || io_nonpref_high))
			insert(mmio64_bars, bar);
		else
			insert(mmio32_bars, bar);

		return skip;
	}

	void device(const int dev, const int fn) {
		printf(" > dev %x:%02x.%x", bus, dev, fn);

		for (int offset = 0x10; offset <= 0x30; offset += 4) {
			/* Skip gap between last BAR and expansion ROM address */
			if (offset == 0x28)
				offset = 0x30;

			offset += probe(bus, dev, fn, offset);
		}

		/* Assign BARs in particular capabilities */
		uint16_t cap = extcapability(PCI_ECAP_SRIOV, node->sci, bus, dev, fn);
		if (cap != PCI_CAP_NONE)
			for (int offset = 0x24; offset <= 0x38; offset += 4)
				offset += probe(bus, dev, fn, cap + offset);

		printf("\n");

		/* Disable IO on slaves */
		if (node->sci) {
			uint32_t val = dnc_read_conf(node->sci, bus, dev, fn, 4);
			dnc_write_conf(node->sci, bus, dev, fn, 4, val & ~1);
		}
	}

public:
	node_info_t *const node;

	Container(node_info_t *const _node, const int _pbus, const int _pdev, const int _pfn):
	  pbus(_pbus), pdev(_pdev), pfn(_pfn), node(_node) {
		bus = (dnc_read_conf(node->sci, pbus, pdev, pfn, 0x18) >> 8) & 0xff;
		printf("- bus %d\n", bus);
		const int limit = bus == 0 ? 24 : 32;

		/* Allocate bridge BARs and expansion ROM */
		for (int offset = 0x10; offset <= 0x38; offset += 4) {
			/* Skip gap between last BAR and expansion ROM address */
			if (offset == 0x18)
				offset = 0x38;

			offset += probe(pbus, pdev, pfn, offset);
		}

		for (int dev = 0; dev < limit; dev++) {
			for (int fn = 0; fn < 8; fn++) {
				uint32_t val = dnc_read_conf(node->sci, bus, dev, fn, 0xc);
				/* PCI device functions are not necessarily contiguous */
				if (val == 0xffffffff)
					continue;

				uint8_t type = val >> 16;
				if ((type & 0x7f) == 1)
					containers.add(new Container(node, bus, dev, fn));
				else {
					assert((type & 0x7f) == 0);
					device(dev, fn);
				}

				/* If not multi-function, break out of function loop */
				if (!fn && !(type & 0x80))
					break;
			}
		}
	}

	/* Exclude existing allocated ranges from map */
	void exclude(void) {
		for (BAR **bar = mmio32_bars.elements; bar < mmio32_bars.limit; bar++) {
			if ((*bar)->assigned)
				map32->exclude((*bar)->assigned, (*bar)->assigned + (*bar)->len - 1);
		}

		for (BAR **bar = mmio64_bars.elements; bar < mmio64_bars.limit; bar++)
			if ((*bar)->assigned && (*bar)->assigned < 0xffffffff)
				map32->exclude((*bar)->assigned, (*bar)->assigned + (*bar)->len - 1);

		for (Container **c = containers.elements; c < containers.limit; c++)
			(*c)->exclude();
	}

	/* Set BAR addresses; return 1 if ran into an exclusion zone; caller will retry */
	bool allocate(void) {
		map32->next = roundup(map32->next, 1 << NC_ATT_MMIO32_GRAN);
		assert(map32->next < MMIO32_LIMIT);
		io_cur = roundup(io_cur, 1 << 12);

		uint32_t io_start = io_cur, mmio32_start = map32->next;
		uint64_t mmio64_start = mmio64_cur;

		/* Only reallocate 32-bit BARs on slaves */
		if (node->sci)
			for (BAR **bar = mmio32_bars.elements; bar < mmio32_bars.limit; bar++)
				if ((*bar)->allocate(&map32->next))
					return 1;

		/* Allocate child containers, retrying until allocation succeeds */
		for (Container **c = containers.elements; c < containers.limit; c++)
			if ((*c)->allocate())
				return 1;

		if (node->sci)
			for (BAR **bar = mmio64_bars.elements; bar < mmio64_bars.limit; bar++)
				assert(!(*bar)->allocate(&mmio64_cur));

		/* Only reallocate IO BARs on slaves */
		if (node->sci)
			for (BAR **bar = io_bars.elements; bar < io_bars.limit; bar++)
				assert(!(*bar)->allocate(&io_cur));

		/* Align start of bridge windows */
		map32->next = roundup(map32->next, 1 << NC_ATT_MMIO32_GRAN);
		assert(map32->next < MMIO32_LIMIT);
		mmio64_cur = roundup(mmio64_cur, MMIO64_GRAN);
		io_cur = roundup(io_cur, 1 << 12);

		uint32_t sec = (dnc_read_conf(node->sci, pbus, pdev, pfn, 0x18) >> 8) & 0xff;
		printf("SCI%03x %02x:%02x.%x bridge to %d configured for IO 0x%x:0x%llx, MMIO32 0x%x:0x%llx, MMIO64 0x%llx:0x%llx\n",
			node->sci, pbus, pdev, pfn, sec, io_start, io_cur - 1, mmio32_start, map32->next - 1, mmio64_start, mmio64_cur - 1);

		/* Only re-allocate IO and 32-bit BARs on slaves */
		if (node->sci) {
			if (io_bars.used == 0 || io_cur == io_start ||
			  (pbus == 0 && pdev == 0 && pfn == 0)) {
				dnc_write_conf(node->sci, pbus, pdev, pfn, 0x1c, 0xf0);
				dnc_write_conf(node->sci, pbus, pdev, pfn, 0x30, 0);
			} else {
				uint32_t val = ((io_cur - 1) & 0xf000) | ((io_start  >> 8) & 0xf0);
				dnc_write_conf(node->sci, pbus, pdev, pfn, 0x1c, val);
				val = (io_start >> 16) | ((io_cur - 1) & ~0xffff);
				dnc_write_conf(node->sci, pbus, pdev, pfn, 0x30, val);
			}

			if (mmio32_bars.used == 0 || map32->next == mmio32_start ||
			  (pbus == 0 && pdev == 0 && pfn == 0))
				dnc_write_conf(node->sci, pbus, pdev, pfn, 0x20, 0x0000fffff);
			else {
				uint32_t val = (mmio32_start >> 16) | ((map32->next - 1) & 0xffff0000);
				dnc_write_conf(node->sci, pbus, pdev, pfn, 0x20, val);
			}

			if (mmio64_bars.used == 0 || mmio64_cur == mmio64_start ||
			  (pbus == 0 && pdev == 0 && pfn == 0)) {
				dnc_write_conf(node->sci, pbus, pdev, pfn, 0x24, 0x0000ffff);
				dnc_write_conf(node->sci, pbus, pdev, pfn, 0x28, 0x00000000);
				dnc_write_conf(node->sci, pbus, pdev, pfn, 0x2c, 0x00000000);
			} else {
				uint32_t val = (mmio64_start >> 16) | ((mmio64_cur - 1) & 0xffff0000);
				dnc_write_conf(node->sci, pbus, pdev, pfn, 0x24, val);
				dnc_write_conf(node->sci, pbus, pdev, pfn, 0x28, mmio64_start >> 32);
				dnc_write_conf(node->sci, pbus, pdev, pfn, 0x2c, (mmio64_cur - 1) >> 32);
			}
		}

		return 0;
	}
};

void setup_mmio(void) {
	for (int i = 0; i < dnc_node_count; i++) {
		sci_t sci = nodes[i].sci;

		/* Enable SP5100 SATA MSI support */
		uint32_t val2 = dnc_read_conf(sci, 0, 17, 0, 0x40);
		dnc_write_conf(sci, 0, 17, 0, 0x40, val2 | 1);
		uint32_t val = dnc_read_conf(sci, 0, 17, 0, 0x60);
		dnc_write_conf(sci, 0, 17, 0, 0x60, (val & ~0xff00) | 0x5000);
		dnc_write_conf(sci, 0, 17, 0, 0x40, val2);

		if (i > 0) {
			/* Disable IO, memory and interrupts on devices behind bridge */
			uint32_t val = dnc_read_conf(sci, 0, 20, 4, 0x18);
			int sec = (val >> 8) & 0xff;

			const struct devspec devices[] = {
				/* FIXME: hangs on x3755 */
				{PCI_CLASS_ANY, 0, PCI_TYPE_ENDPOINT, disable_device},
				{PCI_CLASS_ANY, 0, PCI_TYPE_BRIDGE, disable_bridge},
				{PCI_CLASS_FINAL, 0, PCI_TYPE_ANY, NULL}
			};
			pci_search(devices, sec);
		}
	}

	/* If remote IO is disabled, disable remote bridge ports */
	if (!remote_io) {
		for (int i = 1; i < dnc_node_count; i++) {
			sci_t sci = nodes[i].sci;

			/* Hide SR56x0 PCIe ports */
			uint32_t val = ioh_nbmiscind_read(sci, 0xc);
			ioh_nbmiscind_write(sci, 0xc, val | 0x1f00fc);

			/* Disable B-link pads to SB and GPP3b */
			/* FIXME: hangs on x3755 */
			ioh_nbpcieind_write(sci, 3, 0x65, 0xffff);
			ioh_nbpcieind_write(sci, 5, 0x65, 0xffff);
		}

		return;
	}

	for (int i = 1; i < dnc_node_count; i++) {
		sci_t sci = nodes[i].sci;

		/* Hide devices behind bridge (eg VGA controller) */
		uint32_t val = dnc_read_conf(sci, 0, 20, 4, 0xfc);
		dnc_write_conf(sci, 0, 20, 4, 0x5c, val & ~0xffff);

		/* Disable and hide SP5100 IDE controller */
		dnc_write_conf(sci, 0, 20, 1, 4, 0);
		val = dnc_read_conf(sci, 0, 20, 0, 0xac);
		dnc_write_conf(sci, 0, 20, 0, 0xac, val | (1 << 19));

		/* Disable the LPC controller; hiding it causes hanging */
		dnc_write_conf(sci, 0, 20, 3, 4, 0);

		/* Disable and hide OHCI */
		dnc_write_conf(sci, 0, 18, 0, 4, 0);
		dnc_write_conf(sci, 0, 18, 1, 4, 0);
		dnc_write_conf(sci, 0, 18, 2, 4, 0);
		dnc_write_conf(sci, 0, 19, 0, 4, 0);
		dnc_write_conf(sci, 0, 19, 1, 4, 0);
		dnc_write_conf(sci, 0, 19, 2, 4, 0);

		/* Disable all USB controllers */
		val = dnc_read_conf(sci, 0, 20, 0, 0x68);
		dnc_write_conf(sci, 0, 20, 0, 0x68, val & ~0xf7);

		/* Disable HPET MMIO decoding */
		val = dnc_read_conf(sci, 0, 20, 0, 0x40);
		dnc_write_conf(sci, 0, 20, 0, 0x40, val & ~(1 << 28));

		/* Disable all bits in the PCI_COMMAND register of the ACPI/SMBus function */
		dnc_write_conf(sci, 0, 20, 0, 4, 0);

#ifdef FIXME /* Causes bus enumeration to loop */
		/* Disable legacy bridge; unhides PCI bridge at device 8 */
		val = ioh_nbmiscind_read(sci, 0x0);
		ioh_nbmiscind_write(sci, 0x0, val | (1 << 6));
#endif
	}

	Vector<Container *> containers;

	critical_enter();
	/* Enumerate PCI busses */
	foreach_node(node) {
		printf("SCI%03x\n", node->sci);
		/* Broadcom HT1000 PCI host bridges start at device 1 */
		int dev = southbridge_id == 0x43851002 ? 0 : 1;
		containers.add(new Container(node, 0, dev, 0));
	}
	critical_leave();

	io_cur = IO_BASE;
	map32 = new Map();

	/* Scope master BARs and exclude from map */
	Container **c = containers.elements;
	(*c)->exclude();
	map32->dump();

	/* Start MMIO64 after the HyperTransport decode range to avoid interference */
	mmio64_base = mmio64_cur = max((uint64_t)dnc_top_of_mem << DRAM_MAP_SHIFT, HT_LIMIT);

	/* Skip reassigning on master */
	c++;

	/* Assign slave BARs */
	while (c < containers.limit) {
		do {
			map32->next = roundup(map32->next, 1 << NC_ATT_MMIO32_GRAN);
			assert(map32->next < MMIO32_LIMIT);
			/* Retry until allocation succeeds */
			(*c)->node->mmio32_base = map32->next;
			(*c)->node->mmio64_base = mmio64_cur;
			(*c)->node->io_base = io_cur;
		} while ((*c)->allocate());

		io_cur = roundup(io_cur, 1 << NC_ATT_IO_GRAN);
		map32->next = roundup(map32->next, 1 << NC_ATT_MMIO32_GRAN);
		assert(map32->next < MMIO32_LIMIT);

		/* May need to address Numachip extended MMIO mask constraint */
		mmio64_cur = (*c)->node->mmio64_base + roundup_nextpow2(mmio64_cur - (*c)->node->mmio64_base);

		(*c)->node->mmio32_limit = map32->next - 1;
		(*c)->node->mmio64_limit = mmio64_cur - 1;
		(*c)->node->io_limit = io_cur - 1;
		printf("SCI%03x covers IO 0x%x:0x%x, MMIO32 0x%x:0x%x, MMIO64 0x%llx:0x%llx\n",
			(*c)->node->sci,
			(*c)->node->io_base, (*c)->node->io_limit,
			(*c)->node->mmio32_base, (*c)->node->mmio32_limit,
			(*c)->node->mmio64_base, (*c)->node->mmio64_limit);
		c++;
	}

	mmio64_limit = mmio64_cur;

	printf("Setting up MMIO32 ATTs (default SCI%03x):\n", nodes[0].sci);
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

	printf("Setting up IO ATTs (default SCI%03x):\n", nodes[0].sci);
	for (int node = 0; node < dnc_node_count; node++)
		dnc_write_csr(nodes[node].sci, H2S_CSR_G3_NC_ATT_MAP_SELECT, NC_ATT_IO);

	/* Skip range to the master, as it's default */
	for (int dnode = 1; dnode < dnc_node_count; dnode++) {
		if (nodes[dnode].io_limit < nodes[dnode].io_base)
			continue;

		printf("- 0x%x:0x%x -> SCI%03x\n", nodes[dnode].io_base, nodes[dnode].io_limit, nodes[dnode].sci);

		/* Ensure alignment */
		const uint32_t mask = (1 << NC_ATT_IO_GRAN) - 1;
		assert((nodes[dnode].io_base & mask) == 0 && (nodes[dnode].io_limit & mask) == mask);

		for (uint32_t k = nodes[dnode].io_base >> NC_ATT_IO_GRAN; k < (nodes[dnode].io_limit + 1) >> NC_ATT_IO_GRAN; k++)
			for (int node = 0; node < dnc_node_count; node++)
				dnc_write_csr(nodes[node].sci, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + (k % 256) * 4, nodes[dnode].sci);
	}

	printf("Setting up SCC ATTs for MMIO64 (default SCI%03x):\n", nodes[0].sci);
	for (int dnode = 1; dnode < dnc_node_count; dnode++) {
		if (nodes[dnode].mmio64_limit < nodes[dnode].mmio64_base)
			continue;

		printf("- 0x%llx:0x%llx -> SCI%03x\n", nodes[dnode].mmio64_base, nodes[dnode].mmio64_limit, nodes[dnode].sci);

		for (int i = 0; i < dnc_node_count; i++) {
			uint64_t addr = nodes[dnode].mmio64_base;
			uint64_t end  = nodes[dnode].mmio64_limit;

			dnc_write_csr(nodes[i].sci, H2S_CSR_G0_ATT_INDEX, (1 << 31) |
			  (1 << (27 + scc_att_index_range)) | (addr / (SCC_ATT_GRAN << DRAM_MAP_SHIFT)));

			while (addr < end) {
				dnc_write_csr(nodes[i].sci, H2S_CSR_G0_ATT_ENTRY, nodes[dnode].sci);
				addr += SCC_ATT_GRAN << DRAM_MAP_SHIFT;
			}
		}
	}

	uint64_t tom = rdmsr(MSR_TOPMEM);

	/* Program IOAPIC address */
	for (int i = 1; i < dnc_node_count; i++) {
		/* Disable IOAPIC memory decode */
		uint32_t val = dnc_read_conf(nodes[i].sci, 0, 0x14, 0, 0x64);
		dnc_write_conf(nodes[i].sci, 0, 0x14, 0, 0x64, val & ~(1 << 3));

		/* Disable IOAPIC */
		ioh_ioapicind_write(nodes[i].sci, 0, 0);
	}

	for (int i = 0; i < dnc_node_count; i++) {
		uint16_t sci = nodes[i].sci;
		uint8_t ioh_ht = (dnc_read_conf(sci, 0, 24 + nodes[i].bsp_ht, FUNC0_HT, 0x60) >> 8) & 7;
		int range = 0;

		printf("Setting up NC MMIO ranges on SCI%03x with IOH at %d\n", sci, ioh_ht);

		/* Local MMIO to IOH */
		if (i == 0) {
			nc_mmio_range(sci, range++, MMIO_VGA_BASE, MMIO_VGA_LIMIT, ioh_ht);

			for (unsigned erange = 0; erange < map32->excluded.used; erange++)
				nc_mmio_range(sci, range++, map32->excluded.elements[erange].start, map32->excluded.elements[erange].end, ioh_ht);
		} else
			if (nodes[i].mmio32_limit > nodes[i].mmio32_base)
				nc_mmio_range(sci, range++, nodes[i].mmio32_base, nodes[i].mmio32_limit, ioh_ht);

		while (range < 8)
			nc_mmio_range_del(sci, range++);

		if (nodes[i].mmio64_limit > nodes[i].mmio64_base)
			nc_mmio_range_high(sci, 0, nodes[i].mmio64_base, nodes[i].mmio64_limit, ioh_ht);
	}

	/* Slaves */
	for (int i = 1; i < dnc_node_count; i++) {
		uint16_t sci = nodes[i].sci;
		uint8_t ioh_ht = (dnc_read_conf(sci, 0, 24 + nodes[i].bsp_ht, FUNC0_HT, 0x60) >> 8) & 7;
		uint8_t ioh_link = (dnc_read_conf(sci, 0, 24 + nodes[i].bsp_ht, FUNC0_HT, 0x64) >> 8) & 3;

		printf("Setting up NB MMIO ranges on SCI%03x with IOH at %d.%d\n", sci, ioh_ht, ioh_link);

		for (int ht = nodes[i].nb_ht_lo; ht <= nodes[i].nb_ht_hi; ht++) {
			int range = 0;

			mmio_range(sci, ht, range++, MMIO_VGA_BASE, MMIO_VGA_LIMIT, nodes[i].nc_ht, 0, 1, 0);

			/* 32-bit under local range */
			if (nodes[i].mmio32_base > tom)
				mmio_range(sci, ht, range++, tom, nodes[i].mmio32_base - 1, nodes[i].nc_ht, 0, 1, 0);

			/* 32-bit local range */
			if (nodes[i].mmio32_limit > nodes[i].mmio32_base)
				mmio_range(sci, ht, range++, nodes[i].mmio32_base, nodes[i].mmio32_limit, ioh_ht, ioh_link, 1, 0);

			/* 32-bit above local range */
			mmio_range(sci, ht, range++, nodes[i].mmio32_limit + 1, 0xffffffff, nodes[i].nc_ht, 0, 1, 0);

			while (range < 8)
				mmio_range_del(sci, ht, range++);

			/* Skip CSR ranges */
			range = 10;

			/* 64-bit under local range */
			if (nodes[i].mmio64_base > ((uint64_t)dnc_top_of_mem << DRAM_MAP_SHIFT))
				mmio_range(sci, ht, range++, (uint64_t)dnc_top_of_mem << DRAM_MAP_SHIFT, nodes[i].mmio64_base - 1, nodes[i].nc_ht, 0, 1, 0);

			/* 64-bit local range */
			if (nodes[i].mmio64_limit > nodes[i].mmio64_base)
				mmio_range(sci, ht, range++, nodes[i].mmio64_base, nodes[i].mmio64_limit, ioh_ht, ioh_link, 1, 0);

			/* Fam15h has 12 MMIO ranges, so use 7 */
			if (family > 0x10)
				range = 7;

			/* 64-bit above local range */
			if (nodes[dnc_node_count - 1].mmio64_limit > nodes[i].mmio64_limit && i < (dnc_node_count - 1))
				mmio_range(sci, ht, range++, nodes[i].mmio64_limit + 1, nodes[dnc_node_count - 1].mmio64_limit, nodes[i].nc_ht, 0, 1, 0);
		}
	}

	/* Master */
	{
		uint8_t ioh_ht = (dnc_read_conf(nodes[0].sci, 0, 24 + nodes[0].bsp_ht, FUNC0_HT, 0x60) >> 8) & 7;
		uint8_t ioh_link = (dnc_read_conf(nodes[0].sci, 0, 24 + nodes[0].bsp_ht, FUNC0_HT, 0x64) >> 8) & 3;
		printf("Setting up NB MMIO ranges on SCI%03x with IOH at %d.%d\n", nodes[0].sci, ioh_ht, ioh_link);

		critical_enter();
		for (int ht = nodes[0].nb_ht_lo; ht <= nodes[0].nb_ht_hi; ht++) {
			int range = 0;

			mmio_range(nodes[0].sci, ht, range++, MMIO_VGA_BASE, MMIO_VGA_LIMIT, ioh_ht, ioh_link, 1, 0);

			/* Merge adjacent remote ranges */
			uint32_t start = nodes[1].mmio32_base;
			uint32_t end = nodes[1].mmio32_limit;

			for (int i = 2; i < dnc_node_count; i++) {
				if (nodes[i].mmio32_base == (end + 1)) {
					end = nodes[i].mmio32_limit;
					continue;
				}

				mmio_range(nodes[0].sci, ht, range++, start, end, nodes[0].nc_ht, 0, 1, 0);
				start = nodes[i].mmio32_base;
				end = nodes[i].mmio32_limit;
			}

			if (end > start)
				mmio_range(nodes[0].sci, ht, range++, start, end, nodes[0].nc_ht, 0, 1, 0);

			for (unsigned erange = 0; erange < map32->excluded.used; erange++)
				mmio_range(nodes[0].sci, ht, range++, map32->excluded.elements[erange].start, map32->excluded.elements[erange].end, ioh_ht, ioh_link, 1, 0);

			while (range < 8)
				mmio_range_del(nodes[0].sci, ht, range++);

			/* Skip CSR ranges */
			range = 10;

			if (nodes[0].mmio64_limit > nodes[0].mmio64_base)
				mmio_range(nodes[0].sci, ht, range++, nodes[0].mmio64_base, nodes[0].mmio64_limit, ioh_ht, ioh_link, 1, 0);

			if (nodes[dnc_node_count - 1].mmio64_limit > nodes[1].mmio64_base)
				mmio_range(nodes[0].sci, ht, range++, nodes[1].mmio64_base, nodes[dnc_node_count - 1].mmio64_limit, nodes[0].nc_ht, 0, 1, 0);
		}
		critical_leave();
	}
}
