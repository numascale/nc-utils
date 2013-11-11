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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>

#include "dnc-regs.h"
#include "dnc-defs.h"
#include "dnc-access.h"
#include "dnc-fabric.h"
#include "dnc-config.h"
#include "dnc-devices.h"
#include "dnc-bootloader.h"
#include "dnc-commonlib.h"
#include "dnc-maps.h"
#include "ddr_spd.h"

IMPORT_RELOCATED(cpu_status);
IMPORT_RELOCATED(init_dispatch);
IMPORT_RELOCATED(msr_readback);
IMPORT_RELOCATED(new_cucfg2_msr);
IMPORT_RELOCATED(apic_offset);
IMPORT_RELOCATED(apic_readback);

const char *config_file_name = "nc-config/fabric.json";
const char *next_label = "menu.c32";
const char *microcode_path = "";
static bool init_only = 0;
static bool route_only = 0;
static int enable_nbmce = -1;
int enable_nbwdt = 0;
static bool disable_sram = 0;
int force_probefilteroff = 0;
int force_probefilteron = 0;
static int ht_force_ganged = 1;
bool disable_smm = 0;
bool disable_c1e = 1;
int renumber_bsp = -1;
int remote_io = 0;
bool boot_wait = false;
int forwarding_mode = 3; /* 0=store-and-forward, 1-2=intermediate, 3=full cut-through */
int sync_interval = 1; /* bit[8]=disable prescaler, bit[7:0] sync_interval value */
bool enable_relfreq = 0;
bool singleton = 0;
static bool ht_200mhz_only = 0;
static bool ht_8bit_only = 0;
static int ht_suppress = 0xffff;
static int ht_lockdelay = 0;
bool handover_acpi = 0;
bool mem_offline = 0;
uint64_t trace_buf_size = 0;
int verbose = 0;
int family = 0;
uint32_t tsc_mhz = 2200;
uint32_t max_mem_per_node;
static int dimmtest = 1;
static bool workaround_hreq = 1;
static bool workaround_rtt = 0;
bool workaround_locks = 0;
bool pf_cstate6 = 0;
uint64_t mem_gap = 0;
int disable_kvm = -1;

const char *node_state_name[] = { NODE_SYNC_STATES(ENUM_NAMES) };
static struct dimm_config dimms[2]; /* 0 - MCTag, 1 - CData */

/* Return string pointer using rotated static buffer to avoid heap */
const char *pr_size(uint64_t val)
{
	static char strs[4][8];
	static int index = 0;
	const char suffix[] = " KMGTPE!";
	unsigned int offset = 0;
	/* Use new string buffer */
	index = (index + 1) % 4;

	while (offset < sizeof(suffix) && val >= 1024) {
		val /= 1024;
		offset++;
	}

	if (offset > 0)
		snprintf(strs[index], 8, "%llu%cB", val, suffix[offset]);
	else
		snprintf(strs[index], 8, "%lluB", val);

	return strs[index];
}

void udelay(const uint32_t usecs)
{
	uint64_t limit = rdtscll() + (uint64_t)usecs * tsc_mhz;

	while (rdtscll() < limit)
		cpu_relax();
}

void wait_key(void)
{
	char ch;
	printf("... ( press any key to continue ) ... ");

	while (fread(&ch, 1, 1, stdin) == 0)
		;

	printf("\n");
}

static void read_spd_info(char p_type[16], const bool cdata, struct dimm_config *dimm)
{
	uint16_t spd_addr;
	uint8_t addr_bits;
	ddr2_spd_eeprom_t *spd = &dimm->spd;
	uint32_t *dataw = (uint32_t *)spd;

	/* On N313025 and N323024, the SPD addresses are reversed */
	if ((strncmp("N313025", p_type, 7) == 0) ||
	    (strncmp("N323024", p_type, 7) == 0))
		spd_addr = cdata ? 0 : 1;
	else
		spd_addr = cdata ? 1 : 0;

	/* Read entire SPD */
	for (uint32_t i = 0; i < sizeof(ddr2_spd_eeprom_t)/sizeof(uint32_t); i++)
		dataw[i] = uint32_tbswap(dnc_read_csr(0xfff0, (1 << 12) + (spd_addr << 8) + (i<<2)));

	ddr2_spd_check(spd);

	assertf(spd->config & 2, "Unsupported non-ECC %s DIMM", cdata ? "CData" : "MCTag");
	assertf(spd->dimm_type & 0x11, "Unsupported non-Registered %s DIMM", cdata ? "CData" : "MCTag");
	assertf((spd->mod_ranks & 7) <= 1, "Unsupported %s rank count %d", cdata ? "CData" : "MCTag", (spd->mod_ranks & 7) + 1);
	assertf(spd->primw == 4 || spd->primw == 8, "Unsupported %s SDRAM width %d", cdata ? "CData" : "MCTag", spd->primw);

	dimm->addr_pins = 16 - (spd->nrow_addr & 0xf); /* Number of Row address bits (max 16) */
	dimm->column_size = 13 - (spd->ncol_addr & 0xf); /* Number of Column address bits (max 13) */
	dimm->cs_map = (spd->mod_ranks & 1) ? 3 : 1; /* Single or Dual rank */
	dimm->width = spd->primw;
	dimm->eight_bank = (spd->nbanks == 8);
	addr_bits = (spd->nrow_addr & 0xf) + (spd->ncol_addr & 0xf) + (spd->mod_ranks & 1) + ((spd->nbanks == 8) ? 3 : 2);

	/* Make sure manufacturer's part-number is null-terminated */
	if (spd->mpart[17])
		spd->mpart[17] = 0;

	printf("%s is a x%d %dMB %s-rank module (%s)\n", cdata ? "CData" : "MCTag",
	       dimm->width, 1 << (addr_bits - 17),
	       (spd->mod_ranks & 1) ? "dual" : "single",
	       spd->mpart[0] ? (char *)spd->mpart : "unknown");

	switch (addr_bits) {
	case 31:
		dimm->mem_size = 4;
		break; /* 16G */
	case 30:
		dimm->mem_size = 3;
		break; /*  8G */
	case 29:
		dimm->mem_size = 2;
		break; /*  4G */
	case 28:
		dimm->mem_size = 1;
		break; /*  2G */
	case 27:
		dimm->mem_size = 0;
		break; /*  1G */
	default:
		fatal("Unsupported %s DIMM size of %dMB", cdata ? "CData" : "MCTag", 1 << (addr_bits - 17));
	}
}

#include "../interface/mctr_define_register_C.h"
#include "../interface/regconfig_200_cl4_bl4_genericrdimm.h"

#define SCALAR 16

uint32_t clocks(const uint16_t raw, int div)
{
	/* Use fixed-point integer arithmetic, as floating-point fails */
	/* 5ns clock period; 200 MHz */
	uint32_t val = ((uint32_t)raw << 16) / (div * 5);
	uint8_t a = val >> 16;
	if (val > ((uint32_t)a << 16))
		return a + 1;
	return a;
}

uint32_t mod(const uint32_t initial, const uint16_t raw, const int div, const uint32_t mask, const char *name)
{
	assert(mask && div);

	if (raw == 0) {
		warning("SPD has %s is unset; skipping", name);
		return initial;
	}

	uint32_t mask2 = mask;
	int shift = 0;

	/* Find first set bit in mask */
	while (!(mask2 & 1)) {
		mask2 >>= 1;
		shift++;
	}

	uint32_t a = (initial & mask) >> shift;
	uint32_t b = clocks(raw, div);

	/* Ensure new value is within mask */
	assert(!((b << shift) & ~mask));

	uint32_t newval = (initial & ~mask) | (b << shift);

	if (b != a)
		printf("Using SPD value %d rather than default %d for %s; raw %d; %08x -> %08x\n", b, a, name, raw, initial, newval);
	return initial;
}

static void _denali_mctr_reset(int cdata, struct dimm_config *dimm)
{
	uint32_t val;
	#define COEFF 12
	const static int part[] = {0, COEFF / 4, COEFF / 3, COEFF / 2, 4 * COEFF / 6, 3 * COEFF / 4};

	int mctrbase = cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00;
	dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00) + (0 << 2), DENALI_CTL_00_DATA);

	dnc_write_csr(0xfff0, mctrbase + (1 << 2), DENALI_CTL_01_DATA);
	dnc_write_csr(0xfff0, mctrbase + (2 << 2), DENALI_CTL_02_DATA);
	dnc_write_csr(0xfff0, mctrbase + (3 << 2), DENALI_CTL_03_DATA);
	dnc_write_csr(0xfff0, mctrbase + (4 << 2),
	              (DENALI_CTL_04_DATA & ~(1 << 24)) | (dimm->eight_bank << 24));
	dnc_write_csr(0xfff0, mctrbase + (5 << 2), DENALI_CTL_05_DATA);
	dnc_write_csr(0xfff0, mctrbase + (7 << 2), DENALI_CTL_07_DATA);
	dnc_write_csr(0xfff0, mctrbase + (8 << 2), DENALI_CTL_08_DATA);
	dnc_write_csr(0xfff0, mctrbase + (9 << 2), DENALI_CTL_09_DATA);
	dnc_write_csr(0xfff0, mctrbase + (10 << 2), DENALI_CTL_10_DATA);
	dnc_write_csr(0xfff0, mctrbase + (11 << 2), DENALI_CTL_11_DATA);
	dnc_write_csr(0xfff0, mctrbase + (12 << 2),
	              (DENALI_CTL_12_DATA & ~(3 << 16)) | ((dimm->width == 8 ? 1 : 0) << 16));
	dnc_write_csr(0xfff0, mctrbase + (13 << 2),
	              (DENALI_CTL_13_DATA & ~(0x7 << 16)) | ((dimm->addr_pins & 0x7) << 16));
	dnc_write_csr(0xfff0, mctrbase + (14 << 2),
	              (DENALI_CTL_14_DATA & ~(0x7 << 16)) | ((dimm->column_size & 0x7) << 16));
	dnc_write_csr(0xfff0, mctrbase + (16 << 2), DENALI_CTL_16_DATA);

	val = mod(DENALI_CTL_17_DATA, dimm->spd.twr, 4, 0x1f << 24, "tWR");
	val = mod(val, dimm->spd.trtp, 4, 7 << 16, "tRTP");
	val = mod(val, dimm->spd.trrd, 4, 7 << 8, "tRRD");
	dnc_write_csr(0xfff0, mctrbase + (17 << 2), val);

	val = mod(DENALI_CTL_18_DATA, dimm->spd.twtr, 4, 0xf, "tWTR");
	dnc_write_csr(0xfff0, mctrbase + (18 << 2), val);
	dnc_write_csr(0xfff0, mctrbase + (19 << 2),
	              (DENALI_CTL_19_DATA & ~(0x3 << 24)) | ((dimm->cs_map & 0x3) << 24));
	dnc_write_csr(0xfff0, mctrbase + (20 << 2), DENALI_CTL_20_DATA);
	dnc_write_csr(0xfff0, mctrbase + (21 << 2), DENALI_CTL_21_DATA);
	dnc_write_csr(0xfff0, mctrbase + (22 << 2), DENALI_CTL_22_DATA);
	dnc_write_csr(0xfff0, mctrbase + (23 << 2), DENALI_CTL_23_DATA);
	dnc_write_csr(0xfff0, mctrbase + (24 << 2), DENALI_CTL_24_DATA);
	dnc_write_csr(0xfff0, mctrbase + (25 << 2), DENALI_CTL_25_DATA);

	val = mod(DENALI_CTL_26_DATA, dimm->spd.trp, 4, 0xf << 24, "tRP");
	dnc_write_csr(0xfff0, mctrbase + (26 << 2), val);

	val = mod(DENALI_CTL_28_DATA, dimm->spd.trc * COEFF + part[(dimm->spd.trctrfc_ext >> 4) & 0x7], COEFF, 0x1f << 24, "tRC");
	dnc_write_csr(0xfff0, mctrbase + (28 << 2), val);

	val = mod(DENALI_CTL_30_DATA,
		dimm->spd.trfc * COEFF + part[(dimm->spd.trctrfc_ext >> 1) & 0x7] + (dimm->spd.trctrfc_ext & 1) * 256 * COEFF, COEFF, 0xff << 16, "tRFC");
	val = mod(val, dimm->spd.trcd, 4, 0xff << 8, "tRCD");
	val = mod(val, dimm->spd.tras, 1, 0xff, "tRAS");
	dnc_write_csr(0xfff0, mctrbase + (30 << 2), val);
	dnc_write_csr(0xfff0, mctrbase + (31 << 2), DENALI_CTL_31_DATA);
	dnc_write_csr(0xfff0, mctrbase + (33 << 2), DENALI_CTL_33_DATA);

	assert(dimm->spd.refresh == 0x82);
	dnc_write_csr(0xfff0, mctrbase + (35 << 2), DENALI_CTL_35_DATA);
	dnc_write_csr(0xfff0, mctrbase + (36 << 2), DENALI_CTL_36_DATA);
	dnc_write_csr(0xfff0, mctrbase + (37 << 2), DENALI_CTL_37_DATA);
	dnc_write_csr(0xfff0, mctrbase + (38 << 2), DENALI_CTL_38_DATA);
	dnc_write_csr(0xfff0, mctrbase + (46 << 2), DENALI_CTL_46_DATA);
	dnc_write_csr(0xfff0, mctrbase + (48 << 2), DENALI_CTL_48_DATA);
	dnc_write_csr(0xfff0, mctrbase + (49 << 2), DENALI_CTL_49_DATA);
	dnc_write_csr(0xfff0, mctrbase + (50 << 2), DENALI_CTL_50_DATA);
	dnc_write_csr(0xfff0, mctrbase + (51 << 2), DENALI_CTL_51_DATA);
	dnc_write_csr(0xfff0, mctrbase + (52 << 2), DENALI_CTL_52_DATA);
	dnc_write_csr(0xfff0, mctrbase + (53 << 2), DENALI_CTL_53_DATA);
	dnc_write_csr(0xfff0, mctrbase + (54 << 2), DENALI_CTL_54_DATA);
	dnc_write_csr(0xfff0, mctrbase + (55 << 2), DENALI_CTL_55_DATA);
	dnc_write_csr(0xfff0, mctrbase + (56 << 2), DENALI_CTL_56_DATA);
	dnc_write_csr(0xfff0, mctrbase + (57 << 2), DENALI_CTL_57_DATA);
	dnc_write_csr(0xfff0, mctrbase + (59 << 2), DENALI_CTL_59_DATA);
	dnc_write_csr(0xfff0, mctrbase + (60 << 2), DENALI_CTL_60_DATA);
	dnc_write_csr(0xfff0, mctrbase + (61 << 2), DENALI_CTL_61_DATA);
	dnc_write_csr(0xfff0, mctrbase + (62 << 2), DENALI_CTL_62_DATA);
	dnc_write_csr(0xfff0, mctrbase + (63 << 2), DENALI_CTL_63_DATA);
	dnc_write_csr(0xfff0, mctrbase + (64 << 2), DENALI_CTL_64_DATA);
	dnc_write_csr(0xfff0, mctrbase + (65 << 2), DENALI_CTL_65_DATA);
	dnc_write_csr(0xfff0, mctrbase + (66 << 2), DENALI_CTL_66_DATA);
	dnc_write_csr(0xfff0, mctrbase + (67 << 2), DENALI_CTL_67_DATA);
	dnc_write_csr(0xfff0, mctrbase + (69 << 2), DENALI_CTL_69_DATA);
	dnc_write_csr(0xfff0, mctrbase + (94 << 2), DENALI_CTL_94_DATA);
	dnc_write_csr(0xfff0, mctrbase + (95 << 2), DENALI_CTL_95_DATA);
	dnc_write_csr(0xfff0, mctrbase + (96 << 2), DENALI_CTL_96_DATA);
	dnc_write_csr(0xfff0, mctrbase + (97 << 2), DENALI_CTL_97_DATA);
	dnc_write_csr(0xfff0, mctrbase + (98 << 2), DENALI_CTL_98_DATA);
	dnc_write_csr(0xfff0, mctrbase + (99 << 2), DENALI_CTL_99_DATA);
	dnc_write_csr(0xfff0, mctrbase + (100 << 2), DENALI_CTL_100_DATA);
	dnc_write_csr(0xfff0, mctrbase + (101 << 2), DENALI_CTL_101_DATA);
	dnc_write_csr(0xfff0, mctrbase + (102 << 2), DENALI_CTL_102_DATA);
	dnc_write_csr(0xfff0, mctrbase + (103 << 2), DENALI_CTL_103_DATA);
	dnc_write_csr(0xfff0, mctrbase + (104 << 2), DENALI_CTL_104_DATA);
	dnc_write_csr(0xfff0, mctrbase + (105 << 2), DENALI_CTL_105_DATA);
	dnc_write_csr(0xfff0, mctrbase + (106 << 2), DENALI_CTL_106_DATA);
	dnc_write_csr(0xfff0, mctrbase + (107 << 2), DENALI_CTL_107_DATA);
	dnc_write_csr(0xfff0, mctrbase + (108 << 2), DENALI_CTL_108_DATA);
	dnc_write_csr(0xfff0, mctrbase + (109 << 2), DENALI_CTL_109_DATA);
	dnc_write_csr(0xfff0, mctrbase + (110 << 2), DENALI_CTL_110_DATA);
	dnc_write_csr(0xfff0, mctrbase + (111 << 2), DENALI_CTL_111_DATA);
	dnc_write_csr(0xfff0, mctrbase + (184 << 2), DENALI_CTL_184_DATA);
	dnc_write_csr(0xfff0, mctrbase + (185 << 2), DENALI_CTL_185_DATA);
	dnc_write_csr(0xfff0, mctrbase + (186 << 2), DENALI_CTL_186_DATA);
	dnc_write_csr(0xfff0, mctrbase + (188 << 2), DENALI_CTL_188_DATA);
}

uint32_t dnc_check_mctr_status(const int cdata)
{
	uint32_t val;
	uint32_t ack = 0;
	const int mctrbase = cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00;
	const char *me = cdata ? "CData" : "MCTag";

	if (!dnc_asic_mode)
		return 0;

	val = dnc_read_csr(0xfff0, mctrbase + (INT_STATUS_ADDR << 2));
#ifdef BROKEN

	if (val & 0x001) {
		error("%s single access outside the defined physical memory space detected", me);
		ack |= 0x001;
	}

	if (val & 0x002) {
		error("%s multiple access outside the defined physical memory space detected", me);
		ack |= 0x002;
	}

#endif

	if (val & 0x004) {
		error("%s single correctable ECC event detected", me);
		ack |= 0x004;
	}

	if (val & 0x008) {
		error("%s multiple correctable ECC event detected", me);
		ack |= 0x008;
	}

	if (val & 0x010) {
		error("%s single uncorrectable ECC event detected", me);
		ack |= 0x010;
	}

	if (val & 0x020) {
		error("%s multiple uncorrectable ECC event detected", me);
		ack |= 0x020;
	}

	if (val & 0xf80) {
		error("%s error interrupts detected INT_STATUS=%03x", me, val & 0xfff);
		ack |= (val & 0xf80);
	}

	if (ack) {
		ack |= dnc_read_csr(0xfff0, mctrbase + (INT_ACK_ADDR << 2));
		dnc_write_csr(0xfff0, mctrbase + (INT_ACK_ADDR << 2), ack);
	}

	return val;
}

static void dnc_initialize_sram(void)
{
	uint32_t val;

	/* ASIC */
	if (dnc_chip_rev < 2) {
		val = dnc_read_csr(0xfff0, H2S_CSR_G2_DDL_STATUS);

		if (((val >> 24) & 0xff) != 0x3f)
			printf("Waiting for SRAM clock calibration\n");

		while (((val >> 24) & 0xff) != 0x3f) {
			udelay(100);
			val = dnc_read_csr(0xfff0, H2S_CSR_G2_DDL_STATUS);
			assertf(!(val & 0xc000000), "SRAM clock calibration overflow detected");
		}

		printf("SRAM clock calibration complete\n");
		/* Zero out FTag SRAM */
		dnc_write_csr(0xfff0, H2S_CSR_G2_SRAM_MODE, 0x00000002);

		while (1) {
			udelay(100);
			val = dnc_read_csr(0xfff0, H2S_CSR_G2_FTAG_STATUS);

			if ((val & 1) == 0)
				break;
		}

		printf("Enabling FTag SRAM\n");
		dnc_write_csr(0xfff0, H2S_CSR_G2_SRAM_MODE, 0x00000000);
	} else
		printf("ASIC revision %d detected, no FTag SRAM\n", dnc_chip_rev);
}

void dnc_dram_initialise(void)
{
	int cdata;
	uint32_t val;

	printf("Initialising DIMMs...");
	for (cdata = 0; cdata < 2; cdata++) {
		dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_SCRUBBER_ADDR : H2S_CSR_G4_MCTAG_SCRUBBER_ADDR, 0);
		dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_STATR : H2S_CSR_G4_MCTAG_COM_STATR, (1 << 1)); /* Clear INI_FINISHED flag */
		val = dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_MAINTR : H2S_CSR_G4_MCTAG_MAINTR);
		dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_MAINTR : H2S_CSR_G4_MCTAG_MAINTR, val | (1 << 4)); /* Set MEM_INI_ENABLE */
	}

	for (cdata = 0; cdata < 2; cdata++) {
		do {
			udelay(100);
			val = dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_STATR : H2S_CSR_G4_MCTAG_COM_STATR);
		} while (!(val & (1 << 1))); /* Wait for INI_FINISHED flag */

		val = dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_MAINTR : H2S_CSR_G4_MCTAG_MAINTR);
		dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_MAINTR : H2S_CSR_G4_MCTAG_MAINTR, val & ~(1 << 4)); /* Clear MEM_INI_ENABLE */
	}
	printf("done\n");
}

void dnc_init_caches(void)
{
	uint32_t val;

	for (int cdata = 0; cdata < 2; cdata++) {
		const char *name = cdata ? "CData" : "MCTag";

		if (!dnc_asic_mode) {
			/* On FPGA, just check that the phy is initialized */
			val = dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_STATR : H2S_CSR_G4_MCTAG_COM_STATR);
			if (!(val & 0x1000))
				fatal("%s controller not calibrated\n", name);
		} else {
			int mctbase = cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00;
			val = dnc_read_csr(0xfff0, mctbase + (START_ADDR << 2)); /* Read the Denali START parameter */

			if (((val >> START_OFFSET) & 1) == 0) {
				printf("Resetting and loading DDR2 MCT, DDR2 PHY and external DDR2 RAM (EMODE-register)...");
				_denali_mctr_reset(cdata, &dimms[cdata]);

				/* Start DRAM initialization */
				val = dnc_read_csr(0xfff0, mctbase + (START_ADDR << 2));
				dnc_write_csr(0xfff0, mctbase + (START_ADDR << 2), val | (1 << START_OFFSET));

				/* Wait for it to signal completion */
				do {
					udelay(100);
					val = dnc_check_mctr_status(cdata);
				} while (!(val & (1<<6)));

				/* Reset DRAM initialization complete interrupt */
				val = dnc_read_csr(0xfff0, mctbase + (INT_ACK_ADDR << 2));
				dnc_write_csr(0xfff0, mctbase + (INT_ACK_ADDR << 2), val | (1<<6));
				/* Mask BIST complete interrupt (bit7) */
				/* Mask DRAM initialization complete interrupt (bit6) */
				/* Mask the out-of-bounds interrupts since they happen all the time (bit0+1) */
				val = dnc_read_csr(0xfff0, mctbase + (INT_MASK_ADDR << 2));
				dnc_write_csr(0xfff0, mctbase + (INT_MASK_ADDR << 2),
				              val | (((1<<7)|(1<<6)|(3<<0)) << INT_MASK_OFFSET));

				/* Clear H2S Error Status (W1TC) */
				val = dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_ERROR_STATR : H2S_CSR_G4_MCTAG_ERROR_STATR);
				dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_ERROR_STATR : H2S_CSR_G4_MCTAG_ERROR_STATR, val);

				printf("done\n");
			} else {
				/* Controller has already been started, just check for interrupts */
				(void)dnc_check_mctr_status(cdata);
			}
		}

		if ((dnc_asic_mode && dnc_chip_rev >= 2) || !dnc_asic_mode) {
			int tmp = dimms[cdata].mem_size;

			/* New RevC MCTag setting for supporting larger local memory */
			if (!cdata) {
				if (dimms[0].mem_size == 0 && dimms[1].mem_size == 1) {        /* 1GB MCTag_Size  2GB Remote Cache   24GB Coherent Local Memory */
					tmp = 0;
				} else if (dimms[0].mem_size == 0 && dimms[1].mem_size == 2) { /* 1GB MCTag_Size  4GB Remote Cache   16GB Coherent Local Memory */
					tmp = 1;
				} else if (dimms[0].mem_size == 1 && dimms[1].mem_size == 1) { /* 2GB MCTag_Size  2GB Remote Cache   56GB Coherent Local Memory */
					tmp = 2;
				} else if (dimms[0].mem_size == 1 && dimms[1].mem_size == 2) { /* 2GB MCTag_Size  4GB Remote Cache   48GB Coherent Local Memory */
					tmp = 3;
				} else if (dimms[0].mem_size == 2 && dimms[1].mem_size == 2) { /* 4GB MCTag_Size  4GB Remote Cache  112GB Coherent Local Memory */
					tmp = 4;
				} else if (dimms[0].mem_size == 2 && dimms[1].mem_size == 3) { /* 4GB MCTag_Size  8GB Remote Cache   96GB Coherent Local Memory */
					tmp = 5;
				} else if (dimms[0].mem_size == 3 && dimms[1].mem_size == 2) { /* 8GB MCTag_Size  4GB Remote Cache  240GB Coherent Local Memory */
					tmp = 6;
				} else if (dimms[0].mem_size == 3 && dimms[1].mem_size == 3) { /* 8GB MCTag_Size  8GB Remote Cache  224GB Coherent Local Memory */
					tmp = 7;
				} else
					fatal("Unsupported MCTag/CData combination (%d/%dGB)",
					       (1 << dimms[0].mem_size), (1 << dimms[1].mem_size));
			}

			if (!cdata) {
				max_mem_per_node = (1U << (5 + dimms[0].mem_size)) - (1U << (2 + dimms[1].mem_size));
				printf("%dGB MCTag, %dGB Remote Cache, %3dGB Max Coherent Local Memory\n",
				       (1 << dimms[0].mem_size), (1 << dimms[1].mem_size), max_mem_per_node);
				max_mem_per_node = max_mem_per_node << (30 - DRAM_MAP_SHIFT);
			}

			val = dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_CTRLR : H2S_CSR_G4_MCTAG_COM_CTRLR);
			dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_CTRLR : H2S_CSR_G4_MCTAG_COM_CTRLR,
			              (val & ~7) | (1 << 12) | ((tmp & 7) << 4)); /* DRAM_AVAILABLE, MEMSIZE[2:0] */
		} else { /* Older than Rev2 */
			/* On Rev1 and older there's a direct relationship between the MTag size and RCache size */
			if (cdata && dimms[cdata].mem_size == 0)
				fatal("Unsupported CData size of %dGB", (1 << dimms[cdata].mem_size));

			if (!cdata) {
				max_mem_per_node = 16U << dimms[0].mem_size;
				printf("%dGB tag and %dGB remote cache; %dGB max coherent local memory\n",
				       (1 << dimms[0].mem_size), (1 << dimms[1].mem_size), max_mem_per_node);
				max_mem_per_node = max_mem_per_node << (30 - DRAM_MAP_SHIFT);
			}

			val = dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_CTRLR : H2S_CSR_G4_MCTAG_COM_CTRLR);
			dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_CTRLR : H2S_CSR_G4_MCTAG_COM_CTRLR,
			              (val & ~7) | (1 << 12) | (1 << 7) | ((dimms[cdata].mem_size & 7) << 4)); /* DRAM_AVAILABLE, FTagBig, MEMSIZE[2:0] */
		}
	}

	dnc_dram_initialise();
	dnc_dimmtest(dimmtest, dimms);

	printf("Setting RCache size to %dGB\n", 1 << dimms[1].mem_size);
	/* Set the cache size in HReq and MIU */
	val = dnc_read_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL);
	dnc_write_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL, (val & ~(3 << 26)) | ((dimms[1].mem_size - 1) << 26)); /* [27:26] Cache size: 0 - 2GB, 1 - 4GB, 2 - 8GB, 3 - 16GB */
	dnc_write_csr(0xfff0, H2S_CSR_G0_MIU_CACHE_SIZE, dimms[1].mem_size + 1); /* ((2 ^ (N - 1)) * 1GB) */

	/* Initialize SRAM */
	if (!dnc_asic_mode) { /* Special FPGA considerations for Ftag SRAM */
		printf("FPGA revision %d_%d detected, no FTag SRAM\n", dnc_chip_rev >> 16, dnc_chip_rev & 0xffff);
	} else {
		/* ASIC; initialize SRAM if disable_sram is unset */
		if (!disable_sram)
			dnc_initialize_sram();
		else {
			printf("No SRAM will be initialized\n");
			dnc_write_csr(0xfff0, H2S_CSR_G2_SRAM_MODE, 0x00000001);  /* Disable SRAM on NC */
		}
	}
}

int cpu_family(const sci_t sci, const ht_t ht)
{
	uint32_t val;
	int fam, model, stepping;
	val = dnc_read_conf(sci, 0, 24 + ht, FUNC3_MISC, 0xfc);
	fam = ((val >> 20) & 0xf) + ((val >> 8) & 0xf);
	model = ((val >> 12) & 0xf0) | ((val >> 4) & 0xf);
	stepping = val & 0xf;
	return (fam << 16) | (model << 8) | stepping;
}

#ifdef __i386
static uint32_t get_phy_register(int node, int link, int idx, int direct)
{
	int base = 0x180 + link * 8;
	int i;
	uint32_t reg;
	cht_write_conf(node, FUNC4_LINK, base, idx | (direct << 29));

	for (i = 0; i < 1000; i++) {
		reg = cht_read_conf(node, FUNC4_LINK, base);
		if (reg & 0x80000000)
			return cht_read_conf(node, FUNC4_LINK, base + 4);
	}

	printf("Read from phy register HT#%d F4x%x idx %x did not complete\n",
	       node, base, idx);
	return 0;
}

static void set_phy_register(int node, int link, int idx, int direct, uint32_t val)
{
	int base = 0x180 + link * 8;
	int i;
	uint32_t reg;
	cht_write_conf(node, FUNC4_LINK, base + 4, val);
	cht_write_conf(node, FUNC4_LINK, base, idx | (direct << 29) | (1 << 30));

	for (i = 0; i < 1000; i++) {
		reg = cht_read_conf(node, FUNC4_LINK, base);
		if (reg & 0x80000000)
			return;
	}

	printf("Write to phy register HT#%d F4x%x idx %x did not complete\n",
	       node, base, idx);
}
#endif /* __i386 */

#ifdef UNUSED
static void reorganize_mmio(int nc)
{
	/* Stub for now */
	uint64_t mmio_start;
	uint64_t base, lim;
	int i;
	mmio_start = ~0;

	for (i = 0; i < 8; i++) {
		base = cht_read_conf(0, FUNC1_MAPS, 0x80 + i * 8);
		lim  = cht_read_conf(0, FUNC1_MAPS, 0x84 + i * 8);

		if (!(base & 3))
			continue;

		if (((base & ~0xff) << 8) < mmio_start)
			mmio_start = (base & ~0xff) << 8;

		printf("HT#0 MMIO range %d: %08x:%08x\n", i, base, lim);

		if (base & 0x8) {
			printf("Range locked, remapping...\n");
		}
	}

	printf("MMIO start: %08x\n", mmio_start);
}
#endif /* UNUSED */

static uint32_t southbridge_id = -1;
static uint8_t smi_state;

void detect_southbridge(void)
{
	southbridge_id = dnc_read_conf(0xfff0, 0, 0x14, 0, 0);

	if (southbridge_id != 0x43851002)
		printf("Warning: Unable to disable SMI due to unknown southbridge 0x%08x; this may cause hangs\n", southbridge_id);
}

/* Mask southbridge SMI generation */
void disable_smi(void)
{
	if (southbridge_id == 0x43851002) {
		smi_state = pmio_readb(0x53);
		pmio_writeb(0x53, smi_state | (1 << 3));
	}
}

/* Restore previous southbridge SMI mask */
void enable_smi(void)
{
	if (southbridge_id == 0x43851002) {
		pmio_writeb(0x53, smi_state);
	}
}

void critical_enter(void)
{
	cli();
	disable_smi();
}

void critical_leave(void)
{
	enable_smi();
	sti();
}

#ifdef __i386
static void print_phy_gangs(int neigh, int link, const char *name, int base)
{
	printf("%s:\n", name);
	printf("- CAD[ 7:0],CTL0,CLK0=0x%08x\n", get_phy_register(neigh, link, base, 0));
	printf("- CAD[15:8],CTL1,CLK1=0x%08x\n", get_phy_register(neigh, link, base + 0x10, 0));
}

static void print_phy_lanes(int neigh, int link, const char *name, int base, int clk)
{
	int i;
	printf("%s:\n", name);

	for (i = 0; i <= 15; i++)
		printf("- CAD[%2d]=0x%08x\n", i, get_phy_register(neigh, link, base + i * 0x80, 1));

	printf("- CTL[ 0]=0x%08x\n", get_phy_register(neigh, link, base + 16 * 0x80, 1));

	if (clk)
		printf("- CLK[ 0]=0x%08x\n", get_phy_register(neigh, link, base + 17 * 0x80, 1));

	printf("- CTL[ 1]=0x%08x\n", get_phy_register(neigh, link, base + 18 * 0x80, 1));

	if (clk)
		printf("- CLK[ 1]=0x%08x\n", get_phy_register(neigh, link, base + 19 * 0x80, 1));
}

/* Set upper sublink to same phy settings as lower sublink */
static void cht_mirror(int neigh, int link)
{
	const int gang_regs[] = {0xc0, 0xc1, 0xc3, 0xc4, 0xc5, 0xcf, 0};
	const int lane_regs[] = {0x4006, 0x400a, 0x6000, 0};
	int lane, offset;

	for (offset = 0; lane_regs[offset]; offset++)
		for (lane = 0; lane < 8; lane++)
			set_phy_register(neigh, link, lane_regs[offset] + (lane + 8) * 0x80, 1,
			                 get_phy_register(neigh, link, lane_regs[offset] + lane * 0x80, 1));

	for (offset = 0; gang_regs[offset]; offset++)
		set_phy_register(neigh, link, gang_regs[offset] + 0x10, 0,
		                 get_phy_register(neigh, link, gang_regs[offset], 0));
}

static void cht_print(int neigh, int link)
{
	uint32_t val;
	printf("HT#%d L%d Link Control      : 0x%08x\n", neigh, link,
	      cht_read_conf(neigh, FUNC0_HT, 0x84 + link * 0x20));
	printf("HT#%d L%d Link Freq/Revision: 0x%08x\n", neigh, link,
	       cht_read_conf(neigh, FUNC0_HT, 0x88 + link * 0x20));
	printf("HT#%d L%d Link Ext. Control : 0x%08x\n", neigh, link,
	       cht_read_conf(neigh, 0, 0x170 + link * 4));
	val = get_phy_register(neigh, link, 0xe0, 0); /* Link phy compensation and calibration control 1 */

	uint8_t rtt = (val >> 23) & 0x1f;
	printf("HT#%d L%d Link Phy Settings : Rtt=%d Ron=%d\n", neigh, link, rtt, (val >> 18) & 0x1f);

	if (rtt == 0) {
		if (workaround_rtt)
			warning("Hypertransport interface phy calibration failure");
		else
			fatal_reboot("Hypertransport interface phy calibration failure");
	}

	if (!(ht_testmode & HT_TESTMODE_PRINT))
		return;

	print_phy_gangs(neigh, link, "link phy impedance", 0xc0);
	print_phy_gangs(neigh, link, "link phy receiver loop filter", 0xc1);
	print_phy_gangs(neigh, link, "link phy timing margin", 0xc3);
	print_phy_gangs(neigh, link, "link phy DFE and DFR control", 0xc4);
	print_phy_gangs(neigh, link, "link phy transmit control", 0xc5);
	print_phy_gangs(neigh, link, "link FIFO read pointer optimisation", 0xcf);
	printf("link phy compensation and calibration control 1: 0x%08x\n",
	       get_phy_register(neigh, link, 0xe0, 0));
	printf("link phy PLL control: 0x%08x\n",
	       get_phy_register(neigh, link, 0xe3, 0));
	printf("link BIST control: 0x%08x\n",
	       get_phy_register(neigh, link, 0x100, 0));
	print_phy_lanes(neigh, link, "link phy DFE and DFR control", 0x4006, 0);
	print_phy_lanes(neigh, link, "link phy DLL control", 0x400a, 0);
	print_phy_lanes(neigh, link, "link phy transmit control", 0x6000, 1);
	printf("link phy transmit clock phase control:\n- CLKOUT[ 0]=0x%08x\n- CLKOUT[ 1]=0x%08x\n",
	       get_phy_register(neigh, link, 0x6884, 1),
	       get_phy_register(neigh, link, 0x6984, 1));

	if (family >= 0x15) {
		print_phy_lanes(neigh, link, "link phy receiver DLL control and test 5", 0x400f, 0);
		print_phy_lanes(neigh, link, "link phy receiver process control", 0x4011, 0);
		print_phy_lanes(neigh, link, "link phy tx deemphasis and margin test control", 0x600c, 1);
	}
}

static void optimise_linkbuffers(const ht_t ht, const int link)
{
	const int IsocRspData = 0, IsocNpReqData = 0, IsocRspCmd = 0, IsocPReq = 0, IsocNpReqCmd = 1;
	const int RspData = 1, NpReqData = 3, ProbeCmd = 4, RspCmd = 9, PReq = 2, NpReqCmd = 8;

	/* Ensure constraints are met */
	int FreeCmd = 32 - NpReqCmd - PReq - RspCmd - ProbeCmd - IsocNpReqCmd - IsocPReq - IsocRspCmd;
	int FreeData = 8 - NpReqData - RspData - PReq - IsocPReq - IsocNpReqData - IsocRspData;
	assert((ProbeCmd + RspCmd + PReq + NpReqCmd + IsocRspCmd + IsocPReq + IsocNpReqCmd) <= 24);

	uint32_t val = NpReqCmd | (PReq << 5) | (RspCmd << 8) | (ProbeCmd << 12) |
	  (NpReqData << 16) | (RspData << 18) | (FreeCmd << 20) | (FreeData << 25);
	cht_write_conf(ht, FUNC0_HT, 0x90 + link * 0x20, val | (1 << 31));

	val = (IsocNpReqCmd << 16) | (IsocPReq << 19) | (IsocRspCmd << 22) |
	  (IsocNpReqData << 25) | (IsocRspData << 27);
	cht_write_conf(ht, FUNC0_HT, 0x94 + link * 0x20, val);
}

static void optimise_all_linkbuffers(const ht_t max_ht)
{
	/* Reprogram HT link buffering */
	for (ht_t ht = 0; ht < max_ht; ht++) {
		for (int link = 0; link < 4; link++) {
			/* Probe Filter doesn't affect IO link buffering */
			uint32_t val = cht_read_conf(ht, FUNC0_HT, 0x98 + link * 0x20);
			if ((!(val & 1)) || (val & 4))
				continue;

			optimise_linkbuffers(ht, link);
		}
	}

	printf("Asserting LDTSTOP# to optimise HT buffer allocation...");
	uint8_t val8 = pmio_readb(0x8a);
	pmio_writeb(0x8a, 0xf0);
	pmio_writeb(0x87, 1);
	pmio_writeb(0x8a, val8);
	printf("done\n");
}

void probefilter_tokens(const ht_t max_ht)
{
	/* Reprogram HT link buffering */
	for (ht_t ht = 0; ht < max_ht; ht++) {
		for (int link = 0; link < 4; link++) {
			uint32_t val = cht_read_conf(ht, FUNC0_HT, 0x98 + link * 0x20);

			/* Probe Filter doesn't affect IO link buffering */
			if ((!(val & 1)) || (val & 4))
				continue;

			/* Same buffer counts for ganged and unganged */
			val = (8 << 20) | (3 << 18) | (3 << 16) | (4 << 12) | (9 << 8) | (2 << 5) | 8;
			cht_write_conf(ht, FUNC0_HT, 0x90 + link * 0x20, val | (1 << 31));
			cht_write_conf(ht, FUNC0_HT, 0x94 + link * 0x20, 1 << 16);
		}
	}

	printf("Asserting LDTSTOP# to optimise HT buffer allocation...");
	uint8_t val = pmio_readb(0x8a);
	pmio_writeb(0x8a, 0xf0);
	pmio_writeb(0x87, 1);
	pmio_writeb(0x8a, val);
	printf("done\n");
}
#endif /* __i386 */

static const int ht_freqs[] = {200, 0, 400, 0, 600, 800, 1000, 1200, 1400, 1600, 1800, 2000, 2200, 2400, 2600, 0, 0, 2800, 3000, 3200};

static void print_ht_path(const uint32_t val)
{
	if (val & 1)
		printf(" this");

	for (int i = 1; i < 9; i++)
		if (val & (1 << i))
			printf(" L%d.%d", (i - 1) % 4, (i - 1) / 4);
}

struct ht_link {
	int bandwidth;
	int usage;
};

static struct ht_link ht_perf[8][4][2];
static const int ht_neigh[4][4][2] = {
	{{2, 8}, {8, 8}, {1, 8}, {8, 8}},
	{{8, 8}, {0, 8}, {8, 8}, {3, 8}},
	{{8, 8}, {8, 8}, {3, 8}, {8, 0}},
	{{8, 8}, {2, 8}, {8, 8}, {1, 8}},
};

static void recurse(ht_t cur, const ht_t dst)
{
	printf(" > HT%d", cur);
	if (cur == 8 || cur == dst)
		return;

	/* Examine request bits only */
	uint32_t val = cht_read_conf(cur, FUNC0_HT, 0x40 + dst * 4) & 0x1ff;
	for (int i = 1; i < 9; i++) {
		/* Check if route is active */
		if (val & (1 << i)) {
			int link = (i - 1) % 4;
			int sublink = (i - 1) / 4;
			printf(" L%d.%d", link, sublink);
			ht_perf[cur][link][sublink].usage += 1;
			recurse(ht_neigh[cur][link][sublink], dst);
		}
	}
}

static void print_ht_routing(const ht_t max_ht)
{
	printf("HT config max_ht=%d:\n", max_ht);
	for (ht_t src = 0; src < max_ht; src++) {
		uint32_t val = cht_read_conf(src, FUNC0_HT, 0x1a0);
		for (int link = 0; link < 4; link++) {
			for (int sublink = 0; sublink < 2; sublink++) {
				/* Skip links where InitComplete is unset */
				if (!(val & (1 << (link * 2 + sublink * 8))))
					continue;

				bool internal = (val >> (16 + link + sublink * 4)) & 1;
				bool noncoh = (val >> (link * 2 + sublink * 8 + 1)) & 1;
				uint32_t val2 = cht_read_conf(src, FUNC0_HT, 0x170 + link * 4 + sublink * 0x10);
				bool ganged = val2 & 1;

				uint32_t val3 = cht_read_conf(src, FUNC0_HT + sublink * 4, 0x84 + link * 0x20);

				/* Only look at width out */
				uint32_t width = (val3 >> 28) ? 16 : 8;

				uint32_t val4 = cht_read_conf(src, FUNC0_HT + sublink * 4, 0x88 + link * 0x20);
				uint32_t val5 = cht_read_conf(src, FUNC0_HT + sublink * 4, 0x9c + link * 0x20);
				uint32_t freq = ((val4 >> 8) & 0x1f) | ((val5 & 1) << 4);

				if (!noncoh)
					ht_perf[src][link][sublink].bandwidth = freq * width;
				printf("- HT%d L%d.%d %s %s %s: %d @ %dMHz bandwidth=%d\n",
					src, link, sublink, noncoh ? "noncoherent" : "coherent", ganged ? "ganged" : "unganged",
					internal ? "internal" : "external", width, ht_freqs[freq], ht_perf[src][link][sublink].bandwidth);
			}
		}
	}

	printf("\nHT routing:\n");
	for (ht_t src = 0; src < max_ht; src++) {
		for (ht_t dst = 0; dst <= max_ht; dst++) {
			printf("HT%d to HT%d:", src, dst);
			recurse(src, dst);
			printf("\n");
/*			printf("- HT%d->HT%d:", src, dst);
			print_ht_path(val & 0x1ff);
			printf(",");
			print_ht_path((val >> 9) & 0x1ff);
			printf(",");
			print_ht_path((val >> 18) & 0x1ff);
			printf("\n"); */
		}
	}

	printf("\nHT performance:\n");
	for (ht_t src = 0; src < max_ht; src++) {
		for (int link = 0; link < 4; link++) {
			for (int sublink = 0; sublink < 2; sublink++) {
				if (ht_perf[src][link][sublink].bandwidth == 0)
					continue;

				int goodness = ht_perf[src][link][sublink].bandwidth / ht_perf[src][link][sublink].usage;
				printf("- HT%d L%d.%d bandwidth=%d usage=%d goodness=%d\n", src, link, sublink, ht_perf[src][link][sublink].bandwidth, ht_perf[src][link][sublink].usage, goodness);
			}
		}
	}
}

static void ht_optimize_link(int nc, int rev, int asic_mode)
{
#ifndef __i386
	printf("(Only doing HT reconfig in 32-bit mode)\n");
	return;
#else
	bool reboot = 0;
	int ganged;
	int neigh;
	int link;
	int next, i;
	uint32_t rqrt, val;
	/* Start looking from node 0 */
	neigh = 0;

	while (1) {
		next = 0;
		rqrt = cht_read_conf(neigh, FUNC0_HT, 0x40 + 4 * nc) & 0x1f;

		/* Look for other CPUs routed on same link as NC */
		for (i = 0; i < nc; i++) {
			if (rqrt == (cht_read_conf(neigh, FUNC0_HT, 0x40 + 4 * i) & 0x1f)) {
				next = i;
				break;
			}
		}

		if (next > 0)
			neigh = next;
		else
			break;
	}

	link = 0;

	while ((2U << link) < rqrt)
		link ++;

	nodes[0].nc_neigh_ht = neigh;
	nodes[0].nc_neigh_link = link;
	ganged = cht_read_conf(neigh, 0, 0x170 + link * 4) & 1;
	printf("Found %s link to NC on HT#%d L%d\n", ganged ? "ganged" : "unganged", neigh, link);
	printf("Checking HT width/freq");

	/* Set T0Time to max on revB and older to avoid high LDTSTOP exit latency */
	if (asic_mode && rev < 2) {
		val = cht_read_conf(neigh, FUNC0_HT, 0x16c);
		cht_write_conf(neigh, FUNC0_HT, 0x16c, (val & ~0x3f) | 0x3a);
		printf(".");
	}

	/* Gang link when appropriate, as the BIOS may not */
	if (ht_force_ganged) {
		val = cht_read_conf(neigh, FUNC0_HT, 0x170 + link * 4);
		if ((val & 1) == 0) {
			printf("<ganging>");
			cht_write_conf(neigh, FUNC0_HT, 0x170 + link * 4, val | 1);
		}
	}

	/* For ASIC revision 2 and later, optimize width (16b) */
	/* For FPGA, optimize width (16b) */
	printf(".");
	val = cht_read_conf(neigh, FUNC0_HT, 0x84 + link * 0x20);
	val &= ~(1 << 13); /* Disable LdtStopTriEn */
	cht_write_conf(neigh, FUNC0_HT, 0x84 + link * 0x20, val);

	if (!ht_8bit_only && (ht_force_ganged || (ganged && ((val >> 16) == 0x11) &&
	                      ((asic_mode && rev >= 2) || !asic_mode)))) {
		printf("*");
		if ((val >> 24) != 0x11) {
			printf("<CPU width>");
			cht_write_conf(neigh, FUNC0_HT, 0x84 + link * 0x20, (val & 0x00ffffff) | 0x11000000);
			reboot = 1;
		}

		udelay(50);
		printf(".");
		val = cht_read_conf(nc, 0, H2S_CSR_F0_LINK_CONTROL_REGISTER);
		printf(".");

		if ((val >> 24) != 0x11) {
			printf("<NC width>");
			cht_write_conf(nc, 0, H2S_CSR_F0_LINK_CONTROL_REGISTER, (val & 0x00ffffff) | 0x11000000);
			reboot = 1;
		}

		printf(".");
	}

	/* On ASIC optimize link frequency (800MHz), if option to disable this is not set */
	if (asic_mode && !ht_200mhz_only) {
		printf("+");
		val = cht_read_conf(neigh, FUNC0_HT, 0x88 + link * 0x20);
		printf(".");

		if (((val >> 8) & 0xf) != 0x5) {
			printf("<CPU freq>");
			cht_write_conf(neigh, FUNC0_HT, 0x88 + link * 0x20, (val & ~0xf00) | 0x500);
			reboot = 1;
		}

		udelay(50);
		printf(".");
		val = cht_read_conf(nc, 0, H2S_CSR_F0_LINK_FREQUENCY_REVISION_REGISTER);
		printf(".");

		if (((val >> 8) & 0xf) != 0x5) {
			printf("<NC freq>");
			cht_write_conf(nc, 0, H2S_CSR_F0_LINK_FREQUENCY_REVISION_REGISTER, (val & ~0xf00) | 0x500);
			reboot = 1;
		}
	}

	if (ht_200mhz_only)
		printf(".<Keep 200MHz>");

	if (ht_force_ganged == 2)
		cht_mirror(neigh, link);

/*	optimise_all_linkbuffers(nc); */

	printf("done\n");

	if (verbose > 1)
		print_ht_routing(nc);

	if (reboot) {
		printf("Rebooting to make new link settings effective...");
		reset_cf9(2, nc - 1);
	}
#endif
}

#ifdef __i386
static void disable_atmmode(const ht_t max_ht)
{
	uint32_t scrub[8];
	uint32_t val = cht_read_conf(0, FUNC0_HT, 0x68);

	if (!(val & (1 << 12))) {
		if (verbose) printf("ATMModeEn already disabled\n");

		return;
	}

	printf("Disabling ATMMode...");

	/* 1. Disable the L3 and DRAM scrubbers on all nodes in the system:
	   - F3x58[L3Scrub]=00h
	   - F3x58[DramScrub]=00h
	   - F3x5C[ScrubRedirEn]=0 */
	for (ht_t ht = 0; ht <= max_ht; ht++) {
		/* Fam15h: Accesses to this register must first set F1x10C [DctCfgSel]=0;
		   Accesses to this register with F1x10C [DctCfgSel]=1 are undefined;
		   See erratum 505 */
		cht_write_conf(ht, FUNC1_MAPS, 0x10c, 0);
		scrub[ht] = cht_read_conf(ht, FUNC3_MISC, 0x58);
		cht_write_conf(ht, FUNC3_MISC, 0x58, scrub[ht] & ~0x1f00001f);
		val = cht_read_conf(ht, FUNC3_MISC, 0x5c);
		cht_write_conf(ht, FUNC3_MISC, 0x5c, val & ~1);
	}

	/* 2.  Wait 40us for outstanding scrub requests to complete */
	udelay(40);
	/* 3.  Disable all cache activity in the system by setting
	   CR0.CD for all active cores in the system */
	/* 4.  Issue WBINVD on all active cores in the system */
	disable_cache();
#ifdef BROKEN

	/* 5.  Set F3x1C4[L3TagInit]=1 */
	for (ht_t ht = 0; ht <= max_ht; ht++) {
		val = cht_read_conf(ht, FUNC3_MISC, 0x1c4);
		cht_write_conf(ht, FUNC3_MISC, 0x1c4, val | (1 << 31));
	}

	/* 6.  Wait for F3x1C4[L3TagInit]=0 */
	for (ht_t ht = 0; ht <= max_ht; ht++)
		while (cht_read_conf(ht, FUNC3_MISC, 0x1c4) & (1 << 31))
			cpu_relax();

#endif

	/* 7.  Set F0x68[ATMModeEn]=0 and F3x1B8[L3ATMModeEn]=0 */
	for (ht_t ht = 0; ht <= max_ht; ht++) {
		val = cht_read_conf(ht, FUNC0_HT, 0x68);
		cht_write_conf(ht, FUNC0_HT, 0x68, val & ~(1 << 12));

		val = cht_read_conf(ht, FUNC3_MISC, 0x1b8);
		cht_write_conf(ht, FUNC3_MISC, 0x1b8, val & ~(1 << 27));
	}

	/* 8.  Enable all cache activity in the system by clearing
	   CR0.CD for all active cores in the system */
	enable_cache();

	/* 9. Restore L3 and DRAM scrubber register values */
	for (ht_t ht = 0; ht <= max_ht; ht++) {
		/* Fam15h: Accesses to this register must first set F1x10C [DctCfgSel]=0;
		   Accesses to this register with F1x10C [DctCfgSel]=1 are undefined;
		   See erratum 505 */
		cht_write_conf(ht, FUNC1_MAPS, 0x10c, 0);
		cht_write_conf(ht, FUNC3_MISC, 0x58, scrub[ht]);
		val = cht_read_conf(ht, FUNC3_MISC, 0x5c);
		cht_write_conf(ht, FUNC3_MISC, 0x5c, val | 1);
	}

	printf("done\n");
}

static void disable_probefilter(const ht_t max_ht)
{
	uint32_t val;
	uint32_t scrub[8];
	ht_t i;
	val = cht_read_conf(0, FUNC3_MISC, 0x1d4);

	/* Probe filter not active? */
	if ((val & 3) == 0) {
		if (verbose) printf("Probe filter already disabled\n");

		return;
	}

	printf("Disabling probe filter...");

	/* 1. Disable the L3 and DRAM scrubbers on all nodes in the system:
	   - F3x58[L3Scrub]=00h
	   - F3x58[DramScrub]=00h
	   - F3x5C[ScrubRedirEn]=0 */
	for (i = 0; i <= max_ht; i++) {
		/* Fam15h: Accesses to this register must first set F1x10C [DctCfgSel]=0;
		   Accesses to this register with F1x10C [DctCfgSel]=1 are undefined;
		   See erratum 505 */
		if (family >= 0x15)
			cht_write_conf(i, FUNC1_MAPS, 0x10c, 0);

		scrub[i] = cht_read_conf(i, FUNC3_MISC, 0x58);
		cht_write_conf(i, FUNC3_MISC, 0x58, scrub[i] & ~0x1f00001f);
		val = cht_read_conf(i, FUNC3_MISC, 0x5c);
		cht_write_conf(i, FUNC3_MISC, 0x5c, val & ~1);
	}

	/* 2.  Wait 40us for outstanding scrub requests to complete */
	udelay(40);
	critical_enter();
	/* 3.  Disable all cache activity in the system by setting
	   CR0.CD for all active cores in the system */
	/* 4.  Issue WBINVD on all active cores in the system */
	disable_cache();

	/* 5.  Set F3x1C4[L3TagInit]=1 */
	for (i = 0; i <= max_ht; i++) {
		val = cht_read_conf(i, FUNC3_MISC, 0x1c4);
		cht_write_conf(i, FUNC3_MISC, 0x1c4, val | (1 << 31));
	}

	/* 6.  Wait for F3x1C4[L3TagInit]=0 */
	for (i = 0; i <= max_ht; i++)
		while (cht_read_conf(i, FUNC3_MISC, 0x1c4) & (1 << 31))
			cpu_relax();

	/* 7.  Set F3x1D4[PFMode]=00b */
	for (i = 0; i <= max_ht; i++) {
		val = cht_read_conf(i, FUNC3_MISC, 0x1d4);
		cht_write_conf(i, FUNC3_MISC, 0x1d4, val & ~3);
	}

	for (i = 0; i <= max_ht; i++) {
		val = cht_read_conf(i, FUNC3_MISC, 0x1d4);
	}

	/* 8.  Enable all cache activity in the system by clearing
	   CR0.CD for all active cores in the system */
	enable_cache();
	critical_leave();

	/* 9. Restore L3 and DRAM scrubber register values */
	for (i = 0; i <= max_ht; i++) {
		/* Fam15h: Accesses to this register must first set F1x10C [DctCfgSel]=0;
		   Accesses to this register with F1x10C [DctCfgSel]=1 are undefined;
		   See erratum 505 */
		if (family >= 0x15)
			cht_write_conf(i, FUNC1_MAPS, 0x10c, 0);

		cht_write_conf(i, FUNC3_MISC, 0x58, scrub[i]);
		val = cht_read_conf(i, FUNC3_MISC, 0x5c);
		cht_write_conf(i, FUNC3_MISC, 0x5c, val | 1);
	}

	printf("done\n");
}

void wake_core_local(const int apicid, const int vector)
{
	assert(apicid < 0xff);

	uint64_t val = rdmsr(MSR_APIC_BAR);
	volatile uint32_t *const apic = (volatile uint32_t *const)((uint32_t)val & ~0xfff);
	volatile uint32_t *const icr = &apic[0x300 / 4];

	/* Deliver initialize IPI */
	apic[0x310 / 4] = apicid << 24;
	*icr = 0x00004500;

	while (*icr & (1 << 12))
		cpu_relax();

	assert(!apic[0x280 / 4]); /* Check for errors */

	*REL32(cpu_status) = vector;
	/* Deliver startup IPI */
	apic[0x310 / 4] = apicid << 24;
	assert(((uint32_t)REL32(init_dispatch) & ~0xff000) == 0);
	*icr = 0x00004600 | ((uint32_t)REL32(init_dispatch) >> 12);

	while (*icr & 0x1000)
		cpu_relax();

	assert(!apic[0x280 / 4]); /* Check for errors */

	/* Wait until execution completed */
	while (*REL32(cpu_status))
		cpu_relax();
}

void wake_core_global(const int apicid, const int vector)
{
	*REL32(cpu_status) = vector;

	/* Deliver initialize IPI */
	dnc_write_csr(0xfff0, H2S_CSR_G3_EXT_INTERRUPT_GEN, 0xff001500 | (apicid << 16));

	/* Deliver startup IPI */
	assert(((uint32_t)REL32(init_dispatch) & ~0xff000) == 0);
	dnc_write_csr(0xfff0, H2S_CSR_G3_EXT_INTERRUPT_GEN,
		0xff002600 | (apicid << 16) | ((uint32_t)REL32(init_dispatch) >> 12));

	/* Wait until execution completed */
	while (*REL32(cpu_status))
		cpu_relax();
}

static void wake_cores_local(const int vector)
{
	for (int i = 1; post_apic_mapping[i] != 255; i++)
		wake_core_local(post_apic_mapping[i], vector);
}

void enable_probefilter(const ht_t max_ht)
{
	uint32_t val;
	uint32_t scrub[8];
	uint64_t val6;
	int i;
	val = cht_read_conf(0, FUNC3_MISC, 0x1d4);

	/* Probe filter already active? */
	if (val & 3) {
		if (verbose) printf("Probe filter already enabled\n");

		return;
	}

	printf("Enabling probe filter...");

	/* 1. Disable the L3 and DRAM scrubbers on all nodes in the system:
	   - F3x58[L3Scrub]=00h
	   - F3x58[DramScrub]=00h
	   - F3x5C[ScrubRedirEn]=0 */
	for (i = 0; i <= max_ht; i++) {
		/* Fam15h: Accesses to this register must first set F1x10C [DctCfgSel]=0;
		   Accesses to this register with F1x10C [DctCfgSel]=1 are undefined;
		   See erratum 505 */
		if (family >= 0x15)
			cht_write_conf(i, FUNC1_MAPS, 0x10c, 0);

		scrub[i] = cht_read_conf(i, FUNC3_MISC, 0x58);
		cht_write_conf(i, FUNC3_MISC, 0x58, scrub[i] & ~0x1f00001f);
		val = cht_read_conf(i, FUNC3_MISC, 0x5c);
		cht_write_conf(i, FUNC3_MISC, 0x5c, val & ~1);
	}

	/* 2. Wait 40us for outstanding scrub requests to complete */
	udelay(40);
	critical_enter();

	/* 3. Ensure CD bit is shared amongst cores */
	if (family >= 0x15) {
		val6 = rdmsr(MSR_CU_CFG3);
		wrmsr(MSR_CU_CFG3, val6 | (1ULL << 49));
	}

	/* 4.  Issue WBINVD on all active cores in the system */
	disable_cache();
	/* 5. Enable Probe Filter support */
	val6 = rdmsr(MSR_CU_CFG2) | (1ULL << 42);
	wrmsr(MSR_CU_CFG2, val6);
	*REL64(new_cucfg2_msr) = val6;

	if (family >= 0x15)
		wake_cores_local(VECTOR_PROBEFILTER_EARLY_f15);
	else
		wake_cores_local(VECTOR_PROBEFILTER_EARLY_f10);

	/* 6. Set F3x1C4[L3TagInit]=1 */
	for (i = 0; i <= max_ht; i++) {
		val = cht_read_conf(i, FUNC3_MISC, 0x1c4);
		cht_write_conf(i, FUNC3_MISC, 0x1c4, val | (1 << 31));
	}

	/* 7. Wait for F3x1C4[L3TagInit]=0 */
	for (i = 0; i <= max_ht; i++)
		while (cht_read_conf(i, FUNC3_MISC, 0x1c4) & (1 << 31))
			cpu_relax();

	/* 8. Set PF flags depending on cache size */
	for (i = 0; i <= max_ht; i++) {
		uint32_t pfctrl = (2 << 2) | (0xf << 12) | (1 << 17) | (1 << 29);

		if ((cht_read_conf(i, FUNC3_MISC, 0x1c4) & 0xffff) == 0xcccc)
			pfctrl |= 3 | (1 << 4) | (1 << 6) | (1 << 8) | (1 << 10) | (2 << 20);
		else
			pfctrl |= 2;

		cht_write_conf(i, FUNC3_MISC, 0x1d4, pfctrl);
	}

	/* 6. Wait for F3x1D4[PFInitDone]=1 */
	for (i = 0; i <= max_ht; i++) {
		while ((cht_read_conf(i, FUNC3_MISC, 0x1d4) & (1 << 19)) == 0)
			cpu_relax();
	}

	/* 8.  Enable all cache activity in the system by clearing
	   CR0.CD for all active cores in the system */
	enable_cache();
	wake_cores_local(VECTOR_ENABLE_CACHE);
	critical_leave();

	/* 9. Restore L3 and DRAM scrubber register values */
	for (i = 0; i <= max_ht; i++) {
		/* Fam15h: Accesses to this register must first set F1x10C [DctCfgSel]=0;
		   Accesses to this register with F1x10C [DctCfgSel]=1 are undefined;
		   See erratum 505 */
		if (family >= 0x15)
			cht_write_conf(i, FUNC1_MAPS, 0x10c, 0);

		cht_write_conf(i, FUNC3_MISC, 0x58, scrub[i]);
		val = cht_read_conf(i, FUNC3_MISC, 0x5c);
		cht_write_conf(i, FUNC3_MISC, 0x5c, val | 1);
	}

	printf("done\n");
}

static void disable_link(int node, int link)
{
	uint32_t val;
	val = cht_read_conf(node, FUNC0_HT, 0x16c);
	cht_write_conf(node, FUNC0_HT, 0x16c, val & ~(1 << 8));
	printf("HT#%d F0x16c: %08x\n", node, cht_read_conf(node, FUNC0_HT, 0x16c));
	printf("HT#%d F0x%02x: %08x\n", node, 0x84 + 0x20 * link,
	       cht_read_conf(node, FUNC0_HT, 0x84 + 0x20 * link));
	val = cht_read_conf(node, FUNC0_HT, 0x84 + 0x20 * link);
	cht_write_conf(node, FUNC0_HT, 0x84 + 0x20 * link, val | 0xc0);
	printf("HT#%d F0x%02x: %08x\n", node, 0x84 + 0x20 * link,
	       cht_read_conf(node, FUNC0_HT, 0x84 + 0x20 * link));
}
#endif /* __i386 */

static bool disable_nc = 0;

static int ht_fabric_find_nc(bool *p_asic_mode, uint32_t *p_chip_rev)
{
#ifndef __i386
	printf("(Only doing HT discovery and reconfig in 32-bit mode)\n");
	return -1;
#else
	int max_ht, neigh_ht, link = 0, rt, nc_ht;
	uint32_t val;
	val = cht_read_conf(0, FUNC0_HT, 0x60);
	max_ht = (val >> 4) & 7;
	bool use = 1;

	for (neigh_ht = 0; neigh_ht <= max_ht; neigh_ht++) {
		uint32_t aggr = cht_read_conf(neigh_ht, FUNC0_HT, 0x164);

		for (link = 0; link < 4; link++) {
			val = cht_read_conf(neigh_ht, FUNC0_HT, 0x98 + link * 0x20);

			if ((val & 0x1f) != 0x3)
				continue; /* Not coherent */

			use = 0;

			if (aggr & (0x10000 << link))
				use = 1;

			for (rt = 0; rt <= max_ht; rt++) {
				val = cht_read_conf(neigh_ht, FUNC0_HT, 0x40 + rt * 4);

				if (val & (2 << link))
					use = 1; /* Routing entry "rt" uses link "link" */
			}

			if (!use)
				break;
		}

		if (!use)
			break;
	}

	if (use) {
		error("No unrouted coherent links found");
		return -1;
	}

	printf("HT#%d L%d is coherent and unrouted\n", neigh_ht, link);

	if (disable_nc) {
		printf("Disabling NC link.\n");
		disable_link(neigh_ht, link);
		return -1;
	}

	nc_ht = max_ht + 1;
	/* "neigh" request/response routing, copy bcast values from self */
	val = cht_read_conf(neigh_ht, FUNC0_HT, 0x40 + neigh_ht * 4);
	cht_write_conf(neigh_ht, FUNC0_HT, 0x40 + nc_ht * 4,
	               (val & 0x07fc0000) | (0x402 << link));

	for (ht_t ht = 0; ht <= max_ht; ht++) {
		val = cht_read_conf(ht, FUNC0_HT, 0x68);
		cht_write_conf(ht, FUNC0_HT, 0x68, val & ~(1 << 15)); /* LimitCldtCfg */

		if (ht == neigh_ht)
			continue;

		/* Route "nc_ht" same as "neigh_ht" for all other nodes */
		val = cht_read_conf(ht, FUNC0_HT, 0x40 + neigh_ht * 4);
		cht_write_conf(ht, FUNC0_HT, 0x40 + nc_ht * 4, val);
	}

#ifdef __i386
	cht_print(neigh_ht, link);
#endif

	/* Earliest opportunity to test HT link */
	if (ht_testmode & HT_TESTMODE_TEST)
		cht_test(nc_ht, neigh_ht, link);

	val = cht_read_conf(nc_ht, 0, H2S_CSR_F0_DEVICE_VENDOR_ID_REGISTER);

	if (val != 0x06011b47) {
		error("Unrouted coherent device %08x is not NumaChip", val);
		return -1;
	}

	printf("NumaChip found (%08x)\n", val);
	/* Ramp up link speed and width before adding NC to coherent fabric */
	val = cht_read_conf(nc_ht, 0, 0xec);

	if (val == 0) {
		val = cht_read_conf(nc_ht, 0, H2S_CSR_F0_CLASS_CODE_REVISION_ID_REGISTER);
		printf("Doing link calibration of ASIC chip rev %d\n", val & 0xffff);
		ht_optimize_link(nc_ht, val & 0xffff, 1);
		*p_asic_mode = 1;
		*p_chip_rev = val & 0xffff;
	} else {
		printf("Doing link calibration of FPGA chip rev %d_%d\n", val >> 16, val & 0xffff);
		ht_optimize_link(nc_ht, val, 0);
		*p_asic_mode = 0;
		*p_chip_rev = val;
	}

	/* RevB ASIC requires syncflood to be disabled */
	if (*p_asic_mode && *p_chip_rev < 2)
		ht_suppress = 0x3fff; /* Suppress most sync-flood generation */

	if (ht_suppress) {
		printf("Setting HT features to 0x%x...", ht_suppress);

		for (ht_t ht = 0; ht <= max_ht; ht++) {
			val = cht_read_conf(ht, FUNC3_MISC, 0x44);

			/* SyncOnUcEccEn: sync flood on uncorrectable ECC error enable */
			if (ht_suppress & 0x1) val &= ~(1 << 2);

			/* SyncPktGenDis: sync packet generation disable */
			if (ht_suppress & 0x2) val |= 1 << 3;

			/* SyncPktPropDis: sync packet propagation disable */
			if (ht_suppress & 0x4) val |= 1 << 4;

			/* SyncOnWDTEn: sync flood on watchdog timer error enable */
			if (ht_suppress & 0x8) val &= ~(1 << 20);

			/* SyncOnAnyErrEn: sync flood on any error enable */
			if (ht_suppress & 0x10) val &= ~(1 << 21);

			/* SyncOnDramAdrParErrEn: sync flood on DRAM address parity error enable */
			if (ht_suppress & 0x20) val &= ~(1 << 30);

			cht_write_conf(ht, FUNC3_MISC, 0x44, val);
			val = cht_read_conf(ht, FUNC3_MISC, 0x180);

			/* SyncFloodOnUsPwDataErr: sync flood on upstream posted write data error */
			if (ht_suppress & 0x40) val &= ~(1 << 1);

			/* SyncFloodOnDatErr */
			if (ht_suppress & 0x80) val &= ~(1 << 6);

			/* SyncFloodOnTgtAbtErr */
			if (ht_suppress & 0x100) val &= ~(1 << 7);

			/* SyncOnProtEn: sync flood on protocol error enable */
			if (ht_suppress & 0x200) val &= ~(1 << 8);

			/* SyncOnUncNbAryEn: sync flood on uncorrectable NB array error enable */
			if (ht_suppress & 0x400) val &= ~(1 << 9);

			/* SyncFloodOnL3LeakErr: sync flood on L3 cache leak error enable */
			if (ht_suppress & 0x800) val &= ~(1 << 20);

			/* SyncFloodOnCpuLeakErr: sync flood on CPU leak error enable */
			if (ht_suppress & 0x1000) val &= ~(1 << 21);

			/* SyncFloodOnTblWalkErr: sync flood on table walk error enable */
			if (ht_suppress & 0x2000) val &= ~(1 << 22);

			cht_write_conf(ht, FUNC3_MISC, 0x180, val);

			/* SERR_EN: initiate sync flood when PCIe System Error detected in IOH */
			if (ht_suppress & 0x4000) {
				val = dnc_read_conf(0xfff0, 0, 0, 0, 0x4);
				dnc_write_conf(0xfff0, 0, 0, 0, 0x4, val & ~(1 << 8));
			}

			for (int i = 0; i < 4; i++) {
				/* CrcFloodEn: Enable sync flood propagation upon link failure */
				if (ht_suppress & 0x8000) {
					val = cht_read_conf(i, FUNC0_HT, 0x84 + i * 0x20);
					cht_write_conf(i, FUNC0_HT, 0x84 + i * 0x20, val & ~2);
				}
			}
		}

		printf("done\n");
	}

	/* HT looping testmode useful after link is up at 16-bit 800MHz */
	if (ht_testmode & HT_TESTMODE_LOOP)
		while (1)
			cht_test(nc_ht, neigh_ht, link);

	if (route_only)
		return nc_ht;

	/* On ASIC revB we need to make sure probefilter is disabled */
	if ((*p_asic_mode && (*p_chip_rev < 2)) || force_probefilteroff)
		disable_probefilter(max_ht);

	/* On Fam15h disable the Accelerated Transiton to Modified protocol */
	if (family >= 0x15)
		disable_atmmode(max_ht);

	printf("Adjusting HT fabric...");
	critical_enter();

	for (int ht = max_ht; ht >= 0; ht--) {
		uint32_t ltcr, val2;
		/* Disable probes while adjusting */
		ltcr = cht_read_conf(ht, FUNC0_HT, 0x68);
		cht_write_conf(ht, FUNC0_HT, 0x68,
		               ltcr | (1 << 10) | (1 << 3) | (1 << 2) | (1 << 1) | (1 << 0));
		/* Update "neigh" bcast values for node about to increment fabric size */
		val = cht_read_conf(neigh_ht, FUNC0_HT, 0x40 + ht * 4);
		val2 = cht_read_conf(ht, FUNC0_HT, 0x60);
		cht_write_conf(neigh_ht, FUNC0_HT, 0x40 + ht * 4, val | (0x80000 << link));
		/* FIXME: Race condition observered to cause lockups at this point */
		/* Increase fabric size */
		cht_write_conf(ht, FUNC0_HT, 0x60, val2 + (1 << 4));
		/* Reassert LimitCldtCfg */
		cht_write_conf(ht, FUNC0_HT, 0x68, ltcr | (1 << 15));
	}

	critical_leave();
	printf("done\n");
	/* reorganize_mmio(nc_ht); */
	return nc_ht;
#endif
}

static int ht_fabric_fixup(bool *p_asic_mode, uint32_t *p_chip_rev)
{
	uint32_t val;
	uint8_t node;
	int nc_ht;

	/* Ensure SMIs are invoked when accessing config space */
	assert(!rdmsr(MSR_TRAP_CTL));

	val = cht_read_conf(0, FUNC0_HT, 0x60);
	nc_ht = (val >> 4) & 7;
	val = cht_read_conf(nc_ht, 0, H2S_CSR_F0_DEVICE_VENDOR_ID_REGISTER);

	if (val == 0x06011b47) {
		printf("NumaChip already present on HT node %d\n", nc_ht);
		/* Chip already found; make sure the desired width/frequency is set */
		val = cht_read_conf(nc_ht, 0, 0xec);

		if (val == 0) {
			val = cht_read_conf(nc_ht, 0, H2S_CSR_F0_CLASS_CODE_REVISION_ID_REGISTER);
			printf("Doing link calibration of ASIC chip rev %d\n", val & 0xffff);
			ht_optimize_link(nc_ht, val & 0xffff, 1);
			*p_asic_mode = 1;
			*p_chip_rev = val & 0xffff;
		} else {
			printf("Doing link calibration of FPGA chip rev %d_%d\n", val >> 16, val & 0xffff);
			ht_optimize_link(nc_ht, val, 0);
			*p_asic_mode = 0;
			*p_chip_rev = val;
		}
	} else {
		nc_ht = ht_fabric_find_nc(p_asic_mode, p_chip_rev);

		if (nc_ht < 0) {
			printf("NumaChip not found\n");
			*p_asic_mode = -1;
			*p_chip_rev = -1;
			return -1;
		}

		printf("NumaChip incorporated as HT node %d\n", nc_ht);
	}

	val = cht_read_conf(nc_ht, 0, H2S_CSR_F0_DEVICE_VENDOR_ID_REGISTER);
	printf("Node #%d F0x00: %x\n", nc_ht, val);
	val = cht_read_conf(0, FUNC0_HT, 0x60);
	cht_write_conf(nc_ht, 0,
	               H2S_CSR_F0_CHTX_NODE_ID,
	               (((val >> 12) & 7) << 24) | /* LkNode */
	               (((val >> 8)  & 7) << 16) | /* SbNode */
	               (nc_ht << 8) | /* NodeCnt */
	               nc_ht); /* NodeId */
	/* Adjust global CSR_BASE to 46 bits in order to access it in Linux */
	DNC_CSR_BASE = 0x3fff00000000ULL;
	DNC_CSR_LIM = 0x3fffffffffffULL;

	/* Enable IOH high addressing before enabling on IO links from the NBs */
	val = dnc_read_conf(0xfff0, 0, 0, 0, 0);
	if (val == VENDEV_SR5690 || val == VENDEV_SR5670 || val == VENDEV_SR5650) {
		/* Enable 52-bit PCIe address generation */
		val = dnc_read_conf(0xfff0, 0, 0, 0, 0xc8);
		dnc_write_conf(0xfff0, 0, 0, 0, 0xc8, val | (1 << 15));
	}

	/* Since we use high addresses for our CSR and MCFG space, make sure the necessary
	   features in the CPU is enabled before we start using them */
	for (node = 0; node < nc_ht; node++) {
		/* Enable CF8 extended access for all NBs, as Linux needs this later */
		set_cf8extcfg_enable(node);

		val = cht_read_conf(node, FUNC0_HT, 0x68);
		if ((val & ((1 << 25) | (1 << 18) | (1 << 17))) != ((1 << 25) | (1 << 18) | (1 << 17))) {
			if (verbose > 0)
				printf("Enabling cHtExtAddrEn, ApicExtId and ApicExtBrdCst on node %d\n", node);

			cht_write_conf(node, FUNC0_HT, 0x68,
			               val | (1 << 25) | (1 << 18) | (1 << 17));
		}

		/* Enable 128MB-granularity on extended MMIO maps */
		if (family < 0x15) {
			val = cht_read_conf(node, FUNC0_HT, 0x168);
			if ((val & 0x300) != 0x200)
				cht_write_conf(node, FUNC0_HT, 0x168, (val & ~0x300) | 0x200);
		}

		/* Enable Addr64BitEn on IO links */
		for (int i = 0; i < 4; i++) {
			/* Skip coherent/disabled links */
			val = cht_read_conf(node, FUNC0_HT, 0x98 + i * 0x20);

			if (!(val & (1 << 2)))
				continue;

			val = cht_read_conf(node, FUNC0_HT, 0x84 + i * 0x20);

			if (!(val & (1 << 15))) {
				printf("Enabling 64bit I/O addressing on %x#%x...\n", node, i);
				cht_write_conf(node, FUNC0_HT, 0x84 + i * 0x20, val | (1 << 15));
			}
		}
	}

	/* Check if BIOS has assigned a BAR0, if so clear it */
	val = cht_read_conf(nc_ht, 0, H2S_CSR_F0_STATUS_COMMAND_REGISTER);
	printf("Command/Status: %08x\n", val);

	if (val & (1 << 1)) {
		cht_write_conf(nc_ht, 0,
		               H2S_CSR_F0_STATUS_COMMAND_REGISTER, val & ~(1 << 1));
		cht_write_conf(nc_ht, 0,
		               H2S_CSR_F0_BASE_ADDRESS_REGISTER_0, 0);
		cht_write_conf(nc_ht, 0,
		               H2S_CSR_F0_EXPANSION_ROM_BASE_ADDRESS, 0);
	}

	/* Check the expansion rom base address register if this has already been done */
	val = cht_read_conf(nc_ht, 0, H2S_CSR_F0_EXPANSION_ROM_BASE_ADDRESS);

	if (val != 0 && !(val & 1)) {
		if ((val & 0xffff0000) != (DNC_CSR_BASE >> 16)) {
			printf("Mismatching CSR space hi addresses %04x and %04x; warm-reset needed\n",
			       (uint32_t)(val >> 16), (uint32_t)(DNC_CSR_BASE >> 32));
			return -1;
		}
	} else {
#ifdef __i386
		/* Bootloader mode, modify CSR_BASE_ADDRESS through the default global maps,
		 * and set this value in expansion rom base address register */
		printf("Setting default CSR maps...\n");
		for (node = 0; node < nc_ht; node++)
			mmio_range(0xfff0, node, 8, DEF_DNC_CSR_BASE, DEF_DNC_CSR_LIM, nc_ht, 0, 0);

		printf("Setting CSR_BASE_ADDRESS to %04llx using default address\n", (DNC_CSR_BASE >> 32));
		mem64_write32(DEF_DNC_CSR_BASE | (0xfff0 << 16) | (1 << 15) | H2S_CSR_G3_CSR_BASE_ADDRESS,
		              uint32_tbswap(DNC_CSR_BASE >> 32));
		cht_write_conf(nc_ht, 0,
		               H2S_CSR_F0_EXPANSION_ROM_BASE_ADDRESS,
		               (DNC_CSR_BASE >> 16)); /* Put DNC_CSR_BASE[47:32] in the rom address register offset[31:16] */
#else
		/* XXX: Perhaps we could try default global map in userspace too ? */
		printf("Unable to determine CSR space address\n");
		return -1;
#endif
	}

	printf("Setting CSR and MCFG maps...\n");
	for (node = 0; node < nc_ht; node++) {
		mmio_range(0xfff0, node, 8, DNC_CSR_BASE, DNC_CSR_LIM, nc_ht, 0, 1);
		mmio_range(0xfff0, node, 9, DNC_MCFG_BASE, DNC_MCFG_LIM, nc_ht, 0, 0);
	}

	/* Set MMCFG base register so local NC will forward correctly */
	val = dnc_read_csr(0xfff0, H2S_CSR_G3_MMCFG_BASE);

	if (val != (DNC_MCFG_BASE >> 24)) {
		printf("Setting local MCFG_BASE to %08llx\n", DNC_MCFG_BASE >> 24);
		dnc_write_csr(0xfff0, H2S_CSR_G3_MMCFG_BASE, DNC_MCFG_BASE >> 24);
	}

	cht_write_conf(nc_ht, 0, H2S_CSR_F0_CHTX_LINK_INITIALIZATION_CONTROL, 0);
	cht_write_conf(nc_ht, 0, H2S_CSR_F0_CHTX_ADDITIONAL_LINK_TRANSACTION_CONTROL, 6);
	return nc_ht;
}

#define SPI_INSTR_WRSR  0x01
#define SPI_INSTR_WRITE 0x02
#define SPI_INSTR_READ  0x03
#define SPI_INSTR_WRDI  0x04
#define SPI_INSTR_RDSR  0x05
#define SPI_INSTR_WREN  0x06

static void set_eeprom_instruction(uint32_t instr)
{
	uint32_t reg;
	dnc_write_csr(0xfff0, H2S_CSR_G3_SPI_INSTRUCTION_AND_STATUS, instr);
	reg = 0x100;

	while (reg & 0x100) {
		reg = dnc_read_csr(0xfff0, H2S_CSR_G3_SPI_INSTRUCTION_AND_STATUS);

		if (reg & 0x100) udelay(100);
	}
}

#ifdef UNUSED
static int write_eeprom_dword(uint32_t addr, uint32_t val)
{
	uint32_t status;
	int i;
	set_eeprom_instruction(SPI_INSTR_WREN);
	udelay(100);
	dnc_write_csr(0xfff0, H2S_CSR_G3_SPI_READ_WRITE_DATA, val);
	set_eeprom_instruction((addr << 16) | SPI_INSTR_WRITE);
	status = 1;
	i = 0;

	while (status & 1) {
		if (i++ > 1000) {
			if (verbose)
				printf("EEPROM WRITE instruction did not complete: %08x\n", status);

			return -1;
		}

		set_eeprom_instruction(SPI_INSTR_RDSR);
		status = dnc_read_csr(0xfff0, H2S_CSR_G3_SPI_READ_WRITE_DATA);
	}

	set_eeprom_instruction(SPI_INSTR_WRDI);
	return 0;
}
#endif

static uint32_t read_eeprom_dword(uint32_t addr)
{
	set_eeprom_instruction(SPI_INSTR_WRDI);
	udelay(100);
	set_eeprom_instruction((addr << 16) | SPI_INSTR_READ);
	return dnc_read_csr(0xfff0, H2S_CSR_G3_SPI_READ_WRITE_DATA);
}

static uint32_t identify_eeprom(char p_type[16])
{
	uint32_t reg;
	int i;

	/* Read print type */
	for (i = 0; i < 4; i++) {
		reg = read_eeprom_dword(0xffc0 + i * 4);
		memcpy(&p_type[i * 4], &reg, 4);
	}

	p_type[15] = '\0';
	/* Read UUID */
	return read_eeprom_dword(0xfffc);
}

static void _pic_reset_ctrl(void)
{
	/* Pulse reset */
	dnc_write_csr(0xfff0, H2S_CSR_G1_PIC_RESET_CTRL, 1);
	udelay(20000);
	(void)dnc_read_csr(0xfff0, H2S_CSR_G1_PIC_INDIRECT_READ); /* Use a read operation to terminate the current i2c transaction, to avoid a bug in the uC */
	udelay(2000000);
}

static bool _is_pic_present(const char p_type[16])
{
	if ((strncmp("313001", p_type, 6) == 0) ||
	    (strncmp("N313001", p_type, 7) == 0) ||
	    (strncmp("N313002", p_type, 7) == 0) ||
	    (strncmp("N313025", p_type, 7) == 0) ||
	    (strncmp("N323011", p_type, 7) == 0) ||
	    (strncmp("N323023", p_type, 7) == 0) ||
	    (strncmp("N323024", p_type, 7) == 0)) {
		return 1;
	}

	return 0;
}

void adjust_oscillator(const char p_type[16], const uint32_t osc_setting)
{
	assertf(osc_setting <= 2, "Invalid oscillator setting %d", osc_setting);

	if (enable_relfreq) {
		printf("PHY Relative Frequency Correction mode enabled, not adjusting oscillators\n");
		return;
	}

	/* Check if adjusting the frequency is possible */
	if (!_is_pic_present(p_type)) {
		printf("Oscillator not set\n");
		return;
	}

	dnc_write_csr(0xfff0, H2S_CSR_G1_PIC_INDIRECT_READ, 0x40); /* Set the indirect read address register */
	udelay(10000);
	uint32_t val = dnc_read_csr(0xfff0, H2S_CSR_G1_PIC_INDIRECT_READ);

	/* Mask out bit7 of every byte because it's missing.. */
	assertf(((val & 0x007f7f7f) == (0x0000b0e9 & 0x007f7f7f)), "External microcontroller not working");

	/* On the RevC cards the micro controller isn't quite fast enough
	 * to send bit7 of every byte correctly on the I2C bus when reading;
	 * the bits we care about are bit[1:0] of the high order byte */
	if (((val >> 24) & 3) != osc_setting) {
		/* Write directly to the frequency selct register */
		val = 0x0000b0e9 | (osc_setting << 24);
		dnc_write_csr(0xfff0, H2S_CSR_G1_PIC_INDIRECT_READ + 0x40, val);
		/* Wait for the new frequency to settle */
		udelay(10000);
		/* Read back value, to verify */
		dnc_write_csr(0xfff0, H2S_CSR_G1_PIC_INDIRECT_READ, 0x40); /* Set the indirect read address register */
		udelay(10000);
		val = dnc_read_csr(0xfff0, H2S_CSR_G1_PIC_INDIRECT_READ);
		assertf(((val >> 24) & 3) == osc_setting, "Oscillator setting not set correctly at %08x", val);
		printf("Oscillator set to %d\n", (val >> 24) & 3);
		/* Trigger a HSS PLL reset */
		_pic_reset_ctrl();
	}
}

struct optargs {
	const char label[20];
	void (*handler)(const char *, void *);
	void *result;
};

static void parse_string(const char *val, void *stringp)
{
	assert(val);
	char **string = (char **)stringp;
	*string = strdup(val);
	assert(*string);
}

static void parse_bool(const char *val, void *voidp)
{
	bool *boolp = (bool *)voidp;

	if (val && val[0] != '\0') {
		int res = atoi(val);
		assertf(res == !!res, "Boolean option doesn't take value %d", res);
		*boolp = res;
	} else
		*boolp = true;
}

static void parse_int(const char *val, void *intp)
{
	int *int32 = (int *)intp;
	if (val && val[0] != '\0')
		*int32 = (int)strtol(val, NULL, 0);
	else
		*int32 = 1;
}

static void parse_int64(const char *val, void *intp)
{
	uint64_t *int64 = (uint64_t *)intp;

	if (val && val[0] != '\0') {
		char *endptr;
		uint64_t ret = strtoull(val, &endptr, 0);

		switch (*endptr) {
		case 'T':
		case 't':
			ret <<= 10;
		case 'G':
		case 'g':
			ret <<= 10;
		case 'M':
		case 'm':
			ret <<= 10;
		case 'K':
		case 'k':
			ret <<= 10;
			endptr++;
		default:
			break;
		}

		*int64 = ret;
	} else
		*int64 = 1;
}

void parse_cmdline(const int argc, const char *argv[])
{
	static const struct optargs options[] = {
		{"config",          &parse_string, &config_file_name},/* Config (JSON) file to use */
		{"next-label",	    &parse_string, &next_label},      /* Next PXELINUX label to boot after loader */
		{"microcode",	    &parse_string, &microcode_path},  /* Path to microcode to be loaded into chip */
		{"init-only",	    &parse_bool,   &init_only},       /* Only initialize chip, but then load <nest-label> without setting up a full system */
		{"route-only",	    &parse_bool,   &route_only},
		{"disable-nc",	    &parse_bool,   &disable_nc},      /* Disable the HT link to NumaChip */
		{"enablenbmce",	    &parse_int,    &enable_nbmce},    /* Enable northbridge MCE */
		{"enablenbwdt",	    &parse_int,    &enable_nbwdt},    /* Enable northbridge WDT */
		{"disable-sram",    &parse_bool,   &disable_sram},    /* Disable SRAM chip, needed for newer cards without SRAM */
		{"ht.testmode",	    &parse_int,    &ht_testmode},
		{"ht.force-ganged", &parse_int,    &ht_force_ganged}, /* Force setup of 16bit (ganged) HT link to NC */
		{"ht.8bit-only",    &parse_bool,   &ht_8bit_only},
		{"ht.suppress",     &parse_int,    &ht_suppress},     /* Disable HT sync flood and related */
		{"ht.200mhz-only",  &parse_int,    &ht_200mhz_only},  /* Disable increase in speed from 200MHz to 800Mhz for HT link to ASIC based NC */
		{"ht.lockdelay",    &parse_int,    &ht_lockdelay},    /* HREQ_CTRL lock_delay setting 0-7 */
		{"ht.force-pf-on",  &parse_int,    &force_probefilteron},  /* Enable probe filter if disabled */
		{"ht.force-pf-off", &parse_int,    &force_probefilteroff}, /* Disable probefilter if enabled */
		{"disable-pf",      &parse_int,    &force_probefilteroff}, /* Disable probefilter if enabled */
		{"pf.cstate6",      &parse_bool,   &pf_cstate6},      /* Enable C-state 6 (allowing boosting) */
		{"handover-acpi",   &parse_bool,   &handover_acpi},   /* Workaround Linux not being able to handover ACPI */
		{"disable-smm",     &parse_bool,   &disable_smm},     /* Rewrite start of System Management Mode handler to return */
		{"disable-c1e",     &parse_bool,   &disable_c1e},     /* Prevent C1E sleep state entry and LDTSTOP usage */
		{"renumber-bsp",    &parse_int,    &renumber_bsp},
		{"forwarding-mode", &parse_int,    &forwarding_mode},
		{"sync-interval",   &parse_int,    &sync_interval},
		{"enable-relfreq",  &parse_bool,   &enable_relfreq},
		{"singleton",       &parse_bool,   &singleton},       /* Loopback test with cables */
		{"mem-offline",     &parse_bool,   &mem_offline},
		{"trace-buf",       &parse_int64,  &trace_buf_size},
		{"verbose",         &parse_int,    &verbose},
		{"remote-io",       &parse_int,    &remote_io},
		{"boot-wait",       &parse_bool,   &boot_wait},
		{"dimmtest",	    &parse_int,    &dimmtest},        /* Run on-board DIMM self test */
		{"workaround.hreq", &parse_bool,   &workaround_hreq}, /* Enable half HReq buffers; on by default */
		{"workaround.rtt",  &parse_bool,   &workaround_rtt},  /* Prevent failure when HT Rtt calibration fails */
		{"workaround.locks", &parse_bool,  &workaround_locks},/* Prevent failure when SMI triggers are locked */
		{"mem-gap",         &parse_int64,  &mem_gap},
		{"disable-kvm",     &parse_bool,   &disable_kvm},     /* Disable virtual USB keyboard and mouse ports */
	};

	int errors = 0;
	printf("Options:");
	for (int arg = 1; arg < argc; arg++) {
		/* Break into two strings where '=' found */
		char *val = strchr(argv[arg], '=');
		if (val) {
			*val = '\0';
			val++; /* Points to value */
		}

		bool handled = 0;
		for (unsigned int i = 0; i < (sizeof(options) / sizeof(options[0])); i++) {
			if (!strcmp(argv[arg], options[i].label)) {
				printf(" %s", argv[arg]);
				if (val)
					printf("=%s", val);

				options[i].handler(val, options[i].result);
				handled = 1;
				break;
			}
		}

		if (!handled) {
			printf(" %s (!)", argv[arg]);
			errors++;
		}
	}
	printf("\n");

	assertf(!errors, "Invalid arguments specified");

	/* Constraints */
	if (trace_buf_size && pf_cstate6) {
		warning("Tracing is exclusive of C-state 6; disabling C-state 6");
		pf_cstate6 = 0;
	}
}

static void perform_selftest(int asic_mode, char p_type[16])
{
	int pass;
	uint32_t val;
	printf("Performing internal RAM self test...");

	for (pass = 0; pass < 10; pass++) {
		const uint16_t maxchunk = asic_mode ? 16 : 1; /* On FPGA all these rams are reduced in size */
		uint32_t i, chunk;

		dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, NC_ATT_IO);
		for (i = 0; i < 256; i++)
			dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4, i);

		for (i = 0; i < 256; i++) {
			val = dnc_read_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4);
			assertf(val == i, "MMIO64 address map readback failed; have 0x%x, expected 0x%x", val, i);
			dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4, 0);
		}

		/* MMIO32 ATT has a slightly different layout on FPGA, so skip it for now */
		if (asic_mode) {
			for (chunk = 0; chunk < maxchunk; chunk++) {
				dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, NC_ATT_MMIO32 | chunk);
				for (i = 0; i < 256; i++)
					dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4, (chunk * 256) + i);
			}

			for (chunk = 0; chunk < maxchunk; chunk++) {
				dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, NC_ATT_MMIO32 | chunk);

				for (i = 0; i < 256; i++) {
					val = dnc_read_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4);
					uint32_t want = (chunk * 256) + i;
					assertf(val == want, "MMIO32 address map readback failed; have 0x%x, expected 0x%x", val, want);
					dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4, 0);
				}
			}
		}

		for (chunk = 0; chunk < maxchunk; chunk++) {
			dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, NC_ATT_APIC | chunk);
			for (i = 0; i < 256; i++)
				dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4, (chunk * 256) + i);
		}

		for (chunk = 0; chunk < maxchunk; chunk++) {
			dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, NC_ATT_APIC | chunk);

			for (i = 0; i < 256; i++) {
				val = dnc_read_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4);
				uint32_t want = (chunk * 256) + i;
				assertf(val == want, "APIC address map readback failed; have 0x%x, expected 0x%x", val, want);
				dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4, 0);
			}
		}

		for (chunk = 0; chunk < maxchunk; chunk++) {
			dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, NC_ATT_INT_NODE | chunk);
			for (i = 0; i < 256; i++)
				dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4, (chunk * 256) + i);
		}

		for (chunk = 0; chunk < maxchunk; chunk++) {
			dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, NC_ATT_INT_NODE | chunk);

			for (i = 0; i < 256; i++) {
				val = dnc_read_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4);
				uint32_t want = (chunk * 256) + i;
				assertf(val == want, "Interrupt address map readback failed; have 0x%x, expected 0x%x", val, want);
				dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4, 0);
			}
		}

		/* Test SCC routing tables, no readback verify */
		for (chunk = 0; chunk < maxchunk; chunk++) {
			dnc_write_csr(0xfff0, H2S_CSR_G0_ROUT_TABLE_CHUNK, chunk);

			for (i = 0; i < 16; i++) {
				dnc_write_csr(0xfff0, H2S_CSR_G0_ROUT_BXTBLL00 + i * 4, 0);
				dnc_write_csr(0xfff0, H2S_CSR_G0_ROUT_BLTBL00  + i * 4, 0);
				dnc_write_csr(0xfff0, H2S_CSR_G0_ROUT_BXTBLH00 + i * 4, 0);
			}
		}
	}
	printf("done\n");

	if (asic_mode && _is_pic_present(p_type)) {
again:
		printf("Testing fabric phys...");
		/* Trigger a HSS PLL reset */
		_pic_reset_ctrl();

		for (int phy = 0; phy < 6; phy++) {
			/* 1. Check that the HSS PLL has locked */
			val = dnc_read_csr(0xfff0, H2S_CSR_G0_HSSXA_STAT_1 + 0x40 * phy);
			if (!(val & 0x100)) {
				warning("Phy %d PLL failed to lock with status 0x%08x...retrying",
					phy, dnc_read_csr(0xfff0, H2S_CSR_G0_HSSXA_STAT_1 + 0x40 * phy));
				goto again;
			}

			/* 2. Activate TXLBENABLEx (bit[3:0] of HSSxx_CTR_8) */
			dnc_write_csr(0xfff0, H2S_CSR_G0_HSSXA_CTR_8 + 0x40 * phy, 0x000f);
			/* 3. Allow 6500 bit times (2ns per bit = 13us, use 100 to be safe) */
			udelay(100);
			/* 4. Pulse TXLBRESETx to clear TXLBERRORx (bit[7:4] of HSSxx_CTR_8) */
			dnc_write_csr(0xfff0, H2S_CSR_G0_HSSXA_CTR_8 + 0x40 * phy, 0x00ff);
			dnc_write_csr(0xfff0, H2S_CSR_G0_HSSXA_CTR_8 + 0x40 * phy, 0x000f);
			/* 5. Wait 8 bit times */
			udelay(10);
			/* 6. Monitor the TXLBERRORx flag (bit[7:4] of HSSxx_STAT_1) */
			val = dnc_read_csr(0xfff0, H2S_CSR_G0_HSSXA_STAT_1 + 0x40 * phy);
			if (val & 0xf0) {
				warning("Transmit data mismatch on phy %d with status 0x%08x...retrying",
					phy, dnc_read_csr(0xfff0, H2S_CSR_G0_HSSXA_STAT_1 + 0x40 * phy));
				goto again;
			}
		}

		/* Run test for 200msec */
		udelay(200000);

		for (int phy = 0; phy < 6; phy++) {
			val = dnc_read_csr(0xfff0, H2S_CSR_G0_HSSXA_STAT_1 + 0x40 * phy);
			if (val & 0xf0) {
				warning("Transmit test failed on phy error 3 on phy %d with status 0x%08x...retrying",
					phy, dnc_read_csr(0xfff0, H2S_CSR_G0_HSSXA_STAT_1 + 0x40 * phy));
				goto again;
			}
		}

		for (int phy = 0; phy < 6; phy++) {
			/* 3. Activate FDDATAWRAPx (bit[3:0] of HSSxx_CTR_1) */
			val = dnc_read_csr(0xfff0, H2S_CSR_G0_HSSXA_CTR_1 + 0x40 * phy);
			dnc_write_csr(0xfff0, H2S_CSR_G0_HSSXA_CTR_1 + 0x40 * phy, val | 0xf);
			/* 2. Activate RXLBENABLEx (bit[3:0] of HSSxx_CTR_7) */
			dnc_write_csr(0xfff0, H2S_CSR_G0_HSSXA_CTR_7 + 0x40 * phy, 0x000f);
			/* 3. Allow 6500 bit times (2ns per bit = 13us, use 100 to be safe) */
			udelay(100);
			/* 4. Pulse RXLBRESETx to clear RXLBERRORx (bit[7:4] of HSSxx_CTR_7) */
			dnc_write_csr(0xfff0, H2S_CSR_G0_HSSXA_CTR_7 + 0x40 * phy, 0x00ff);
			dnc_write_csr(0xfff0, H2S_CSR_G0_HSSXA_CTR_7 + 0x40 * phy, 0x000f);
			/* 5. Wait 8 bit times */
			udelay(10);
			/* 6. Monitor the RXLBERRORx flag (bit[3:0] of HSSxx_STAT_1) */
			val = dnc_read_csr(0xfff0, H2S_CSR_G0_HSSXA_STAT_1 + 0x40 * phy);
			if (val & 0xff) {
				warning("Receive data mismatch on phy %d with status 0x%08x...retrying",
					phy, dnc_read_csr(0xfff0, H2S_CSR_G0_HSSXA_STAT_1 + 0x40 * phy));
				goto again;
			}
		}

		/* Run test for 200msec */
		udelay(200000);

		for (int phy = 0; phy < 6; phy++) {
			val = dnc_read_csr(0xfff0, H2S_CSR_G0_HSSXA_STAT_1 + 0x40 * phy);
			if (val & 0xff) {
				warning("Receive test failed on phy %d with status 0x%08x...retrying",
					phy, dnc_read_csr(0xfff0, H2S_CSR_G0_HSSXA_STAT_1 + 0x40 * phy));
				goto again;
			}

			/* Stop BIST test */
			dnc_write_csr(0xfff0, H2S_CSR_G0_HSSXA_CTR_7 + 0x40 * phy, 0x00f0);
			dnc_write_csr(0xfff0, H2S_CSR_G0_HSSXA_CTR_7 + 0x40 * phy, 0x0000);
			val = dnc_read_csr(0xfff0, H2S_CSR_G0_HSSXA_CTR_1 + 0x40 * phy);
			dnc_write_csr(0xfff0, H2S_CSR_G0_HSSXA_CTR_1 + 0x40 * phy, val & ~(0xf));
			dnc_write_csr(0xfff0, H2S_CSR_G0_HSSXA_CTR_8 + 0x40 * phy, 0x00f0);
			dnc_write_csr(0xfff0, H2S_CSR_G0_HSSXA_CTR_8 + 0x40 * phy, 0x0000);
		}

		/* Trigger a HSS PLL reset */
		_pic_reset_ctrl();
	}
	printf("done\n");
}

/* List of the documented MSRs in the fam10h BKDG */
/* MSRs which cause hanging when read are commented with 'H' */
/* MSRs which are expected to vary over cores are commended with 'V' */
static const uint32_t msrs_f10[] = {
	0x00000000, 0x00000001, /* V 0x00000010, V 0x0000001b,*/ 0x0000002a, 0x0000008b, /* H 0x000000e7, H 0x000000e8,*/
	0x000000fe, 0x00000174, 0x00000175, 0x00000176, 0x00000179, 0x0000017a, 0x0000017b, 0x000001d9,
	/* V 0x000001db, V 0x000001dc,*/ 0x000001dd, 0x000001de, 0x00000200, 0x00000201, 0x00000202, 0x00000203,
	0x00000204, 0x00000205, 0x00000206, 0x00000207, 0x00000208, 0x00000209, 0x0000020a, 0x0000020b,
	0x0000020c, 0x0000020d, 0x0000020e, 0x0000020f, 0x00000250, 0x00000258, 0x00000259, 0x00000268,
	0x00000269, 0x0000026a, 0x0000026b, 0x0000026c, 0x0000026d, 0x0000026e, 0x0000026f, 0x00000277,
	0x000002ff, 0x00000400, 0x00000401, 0x00000402, 0x00000403, 0x00000404, 0x00000405, 0x00000406,
	0x00000407, 0x00000408, 0x00000409, 0x0000040a, 0x0000040b, 0x0000040c, 0x0000040d, 0x0000040e,
	0x0000040f, /* V 0x00000410,*/ 0x00000411, /* V 0x00000412 ,*/ 0x00000413, 0x00000414, 0x00000415, 0x00000416,
	0x00000417, 0xc0000080, 0xc0000081, 0xc0000082, 0xc0000083, 0xc0000084, 0xc0000100, 0xc0000101,
	0xc0000102, 0xc0000103, /* V 0xc0000408, 0xc0000409,*/ 0xc000040a, 0xc000040b, 0xc000040c, 0xc000040d,
	0xc000040e, 0xc000040f, /* V 0xc0010000, V 0xc0010001, V 0xc0010002, V 0xc0010003, V 0xc0010004, V 0xc0010005,*/
	/* V 0xc0010006, V 0xc0010007,*/ 0xc0010010, 0xc0010015, 0xc0010016, 0xc0010017, 0xc0010018, 0xc0010019,
	0xc001001a, 0xc001001d, 0xc001001f, /* H 0xc0010020,*/ 0xc0010022, /* V 0xc0010030, V 0xc0010031, V 0xc0010032,*/
	/* V 0xc0010033, V 0xc0010034, V 0xc0010035, V 0xc001003e,*/ 0xc0010044, 0xc0010045, 0xc0010046, 0xc0010047,
	0xc0010048, 0xc0010049, 0xc0010050, 0xc0010051, 0xc0010052, 0xc0010053, 0xc0010054, 0xc0010055,
	0xc0010056, /* V 0xc0010058,*/ 0xc0010059, 0xc001005a, 0xc001005b, 0xc001005c, 0xc001005d, 0xc0010060,
	0xc0010061, 0xc0010062, /* V 0xc0010063, V 0xc0010064, V 0xc0010065, V 0xc0010066, V 0xc0010067, V 0xc0010068, */
	/* V 0xc0010070, V 0xc0010071, H 0xc0010072, H 0xc0010073,*/ 0xc0010074, /* V 0xc0010111,*/ 0xc0010112, 0xc0010113,
	0xc0010114, 0xc0010115, /* H 0xc0010116,*/ 0xc0010117, 0xc0010118, 0xc0010119, 0xc001011a, 0xc0010140,
	0xc0010141, 0xc0011004, 0xc0011005, /* V 0xc001100c,*/ 0xc0011021, 0xc0011022, 0xc0011023, 0xc0011029,
	0xc001102a, 0xc0011030, /* V 0xc0011031, V 0xc0011032,*/ 0xc0011033, /* V 0xc0011034, V 0xc0011035,*/ 0xc0011036,
	0xc0011037, 0xc0011038, 0xc0011039, 0xc001103a,
};

static const uint32_t msrs_f15[] = {
	0x00000000,
};

static uint32_t get_msr(unsigned int index)
{
	const unsigned int f10_size = sizeof(msrs_f10) / sizeof(msrs_f10[0]);

	if (index < f10_size)
		return msrs_f10[index];

	if (family < 0x15)
		return 0xffffffff;

	const unsigned int f15_offset = index - f10_size;

	if (f15_offset < (sizeof(msrs_f15) / sizeof(msrs_f15[0])))
		return msrs_f15[f15_offset];

	return 0xffffffff;
}

static void dump_northbridge_regs(int ht_id)
{
	int ht, func;
	unsigned int offset;
	printf("Dumping AMD Northbridge registers...\n");

	for (ht = 0; ht < ht_id; ht++)
		for (func = 0; func < 6; func++) {
			printf("HT%d F%d:\n", ht, func);

			for (offset = 0; offset < 512; offset += 4) {
				if ((offset % 32) == 0)
					printf("%03x:", offset);

				printf(" %08x", cht_read_conf(ht, func, offset));

				if ((offset % 32) == 28)
					printf("\n");
			}
		}

	printf("Dumping MSRs...\n");
	uint32_t msr;

	for (offset = 0; (msr = get_msr(offset)) != 0xffffffff; offset++)
		printf("MSR 0x%08x: %016llx\n", msr, rdmsr(msr));
}

void selftest_late_msrs(void)
{
	unsigned int offset;
	uint32_t msr;
	printf("Checking MSRs for consistency...\n");

	for (offset = 0; (msr = get_msr(offset)) != 0xffffffff; offset++) {
		bool printed = false;
		uint64_t val0 = rdmsr(msr);

		for (int node = 0; node < dnc_node_count; node++) {
			for (int ht = nodes[node].nb_ht_lo; ht <= nodes[node].nb_ht_hi; ht++) {
				for (int i = 0; i < nodes[node].ht[ht].cores; i++) {
					if ((node == 0) && (ht == 0) && (i == 0))
						continue; /* Skip BSP */

					uint32_t oldid = nodes[node].ht[ht].apic_base + i;
					uint32_t apicid = nodes[node].apic_offset + oldid;
					/* Use limited local IPI distribution until global is fixed */
					if (apicid > 254)
						break;

					*REL32(msr_readback) = msr;
					wake_core_local(apicid, VECTOR_READBACK_MSR);
					if (*REL64(msr_readback) != val0) {
						if (!printed) {
							printf("MSR 0x%08x should be %016llx:", msr, val0);
							printed = true;
						}

						printf(" %d:%016llx", apicid, *REL64(msr_readback));
					}
				}
			}
		}

		if (printed)
			printf("\n");
	}
}

void selftest_late_apiclvt(void)
{
	const unsigned int apic_offsets[] = {0x320, 0x330, 0x340, 0x350, 0x360, 0x370, 0x500, 0x510, 0x520, 0x530};
	uint32_t val;

	uint64_t msr = rdmsr(MSR_APIC_BAR);
	volatile uint32_t *const apic = (volatile uint32_t *const)((uint32_t)msr & ~0xfff);

	printf("\nAdjusting APIC vectors:\n");
	for (const uint32_t *offset = &apic_offsets[0]; offset < (apic_offsets + sizeof(apic_offsets) / sizeof(apic_offsets[0])); offset++) {
		if (apic[*offset / 4] != APIC_VECTOR_MASKED) {
			printf("- masked core 0 APIC vector 0x%x (was 0x%08x)\n", *offset, apic[*offset / 4]);
			apic[*offset / 4] = APIC_VECTOR_MASKED;
		}
	}

	int n = 1;

	for (int node = 0; node < dnc_node_count; node++) {
		for (int ht = nodes[node].nb_ht_lo; ht <= nodes[node].nb_ht_hi; ht++) {
			for (int i = 0; i < nodes[node].ht[ht].cores; i++) {
				if ((node == 0) && (ht == 0) && (i == 0))
					continue; /* Skip BSP */

				uint32_t oldid = nodes[node].ht[ht].apic_base + i;
				uint32_t apicid = nodes[node].apic_offset + oldid;

				for (const uint32_t *offset = &apic_offsets[0]; offset < (apic_offsets + sizeof(apic_offsets) / sizeof(apic_offsets[0])); offset++) {
					/* Use limited local IPI distribution until global is fixed */
					if (apicid > 254)
						break;

					*REL32(apic_offset) = *offset;
					wake_core_local(apicid, VECTOR_READBACK_APIC);
					if (*REL32(apic_readback) != APIC_VECTOR_MASKED)
						printf("- masked core %d APIC vector 0x%x (was 0x%08x)\n", n, *offset, *REL32(apic_readback));
					n++;
				}
			}
		}
	}

	foreach_node(node) {
		foreach_nb(ht, node) {
			for (unsigned int reg = 0x160; reg <= 0x170; reg += 8) {
				val = dnc_read_conf(node->sci, 0, 24 + ht, 3, reg);
				if (val != 0xc0100000) {
					printf("- disabled thresholding F3x%X interrupt vector (was 0x%08x)\n", reg, val);
					dnc_write_conf(node->sci, 0, 24 + ht, 3, reg, 0xc0100000);
				}
			}

			val = dnc_read_conf(node->sci, 0, 24 + ht, 3, 0x1cc);
			if ((val & 0x10f) != 0x100) {
				printf("- disabled IBS vector (was 0x%08x)\n", val);
				val = (val & ~0x10f) | 0x100;
				dnc_write_conf(node->sci, 0, 24 + ht, 3, 0x1cc, val);
			}

			val = dnc_read_conf(node->sci, 0, 24 + ht, 3, 0x1d4);
			if (val & 0xfc00000) {
				printf("- disabled probe filter error interrupt (was 0x%08x)\n", val);
				val &= ~0xfc00000;
				dnc_write_conf(node->sci, 0, 24 + ht, 3, 0x1d4, val);
			}

			val = dnc_read_conf(node->sci, 0, 24 + ht, 3, 0x1e4);
			if ((val & 0xf00) != 0x300) {
				printf("- moved SBI to use default vector 3 (was 0x%08x)\n", val);
				val = (val & ~0xf00) | 0x300;
				dnc_write_conf(node->sci, 0, 24 + ht, 3, 0x1e4, val);
			}
		}
	}
	printf("\n");
}

struct smbios_header {
	uint8_t type;
	uint8_t length;
	uint16_t handle;
	uint8_t *data;
};

static const char *smbios_string(const char *table, uint8_t index) {
	while (--index)
		table += strlen(table) + 1;
	return table;
}

static void smbios_parse(const char *buf, const uint16_t len, const uint16_t num, const char **biosver, const char **biosdate, const char **manuf, const char **product) {
	const char *data = buf;
	int i = 0;
	bool use_system_info = 0;

again:
	while (i < num && data + 4 <= buf + len) {
		const char *next;
		struct smbios_header *h = (struct smbios_header *)data;

		assert(h->length >= 4);
		if (h->type == 127)
			break;

		next = data + h->length;

		if (h->type == 0) {
			*biosver = smbios_string(next, data[5]);
			*biosdate = smbios_string(next, data[8]);
		} else if (use_system_info && h->type == 1) {
			*manuf = smbios_string(next, data[4]);
			*product = smbios_string(next, data[5]);
		} else if (h->type == 2) {
			*manuf = smbios_string(next, data[4]);
			*product = smbios_string(next, data[5]);
		}

		while (next - buf + 1 < len && (next[0] != 0 || next[1] != 0))
			next++;
		next += 2;
		data = next;
		i++;
	}

	if ((!*manuf || !*product) && !use_system_info) {
		i = 0;
		use_system_info = 1;
		goto again;
	}
}

static void platform_quirks(void)
{
	const char *buf = (const char *)0xf0000;
	size_t fp;

	/* Search for signature */
	for (fp = 0; fp <= 0xfff0; fp += 16)
		if (!memcmp(buf + fp, "_SM_", 4))
			break;

	assertf(fp != 0x10000, "Failed to find SMBIOS signature");

	uint16_t len = *(uint16_t *)(buf + fp + 0x16);
	uint16_t num = *(uint16_t *)(buf + fp + 0x1c);
	fp = *(uint32_t *)(buf + fp + 0x18);

	const char *manuf = NULL, *product = NULL, *biosver = NULL, *biosdate = NULL;
	smbios_parse((const char *)fp, len, num, &biosver, &biosdate, &manuf, &product);
	assert(biosver);
	assert(biosdate);
	if (!manuf)
		manuf = "unknown manufacturer";
	if (!product)
		product = "unknown product";
	printf("Platform is %s %s with BIOS %s %s", manuf, product, biosver, biosdate);

	/* Skip if already set */
	if (handover_acpi) {
		printf("\n");
		return;
	}

	/* Systems where ACPI must be handed off early */
	const char *acpi_blacklist[] = {"H8QGL", NULL};

	for (unsigned int i = 0; i < (sizeof acpi_blacklist / sizeof acpi_blacklist[0]); i++) {
		if (!strcmp(product, acpi_blacklist[i])) {
			printf(" (blacklisted)");
			handover_acpi = 1;
			break;
		}
	}

	if (disable_kvm == -1) {
		/* BMC KVM USB port connected to SP5100 */
		const struct kvm_port kvm_blacklist[] = {{"OCTANS", 2}, };
		for (unsigned int i = 0; i < (sizeof kvm_blacklist / sizeof kvm_blacklist[0]); i++) {
			if (!strcmp(product, kvm_blacklist[i].name)) {
				disable_kvm = kvm_blacklist[i].port;
				printf(" (KVM port %d)", disable_kvm);
				break;
			}
		}
	}

	printf("\n");
}

int dnc_init_bootloader(uint32_t *p_chip_rev, char p_type[16], bool *p_asic_mode)
{
	uint32_t val, chip_rev;
	int i, ht_id;
	bool asic_mode;

	platform_quirks();
	detect_southbridge();

	/* SMI often assumes HT nodes are Northbridges, so handover early */
	if (handover_acpi)
		stop_acpi();

	ht_id = ht_fabric_fixup(&asic_mode, &chip_rev);

	/* Indicate immediate jump to next-label (-2) if init-only is also given */
	if (disable_nc && init_only)
		return -2;

	/* Indicate immediate jump to next-label (-2) if route_only issued */
	if (route_only)
		return -2;

	if (verbose > 2)
		dump_northbridge_regs(ht_id);

	/* ====================== START ERRATA WORKAROUNDS ====================== */

	if (asic_mode && (chip_rev == 0)) {
		/* ERRATA #N9: Reduce the number of HReq buffers to 2 (29:28), HPrb buffers to 2 (15:14)
		 * and SReq buffers to 4 (7:4), to take off some pressure on the response channel
		 * ERRATA #N20: Covered by the above, HReq uses different TransIDs (29:28) than HPrb (15:14) */
		val = dnc_read_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL);
		dnc_write_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL, (val & ~(0x1fUL)) | (1 << 4) | (1 << 3) | (1 << 2) | (1 << 0));
		dnc_write_csr(0xfff0, H2S_CSR_G3_HPRB_CTRL, 0x3fff0000);
		dnc_write_csr(0xfff0, H2S_CSR_G3_SREQ_CTRL, 0x000f0000);
	} else if (asic_mode && (chip_rev == 1)) {
		/* ERRATA #N20: Disable buffer 0-15 to ensure that HPrb and HReq have different
		 * transIDs (HPrb: 0-15, HReq: 16-30) */
		val = dnc_read_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL);
		dnc_write_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL, (val & ~(0x1fUL)) | (1 << 4));
	} else if (asic_mode && (chip_rev == 2) && workaround_hreq) {
		/* Unknown ERRATA: Disable buffer 0-15 to ease some pressure */
		val = dnc_read_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL);
		dnc_write_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL, (val & ~(0x1fUL)) | (1 << 4));
	}

	/* Set LockControl=0 to enable HT Lock functionality and avoid data corruption on split transactions.
	 * Set LockDelay=0 to minimize penalty with HT Locking.
	 * We can do this now since we've disabled HT Locking on non-split transactions, even for high contention
	 * cases (ref Errata #N28) */
	val = dnc_read_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL);
	dnc_write_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL, (val & ~((3 << 24) | (7 << 13))) | (3 << 24) | ((ht_lockdelay & 7) << 13));
	/* Since our microcode now supports remote owner state, we disable the
	 * error responses on shared probes to GONE cache lines. */
	val = dnc_read_csr(0xfff0, H2S_CSR_G3_HPRB_CTRL);
	dnc_write_csr(0xfff0, H2S_CSR_G3_HPRB_CTRL, val | (1 << 1)); /* disableErrorResponse=1 */

	if (asic_mode) {
		val = dnc_read_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL);
		/* Unknown ERRATA: Disable the Early CData read. It causes data corruption */
		val = (val & ~(3 << 6)) | (3 << 6);

		if (chip_rev < 2) {
			dnc_write_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL, val);
			/* Unknown ERRATA: Disable CTag cache */
			val = dnc_read_csr(0xfff0, H2S_CSR_G4_MCTAG_MAINTR);
			dnc_write_csr(0xfff0, H2S_CSR_G4_MCTAG_MAINTR, val & ~(1 << 2));
			/* Unknown ERRATA: Reduce the HPrb/FTag pipleline (M_PREF_MODE bit[7:4]) to avoid hang situations */
			val = dnc_read_csr(0xfff0, H2S_CSR_G2_M_PREF_MODE);
			dnc_write_csr(0xfff0, H2S_CSR_G2_M_PREF_MODE, (val & ~(0xf << 4)) | (1 << 4));
		} else { /* ASIC RevC */
			/* Enable WeakOrdering */
			val = val | (1 << 19);
			/* Disable Fast CTag lookup (conflicts with HReq buffer#31) */
			val = val | (1 << 5);
			/* Enable CData writeback on RdBlkMod,ChgToDirty and ValBlk */
			val = val | (7 << 8);
			dnc_write_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL, val);
		}
	} else { /* FPGA */
		val = dnc_read_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL);
		dnc_write_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL, val | (1 << 19)); /* Enable WeakOrdering */
	}

	if (asic_mode && (chip_rev < 2)) {
		/* ERRATA #N17: Disable SCC Context #0 to avoid false baPiuTHit assertions when buffer #0 is active.
		 * ERRATA #N23: Disable 8 SCC Contexts (0-7) to avoid SPrb filling up completely and thus asserting
		 * pmInhRemoteReq and causing the pre-grant of pmGrant to de-assert which causes issues when both
		 * BIU and MIU sends requests to an almost full SReq module.
		 * Also disable the MIB timer completely (bit6) for now (debugging purposes) */
		val = dnc_read_csr(0xfff0, H2S_CSR_G0_MIB_IBC);
		dnc_write_csr(0xfff0, H2S_CSR_G0_MIB_IBC, val | (0x00ff << 8) | (1 << 6));
	} else { /* ASIC Rev2 and FPGA */
		/* ERRATA #N37: On FPGA now we have an RTL fix in SCC buffers to throttle the amount of
		 * local memory requests to accept (bit[27:24] in MIB_IBC). SReq has 4 buffers available
		 * on FGPA, but cannot accept more than coherent requests total (1 reserved for
		 * non-coherent transactions). We need atleast 1 free buffer to receive the retried
		 * coherent requests to ensure forward progress on probes.
		 * Also disable the MIB timer completely (bit6) for now (debugging purposes) */
		val = dnc_read_csr(0xfff0, H2S_CSR_G0_MIB_IBC);
		dnc_write_csr(0xfff0, H2S_CSR_G0_MIB_IBC, val | (1 << 6));
	}

	for (i = 0; i < ht_id; i++) {
		/* Disable Northbridge WatchDog timer and MCE target/master abort for debugging */
		val = cht_read_conf(i, FUNC3_MISC, 0x44);

		if (enable_nbmce > -1)
			val = (val & ~(1 << 6)) | (!enable_nbmce << 6);

		/* Increase NB WDT timeout to 10s; has to be less than core WDT of 21s */
		val &= ~((3 << 12) | (7 << 9));

		if (enable_nbwdt > -1)
			val = (val & ~(1 << 8)) | (!enable_nbwdt << 8);

		cht_write_conf(i, FUNC3_MISC, 0x44, val);

		/* Update WDTCntSel[3] */
		val = cht_read_conf(i, FUNC3_MISC, 0x180);
		cht_write_conf(i, FUNC3_MISC, 0x180, val | (1 << 2));

		if (disable_c1e) {
			/* Disable C1E sleep mode in northbridge */
			val = cht_read_conf(i, FUNC3_MISC, 0xd4);

			if (val & (1 << 13)) {
				printf("Disabling C1E sleep state on HT#%d\n", i);
				cht_write_conf(i, FUNC3_MISC, 0xd4, val & ~(1 << 13));
			}
		}

#ifdef WORKAROUND_NOT_NEEDED
		/* InstallStateS to avoid exclusive state */
		val = cht_read_conf(i, FUNC0_HT, 0x68);
		cht_write_conf(i, FUNC0_HT, 0x68, val | (1 << 23));
#endif

		if (asic_mode && (chip_rev < 2)) {
			/* ERRATA #N26: Disable Write-bursting in the MCT to avoid a MCT "caching" effect on CPU writes (VicBlk)
			 * which have bad side-effects with NumaChip in certain scenarios */
			val = cht_read_conf(i, FUNC2_DRAM, 0x11c);
			cht_write_conf(i, FUNC2_DRAM, 0x11c, val | (0x1f << 2));
		}

		/* ERRATA #N27: Disable Coherent Prefetch Probes (Query probes), as NumaChip don't handle them correctly and they are required to be disabled for Probe Filter */
		val = cht_read_conf(i, FUNC2_DRAM, 0x1b0);
		cht_write_conf(i, FUNC2_DRAM, 0x1b0, val & ~(7 << 8)); /* CohPrefPrbLimit=000b */
		/* XXX: In case Traffic distribution is enabled on 2 socket systems, we
		 * need to disable it for Directed Probes. Ref email to AMD dated 4/28/2010 */
		val = cht_read_conf(i, FUNC0_HT, 0x164);
		cht_write_conf(i, FUNC0_HT, 0x164, val & ~0x1); /* Disable Traffic distribution for requests */

		/* On Fam15h disable the core prefetch hits as NumaChip doesn't support these */
		if (family >= 0x15) {
			val = cht_read_conf(i, FUNC5_EXTD, 0x88);

			if (!(val & (1 << 9))) {
				if (verbose > 0)
					printf("Setting DisHintInHtMskCnt for node %d\n", i);

				cht_write_conf(i, FUNC5_EXTD, 0x88, val | (1 << 9));
			}
		}

		/* Disable GARTs; Linux will reenable them on the master, so it's safer to disable
		 * them to prevent interpreting false entries from application memory */
		for (int reg = 0x90; reg <= 0x9c; reg += 4)
			cht_write_conf(i, FUNC3_MISC, reg, 0);

		/* Clear Machine Check address registers, since on some platforms they aren't initialised */
		for (int reg = 0x50; reg <= 0x54; reg += 4)
			cht_write_conf(i, FUNC3_MISC, reg, 0);
	}

	/* ====================== END ERRATA WORKAROUNDS ====================== */
	local_info->uuid = identify_eeprom(p_type);
	printf("UUID: %08X, TYPE: %s\n", local_info->uuid, p_type);

	/* Read the SPD info from our DIMMs to see if they are supported */
	for (i = 0; i < 2; i++)
		read_spd_info(p_type, i, &dimms[i]);

	perform_selftest(asic_mode, p_type);

	/* If init-only parameter is given, stop here and return */
	if (init_only)
		return -2;

	*p_asic_mode = asic_mode;
	*p_chip_rev = chip_rev;
	return ht_id;
}

static void save_scc_routing(uint16_t rtbll[], uint16_t rtblm[], uint16_t rtblh[])
{
	uint16_t chunk, offs;
	uint16_t maxchunk = dnc_asic_mode ? 16 : 1;
	printf("Setting routing table on SCC...");

	for (chunk = 0; chunk < maxchunk; chunk++) {
		dnc_write_csr(0xfff0, H2S_CSR_G0_ROUT_TABLE_CHUNK, chunk);

		for (offs = 0; offs < 16; offs++) {
			/* SCC */
			dnc_write_csr(0xfff0, H2S_CSR_G0_ROUT_BXTBLL00 + (offs << 2), rtbll[(chunk << 4) + offs]);
			dnc_write_csr(0xfff0, H2S_CSR_G0_ROUT_BLTBL00  + (offs << 2), rtblm[(chunk << 4) + offs]);
			dnc_write_csr(0xfff0, H2S_CSR_G0_ROUT_BXTBLH00 + (offs << 2), rtblh[(chunk << 4) + offs]);
		}
	}

	printf("\n");
}

static uint16_t shadow_rtbll[7][256];
static uint16_t shadow_rtblm[7][256];
static uint16_t shadow_rtblh[7][256];
static uint16_t shadow_ltbl[7][256];

/* Add route on "bxbarid" towards "dest" over "link" */
static void _add_route(uint16_t dest, uint8_t bxbarid, uint8_t link)
{
	uint16_t offs = (dest >> 4) & 0xff;
	uint16_t mask = 1 << (dest & 0xf);

	if (bxbarid > 0) shadow_ltbl[bxbarid][offs] |= mask;

	shadow_rtbll[bxbarid][offs] |= ((link & 1) ? mask : 0);
	shadow_rtblm[bxbarid][offs] |= ((link & 2) ? mask : 0);
	shadow_rtblh[bxbarid][offs] |= ((link & 4) ? mask : 0);
	/*    printf("add_route: ltbl[%d][%02x] = %04x\n", bxbarid, offs, shadow_ltbl[bxbarid][offs]); */
	/*    printf("add_route: rtbll[%d][%02x] = %04x\n", bxbarid, offs, shadow_rtbll[bxbarid][offs]); */
	/*    printf("add_route: rtblm[%d][%02x] = %04x\n", bxbarid, offs, shadow_rtblm[bxbarid][offs]); */
	/*    printf("add_route: rtblh[%d][%02x] = %04x\n", bxbarid, offs, shadow_rtblh[bxbarid][offs]); */
}

#ifdef UNUSED
static void test_route(uint8_t bxbarid, uint16_t dest)
{
	uint16_t offs = (dest >> 4) & 0xff;
	uint16_t mask = 1 << (dest & 0xf);
	uint8_t out = 0;
	printf("Testing route on bxbarid %d to target ID %04x (offs=%02x, mask=%04x)\n", bxbarid, dest, offs, mask);

	/*    printf("ltbl[%d][%02x] = %04x\n", bxbarid, offs, shadow_ltbl[bxbarid][offs]); */
	/*    printf("rtbll[%d][%02x] = %04x\n", bxbarid, offs, shadow_rtbll[bxbarid][offs]); */
	/*    printf("rtblm[%d][%02x] = %04x\n", bxbarid, offs, shadow_rtblm[bxbarid][offs]); */
	/*    printf("rtblh[%d][%02x] = %04x\n", bxbarid, offs, shadow_rtblh[bxbarid][offs]); */
	if (bxbarid > 0 && (shadow_ltbl[bxbarid][offs] & mask)) {
		printf("bxbarid %d will pick up packet\n", bxbarid);
		out += ((shadow_rtbll[bxbarid][offs] & mask) >> (dest & 0xf)) * 1;
		out += ((shadow_rtblm[bxbarid][offs] & mask) >> (dest & 0xf)) * 2;
		out += ((shadow_rtblh[bxbarid][offs] & mask) >> (dest & 0xf)) * 4;
		printf("Packet will be routed to bxbarid %d\n", out);
	} else if (bxbarid == 0) {
		out += ((shadow_rtbll[bxbarid][offs] & mask) >> (dest & 0xf)) * 1;
		out += ((shadow_rtblm[bxbarid][offs] & mask) >> (dest & 0xf)) * 2;
		out += ((shadow_rtblh[bxbarid][offs] & mask) >> (dest & 0xf)) * 4;
		printf("Packet will be routed to bxbarid %d\n", out);
	}
}
#endif

static bool _verify_save_id(uint16_t nodeid, int lc)
{
	const char *linkname = _get_linkname(lc);
	uint16_t expected_id = (nodeid | ((lc + 1) << 13));
	uint32_t val;

	if (dnc_raw_read_csr(0xfff1 + lc, LC3_CSR_SAVE_ID, &val) != 0)
		return 1;

	if (val != expected_id) {
		printf("LC3%s SAVE_ID (%04x) doesn't match expected value (%04x)\n",
		       linkname, val, expected_id);
		return 1;
	}

	return 0;
}

static bool _check_dim(const int dim)
{
	const uint16_t linkida = 2 * dim + 0;
	const uint16_t linkidb = 2 * dim + 1;

	bool err = dnc_check_phy(linkida) | dnc_check_phy(linkidb);
	if (err) {
		warning("Errors detected on PHY%s and PHY%s; resetting",
		       _get_linkname(linkida), _get_linkname(linkidb));
		/* Counter-rotating rings, reset both phys */
		dnc_reset_phy(linkida);
		dnc_reset_phy(linkidb);
		udelay(1000);
		err = dnc_check_phy(linkida) | dnc_check_phy(linkidb);
		if (err) {
			warning("Errors still present on PHY%s and PHY%s",
			       _get_linkname(linkida), _get_linkname(linkidb));
			return 1;
		}
	}

	err = dnc_check_lc3(linkida) | dnc_check_lc3(linkidb);
	if (err) {
		warning("LC3 errors detected on LC3%s and LC3%s; resetting",
		       _get_linkname(linkida), _get_linkname(linkidb));
		/* Counter-rotating rings, reset both phys */
		dnc_reset_phy(linkida);
		dnc_reset_phy(linkidb);
		udelay(1000);
		err = dnc_check_phy(linkida) | dnc_check_phy(linkidb);
		err |= dnc_check_lc3(linkida) | dnc_check_lc3(linkidb);
		if (err) {
			error("LC3 reset not successful");
			return 1;
		}
	}

	return err;
}

static uint8_t route1(sci_t src, sci_t dst) {
	uint8_t dim = 0;

	while ((src ^ dst) & ~0xf) {
		dim++;
		src >>= 4;
		dst >>= 4;
	}

	int out = dim * 2 + 1;
	out += ((dst & 0xf) + ((dst >> 4) & 0xf) + ((dst >> 8) & 0xf) +
		(src & 0xf) + ((src >> 4) & 0xf) + ((src >> 8) & 0xf)) & 1; /* Load balance */
	return out;
}

static enum node_state setup_fabric(struct node_info *info)
{
	int i;
	uint8_t lc;
	dnc_write_csr(0xfff0, H2S_CSR_G0_NODE_IDS, info->sci << 16);
	memset(shadow_ltbl, 0, sizeof(shadow_ltbl));
	memset(shadow_rtbll, 0, sizeof(shadow_rtbll));
	memset(shadow_rtblm, 0, sizeof(shadow_rtblm));
	memset(shadow_rtblh, 0, sizeof(shadow_rtblh));

	if (cfg_fabric.x_size > 0)
		_add_route(info->sci, 0, 1); /* Self route via LC3XA */
	else if (cfg_fabric.y_size > 0)
		_add_route(info->sci, 0, 3); /* Self route via LC3YA */
	else
		_add_route(info->sci, 0, 5); /* Self route via LC3ZA */

	/* Make sure responses get back to SCC */
	for (lc = 1; lc <= 6; lc++)
		_add_route(info->sci, lc, 0);

	for (i = 0; i < cfg_nodes; i++) {
		if (info->sci == cfg_nodelist[i].sci)
			continue;

		uint8_t out = route1(info->sci, cfg_nodelist[i].sci);
		_add_route(cfg_nodelist[i].sci, 0, out);

		for (lc = 1; lc <= 6; lc++) {
			/* Don't touch packets already on correct dim */
			if ((lc - 1) / 2 != (out - 1) / 2)
				_add_route(cfg_nodelist[i].sci, lc, out);
		}
	}

	save_scc_routing(shadow_rtbll[0], shadow_rtblm[0], shadow_rtblh[0]);
	/* Make sure all necessary links are up and working */
	int res = 1;
	printf("Initialising LC3s:");

	if (cfg_fabric.x_size > 0) {
		if (_check_dim(0))
			return RSP_FABRIC_NOT_READY;

		res = (0 == dnc_init_lc3(info->sci, 0, dnc_asic_mode ? 16 : 1,
		                         shadow_rtbll[1], shadow_rtblm[1],
		                         shadow_rtblh[1], shadow_ltbl[1])) && res;
		printf(" XA");
		res = (0 == dnc_init_lc3(info->sci, 1, dnc_asic_mode ? 16 : 1,
		                         shadow_rtbll[2], shadow_rtblm[2],
		                         shadow_rtblh[2], shadow_ltbl[2])) && res;
		printf(" XB");
	}

	if (cfg_fabric.y_size > 0) {
		if (_check_dim(1))
			return RSP_FABRIC_NOT_READY;

		res = (0 == dnc_init_lc3(info->sci, 2, dnc_asic_mode ? 16 : 1,
		                         shadow_rtbll[3], shadow_rtblm[3],
		                         shadow_rtblh[3], shadow_ltbl[3])) && res;
		printf(" YA");
		res = (0 == dnc_init_lc3(info->sci, 3, dnc_asic_mode ? 16 : 1,
		                         shadow_rtbll[4], shadow_rtblm[4],
		                         shadow_rtblh[4], shadow_ltbl[4])) && res;
		printf(" YB");
	}

	if (cfg_fabric.z_size > 0) {
		if (_check_dim(2))
			return RSP_FABRIC_NOT_READY;

		res = (0 == dnc_init_lc3(info->sci, 4, dnc_asic_mode ? 16 : 1,
		                         shadow_rtbll[5], shadow_rtblm[5],
		                         shadow_rtblh[5], shadow_ltbl[5])) && res;
		printf(" ZA");
		res = (0 == dnc_init_lc3(info->sci, 5, dnc_asic_mode ? 16 : 1,
		                         shadow_rtbll[6], shadow_rtblm[6],
		                         shadow_rtblh[6], shadow_ltbl[6])) && res;
		printf(" ZB");
	}

	printf("\n");
	return res ? RSP_FABRIC_READY : RSP_FABRIC_NOT_READY;
}

static enum node_state validate_fabric(struct node_info *info, struct part_info *part)
{
	bool res = 0;
	uint32_t val;

	if (dnc_check_fabric(info))
		return RSP_FABRIC_NOT_OK;

	/* Builder is checking that it can access all other nodes via CSR */
	if (part->builder == info->sci) {
		printf("Validating fabric");

		for (int iter = 0; res || iter < 10; iter++) {
			printf(".");

			for (int i = 1; i < cfg_nodes; i++) {
				uint16_t node = cfg_nodelist[i].sci;
				res |= dnc_raw_write_csr(node, H2S_CSR_G3_NC_ATT_MAP_SELECT, NC_ATT_APIC);

				for (int j = 0; res && j < 16; j++)
					res |= dnc_raw_read_csr(node, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + j * 4, &val);
			}

			udelay(1000);
		}

		printf("done\n");
	}

	return res ? RSP_FABRIC_NOT_OK : RSP_FABRIC_OK;
}

bool dnc_check_fabric(struct node_info *info)
{
	bool err = 0;

	if (cfg_fabric.x_size > 0) {
		if (_check_dim(0))
			return 1;

		if (_verify_save_id(info->sci, 0))
			err |= dnc_init_lc3(info->sci, 0, dnc_asic_mode ? 16 : 1,
			                         shadow_rtbll[1], shadow_rtblm[1],
			                         shadow_rtblh[1], shadow_ltbl[1]);

		if (_verify_save_id(info->sci, 1))
			err |= dnc_init_lc3(info->sci, 1, dnc_asic_mode ? 16 : 1,
			                         shadow_rtbll[2], shadow_rtblm[2],
			                         shadow_rtblh[2], shadow_ltbl[2]);
	}

	if (cfg_fabric.y_size > 0) {
		if (_check_dim(1))
			return 1;

		if (_verify_save_id(info->sci, 2))
			err |= dnc_init_lc3(info->sci, 2, dnc_asic_mode ? 16 : 1,
			                         shadow_rtbll[3], shadow_rtblm[3]
			                         , shadow_rtblh[3], shadow_ltbl[3]);

		if (_verify_save_id(info->sci, 3))
			err |= dnc_init_lc3(info->sci, 3, dnc_asic_mode ? 16 : 1,
			                         shadow_rtbll[4], shadow_rtblm[4],
			                         shadow_rtblh[4], shadow_ltbl[4]);
	}

	if (cfg_fabric.z_size > 0) {
		if (_check_dim(2))
			return 1;

		if (_verify_save_id(info->sci, 4))
			err |= dnc_init_lc3(info->sci, 4, dnc_asic_mode ? 16 : 1,
			                         shadow_rtbll[5], shadow_rtblm[5],
			                         shadow_rtblh[5], shadow_ltbl[5]);

		if (_verify_save_id(info->sci, 5))
			err |= dnc_init_lc3(info->sci, 5, dnc_asic_mode ? 16 : 1,
			                         shadow_rtbll[6], shadow_rtblm[6],
			                         shadow_rtblh[6], shadow_ltbl[6]);
	}

	return err;
}

static bool phy_check_status(const int phy, const bool print)
{
	uint32_t val = dnc_read_csr(0xfff0, H2S_CSR_G0_PHYXA_ELOG + 0x40 * phy);
	if (val & 0xf0) {
		if (print)
			warning("Issuing fabric %s reset due to ELOG 0x%x", _get_linkname(phy), val);

		/* Clock compensation error, try forced retraining */
		dnc_reset_phy(phy);
		return 1;
	}

	/* Other errors */
	if (val > 0) {
		if (print)
			warning("Fabric %s ELOG 0x%x", _get_linkname(phy), val);
		return 1;
	}

	val = dnc_read_csr(0xfff0, H2S_CSR_G0_PHYXA_LINK_STAT + 0x40 * phy);
	if (val != 0x1fff) {
		if (print)
			warning("Fabric %s link status 0x%x", _get_linkname(phy), val);
		return 1;
	}

	return 0;
}

static enum node_state reset_fabric(struct node_info *info __attribute__((unused)))
{
	printf("Resetting fabric...");
	_pic_reset_ctrl();
	printf("done\n");

	return RSP_RESET_COMPLETE;
}

static enum node_state train_fabric(struct node_info *info __attribute__((unused)))
{
	printf("Training fabric phys...");

	/* No external reset control, simply reset all phys to start training sequence */
	for (int phy = 0; phy < 6; phy++)
		dnc_reset_phy(phy);

	int last = 8;

	/* Verify that all relevant PHYs are training */
	for (int i = 0; i <= last; i++) {
		udelay(500);
		bool pending = 0;

		if (cfg_fabric.x_size > 0) {
			pending |= phy_check_status(0, i == last);
			pending |= phy_check_status(1, i == last);
		}

		if (cfg_fabric.y_size > 0) {
			pending |= phy_check_status(2, i == last);
			pending |= phy_check_status(3, i == last);
		}

		if (cfg_fabric.z_size > 0) {
			pending |= phy_check_status(4, i == last);
			pending |= phy_check_status(5, i == last);
		}

		if (!pending) {
			printf("done\n");
			return RSP_PHY_TRAINED;
		}
	}

	return RSP_PHY_NOT_TRAINED;
}

static bool lc_check_status(int lc, int dimidx)
{
	if (dnc_check_lc3(lc) == 0)
		return 0;

	udelay(200);

	/* Only initiate resets from one of the ring nodes */
	if (dimidx == 0) {
		printf("Initiating reset on LC%s\n", _get_linkname(lc));
		dnc_reset_lc3(lc);
	}

	return 1;
}

static enum node_state validate_rings(struct node_info *info)
{
	int pending, i;
	printf("Validating rings\n");
	i = 0;

	while (1) {
		pending = 0;

		if (cfg_fabric.x_size > 0) {
			pending += lc_check_status(0, info->sci & 0x00f);
			pending += lc_check_status(1, info->sci & 0x00f);
		}

		if (cfg_fabric.y_size > 0) {
			pending += lc_check_status(2, info->sci & 0x0f0);
			pending += lc_check_status(3, info->sci & 0x0f0);
		}

		if (cfg_fabric.z_size > 0) {
			pending += lc_check_status(4, info->sci & 0xf00);
			pending += lc_check_status(5, info->sci & 0xf00);
		}

		if (!pending)
			return RSP_RINGS_OK;

		if (i++ > 1000)
			return RSP_RINGS_NOT_OK;

		udelay(200);
	}
}

int handle_command(enum node_state cstate, enum node_state *rstate,
                   struct node_info *info,
                   struct part_info *part)
{
	switch (cstate) {
	case CMD_RESET_FABRIC:
		*rstate = reset_fabric(info);
		return 1;
	case CMD_TRAIN_FABRIC:
		*rstate = train_fabric(info);
		return 1;
	case CMD_VALIDATE_RINGS:
		*rstate = validate_rings(info);
		return 1;
	case CMD_SETUP_FABRIC:
		*rstate = setup_fabric(info);
		return 1;
	case CMD_VALIDATE_FABRIC:
		*rstate = validate_fabric(info, part);
		return 1;
	default:
		return 0;
	}
}

void broadcast_error(const bool persistent, const char *format, ...)
{
	char buf[UDP_MAXLEN];
	struct state_bcast *rsp = (struct state_bcast *)buf;
	char *msg = buf + sizeof(struct state_bcast);

	va_list args;
	va_start(args, format);
	vsnprintf(msg, sizeof buf - sizeof(struct state_bcast), format, args);
	va_end(args);

	int len = sizeof(struct state_bcast) + strlen(msg) + 1;

	rsp->sig = UDP_SIG;
	rsp->state = RSP_ERROR;
	rsp->uuid = local_info->uuid;
	rsp->sci = local_info->sci;
	rsp->tid = 0;

	do {
		udp_broadcast_state(rsp, len);
		udelay(5000000);
	} while (persistent);
}

void check_error(void)
{
	char buf[UDP_MAXLEN];
	uint32_t ip;
	size_t len = udp_read_state(buf, sizeof buf, &ip);
	if (!len)
		return;

	struct state_bcast *rsp = (struct state_bcast *)buf;
	if (len < sizeof(struct state_bcast) || rsp->sig != UDP_SIG)
		return;

	if (rsp->state != RSP_ERROR)
		return;

	for (int i = 0; i < cfg_nodes; i++) {
		if ((!name_matching && (cfg_nodelist[i].uuid == rsp->uuid)) ||
		    ( name_matching && (cfg_nodelist[i].sci == rsp->sci))) {
			error_remote(rsp->sci, cfg_nodelist[i].desc, ip, buf + sizeof(struct state_bcast));
			return;
		}
	}
}

void wait_for_master(struct node_info *info, struct part_info *part)
{
	struct state_bcast rsp, cmd;
	int count, backoff, i;
	int go_ahead = 0;
	uint32_t last_cmd = ~0;
	uint32_t ip, builduuid = ~0;

	udp_open();

	rsp.sig = UDP_SIG;
	rsp.state = RSP_SLAVE_READY;
	rsp.uuid  = info->uuid;
	rsp.sci = info->sci;
	rsp.tid   = 0;

	for (i = 0; i < cfg_nodes; i++)
		if (cfg_nodelist[i].sci == part->builder)
			builduuid = cfg_nodelist[i].uuid;

	count = 0;
	backoff = 1;

	while (!go_ahead) {
		if (++count >= backoff) {
			if (name_matching)
				printf("Broadcasting state: %s (sciid 0x%03x, tid %d)\n",
				       node_state_name[rsp.state], rsp.sci, rsp.tid);
			else
				printf("Broadcasting state: %s (sciid 0x%03x, uuid %08X, tid %d)\n",
				       node_state_name[rsp.state], rsp.sci, rsp.uuid, rsp.tid);
			udp_broadcast_state(&rsp, sizeof(rsp));
			udelay(100 * backoff);

			if (backoff < 32)
				backoff = backoff * 2;

			count = 0;
		}

		int len;

		/* In order to avoid jamming, broadcast own status at least
		 * once every 2*cfg_nodes packet seen */
		for (i = 0; i < 2 * cfg_nodes; i++) {
			len = udp_read_state(&cmd, sizeof(cmd), &ip);

			if (!len)
				break;

			if (len != sizeof(cmd) || cmd.sig != UDP_SIG)
				continue;

			/* printf("Got cmd packet (state %d, sciid %03x, uuid %08X, tid %d)\n",
			 *       cmd.state, cmd.sci, cmd.uuid, cmd.tid); */
			if ((!name_matching && (cmd.uuid == builduuid)) ||
			    ( name_matching && (cmd.sci == part->builder))) {
				if (cmd.tid == last_cmd) {
					/* Ignoring seen command */
					continue;
				}

				last_cmd = cmd.tid;
				count = 0;
				backoff = 1;

				if (handle_command(cmd.state, &rsp.state, info, part)) {
					rsp.tid = cmd.tid;
				} else if (cmd.state == CMD_CONTINUE) {
					printf("Master signalled go-ahead\n");
					/* Belt and suspenders: slaves re-broadcast go-ahead command */
					udp_broadcast_state(&cmd, sizeof(cmd));
					go_ahead = 1;
					break;
				}
			}
		}
	}
}

