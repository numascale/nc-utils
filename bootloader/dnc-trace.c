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

#include "dnc-types.h"
#include "dnc-defs.h"
#include "dnc-access.h"

void tracing_arm(const sci_t sci, const uint8_t ht, const uint64_t trace_base, const uint64_t trace_limit)
{
	dnc_write_conf(sci, 0, 0x18 + ht, FUNC2_DRAM, 0xb8, ((trace_base >> 24) & 0xffff) | (((trace_limit >> 24) & 0xffff) << 16));
	dnc_write_conf(sci, 0, 0x18 + ht, FUNC2_DRAM, 0x120, (trace_base >> 40) | ((trace_limit >> 40) << 8) | ((trace_base >> 40) << 16));
	dnc_write_conf(sci, 0, 0x18 + ht, FUNC2_DRAM, 0xbc, trace_base >> 6);
}

void tracing_start(const sci_t sci, const uint8_t ht)
{
	dnc_write_conf(sci, 0, 0x18 + ht, FUNC2_DRAM, 0xc4, 0);
	dnc_write_conf(sci, 0, 0x18 + ht, FUNC2_DRAM, 0xc8, 1 < 31);

	uint32_t val = dnc_read_conf(sci, 0, 0x18 + ht, FUNC2_DRAM, 0xc0);
	dnc_write_conf(sci, 0, 0x18 + ht, FUNC2_DRAM, 0xc0, val & ~1);
	dnc_write_conf(sci, 0, 0x18 + ht, FUNC2_DRAM, 0xc0, 1 | (0 << 1) | (0 << 4) | (1 << 13) | (1 << 20) | (0 << 21) | (0 << 23) | (1 << 25));
	dnc_write_conf(sci, 0, 0x18 + ht, FUNC2_DRAM, 0xc8, 1 | (1 << 29));
	dnc_write_conf(sci, 0, 0x18 + ht, FUNC2_DRAM, 0xcc, (1 << 31) | (1 << 14) | (0x3f << 24) | (0x3f << 16));

	dnc_write_conf(sci, 0, 0x18 + ht, FUNC2_DRAM, 0xc4, 1 << 31);
}

void tracing_stop(const sci_t sci, const uint8_t ht)
{
	dnc_write_conf(sci, 0, 0x18 + ht, FUNC2_DRAM, 0xc8, 1 << 31);

	uint32_t val = dnc_read_conf(sci, 0, 0x18 + ht, FUNC2_DRAM, 0xc0);
	dnc_write_conf(sci, 0, 0x18 + ht, FUNC2_DRAM, 0xc0, val | (1 << 12));
	val = dnc_read_conf(sci, 0, 0x18 + ht, FUNC2_DRAM, 0xc0);
	dnc_write_conf(sci, 0, 0x18 + ht, FUNC2_DRAM, 0xc0, val & ~1);
}
