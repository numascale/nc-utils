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
#include "dnc-maps.h"
#include "dnc-devices.h"
#include "dnc-platform.h"
#include "../interface/numachip-autodefs.h"

#define GRAN (1 << 20)

static uint64_t mmio_cur;
static uint64_t tmp_base[2], tmp_lim[2];

static bool size_bar(const uint16_t sci, const int bus, const int dev, const int fn, const int reg, uint64_t *addr, uint64_t *len, bool *pref, bool *io)
{
	uint32_t save = dnc_read_conf(sci, bus, dev, fn, reg);
	*io = save & 1;
	uint32_t mask = *io ? 3 : 15;
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
		printf(" 0x%llx,%s%s", *addr, pr_size(*len), *pref ? "(P)" : "");

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
							tmp_lim[i] = roundup(tmp_lim[i], GRAN) - 1;

						printf("window %d: 0x%08llx - 0x%08llx\n", i, tmp_base[i], tmp_lim[i]);

						if (tmp_base[i] < nc_node[0].mmio_base)
							nc_node[0].mmio_base = tmp_base[i];
						if (tmp_lim[i] > nc_node[0].mmio_limit)
							nc_node[0].mmio_limit = tmp_lim[i];
					}
				} else {
					/* Bridge requires 1MB alignment */
					mmio_cur = roundup(mmio_cur, GRAN);

					assert(mmio_cur < 0xffffffff);
					uint32_t bridge_start = mmio_cur & 0xffffffff;
					pci_search(sci, sec, scope, barfn);
					mmio_cur = roundup(mmio_cur, GRAN);
					assert(mmio_cur < 0xffffffff);
					uint32_t bridge_end = (mmio_cur - 1) & 0xffffffff;

					if (bridge_end > bridge_start) {
						printf("window: 0x%08x - 0x%08x\n", bridge_start, bridge_end);
						val = ((roundup(bridge_end, GRAN) - 1) & ~0xfffff) | ((bridge_start >> 16) & ~0xf);
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

/* Called for the master only */
void setup_mmio_master(void)
{
	nc_node[0].mmio_base = 0xffffffff;
	nc_node[0].mmio_limit = 0x00000000;

	/* Adjusted down later */
	for (int i = 0; i < 2; i++) {
		tmp_base[i] = 0xffffffff;
		tmp_lim[i] = 0;
	}

	printf("\nScoping master PCI tree:\n");
	critical_enter();
	pci_search(0xfff0, 0, 1, scope_bar);
	critical_leave();
	printf("Master MMIO range 0x%llx - 0x%llx\n\n", nc_node[0].mmio_base, nc_node[0].mmio_limit);

	/* Check if there is space for another MMIO window, else wrap */
	uint64_t len = nc_node[0].mmio_limit - nc_node[0].mmio_base + 1;
	mmio_cur = nc_node[0].mmio_limit + 1;
	if ((mmio_cur + len) >= MMIO_LIMIT) {
		nc_node[0].mmio_limit = 0xffffffff;
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
	uint16_t sci = nc_node[node].sci;

	printf("Setting up PCI routing on SCI%03x from 0x%llx\n", sci, mmio_cur);
	nc_node[node].mmio_base = mmio_cur;
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

	mmio_cur = roundup(mmio_cur, GRAN) - 1;
	nc_node[node].mmio_limit = mmio_cur;
	printf("SCI%03x MMIO range 0x%llx - 0x%llx\n\n", sci, nc_node[node].mmio_base, nc_node[node].mmio_limit);

	/* Check if there is space for another MMIO window, else wrap */
	uint64_t len = nc_node[node].mmio_limit - nc_node[node].mmio_base + 1;
	mmio_cur = nc_node[node].mmio_limit + 1;
	if ((mmio_cur + len) >= MMIO_LIMIT) {
		nc_node[node].mmio_limit = 0xffffffff;
		mmio_cur = rdmsr(MSR_TOPMEM);
	}
}

void setup_mmio_late(void)
{
	printf("Setting up MMIO32 ATT (default SCI000):\n");
	for (int i = 0; i < 16; i++) {
		for (int node = 0; node < dnc_node_count; node++)
			dnc_write_csr(nc_node[node].sci, H2S_CSR_G3_NC_ATT_MAP_SELECT, (1 << 4) | i);

		for (int j = 0; j < 256; j++)
			for (int node = 0; node < dnc_node_count; node++)
				dnc_write_csr(nc_node[node].sci, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + j * 4, 0);
	}

	/* Skip range to SCI000, as it's default */
	for (int dnode = 1; dnode < dnc_node_count; dnode++) {
		printf("SCI%03x: 0x%llx - 0x%llx\n",
			nc_node[dnode].sci, nc_node[dnode].mmio_base, nc_node[dnode].mmio_limit);

		/* Verify constraints */
		assert((nc_node[dnode].mmio_base & 0xfffff) == 0 && (nc_node[dnode].mmio_limit & 0xfffff) == 0xfffff);
		assert(nc_node[dnode].mmio_base < 0xffffffff && nc_node[dnode].mmio_limit <= 0xffffffff);

		for (uint64_t k = nc_node[dnode].mmio_base >> 20; k < (nc_node[dnode].mmio_limit + 1) >> 20; k++) {
			if ((k % 256) == 0)
				for (int node = 0; node < dnc_node_count; node++)
					dnc_write_csr(nc_node[node].sci, H2S_CSR_G3_NC_ATT_MAP_SELECT, (1 << 4) | (k / 256));

			for (int node = 0; node < dnc_node_count; node++)
				dnc_write_csr(nc_node[node].sci, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + (k % 256) * 4, nc_node[dnode].sci);
		}
	}

	uint64_t tom = rdmsr(MSR_TOPMEM);

	for (int i = 0; i < dnc_node_count; i++) {
		uint16_t sci = nc_node[i].sci;
		uint8_t ioh_ht = (dnc_read_conf(sci, 0, 24 + nc_node[i].bsp_ht, FUNC0_HT, 0x60) >> 8) & 7;
		int range = 0;

		printf("Setting up NC MMIO ranges on SCI%03x with IOH at %d\n", sci, ioh_ht);

		/* Local MMIO to IOH */
		nc_mmio_range(sci, range++, nc_node[i].mmio_base, nc_node[i].mmio_limit, ioh_ht);

		/* Master below and above ranges */
		if (i == 0) {
			if (nc_node[i].mmio_base > tom)
				nc_mmio_range(sci, range++, tom, nc_node[i].mmio_base - 1, ioh_ht);

			nc_mmio_range(sci, range++, nc_node[dnc_node_count - 1].mmio_limit + 1, 0xffffffff, ioh_ht);
		}

		while (range < 8)
			nc_mmio_range_del(sci, range++);
	}

	/* Slaves */
	for (int i = 1; i < dnc_node_count; i++) {
		uint16_t sci = nc_node[i].sci;
		uint8_t ioh_ht = (dnc_read_conf(sci, 0, 24 + nc_node[i].bsp_ht, FUNC0_HT, 0x60) >> 8) & 7;
		uint8_t ioh_link = (dnc_read_conf(sci, 0, 24 + nc_node[i].bsp_ht, FUNC0_HT, 0x64) >> 8) & 3;

		printf("Setting up NB MMIO ranges on SCI%03x with IOH at %d.%d\n", sci, ioh_ht, ioh_link);

		for (int ht = 0; ht < 8; ht++) {
			if (!nc_node[i].ht[ht].cpuid)
				continue;

			/* Skip first range; write it later */
			int range = 0;

			/* Ranges to master; local range first, as it can be locked by Supermicro BIOS */
			mmio_range(sci, ht, range++, nc_node[0].mmio_base, nc_node[0].mmio_limit, nc_node[i].nc_ht, 0);
			if (nc_node[0].mmio_base > tom)
				mmio_range(sci, ht, range++, tom, nc_node[0].mmio_base - 1, nc_node[i].nc_ht, 0);

			/* If gap between master and local, route to fabric */
			if (nc_node[i].mmio_base > (nc_node[0].mmio_limit + 1))
				mmio_range(sci, ht, range++, nc_node[0].mmio_limit + 1, nc_node[i].mmio_base - 1, nc_node[i].nc_ht, 0);

			/* Local range */
			mmio_range(sci, ht, range++, nc_node[i].mmio_base, nc_node[i].mmio_limit, ioh_ht, ioh_link);

			/* After-local MMIO range routing */
			mmio_range(sci, ht, range++, nc_node[i].mmio_limit + 1, 0xffffffff, nc_node[i].nc_ht, 0);

			while (range < 8)
				mmio_range_del(sci, ht, range++);
		}
	}

	/* Master */
	{
		uint8_t ioh_ht = (dnc_read_conf(0xfff0, 0, 24 + nc_node[0].bsp_ht, FUNC0_HT, 0x60) >> 8) & 7;
		uint8_t ioh_link = (dnc_read_conf(0xfff0, 0, 24 + nc_node[0].bsp_ht, FUNC0_HT, 0x64) >> 8) & 3;
		printf("Setting up NB MMIO ranges on SCI000 with IOH at %d.%d\n", ioh_ht, ioh_link);

		critical_enter();
		for (int ht = 0; ht < 8; ht++) {
			if (!nc_node[0].ht[ht].cpuid)
				continue;
			int range = 0;

			/* First range to local, as it can be locked by Supermicro BIOS */
			mmio_range(0xfff0, ht, range++, nc_node[0].mmio_base, nc_node[0].mmio_limit, ioh_ht, ioh_link);

			if (nc_node[0].mmio_base > tom)
				mmio_range(0xfff0, ht, range++, tom, nc_node[0].mmio_base - 1, ioh_ht, ioh_link);
			mmio_range(0xfff0, ht, range++, nc_node[0].mmio_limit + 1, nc_node[dnc_node_count - 1].mmio_limit, nc_node[0].nc_ht, 0);
			mmio_range(0xfff0, ht, range++, nc_node[dnc_node_count - 1].mmio_limit + 1, 0xffffffff, ioh_ht, ioh_link);

			while (range < 8)
				mmio_range_del(0xfff0, ht, range++);
		}
		critical_leave();
	}
}
