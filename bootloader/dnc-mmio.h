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

#ifndef __DNC_MMIO_H
#define __DNC_MMIO_H 1

#include <stdbool.h>

#define MAX_BRIDGES 4
#define MMIO_SMALL (2 << 20)

extern void tally_remote_node_mmio(uint16_t sci);
extern bool setup_remote_node_mmio(uint16_t sci);
extern void setup_mmio_early(void);
extern void setup_mmio_late(void);
extern void mmio_range_write(uint16_t sci, int range, uint64_t base, uint64_t limit, int ht, int link, int sublink);
extern void mmio_show(uint16_t sci);
extern void dram_show(uint16_t sci);

#endif

