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

#ifndef __DNC_MASTERLIB_H
#define __DNC_MASTERLIB_H 1

#define SCC_ATT_INDEX_RANGE 2   /* 3 = 47:36, 2 = 43:32, 1 = 39:28, 0 = 35:24 */
#define SCC_ATT_GRAN            ((0x1000000ULL << (SCC_ATT_INDEX_RANGE * 4)) >> DRAM_MAP_SHIFT)

void load_scc_microcode(void);
void tally_local_node(void);
void tally_all_remote_nodes(void);

#endif
