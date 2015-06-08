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

#define MMIO32_LIMIT 0xfe000000
#define MMIO64_GRAN (128 << 20)
#define MMIO_MIN_GAP (32 << 20)
#define MMIO_VGA_BASE 0xa0000
#define MMIO_VGA_LIMIT 0xbffff
#define IO_BASE 0xf00000
#define IO_STEP 0x100000
#define IO_LIMIT 0xffffff

extern uint64_t mmio64_base, mmio64_limit;

void dump_device(const sci_t sci, const int bus, const int dev, const int fn);
void setup_mmio(void);

#endif

