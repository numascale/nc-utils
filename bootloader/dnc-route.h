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

#include "dnc-bootloader.h"

void add_chunk_route(uint16_t dest, const sci_t sci, uint8_t link);
void del_chunk_route(uint16_t dest, const sci_t sci);
void set_route(uint16_t dest, const sci_t sci, uint16_t width, uint8_t link);
void add_route(uint16_t dest, const sci_t sci, uint16_t width, uint8_t link);
void del_route(uint16_t dest, const sci_t sci, uint16_t width);
void set_route_geo(uint16_t dest, const sci_t sci, uint8_t bid, uint16_t width, uint8_t link);
void add_route_geo(uint16_t dest, const sci_t sci, uint8_t bid, uint16_t width, uint8_t link);
void del_route_geo(uint16_t dest, const sci_t sci, uint8_t bid, uint16_t width);

#endif
