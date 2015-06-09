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

		/* SMM may access previous config space */
		if (!disable_smm)
			exclude(old_mcfg_base, old_mcfg_base + old_mcfg_len);

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
	const uint16_t vfs; /* Virtual Functions; used to allow extra space after allocation */
public:
	bool io, s64, pref;
	uint64_t assigned, len;
	BAR(const sci_t _sci, const int _bus, const int _dev, const int _fn, const int _reg, const uint16_t _vfs = 0)
		: sci(_sci), bus(_bus), dev(_dev), fn(_fn), reg(_reg), vfs(_vfs) {
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
				goto err;

			roundup(*addr, max(len, 16));
			return 0;
		}

		if ((!s64 || (s64 && !io_nonpref_high && !pref))) {
			if (io_limit && len >= io_limit)
				goto err;

			if ((*addr + len) > MMIO32_LIMIT)
				goto err;
		}

		/* MMIO BARs are aligned from page size to their size */
		*addr = roundup(*addr, max(len, 4096));
		if (!s64) {
			/* If space is allocated, move past and get parent to retry */
			uint64_t skip = map32->overlap(*addr, len);
			if (*addr + skip > MMIO32_LIMIT)
				goto err;

			if (skip) {
				printf("Skipping reserved region of %lluMB\n", skip >> 20);
				*addr += skip;
				return 1;
			}
		}

		assigned = *addr;
		*addr += len * max(vfs, 1); /* Allocate vfs times the space */

		printf("SCI%03x %02x:%02x.%x allocating %d-bit %s %s BAR 0x%x at 0x%llx\n",
			sci, bus, dev, fn, s64 ? 64 : 32, pref ? "P" : "NP", pr_size(len), reg, assigned);
	done:
		dnc_write_conf(sci, bus, dev, fn, reg, assigned);
		if (s64)
			dnc_write_conf(sci, bus, dev, fn, reg + 4, assigned >> 32);
		return 0;
	err:
		warning("Unable to allocate SCI%03x %02x:%02x.%x BAR 0x%x with len %s", sci, bus, dev, fn, reg, pr_size(len));
		assigned = 0;
		goto done;
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
	int probe(const int bus, const int dev, const int fn, const int offset, const uint16_t vfs = 0) {
		BAR *bar = new BAR(node->sci, bus, dev, fn, offset, vfs);
		if (bar->len == 0) {
			delete bar;
			return 0;
		}

		/* Skip second register if a 64-bit BAR */
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
		if (cap != PCI_CAP_NONE) {
			/* PCI SR-IOV spec needs the number of Virtual Functions times the BAR in space */
			const uint16_t vfs = dnc_read_conf(node->sci, bus, dev, fn, cap + 0x0c) >> 16;

			for (int offset = 0x24; offset <= 0x38; offset += 4)
				offset += probe(bus, dev, fn, cap + offset, vfs);
		}

		printf("\n");
	}

public:
	node_info_t *const node;

	Container(node_info_t *const _node, const int _pbus, const int _pdev, const int _pfn):
	  pbus(_pbus), pdev(_pdev), pfn(_pfn), node(_node) {
		bus = (dnc_read_conf(node->sci, pbus, pdev, pfn, 0x18) >> 8) & 0xff;
		printf("- bus %d\n", bus);

		/* Allocate bridge BARs and expansion ROM */
		for (int offset = 0x10; offset <= 0x38; offset += 4) {
			/* Skip gap between last BAR and expansion ROM address */
			if (offset == 0x18)
				offset = 0x38;

			offset += probe(pbus, pdev, pfn, offset);
		}

		for (int dev = 0; dev < (bus == 0 ? 24 : 32); dev++) {
			for (int fn = 0; fn < 8; fn++) {
				uint32_t val = dnc_read_conf(node->sci, bus, dev, fn, 0xc);
				/* PCI device functions are not necessarily contiguous */
				if (val == 0xffffffff)
					continue;

				uint8_t type = val >> 16;
				if ((type & 0x7f) == 1) /* Bridge */
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

		io_cur = roundup(io_cur, 1 << 12);

		uint32_t io_start = io_cur, mmio32_start = map32->next;
		uint64_t mmio64_start = mmio64_cur;

		/* Only reallocate 32-bit BARs on slaves */
		if (node->sci != local_info->sci)
			for (BAR **bar = mmio32_bars.elements; bar < mmio32_bars.limit; bar++)
				if ((*bar)->allocate(&map32->next))
					return 1;

		/* Allocate child containers, retrying until allocation succeeds */
		for (Container **c = containers.elements; c < containers.limit; c++)
			if ((*c)->allocate())
				return 1;

		if (node->sci != local_info->sci)
			for (BAR **bar = mmio64_bars.elements; bar < mmio64_bars.limit; bar++)
				assert(!(*bar)->allocate(&mmio64_cur));

		/* Only reallocate IO BARs on slaves */
		if (node->sci != local_info->sci)
			for (BAR **bar = io_bars.elements; bar < io_bars.limit; bar++)
				assert(!(*bar)->allocate(&io_cur));

		/* Align start of bridge windows */
		map32->next = roundup(map32->next, 1 << NC_ATT_MMIO32_GRAN);

		mmio64_cur = roundup(mmio64_cur, MMIO64_GRAN);
		io_cur = roundup(io_cur, 1 << 12);

		uint32_t sec = (dnc_read_conf(node->sci, pbus, pdev, pfn, 0x18) >> 8) & 0xff;
		printf("SCI%03x %02x:%02x.%x bridge to %d configured for IO 0x%x:0x%llx, MMIO32 0x%x:0x%llx, MMIO64 0x%llx:0x%llx\n",
			node->sci, pbus, pdev, pfn, sec, io_start, io_cur - 1, mmio32_start, map32->next - 1, mmio64_start, mmio64_cur - 1);

		/* Only re-allocate IO and 32-bit BARs on slaves */
		if (node->sci != local_info->sci) {
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
	foreach_nodes(node) {
		uint32_t vendev = dnc_read_conf(node->sci, 0, 0, 0, 0);
		if ((vendev != VENDEV_SR5690) && (vendev != VENDEV_SR5670) && (vendev != VENDEV_SR5650)) {
			warning("Unable to perform global MMIO setup with hostbridge %08x", vendev);
			return;
		}

		/* Enable SP5100 SATA MSI support */
		uint32_t val2 = dnc_read_conf(node->sci, 0, 17, 0, 0x40);
		dnc_write_conf(node->sci, 0, 17, 0, 0x40, val2 | 1);
		uint32_t val = dnc_read_conf(node->sci, 0, 17, 0, 0x60);
		dnc_write_conf(node->sci, 0, 17, 0, 0x60, (val & ~0xff00) | 0x5000);
		dnc_write_conf(node->sci, 0, 17, 0, 0x40, val2);

		if (node->sci != nodes[0].sci) {
			const struct devspec devices[] = {
				{PCI_CLASS_ANY, 0, PCI_TYPE_ENDPOINT, disable_device},
				{PCI_CLASS_ANY, 0, PCI_TYPE_BRIDGE, disable_bridge},
				{PCI_CLASS_FINAL, 0, PCI_TYPE_ANY, NULL}
			};
			pci_search(devices, node->sci, 0);

			/* Disable HPET MMIO decoding */
			val = dnc_read_conf(node->sci, 0, 20, 0, 0x40);
			dnc_write_conf(node->sci, 0, 20, 0, 0x40, val & ~(1 << 28));

			/* Hide IDE controller */
			val = dnc_read_conf(node->sci, 0, 20, 0, 0xac);
			dnc_write_conf(node->sci, 0, 20, 0, 0xac, val | (1 << 19));

			/* Hide the ISA LPC controller */
			val = dnc_read_conf(node->sci, 0, 20, 0, 0x64);
			dnc_write_conf(node->sci, 0, 20, 0, 0x64, val & ~(1 << 20));

			/* Disable and hide all USB controllers */
			dnc_write_conf(node->sci, 0, 18, 0, 4, 0x0400);
			dnc_write_conf(node->sci, 0, 18, 1, 4, 0x0400);
			dnc_write_conf(node->sci, 0, 18, 2, 4, 0x0400);
			dnc_write_conf(node->sci, 0, 19, 0, 4, 0x0400);
			dnc_write_conf(node->sci, 0, 19, 1, 4, 0x0400);
			dnc_write_conf(node->sci, 0, 19, 2, 4, 0x0400);
			dnc_write_conf(node->sci, 0, 19, 5, 4, 0x0400);
			val = dnc_read_conf(node->sci, 0, 20, 0, 0x68);
			dnc_write_conf(node->sci, 0, 20, 0, 0x68, val & ~0xf7);

			/* Disable the ACPI/SMBus function */
			dnc_write_conf(node->sci, 0, 20, 0, 4, 0x0400);

			/* Disable and hide VGA controller */
			dnc_write_conf(node->sci, 0, 20, 4, 4, 0x0400);
			dnc_write_conf(node->sci, 1, 4, 0, 4, 0x0400);
			val = dnc_read_conf(node->sci, 0, 20, 4, 0x5c);
			dnc_write_conf(node->sci, 0, 20, 4, 0x5c, val & ~0xffff0000);
		}
	}

	/* If remote IO is disabled, disable remote bridge ports */
	if (!remote_io) {
		foreach_slave_nodes(node) {
			/* Hide SR56x0 PCIe ports */
			uint32_t val = ioh_nbmiscind_read(node->sci, 0xc);
			ioh_nbmiscind_write(node->sci, 0xc, val | 0x1f00fc);

			/* Disable B-link pads to GPP1 */
			ioh_nbpcieind_write(node->sci, 2, 0x65, 0xffff);
		}

		return;
	}

	Vector<Container *> containers;

	critical_enter();
	/* Enumerate PCI busses */
	foreach_nodes(node) {
		printf("SCI%03x\n", node->sci);
		uint32_t vendev = dnc_read_conf(node->sci, 0, 0x14, 0, 0);
		/* Broadcom HT1000 PCI host bridges start at device 1 */
		int dev = vendev == VENDEV_SP5100 ? 0 : 1;
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

			/* Retry until allocation succeeds */
			(*c)->node->mmio32_base = map32->next;
			(*c)->node->mmio64_base = mmio64_cur;
			(*c)->node->io_base = io_cur;
		} while ((*c)->allocate());

		io_cur = roundup(io_cur, 1 << NC_ATT_IO_GRAN);
		map32->next = roundup(map32->next, 1 << NC_ATT_MMIO32_GRAN);

		mmio64_cur = roundup(mmio64_cur, (uint64_t)SCC_ATT_GRAN << DRAM_MAP_SHIFT);
		/* Address Numachip extended MMIO mask constraint */
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

	printf("Setting up MMIO32 ATTs:\n");
	foreach_slave_nodes(dnode) {
		printf("- 0x%x:0x%x to %03x\n",
			dnode->mmio32_base, dnode->mmio32_limit, dnode->sci);

		/* Ensure alignment */
		const uint32_t mask = (1 << NC_ATT_MMIO32_GRAN) - 1;
		assert((dnode->mmio32_base & mask) == 0 && (dnode->mmio32_limit & mask) == mask);

		for (uint32_t k = dnode->mmio32_base >> NC_ATT_MMIO32_GRAN; k < ((dnode->mmio32_limit + 1) >> NC_ATT_MMIO32_GRAN); k++) {
			foreach_nodes(node) {
				dnc_write_csr(node->sci, H2S_CSR_G3_NC_ATT_MAP_SELECT, NC_ATT_MMIO32 | (k / 256));
				dnc_write_csr(node->sci, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + (k % 256) * 4, dnode->sci);
			}
		}
	}

	printf("Setting up IO ATTs:\n");
	foreach_nodes(node)
		dnc_write_csr(node->sci, H2S_CSR_G3_NC_ATT_MAP_SELECT, NC_ATT_IO);

	foreach_nodes(dnode) {
		if (!dnode->io_limit || dnode->io_limit < dnode->io_base)
			continue;

		printf("- 0x%x:0x%x to %03x\n", dnode->io_base, dnode->io_limit, dnode->sci);

		/* Ensure alignment */
		const uint32_t mask = (1 << NC_ATT_IO_GRAN) - 1;
		assert((dnode->io_base & mask) == 0 && (dnode->io_limit & mask) == mask);

		for (uint32_t k = dnode->io_base >> NC_ATT_IO_GRAN; k < (dnode->io_limit + 1) >> NC_ATT_IO_GRAN; k++)
			foreach_nodes(node)
				dnc_write_csr(node->sci, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + (k % 256) * 4, dnode->sci);
	}

	printf("Setting up SCC ATTs for MMIO64:\n");
	foreach_nodes(dnode) {
		if (!dnode->mmio64_limit || dnode->mmio64_limit < dnode->mmio64_base)
			continue;

		printf("- 0x%llx:0x%llx to %03x\n", dnode->mmio64_base, dnode->mmio64_limit, dnode->sci);

		foreach_nodes(node)
			if (dnode->mmio64_limit > dnode->mmio64_base)
				scc_att_range(node->sci, dnode->mmio64_base, dnode->mmio64_limit, dnode->sci);
	}

	uint64_t tom = rdmsr(MSR_TOPMEM);

	/* Program IOAPIC address */
	foreach_slave_nodes(node) {
		/* Disable IOAPIC memory decode */
		uint32_t val = dnc_read_conf(node->sci, 0, 0x14, 0, 0x64);
		dnc_write_conf(node->sci, 0, 0x14, 0, 0x64, val & ~(1 << 3));

		/* Disable IOAPIC */
		ioh_ioapicind_write(node->sci, 0, 0);
	}

	foreach_nodes(node) {
		uint8_t ioh_ht = (dnc_read_conf(node->sci, 0, 24 + node->bsp_ht, FUNC0_HT, 0x60) >> 8) & 7;
		int range = 0;

		printf("Setting up NC MMIO ranges on SCI%03x with IOH at %d\n", node->sci, ioh_ht);

		/* Local MMIO to IOH */
		if (node == &nodes[0]) {
			nc_mmio_range(node->sci, range++, MMIO_VGA_BASE, MMIO_VGA_LIMIT, ioh_ht);

			for (unsigned erange = 0; erange < map32->excluded.used; erange++)
				nc_mmio_range(node->sci, range++, map32->excluded.elements[erange].start, map32->excluded.elements[erange].end, ioh_ht);
		} else
			if (node->mmio32_limit > node->mmio32_base)
				nc_mmio_range(node->sci, range++, node->mmio32_base, node->mmio32_limit, ioh_ht);

		while (range < 8)
			nc_mmio_range_del(node->sci, range++);

		if (node->mmio64_limit > node->mmio64_base)
			nc_mmio_range_high(node->sci, 0, node->mmio64_base, node->mmio64_limit, ioh_ht);
	}

	/* Slaves */
	foreach_slave_nodes(node) {
		uint8_t ioh_ht = (dnc_read_conf(node->sci, 0, 24 + node->bsp_ht, FUNC0_HT, 0x60) >> 8) & 7;
		uint8_t ioh_link = (dnc_read_conf(node->sci, 0, 24 + node->bsp_ht, FUNC0_HT, 0x64) >> 8) & 3;

		printf("Setting up NB MMIO ranges on SCI%03x with IOH at %d.%d\n", node->sci, ioh_ht, ioh_link);

		for (ht_t ht = node->nb_ht_lo; ht <= node->nb_ht_hi; ht++) {
			int range = 0;

			mmio_range(node->sci, ht, range++, MMIO_VGA_BASE, MMIO_VGA_LIMIT, node->nc_ht, 0, 1, 0);

			/* 32-bit under local range */
			if (node->mmio32_base > tom)
				mmio_range(node->sci, ht, range++, tom, node->mmio32_base - 1, node->nc_ht, 0, 1, 0);

			/* 32-bit local range */
			if (node->mmio32_limit > node->mmio32_base)
				mmio_range(node->sci, ht, range++, node->mmio32_base, node->mmio32_limit, ioh_ht, ioh_link, 1, 0);

			/* 32-bit above local range */
			mmio_range(node->sci, ht, range++, node->mmio32_limit + 1, 0xffffffff, node->nc_ht, 0, 1, 0);

			while (range < 8)
				mmio_range_del(node->sci, ht, range++);

			/* Skip CSR range */
			range = 9;

			/* 64-bit under local range */
			if (node->mmio64_base > mmio64_base)
				mmio_range(node->sci, ht, range++, mmio64_base, node->mmio64_base - 1, node->nc_ht, 0, 1, 0);

			/* 64-bit local range */
			if (node->mmio64_limit > node->mmio64_base)
				mmio_range(node->sci, ht, range++, node->mmio64_base, node->mmio64_limit, ioh_ht, ioh_link, 1, 0);

			/* 64-bit above local range */
			if (nodes[dnc_node_count - 1].mmio64_limit > node->mmio64_limit && node < &nodes[dnc_node_count - 1])
				mmio_range(node->sci, ht, range++, node->mmio64_limit + 1, nodes[dnc_node_count - 1].mmio64_limit, node->nc_ht, 0, 1, 0);
		}
	}

	/* Master */
	{
		uint8_t ioh_ht = (dnc_read_conf(nodes[0].sci, 0, 24 + nodes[0].bsp_ht, FUNC0_HT, 0x60) >> 8) & 7;
		uint8_t ioh_link = (dnc_read_conf(nodes[0].sci, 0, 24 + nodes[0].bsp_ht, FUNC0_HT, 0x64) >> 8) & 3;
		printf("Setting up NB MMIO ranges on SCI%03x with IOH at %d.%d\n", nodes[0].sci, ioh_ht, ioh_link);

		critical_enter();
		for (ht_t ht = nodes[0].nb_ht_lo; ht <= nodes[0].nb_ht_hi; ht++) {
			int range = 0;

			mmio_range(nodes[0].sci, ht, range++, MMIO_VGA_BASE, MMIO_VGA_LIMIT, ioh_ht, ioh_link, 1, 0);

			/* Merge adjacent remote ranges on slaves */
			uint32_t start = nodes[1].mmio32_base;
			uint32_t end = nodes[1].mmio32_limit;

			for (node_info_t *node = &nodes[2]; node < &nodes[dnc_node_count]; node++) {
				if (node->mmio32_base == (end + 1)) {
					end = node->mmio32_limit;
					continue;
				}

				mmio_range(nodes[0].sci, ht, range++, start, end, nodes[0].nc_ht, 0, 1, 0);
				start = node->mmio32_base;
				end = node->mmio32_limit;
			}

			if (end > start)
				mmio_range(nodes[0].sci, ht, range++, start, end, nodes[0].nc_ht, 0, 1, 0);

			for (unsigned erange = 0; erange < map32->excluded.used; erange++)
				mmio_range(nodes[0].sci, ht, range++, map32->excluded.elements[erange].start, map32->excluded.elements[erange].end, ioh_ht, ioh_link, 1, 0);

			while (range < 8)
				mmio_range_del(nodes[0].sci, ht, range++);

			/* Skip CSR ranges */
			range = 9;

			if (nodes[0].mmio64_limit > nodes[0].mmio64_base)
				mmio_range(nodes[0].sci, ht, range++, nodes[0].mmio64_base, nodes[0].mmio64_limit, ioh_ht, ioh_link, 1, 0);

			if (nodes[dnc_node_count - 1].mmio64_limit > nodes[1].mmio64_base)
				mmio_range(nodes[0].sci, ht, range++, nodes[1].mmio64_base, nodes[dnc_node_count - 1].mmio64_limit, nodes[0].nc_ht, 0, 1, 0);
		}
		critical_leave();
	}
}
