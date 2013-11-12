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

#define MMIO_LIMIT 0xfec00000
#define MMIO_MIN_GAP (32 << 20)
#define MMIO64_MIN_SIZE (32 << 20)
#define MMIO_VGA_BASE 0xa0000
#define MMIO_VGA_LIMIT 0xbffff

extern void setup_mmio(void);
extern void setup_mmio_late(void);

#endif

