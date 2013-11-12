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

#include "dnc-maps.h"

#include <stdio.h>

#include "dnc-access.h"
#include "dnc-commonlib.h"
#include "dnc-masterlib.h"
#include "dnc-defs.h"
#include "dnc-regs.h"

static node_info_t *sci_to_node(const uint16_t sci)
{
	if (sci == 0xfff0)
		return &nodes[0];

	for (int i = 0; i < dnc_node_count; i++)
		if (nodes[i].sci == sci)
			return &nodes[i];

	fatal("Unable to find SCI%03x in nodes table", sci);
}

bool dram_range_read(const uint16_t sci, const int ht, const int range, uint64_t *base, uint64_t *limit, int *dest)
{
	assert(range < 8);

	uint32_t base_l = dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x40 + range * 8);
	uint32_t limit_l = dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x44 + range * 8);
	uint32_t base_h = dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x140 + range * 8);
	uint32_t limit_h = dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x144 + range * 8);

	*base = ((uint64_t)(base_l & ~0xffff) << (24 - 16)) | ((uint64_t)(base_h & 0xff) << 40);
	*limit = ((uint64_t)(limit_l & ~0xffff) << (24 - 16)) | ((uint64_t)(limit_h & 0xff) << 40);
	*dest = limit_l & 7;

	/* Ensure read and write bits are consistent */
	assert(!(base_l & 1) == !(base_l & 2));
	bool en = base_l & 1;
	if (en)
		*limit |= 0xffffff;

	return en;
}

int dram_range_unused(const uint16_t sci, const int ht)
{
	uint64_t base, limit;
	int dest;

	for (int range = 0; range < 8; range++)
		if (!dram_range_read(sci, ht, range, &base, &limit, &dest))
			return range;

	fatal("No free DRAM ranges on SCI%03x\n", sci);
}

void dram_range_print(const uint16_t sci, const int ht, const int range)
{
	uint64_t base, limit;
	int dest;

	assert(range < 8);

	if (dram_range_read(sci, ht, range, &base, &limit, &dest))
		printf("SCI%03x#%d DRAM range %d: 0x%012llx:0x%012llx to %d\n", sci, ht, range, base, limit, dest);
}

void dram_range(const uint16_t sci, const int ht, const int range, const uint64_t base, const uint64_t limit, const int dest)
{
	assert(dest < 8);
	assert(range < 8);

	if (verbose > 1)
		printf("SCI%03x#%d adding DRAM range %d: 0x%012llx:0x%012llx to %d\n", sci, ht, range, base, limit, dest);

	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x144 + range * 8, limit >> 40);
	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x44 + range * 8, ((limit >> 8) & ~0xffff) | dest);
	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x140 + range * 8, base >> 40);
	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x40 + range * 8, (base >> 8) | 3);
}

void dram_range_del(const uint16_t sci, const int ht, const int range)
{
	assert(range < 8);
	if (verbose > 2)
		printf("Deleting DRAM range %d on SCI%03x#%d\n", range, sci, ht);

	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x144 + range * 8, 0);
	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x44 + range * 8, 0);
	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x140 + range * 8, 0);
	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x40 + range * 8, 0);
}

bool mmio_range_read(const uint16_t sci, const int ht, int range, uint64_t *base, uint64_t *limit, int *dest, int *link, bool *lock)
{
	if (family >= 0x15) {
		assert(range < 12);

		int loff = 0, hoff = 0;
		if (range > 7) {
			loff = 0xe0;
			hoff = 0x20;
		}

		/* Skip disabled ranges */
		uint32_t a = dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x80 + loff + range * 8);
		uint32_t b = dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x84 + loff + range * 8);
		uint32_t c = dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x180 + hoff + range * 4);

		*base = ((uint64_t)(a & ~0xff) << 8) | ((uint64_t)(c & 0xff) << 40);
		*limit = ((uint64_t)(b & ~0xff) << 8) | ((uint64_t)(c & 0xff0000) << (40 - 16)) | 0xffff;
		*dest = b & 7;
		*link = (b >> 4) & 3;

		/* Ensure read and write bits are consistent */
		assert(!(a & 1) == !(a & 2));
		*lock = a & 8;
		return a & 3;
	}

	/* Family 10h */
	if (range < 8) {
		/* Skip disabled ranges */
		uint32_t a = dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x80 + range * 8);
		uint32_t b = dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x84 + range * 8);

		*base = (uint64_t)(a & ~0xff) << 8;
		*limit = ((uint64_t)(b & ~0xff) << 8) | 0xffff;
		*dest = b & 7;
		*link = (b >> 4) & 3;
		*lock = a & 8;
		return a & 3;
	}

	assert(range < 12);
	range -= 8;

	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x110, (2 << 28) | range);
	uint32_t a = dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x114);
	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x110, (3 << 28) | range);
	uint32_t b = dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x114);

	/* 128MB granularity is setup earlier */
	*base = (a & ~0xe00000ff) << (27 - 8);
	*limit = (~((b >> 8) & 0x1fffff)) << 20;
	*lock = 0;
	if (a & (1 << 6)) {
		*dest = 0;
		*link = a & 3;
		/* Assert sublink is zero as we ignore it */
		assert((a & 4) == 0);
	} else {
		*dest = a & 7;
		*link = 0;
	}

	return b & 1;
}

void mmio_range_print(const uint16_t sci, const int ht, const int range)
{
	uint64_t base, limit;
	int dest, link;
	bool lock;

	assert(range < 8);

	if (mmio_range_read(sci, ht, range, &base, &limit, &dest, &link, &lock))
		printf("SCI%03x#%d MMIO range %d: 0x%08llx:0x%08llx to %d.%d%s\n",
			sci, ht, range, base, limit, dest, link, lock ? " locked" : "");
}

void mmio_range(const uint16_t sci, const int ht, uint8_t range, uint64_t base, uint64_t limit, const int dest, const int link, const bool ovw)
{
	if (verbose > 1)
		printf("Adding MMIO range %d on SCI%03x#%x: 0x%08llx:0x%08llx to %d.%d\n",
			range, sci, ht, base, limit, dest, link);

	if (family >= 0x15) {
		assert(range < 12);

		int loff = 0, hoff = 0;
		if (range > 7) {
			loff = 0xe0;
			hoff = 0x20;
		}

		uint32_t val = dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x80 + loff + range * 8);
		if ((val & 3) && !ovw) {
			uint64_t base2, limit2;
			int dest2, link2;
			bool lock2;
			mmio_range_read(sci, ht, range, &base2, &limit2, &dest2, &link2, &lock2);
			fatal("Overwriting SCI%03x#%d MMIO range %d on 0x%08llx:0x%08llx to %d.%d%s", sci, ht, range, base2, limit2, dest2, link2, lock2 ? " locked" : "");
		}

		uint32_t val2 = ((base >> 16) << 8) | 3;
		uint32_t val3 = ((limit >> 16) << 8) | dest | (link << 4);
		uint32_t val4 = ((limit >> 40) << 16) | (base >> 40);

		/* Check if locked */
		if ((val & 8) && ((val2 != (val & ~8))
		  || (val3 != dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x84 + range * 8))
		  || (val4 != dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x180 + hoff + range * 4)))) {
			uint64_t old_base, old_limit;
			int old_dest, old_link;
			bool old_lock;

			mmio_range_read(sci, ht, range, &old_base, &old_limit, &old_dest, &old_link, &old_lock);
			warning("Unable to overwrite locked MMIO range %d on SCI%03x#%d 0x%llx:0x%llx to %d.%d with 0x%llx:0x%llx to %d.%d",
				range, sci, ht, old_base, old_limit, old_dest, old_link, base, limit, dest, link);
			return;
		}

		dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x180 + hoff + range * 4, val4);
		dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x84 + loff + range * 8, val3);
		dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x80 + loff + range * 8, val2);
		return;
	}

	/* Family 10h */
	if (range < 8) {
		assert(limit < (1ULL << 40));
		uint32_t val = dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x80 + range * 8);
		if ((val & 3) && !ovw) {
			uint64_t base2, limit2;
			int dest2, link2;
			bool lock2;
			mmio_range_read(sci, ht, range, &base2, &limit2, &dest2, &link2, &lock2);
			fatal("Overwriting SCI%03x#%d MMIO range %d on 0x%08llx:0x%08llx to %d.%d%s", sci, ht, range, base2, limit2, dest2, link2, lock2 ? " locked" : "");
		}

		uint32_t val2 = ((base >> 16) << 8) | 3;
		uint32_t val3 = ((limit >> 16) << 8) | dest | (link << 4);

		/* Check if locked */
		if ((val & 8) && ((val2 != (val & ~8))
		  || (val3 != dnc_read_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x84 + range * 8)))) {
			uint64_t old_base, old_limit;
			int old_dest, old_link;
			bool old_lock;

			mmio_range_read(sci, ht, range, &old_base, &old_limit, &old_dest, &old_link, &old_lock);
			warning("Unable to overwrite locked MMIO range %d on SCI%03x#%d 0x%llx:0x%llx to %d.%d with 0x%llx:0x%llx to %d.%d",
				sci, ht, range, old_base, old_limit, old_dest, old_link, base, limit, dest, link);
			return;
		}

		dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x84 + range * 8, val3);
		dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x80 + range * 8, val2);
		return;
	}

	assert(range < 12);
	range -= 8;

	/* Reading an uninitialised extended MMIO ranges results in MCE, so can't assert */

	uint64_t mask = 0;
	base  >>= 27;
	limit >>= 27;

	while ((base | mask) != (limit | mask))
		mask = (mask << 1) | 1;

	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x110, (2 << 28) | range);
	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x114, (base << 8) | dest);
	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x110, (3 << 28) | range);
	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x114, (mask << 8) | 1);
}

void mmio_range_del(const uint16_t sci, const int ht, uint8_t range)
{
	if (verbose > 2)
		printf("Deleting MMIO range %d on SCI%03x#%x\n", range, sci, ht);

	if (family >= 0x15) {
		assert(range < 12);

		int loff = 0, hoff = 0;
		if (range > 7) {
			loff = 0xe0;
			hoff = 0x20;
		}

		dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x80 + loff + range * 8, 0);
		dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x84 + loff + range * 8, 0);
		dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x180 + hoff + range * 4, 0);
		return;
	}

	/* Family 10h */
	if (range < 8) {
		dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x84 + range * 8, 0);
		dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x80 + range * 8, 0);
		return;
	}

	assert(range < 12);
	range -= 8;

	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x110, (2 << 28) | range);
	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x114, 0);
	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x110, (3 << 28) | range);
	dnc_write_conf(sci, 0, 24 + ht, FUNC1_MAPS, 0x114, 0);
}

void nc_mmio_range(const uint16_t sci, const int range, const uint64_t base, const uint64_t limit, const uint8_t dht)
{
	if (verbose > 1)
		printf("Adding Numachip MMIO range %d on SCI%03x: 0x%08llx:0x%08llx to %d\n",
			range, sci, base, limit, dht);

	uint8_t ht = sci_to_node(sci)->nc_ht;
	uint32_t a = ((base >> 16) << 8) | 3;
	uint32_t b = ((limit >> 16) << 8) | dht;

	dnc_write_conf(sci, 0, 24 + ht, 1, H2S_CSR_F1_RESOURCE_MAPPING_ENTRY_INDEX, range);
	dnc_write_conf(sci, 0, 24 + ht, 1, H2S_CSR_F1_MMIO_BASE_ADDRESS_REGISTERS, a);
	dnc_write_conf(sci, 0, 24 + ht, 1, H2S_CSR_F1_MMIO_LIMIT_ADDRESS_REGISTERS, b);
}

void nc_mmio_range_del(const uint16_t sci, const int range)
{
	if (verbose > 2)
		printf("Deleting Numachip MMIO range %d on SCI%03x\n", range, sci);

	uint8_t ht = sci_to_node(sci)->nc_ht;

	dnc_write_conf(sci, 0, 24 + ht, 1, H2S_CSR_F1_RESOURCE_MAPPING_ENTRY_INDEX, range);
	dnc_write_conf(sci, 0, 24 + ht, 1, H2S_CSR_F1_MMIO_BASE_ADDRESS_REGISTERS, 0);
	dnc_write_conf(sci, 0, 24 + ht, 1, H2S_CSR_F1_MMIO_LIMIT_ADDRESS_REGISTERS, 0);
}

bool nc_mmio_range_read(const uint16_t sci, const int range, uint64_t *base, uint64_t *limit, uint8_t *dht)
{
	uint8_t ht = sci_to_node(sci)->nc_ht;

	dnc_write_conf(sci, 0, 24 + ht, 1, H2S_CSR_F1_RESOURCE_MAPPING_ENTRY_INDEX, range);
	uint32_t a = dnc_read_conf(sci, 0, 24 + ht, 1, H2S_CSR_F1_MMIO_BASE_ADDRESS_REGISTERS);
	uint32_t b = dnc_read_conf(sci, 0, 24 + ht, 1, H2S_CSR_F1_MMIO_LIMIT_ADDRESS_REGISTERS);

	*base = (uint64_t)(a & ~0xff) << (16 - 8);
	*limit = ((uint64_t)(b & ~0xff) << (16 - 8)) | 0xffff;
	*dht = b & 7;

	/* Ensure read and write bits are consistent */
	assert(!(a & 1) == !(a & 2));

	return a & 3;
}

void nc_mmio_range_print(const uint16_t sci, const int range)
{
	uint64_t base, limit;
	uint8_t dht;

	if (nc_mmio_range_read(sci, range, &base, &limit, &dht))
		printf("SCI%03x MMIO range %d: 0x%08llx:0x%08llx to %d\n", sci, range, base, limit, dht);
}

void nc_dram_range(const uint16_t sci, const int range, const uint64_t base, const uint64_t limit, const uint8_t dht)
{
	if (verbose > 1)
		printf("Adding Numachip DRAM range %d on SCI%03x: 0x%012llx:0x%012llx to %d\n",
			range, sci, base, limit, dht);

	uint8_t ht = sci_to_node(sci)->nc_ht;
	uint32_t a = ((base >> 24) << 8) | 3;
	uint32_t b = ((limit >> 24) << 8) | dht;

	dnc_write_conf(sci, 0, 24 + ht, 1, H2S_CSR_F1_RESOURCE_MAPPING_ENTRY_INDEX, range);
	dnc_write_conf(sci, 0, 24 + ht, 1, H2S_CSR_F1_DRAM_BASE_ADDRESS_REGISTERS, a);
	dnc_write_conf(sci, 0, 24 + ht, 1, H2S_CSR_F1_DRAM_LIMIT_ADDRESS_REGISTERS, b);
}

void nc_dram_range_del(const uint16_t sci, const int range)
{
	if (verbose > 2)
		printf("Deleting Numachip DRAM range %d on SCI%03x\n", range, sci);

	uint8_t ht = sci_to_node(sci)->nc_ht;

	dnc_write_conf(sci, 0, 24 + ht, 1, H2S_CSR_F1_RESOURCE_MAPPING_ENTRY_INDEX, range);
	dnc_write_conf(sci, 0, 24 + ht, 1, H2S_CSR_F1_DRAM_BASE_ADDRESS_REGISTERS, 0);
	dnc_write_conf(sci, 0, 24 + ht, 1, H2S_CSR_F1_DRAM_LIMIT_ADDRESS_REGISTERS, 0);
}

bool nc_dram_range_read(const uint16_t sci, const int range, uint64_t *base, uint64_t *limit, uint8_t *dht)
{
	uint8_t ht = sci_to_node(sci)->nc_ht;

	dnc_write_conf(sci, 0, 24 + ht, 1, H2S_CSR_F1_RESOURCE_MAPPING_ENTRY_INDEX, range);
	uint32_t a = dnc_read_conf(sci, 0, 24 + ht, 1, H2S_CSR_F1_DRAM_BASE_ADDRESS_REGISTERS);
	uint32_t b = dnc_read_conf(sci, 0, 24 + ht, 1, H2S_CSR_F1_DRAM_LIMIT_ADDRESS_REGISTERS);

	*base = (uint64_t)(a & ~0xff) << (24 - 8);
	*limit = ((uint64_t)(b & ~0xff) << (24 - 8)) | 0xffffff;
	*dht = b & 7;

	/* Ensure read and write bits are consistent */
	assert(!(a & 1) == !(a & 2));

	return a & 3;
}

void nc_dram_range_print(const uint16_t sci, const int range)
{
	uint64_t base, limit;
	uint8_t dht;

	if (nc_dram_range_read(sci, range, &base, &limit, &dht))
		printf("SCI%03x DRAM range %d: 0x%012llx:0x%012llx to %d\n", sci, range, base, limit, dht);
}

void ranges_print(void)
{
	uint64_t base, base2, limit, limit2;
	int node, range, dest, dest2, link, link2;
	bool en, en2, lock, lock2;
	ht_t ht;

	printf("Northbridge DRAM ranges:\n");
	for (node = 0; node < dnc_node_count; node++) {
		for (range = 0; range < 8; range++) {
			dram_range_print(nodes[node].sci, nodes[node].bsp_ht, range);

			/* Verify consistency */
			en = dram_range_read(nodes[node].sci, nodes[node].bsp_ht, range, &base, &limit, &dest);
			for (ht = nodes[node].nb_ht_lo; ht <= nodes[node].nb_ht_hi; ht++) {
				en2 = dram_range_read(nodes[node].sci, nodes[node].bsp_ht, range, &base2, &limit2, &dest2);
				assert(en2 == en && base2 == base && limit2 == limit && dest2 == dest);
			}
		}
		printf("\n");
	}

	printf("Numachip DRAM ranges:\n");
	for (node = 0; node < dnc_node_count; node++)
		for (range = 0; range < 8; range++)
			nc_dram_range_print(nodes[node].sci, range);

	printf("\nNorthbridge MMIO ranges:\n");
	for (node = 0; node < dnc_node_count; node++) {
		for (range = 0; range < 8; range++) {
			mmio_range_print(nodes[node].sci, nodes[node].bsp_ht, range);

			/* Verify consistency */
			en = mmio_range_read(nodes[node].sci, nodes[node].bsp_ht, range, &base, &limit, &dest, &link, &lock);
			for (ht = nodes[node].nb_ht_lo; ht <= nodes[node].nb_ht_hi; ht++) {
				en2 = mmio_range_read(nodes[node].sci, nodes[node].bsp_ht, range, &base2, &limit2, &dest2, &link2, &lock2);
				assert(en2 == en && base2 == base && limit2 == limit && dest2 == dest && link2 == link && lock2 == lock);
			}
		}
		printf("\n");
	}

	printf("Numachip MMIO ranges:\n");
	for (node = 0; node < dnc_node_count; node++)
		for (range = 0; range < 8; range++)
			nc_mmio_range_print(nodes[node].sci, range);

	printf("\nNumachip SCC routing:\n");

	/* Select SCC ATT base address */
	dnc_write_csr(0xfff0, H2S_CSR_G0_ATT_INDEX, 1 << (27 + SCC_ATT_INDEX_RANGE));
	uint32_t i, last = dnc_read_csr(0xfff0, H2S_CSR_G0_ATT_ENTRY);
	printf("SCI%03x: 0x%012llx:", last, 0ULL);

	/* Select SCC ATT base address, enable autoinc */
	for (node = 0; node < dnc_node_count; node++)
		dnc_write_csr(nodes[node].sci, H2S_CSR_G0_ATT_INDEX, (1 << 31) |  (1 << (27 + SCC_ATT_INDEX_RANGE)));

	for (i = 0; i < 4096; i++) {
		uint32_t sci = dnc_read_csr(0xfff0, H2S_CSR_G0_ATT_ENTRY);

		/* Verify consistency */
		for (node = 1; node < dnc_node_count; node++) {
			uint32_t sci2 = dnc_read_csr(nodes[node].sci, H2S_CSR_G0_ATT_ENTRY);
			if (sci2 != sci)
				warning("SCC address 0x%012llx routes to SCI%03x on SCI000 but routes to SCI%03x on SCI%03x",
				  (uint64_t)i * (SCC_ATT_GRAN << DRAM_MAP_SHIFT), sci, sci2, nodes[node].sci);
		}

		if (sci != last) {
			uint64_t addr = (uint64_t)i * (SCC_ATT_GRAN << DRAM_MAP_SHIFT);
			printf("0x%012llx\nSCI%03x: 0x%012llx:", addr - 1, sci, addr);
			last = sci;
		}
	}

	printf("0x%012llx\n", ((uint64_t)i * (SCC_ATT_GRAN << DRAM_MAP_SHIFT)) - 1);

	printf("\nNumachip MMIO32 routing:\n");
	dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, NC_ATT_MMIO32);
	last = dnc_read_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0);
	printf("SCI%03x: 0x%08x:", last, 0);

	for (i = 0; i < 4096; i++) {
		/* Select MMIO32 ATT RAM on all nodes */
		if ((i % 256) == 0)
			for (node = 0; node < dnc_node_count; node++)
				dnc_write_csr(nodes[node].sci, H2S_CSR_G3_NC_ATT_MAP_SELECT, NC_ATT_MMIO32 | (i / 256));

		uint32_t sci = dnc_read_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + (i % 256) * 4);

		/* Verify consistency */
		for (node = 1; node < dnc_node_count; node++) {
			uint32_t sci2 = dnc_read_csr(nodes[node].sci, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + (i % 256) * 4);
			if (sci2 != sci)
				warning("MMIO32 address 0x%08x routes to SCI%03x on SCI000 but routes to SCI%03x on SCI%03x",
				  i << NC_ATT_MMIO32_GRAN, sci, sci2, nodes[node].sci);
		}

		if (sci != last) {
			uint32_t addr = i << NC_ATT_MMIO32_GRAN;
			printf("0x%08x\nSCI%03x: 0x%08x:", addr - 1, sci, addr);
			last = sci;
		}
	}
	printf("0x%08x\n\n", (i << NC_ATT_MMIO32_GRAN) - 1);

	printf("Numachip IO routing:\n");
	dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, NC_ATT_IO);
	last = dnc_read_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0);
	printf("SCI%03x: 0x%07x:", last, 0);

	/* Select IO ATT RAM on all nodes */
	for (node = 0; node < dnc_node_count; node++)
		dnc_write_csr(nodes[node].sci, H2S_CSR_G3_NC_ATT_MAP_SELECT, NC_ATT_IO);

	for (i = 0; i < 256; i++) {
		uint32_t sci = dnc_read_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4);

		/* Verify consistency */
		for (node = 1; node < dnc_node_count; node++) {
			uint32_t sci2 = dnc_read_csr(nodes[node].sci, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4);
			if (sci2 != sci)
				warning("IO address 0x%07x routes to SCI%03x on SCI000 but routes to SCI%03x on SCI%03x",
				  i << NC_ATT_IO_GRAN, sci, sci2, nodes[node].sci);
		}

		if (sci != last) {
			uint32_t addr = i << NC_ATT_IO_GRAN;
			printf("0x%07x\nSCI%03x: 0x%07x:", addr - 1, sci, addr);
			last = sci;
		}
	}
	printf("0x%07x\n\n", (i << NC_ATT_IO_GRAN) - 1);
}

