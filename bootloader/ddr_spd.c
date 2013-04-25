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
#include "dnc-commonlib.h"

/* used for ddr1 and ddr2 spd */
static int spd_check(const uint8_t *buf, uint8_t spd_rev, uint8_t spd_cksum)
{
	uint32_t cksum = 0;
	int err = 0;

        /*
         * Check SPD revision supported
         * Rev 1.X or less supported by this code
         */
        if ((spd_rev >= 0x20) ||
	    (spd_rev > 0x13)) {
                error("SPD revision %02X not supported",
		      spd_rev);
                err = -1;
        }
	
	/*
	 * Calculate checksum
	 */
	for (int i = 0; i < 63; i++) {
		cksum += *buf++;
	}
	cksum &= 0xFF;

	if (cksum != spd_cksum) {
		error("SPD checksum unexpected. "
		      "Checksum in SPD = %02X, computed = %02X",
		      spd_cksum, cksum);
		err = -1;
	}

	return err;
}

int ddr1_spd_check(const ddr1_spd_eeprom_t *spd)
{
	const uint8_t *p = (const uint8_t *)spd;

	return spd_check(p, spd->spd_rev, spd->cksum);
}

int ddr2_spd_check(const ddr2_spd_eeprom_t *spd)
{
	const uint8_t *p = (const uint8_t *)spd;

	return spd_check(p, spd->spd_rev, spd->cksum);
}

/*
 * CRC16 compute for DDR3 SPD
 * Copied from DDR3 SPD spec.
 */
static int crc16(char *ptr, int count)
{
	int crc, i;

	crc = 0;
	while (--count >= 0) {
		crc = crc ^ (int)*ptr++ << 8;
		for (i = 0; i < 8; ++i)
			if (crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			else
				crc = crc << 1;
	}
	return crc & 0xffff;
}

int ddr3_spd_check(const ddr3_spd_eeprom_t *spd)
{
	char *p = (char *)spd;
	int csum16;
	int len;
	char crc_lsb;	/* byte 126 */
	char crc_msb;	/* byte 127 */

	/*
	 * SPD byte0[7] - CRC coverage
	 * 0 = CRC covers bytes 0~125
	 * 1 = CRC covers bytes 0~116
	 */

	len = !(spd->info_size_crc & 0x80) ? 126 : 117;
	csum16 = crc16(p, len);

	crc_lsb = (char) (csum16 & 0xff);
	crc_msb = (char) (csum16 >> 8);

	if (spd->crc[0] != crc_lsb || spd->crc[1] != crc_msb) {
		error("SPD checksum unexpected. "
		      "Checksum lsb in SPD = %02X, computed = %02X "
		      "Checksum msb in SPD = %02X, computed = %02X",
		      spd->crc[0], crc_lsb, spd->crc[1], crc_msb);
		return -1;
	}

	return 0;
}
