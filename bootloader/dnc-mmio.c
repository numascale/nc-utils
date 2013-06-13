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
#include <stdbool.h>

#include "dnc-defs.h"
#include "dnc-access.h"
#include "dnc-commonlib.h"
#include "dnc-masterlib.h"
#include "dnc-bootloader.h"
#include "dnc-mmio.h"

#define GRAN (1 << 20)

static uint64_t mmio_cur, mmio_lim;
static uint64_t range_base[2], range_lim[2];

static bool size_bar(const uint16_t sci, const int bus, const int dev, const int fn, const int reg, uint64_t *addr, uint64_t *len, bool *pref)
{
	uint32_t save = dnc_read_conf(sci, bus, dev, fn, reg);
	bool mmio = (save & 1) == 0;
	uint32_t mask = mmio ? 15 : 3;
	bool s64 = ((save >> 1) & 3) == 3;
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
		printf(" len=%lld@0x%08llx", *len, *addr);

	/* Ignore low I/O windows for now */
	if (!mmio)
		*len = 0;

out:
	dnc_write_conf(sci, bus, dev, fn, reg, save);
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
	bool pref, more = size_bar(sci, bus, dev, fn, reg, &addr, &len, &pref);

	if (len) {
		if (addr < range_base[pref])
			range_base[pref] = addr;

		if ((addr + len) > range_lim[pref])
			range_lim[pref] = addr + len;
	}

	return more;
}

static bool setup_bar(const uint16_t sci, const int bus, const int dev, const int fn, const int reg)
{
	uint64_t len, addr;
	bool pref, more = size_bar(sci, bus, dev, fn, reg, &addr, &len, &pref);

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
					/* Adjusted down later */
					for (int i = 0; i < 2; i++) {
						range_base[i] = 0xffffffff;
						range_lim[i] = 0;
					}

					pci_search(sci, sec, scope, barfn);

					for (int i = 0; i < 2; i++) {
						/* Round up limit */
						if (range_lim[i])
							range_lim[i] = roundup(range_lim[i], GRAN) - 1;

						val = (range_lim[i] & ~0xfffff) | ((range_base[i] >> 16) & ~0xf);
						printf("bridge range %08llx to %08llx (before %08x, ", range_base[i], range_lim[i],
							dnc_read_conf(sci, bus, dev, fn, 0x20 + i * 4));
						dnc_write_conf(sci, bus, dev, fn, 0x20 + i * 4, val);
						printf("after %08x)\n", dnc_read_conf(sci, bus, dev, fn, 0x20 + i * 4));
					}

					/* Clear prefetchable upper 32-bits */
					dnc_write_conf(sci, bus, dev, fn, 0x28, 0);
					dnc_write_conf(sci, bus, dev, fn, 0x2c, 0);
				} else {
					/* Bridge requires 1MB alignment */
					mmio_cur = roundup(mmio_cur, GRAN);

					assert(mmio_cur < 0xffffffff);
					uint32_t bridge_start = mmio_cur & 0xffffffff;
					pci_search(sci, sec, scope, barfn);
					mmio_cur = roundup(mmio_cur, GRAN);
					assert(mmio_cur < 0xffffffff);
					uint32_t bridge_end = (mmio_cur - 1) & 0xffffffff;

					val = ((roundup(bridge_end, GRAN) - 1) & ~0xfffff) | ((bridge_start >> 16) & ~0xf);
					printf("bridge range %08x to %08x (before %08x)\n", bridge_start, bridge_end, dnc_read_conf(sci, bus, dev, fn, 0x20));
					dnc_write_conf(sci, bus, dev, fn, 0x20, val);
				}
			}

			/* If not multi-function, break out of function loop */
			if (!fn && !(type & 0x80))
				break;
		}
	}
}

/* rationale:
(start = TOM)
- for each server
	- adjust start up to IO granularity
	- for each pci bridge
		- start = cur
		- for each pci endpoint
			- probe and assign bars
			- add 64-bytes margin
		- set bridge decode range
	- map I/O forwarding on all numachips
*/

void setup_mmio_early(void)
{
	mmio_cur = tom;
	mmio_lim = 0xffffffff;

	printf("\nScoping master PCI tree:\n");
	pci_search(0xfff0, 0, 1, scope_bar);
}

void setup_mmio_late(void)
{
	/* Start from first slave */
	for (int i = 1; i < cfg_nodes; i++) {
		printf("\nSetting up PCI routing on SCI%03x from 0x%llx\n", cfg_nodelist[i].sciid, mmio_cur);
		pci_search(cfg_nodelist[i].sciid, 0, 0, setup_bar);
		mmio_cur = roundup(mmio_cur, GRAN);
	}
}

