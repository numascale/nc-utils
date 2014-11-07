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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "ddr_spd.h"
#include "dnc-access.h"
#include "dnc-commonlib.h"

/* Used for DDR1 and DDR2 SPD */
void ddr2_spd_check(const ddr2_spd_eeprom_t *sspd, const char *name)
{
	const uint8_t *spd = (uint8_t *)sspd;
	uint8_t cksum = 0, i;

	if (verbose >= 2) {
		printf("%s DIMM SPD:\n", name);
		dump(spd, 64);
		printf("\n");
	}

	/* If all zero, the I2C bus failed */
	for (i = 0; i < 63; i++)
		if (spd[i])
			break;

	assertf(i < 63, "%s DIMM SPD read as all zeros", name);

	/* Check SPD revision supported; rev 1.x or less supported by this code */
	assertf(sspd->spd_rev < 0x20, "%s DIMM SPD revision 0x%02x not supported", name, sspd->spd_rev);

	/* Calculate checksum */
	for (int i = 0; i < 63; i++)
		cksum += spd[i]; // implicitly truncated

	assertf(cksum == sspd->cksum, "%s DIMM SPD checksum 0x%02x differs from 0x%02x", name, cksum, sspd->cksum);
}
