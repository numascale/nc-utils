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

#include <stdlib.h>

#include "dnc-platform.h"

Platform *platforms;
int nplatforms = 0;

void *operator new(size_t size)
{
    return malloc(size);
}

void *operator new[](size_t size)
{
    return malloc(size);
}

void operator delete(void *p)
{
    free(p);
}

void operator delete[](void *p)
{
    free(p);
}

uint32_t IOAPIC::reg_read(const uint16_t reg)
{
	dnc_write_conf(sci, 0, 0, 2, 0xf8, reg);
	return dnc_read_conf(sci, 0, 0, 2, 0xfc);
};
	
void IOAPIC::reg_write(const uint16_t reg, const uint32_t val)
{
	dnc_write_conf(sci, 0, 0, 2, 0xf8, reg);
	dnc_write_conf(sci, 0, 0, 2, 0xfc, val);
};

IOAPIC::IOAPIC(const uint16_t sci, SR56x0& sr56x0): sci(sci), sr56x0(sr56x0), bar_size(16)
{
	/* Ensure IOAPIC function isn't hidden */
	uint32_t val = sr56x0.nbmiscind_read(0x75);
	if (!(val & 1))
		sr56x0.nbmiscind_write(0x75, val | 1);

	/* Check device ID */
	val = dnc_read_conf(sci, 0, 0, 2, 0);
	assert(val == 0x5a231002);

	bar_addr = (reg_read(1) & ~0x1f) | (uint64_t)reg_read(2) << 32;
	printf("IOAPIC BAR lo=%08x hi=%08x addr=0x%llx\n", reg_read(1), reg_read(2), bar_addr);
};

void IOAPIC::disable(void)
{
	reg_write(0, 0);
};

SR56x0::SR56x0(const uint16_t sci): sci(sci), ioapic(*new IOAPIC(sci, *this))
{
	printf("ID=0x%08x NB_BAR1_RCRB=0x%08x NB_BAR2_PM2=0x%08x NB_BAR3_PCIEXP_MMCFG=0x%08x,0x%08x"
		" IOMMUMMREG=0x%08x,0x%08x TOM=0x%08x MMIOBASE=%08x MMIOLIMIT=%08x\n",
		dnc_read_conf(sci, 0, 0, 2, 0),
		dnc_read_conf(sci, 0, 0, 2, 0x14), dnc_read_conf(sci, 0, 0, 2, 0x18),
		dnc_read_conf(sci, 0, 0, 2, 0x1c), dnc_read_conf(sci, 0, 0, 2, 0x20),
		dnc_read_conf(sci, 0, 0, 2, 0x44), dnc_read_conf(sci, 0, 0, 2, 0x48),
		nbmiscind_read(0x16),
		nbmiscind_read(0x17), nbmiscind_read(0x18));
};

uint32_t SR56x0::nbmiscind_read(const uint16_t reg)
{
	dnc_write_conf(sci, 0, 0, 0, 0x60, reg);
	return dnc_read_conf(sci, 0, 0, 0, 0x64);
}

void SR56x0::nbmiscind_write(const uint16_t reg, const uint32_t val)
{
	dnc_write_conf(sci, 0, 0, 0, 0x60, reg | 0x80);
	dnc_write_conf(sci, 0, 0, 0, 0x64, val);
}

Platform::Platform(const uint16_t sci): sci(sci), ioh(*new SR56x0(sci))
{
};

void platform_init(void)
{
	platforms = new Platform(0xfff0);
	nplatforms = 1;
}

