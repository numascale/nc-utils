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

#ifndef __DNC_ROUTE
#define __DNC_ROUTE 1

#include "dnc-types.h"

void add_chunk_route(u16 dest, u16 node, u8 link);
void del_chunk_route(u16 dest, u16 node);
void set_route(u16 dest, u16 node, u16 width, u8 link);
void add_route(u16 dest, u16 node, u16 width, u8 link);
void del_route(u16 dest, u16 node, u16 width);
void set_route_geo(u16 dest, u16 node, u8 bid, u16 width, u8 link);
void add_route_geo(u16 dest, u16 node, u8 bid, u16 width, u8 link);
void del_route_geo(u16 dest, u16 node, u8 bid, u16 width);

#endif
