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

#define MAX_BRIDGES 4

extern void tally_remote_node_mmio(u16 sci);
extern int setup_remote_node_mmio(u16 sci);
extern void mmio_range_write(u16 sci, int range, u64 base, u64 limit, int ht, int link, int sublink);
extern void mmio_show(u16 sci);
extern void dram_show(u16 sci);

#endif

