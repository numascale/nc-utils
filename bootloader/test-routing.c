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

#define _GNU_SOURCE 1

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <sched.h>
#include <fcntl.h>

#include "dnc-regs.h"
#include "dnc-access.h"
#include "dnc-fabric.h"

static uint16_t shadow_rtbll[7][256];
static uint16_t shadow_rtblm[7][256];
static uint16_t shadow_rtblh[7][256];
static uint16_t shadow_ltbl[7][256];

static void test_route(uint8_t bxbarid, uint16_t dest)
{
	uint16_t offs = (dest >> 4) & 0xff;
	uint16_t mask = 1 << (dest & 0xf);
	uint8_t out = 0;

/*    printf("Testing route on bxbarid %d to target ID %04x (offs=%02x, mask=%04x)\n", bxbarid, dest, offs, mask);
      printf("ltbl[%d][%02x] = %04x\n", bxbarid, offs, shadow_ltbl[bxbarid][offs]);
      printf("rtbll[%d][%02x] = %04x\n", bxbarid, offs, shadow_rtbll[bxbarid][offs]);
      printf("rtblm[%d][%02x] = %04x\n", bxbarid, offs, shadow_rtblm[bxbarid][offs]);
      printf("rtblh[%d][%02x] = %04x\n", bxbarid, offs, shadow_rtblh[bxbarid][offs]); */
	if (bxbarid > 0 && (shadow_ltbl[bxbarid][offs] & mask)) {
		out += ((shadow_rtbll[bxbarid][offs] & mask) >> (dest & 0xf)) * 1;
		out += ((shadow_rtblm[bxbarid][offs] & mask) >> (dest & 0xf)) * 2;
		out += ((shadow_rtblh[bxbarid][offs] & mask) >> (dest & 0xf)) * 4;

		if (out == 0) printf("[%04x] LC3%s -> SCC\n", dest, _get_linkname(bxbarid));
		else          printf("[%04x] LC3%s -> LC3%s\n", dest, _get_linkname(bxbarid), _get_linkname(out));
	} else if (bxbarid == 0) {
		const char *lcname;
		out += ((shadow_rtbll[bxbarid][offs] & mask) >> (dest & 0xf)) * 1;
		out += ((shadow_rtblm[bxbarid][offs] & mask) >> (dest & 0xf)) * 2;
		out += ((shadow_rtblh[bxbarid][offs] & mask) >> (dest & 0xf)) * 4;
		lcname = _get_linkname(out);

		if (lcname) printf("[%04x] SCC   -> LC3%s\n", dest, lcname);
		else        printf("[%04x] SCC   -> NOT ROUTED\n", dest);
	} else {
		printf("[%04x] LC3%s -> BYPASS\n", dest, _get_linkname(bxbarid));
	}
}

static void load_scc_routing(uint16_t rtbll[], uint16_t rtblm[], uint16_t rtblh[])
{
	uint16_t chunk, offs;
	uint16_t maxchunk = 16;
	printf("Loading routing table from SCC...");

	for (chunk = 0; chunk < maxchunk; chunk++) {
		dnc_write_csr(0xfff0, H2S_CSR_G0_ROUT_TABLE_CHUNK, (1 << 7) + chunk);

		for (offs = 0; offs < 16; offs++) {
			uint32_t l, m, h;
			/* SCC */
			l = dnc_read_csr(0xfff0, H2S_CSR_G0_ROUT_BXTBLL00 + (offs << 2));
			m = dnc_read_csr(0xfff0, H2S_CSR_G0_ROUT_BLTBL00  + (offs << 2));
			h = dnc_read_csr(0xfff0, H2S_CSR_G0_ROUT_BXTBLH00 + (offs << 2));
			l = ((l & 0xff) << 8) | ((l & 0xff00) >> 8);
			m = ((m & 0xff) << 8) | ((m & 0xff00) >> 8);
			h = ((h & 0xff) << 8) | ((h & 0xff00) >> 8);
/*            printf("rtbll = %08x, rtblm = %08x, rtblh = %08x\n", l, m, h); */
			rtbll[(chunk << 4) + offs] = l;
			rtblm[(chunk << 4) + offs] = m;
			rtblh[(chunk << 4) + offs] = h;
		}
	}

	dnc_write_csr(0xfff0, H2S_CSR_G0_ROUT_TABLE_CHUNK, 0);
	printf("Done\n");
}

static void load_lc3_routing(int linkno,
                             uint16_t rtbll[], uint16_t rtblm[], uint16_t rtblh[], uint16_t ltbl[])
{
	uint16_t chunk, offs;
	uint16_t maxchunk = 16;
	uint32_t csr;
	printf("Loading routing table from LC3%s...", _get_linkname(linkno));
	csr = dnc_read_csr(0xfff0 + linkno, LC3_CSR_CONFIG4);
	csr |= (1 << 6); /* CONFIG4.tblrd is needed to read routing tables through CSR */
	dnc_write_csr(0xfff0 + linkno, LC3_CSR_CONFIG4, csr);

	for (chunk = 0; chunk < maxchunk; chunk++) {
		dnc_write_csr(0xfff0 + linkno, LC3_CSR_SW_INFO3, chunk);

		for (offs = 0; offs < 16; offs++) {
			ltbl[(chunk << 4) + offs]  = dnc_read_csr(0xfff0 + linkno, LC3_CSR_ROUT_LCTBL00  + (offs << 2));
			rtbll[(chunk << 4) + offs] = dnc_read_csr(0xfff0 + linkno, LC3_CSR_ROUT_BXTBLL00 + (offs << 2));
			rtblm[(chunk << 4) + offs] = dnc_read_csr(0xfff0 + linkno, LC3_CSR_ROUT_BLTBL00  + (offs << 2));
			rtblh[(chunk << 4) + offs] = dnc_read_csr(0xfff0 + linkno, LC3_CSR_ROUT_BXTBLH00 + (offs << 2));
		}
	}

	csr &= ~(1 << 6); /* Disable CONFIG4.tblrd to enable normal routing operation */
	dnc_write_csr(0xfff0 + linkno, LC3_CSR_CONFIG4, csr);
	printf("Done\n");
}


int main(int argc, char **argv)
{
	int cpu_fam  = -1;
	cpu_set_t cset;
	uint32_t val;
	/* Bind to core 0 */
	CPU_ZERO(&cset);
	CPU_SET(0, &cset);

	if (sched_setaffinity(0, sizeof(cset), &cset) != 0) {
		fprintf(stderr, "Unable to bind to core 0.\n");
		return -1;
	}

	asm volatile("mov $1, %%eax; cpuid" : "=a"(val) :: "ebx", "ecx", "edx");
	cpu_fam = (val >> 8) & 0xf;

	if (cpu_fam == 0xf)
		cpu_fam += (val >> 20) & 0xf;

	if (cpu_fam <= 0xf) {
		fprintf(stderr, "*** Unsupported CPU family %d\n", cpu_fam);
		return -1;
	}

	load_scc_routing(shadow_rtbll[0], shadow_rtblm[0], shadow_rtblh[0]);
	load_lc3_routing(1, shadow_rtbll[1], shadow_rtblm[1], shadow_rtblh[1], shadow_ltbl[1]);
	load_lc3_routing(2, shadow_rtbll[2], shadow_rtblm[2], shadow_rtblh[2], shadow_ltbl[2]);
	load_lc3_routing(3, shadow_rtbll[3], shadow_rtblm[3], shadow_rtblh[3], shadow_ltbl[3]);
	load_lc3_routing(4, shadow_rtbll[4], shadow_rtblm[4], shadow_rtblh[4], shadow_ltbl[4]);
	load_lc3_routing(5, shadow_rtbll[5], shadow_rtblm[5], shadow_rtblh[5], shadow_ltbl[5]);
	load_lc3_routing(6, shadow_rtbll[6], shadow_rtblm[6], shadow_rtblh[6], shadow_ltbl[6]);

	test_route(0, 0x000);
	test_route(1, 0x000);
	test_route(3, 0x000);
	test_route(5, 0x000);
	test_route(0, 0x001);
	test_route(1, 0x001);
	test_route(3, 0x001);
	test_route(5, 0x001);
	test_route(0, 0x020);
	test_route(1, 0x020);
	test_route(3, 0x020);
	test_route(5, 0x020);
	test_route(0, 0x021);
	test_route(1, 0x021);
	test_route(3, 0x021);
	test_route(5, 0x021);

	test_route(0, 0x100);
	test_route(1, 0x100);
	test_route(3, 0x100);
	test_route(5, 0x100);
	test_route(0, 0x101);
	test_route(1, 0x101);
	test_route(3, 0x101);
	test_route(5, 0x101);
	test_route(0, 0x120);
	test_route(1, 0x120);
	test_route(3, 0x120);
	test_route(5, 0x120);
	test_route(0, 0x121);
	test_route(1, 0x121);
	test_route(3, 0x121);
	test_route(5, 0x121);

	test_route(0, 0x200);
	test_route(1, 0x200);
	test_route(3, 0x200);
	test_route(5, 0x200);
	test_route(0, 0x201);
	test_route(1, 0x201);
	test_route(3, 0x201);
	test_route(5, 0x201);
	test_route(0, 0x220);
	test_route(1, 0x220);
	test_route(3, 0x220);
	test_route(5, 0x220);
	test_route(0, 0x221);
	test_route(1, 0x221);
	test_route(3, 0x221);
	test_route(5, 0x221);
	return 0;
}
