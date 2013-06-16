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

#include <stdint.h>

bool dram_range_read(const uint16_t sci, const int ht, const int range, uint64_t *base, uint64_t *limit, int *dest);
int dram_range_unused(const uint16_t sci, const int ht);
void dram_range_print(const uint16_t sci, const int ht, const int range);
void dram_range(const uint16_t sci, const int ht, const int range, const uint32_t base, const uint32_t limit, const int dest);
void dram_range_del(const uint16_t sci, const int ht, const int range);
bool mmio_range_read(const uint16_t sci, const int ht, int range, uint64_t *base, uint64_t *limit, int *dest, int *link, bool *lock);
void mmio_range_print(const uint16_t sci, const int ht, const int range);
void mmio_range(const uint16_t sci, const int ht, uint8_t range, uint64_t base, uint64_t limit, const int dest, const int link);
void mmio_range_del(const uint16_t sci, const int ht, uint8_t range);
void nc_mmio_range(const uint16_t sci, const int range, const uint64_t base, const uint64_t limit, const uint8_t dht);
void nc_mmio_range_del(const uint16_t sci, const int range);
bool nc_mmio_range_read(const uint16_t sci, const int range, uint64_t *base, uint64_t *limit, uint8_t *dht);
void nc_mmio_range_print(const uint16_t sci, const int range);
void nc_dram_range_print(const uint16_t sci, const int range);
void nc_dram_range(const uint16_t sci, const int range, const uint64_t base, const uint64_t limit, const uint8_t dht);
void nc_dram_range_del(const uint16_t sci, const int range);
bool nc_dram_range_read(const uint16_t sci, const int range, uint64_t *base, uint64_t *limit, uint8_t *dht);
void nc_dram_range_print(const uint16_t sci, const int range);

