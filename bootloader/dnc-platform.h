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

#include <stdio.h>

#include "dnc-defs.h"
#include "dnc-access.h"
#include "dnc-commonlib.h"

class SR56x0;

class IOAPIC {
private:
	const uint16_t sci;
	const SR56x0& sr56x0;

	uint32_t reg_read(const uint16_t reg);
	void reg_write(const uint16_t reg, const uint32_t val);

public:
	const uint64_t bar_size;
	uint64_t bar_addr;

	IOAPIC(const uint16_t sci, SR56x0& sr56x0);
	void disable(void);
};

class SR56x0 {
private:
	const uint16_t sci;

public:
	const IOAPIC& ioapic;

	SR56x0(const uint16_t sci);
	uint32_t nbmiscind_read(const uint16_t reg);
	void nbmiscind_write(const uint16_t reg, const uint32_t val);
};

class Platform {
private:
	const uint16_t sci;

public:
	const SR56x0& ioh;

	Platform(const uint16_t sci);
};

extern Platform *platforms;
extern int nplatforms;

void platform_init(void);

