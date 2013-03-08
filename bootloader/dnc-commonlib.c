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
#include <inttypes.h>

#include "dnc-regs.h"
#include "dnc-defs.h"
#include "dnc-access.h"
#include "dnc-fabric.h"
#include "dnc-config.h"
#include "dnc-bootloader.h"
#include "dnc-commonlib.h"
#include "hw-config.h"

IMPORT_RELOCATED(cpu_status);
IMPORT_RELOCATED(init_dispatch);
IMPORT_RELOCATED(msr_readback);

char *config_file_name = "nc-config/fabric.json";
char *next_label = "menu.c32";
char *microcode_path = "";
static bool init_only = 0;
static bool route_only = 0;
static int enable_nbmce = -1;
static int enable_nbwdt = 0;
static bool disable_sram = 0;
int force_probefilteroff = 0;
int force_probefilteron = 0;
static int ht_force_ganged = 1;
bool disable_smm = 0;
bool disable_c1e = 0;
int renumber_bsp = -1;
int remote_io = 0;
bool boot_wait = false;
int forwarding_mode = 3; /* 0=store-and-forward, 1-2=intermediate, 3=full cut-through */
int sync_interval = 1; /* bit[8]=disable prescaler, bit[7:0] sync_interval value */
bool enable_relfreq = 0;
bool singleton = 0;
static bool ht_200mhz_only = 0;
static bool ht_8bit_only = 0;
static int ht_suppress = 0;
static int ht_lockdelay = 0;
bool handover_acpi;
bool mem_offline = 0;
uint64_t trace_buf = 0;
uint32_t trace_buf_size = 0;
int verbose = 0;
int family = 0;
uint32_t tsc_mhz = 0;
uint32_t pf_maxmem = 0;
bool pf_vga_local = 0;
uint32_t max_mem_per_node;
static int dimmtest = 0;
static bool workaround_hreq = 1;
static bool workaround_rtt = 0;
uint64_t mem_gap = 0;

const char *node_state_name[] = { NODE_SYNC_STATES(ENUM_NAMES) };

int nc_neigh = -1, nc_neigh_link = -1;
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
		snprintf(strs[index], 8, "%" PRIu64 "%cB", val, suffix[offset]);
	else
		snprintf(strs[index], 8, "%" PRIu64 "B", val);

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

static int read_spd_info(char p_type[16], int cdata, struct dimm_config *dimm)
{
	uint16_t spd_addr;
	uint32_t reg;
	uint8_t addr_bits;
	uint8_t mdata[20];

	/* On N313025 and N323024, the SPD addresses are reversed */
	if ((strncmp("N313025", p_type, 7) == 0) ||
	    (strncmp("N323024", p_type, 7) == 0))
		spd_addr = cdata ? 0 : 1;
	else
		spd_addr = cdata ? 1 : 0;

	reg = dnc_read_csr(0xfff0, (1 << 12) + (spd_addr << 8) +  0); /* Read SPD location 0, 1, 2, 3 */

	if (((reg >> 8) & 0xff) != 0x08) {
		printf("Error: Couldn't find a DDR2 SDRAM memory module attached to the %s memory controller\n",
		       cdata ? "CData" : "MCTag");
		return -1;
	}

	dimm->addr_pins = 16 - (reg & 0xf); /* Number of Row address bits (max 16) */
	addr_bits = (reg & 0xf);
	reg = dnc_read_csr(0xfff0, (1 << 12) + (spd_addr << 8) +  4); /* Read SPD location 4, 5, 6, 7 */
	dimm->column_size = 13 - ((reg >> 24) & 0xf); /* Number of Column address bits (max 13) */
	dimm->cs_map = ((reg >> 16) & 1) ? 3 : 1; /* Single or Dual rank */
	addr_bits = addr_bits + ((reg >> 24) & 0xf) + ((reg >> 16) & 1);
	reg = dnc_read_csr(0xfff0, (1 << 12) + (spd_addr << 8) +  8); /* Read SPD location 8, 9, 10, 11 */

	if (!(reg & 2)) {
		printf("Error: Unsupported non-ECC %s DIMM\n", cdata ? "CData" : "MCTag");
		return -1;
	}

	reg = dnc_read_csr(0xfff0, (1 << 12) + (spd_addr << 8) + 12); /* Read SPD location 12, 13, 14, 15 */
	dimm->width = (reg >> 8) & 0xff;

	if ((dimm->width != 4) && (dimm->width != 8)) {
		printf("Error: Unsupported %s SDRAM width %d\n", cdata ? "CData" : "MCTag", dimm->width);
		return -1;
	}

	reg = dnc_read_csr(0xfff0, (1 << 12) + (spd_addr << 8) + 16); // Read SPD location 16, 17, 18, 19
	dimm->eight_bank = ((reg >> 16) & 0xff) == 8;
	addr_bits = addr_bits + (dimm->eight_bank ? 3 : 2);
	reg = dnc_read_csr(0xfff0, (1 << 12) + (spd_addr << 8) + 20); /* Read SPD location 20, 21, 22, 23 */

	if (!(reg & 0x11000000)) {
		printf("Error: Unsupported non-Registered %s DIMM\n", cdata ? "CData" : "MCTag");
		return -1;
	}

	uint32_t *mdataw = (uint32_t *)mdata;
	mdataw[0] = uint32_tbswap(dnc_read_csr(0xfff0, (1 << 12) + (spd_addr << 8) + 72)); /* Read SPD location 72, 73, 74, 75 */
	mdataw[1] = uint32_tbswap(dnc_read_csr(0xfff0, (1 << 12) + (spd_addr << 8) + 76)); /* Read SPD location 76, 77, 78, 79 */
	mdataw[2] = uint32_tbswap(dnc_read_csr(0xfff0, (1 << 12) + (spd_addr << 8) + 80)); /* Read SPD location 80, 81, 82, 83 */
	mdataw[3] = uint32_tbswap(dnc_read_csr(0xfff0, (1 << 12) + (spd_addr << 8) + 84)); /* Read SPD location 84, 85, 86, 87 */
	mdataw[4] = uint32_tbswap(dnc_read_csr(0xfff0, (1 << 12) + (spd_addr << 8) + 88)); /* Read SPD location 88, 89, 90, 91 */
	mdata[19] = 0;
	printf("%s is a x%d %dMB module (%s)\n", cdata ? "CData" : "MCTag", dimm->width, 1 << (addr_bits - 17), mdata[1] ? (char *)&mdata[1] : "unknown");

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
		dimm->mem_size = 0;
		printf("Error: Unsupported %s DIMM size of %dMB\n",
		       cdata ? "CData" : "MCTag", 1 << (addr_bits - 17));
		return -1;
	}

	return 0;
}

#include "../interface/mctr_define_register_C.h"
#include "../interface/regconfig_200_cl4_bl4_genericrdimm.h"

static void _denali_mctr_reset(int cdata, struct dimm_config *dimm)
{
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
	dnc_write_csr(0xfff0, mctrbase + (17 << 2), DENALI_CTL_17_DATA);
	dnc_write_csr(0xfff0, mctrbase + (18 << 2), DENALI_CTL_18_DATA);
	dnc_write_csr(0xfff0, mctrbase + (19 << 2),
	              (DENALI_CTL_19_DATA & ~(0x3 << 24)) | ((dimm->cs_map & 0x3) << 24));
	dnc_write_csr(0xfff0, mctrbase + (20 << 2), DENALI_CTL_20_DATA);
	dnc_write_csr(0xfff0, mctrbase + (21 << 2), DENALI_CTL_21_DATA);
	dnc_write_csr(0xfff0, mctrbase + (22 << 2), DENALI_CTL_22_DATA);
	dnc_write_csr(0xfff0, mctrbase + (23 << 2), DENALI_CTL_23_DATA);
	dnc_write_csr(0xfff0, mctrbase + (24 << 2), DENALI_CTL_24_DATA);
	dnc_write_csr(0xfff0, mctrbase + (25 << 2), DENALI_CTL_25_DATA);
	dnc_write_csr(0xfff0, mctrbase + (26 << 2), DENALI_CTL_26_DATA);
	dnc_write_csr(0xfff0, mctrbase + (28 << 2), DENALI_CTL_28_DATA);
	dnc_write_csr(0xfff0, mctrbase + (30 << 2), DENALI_CTL_30_DATA);
	dnc_write_csr(0xfff0, mctrbase + (31 << 2), DENALI_CTL_31_DATA);
	dnc_write_csr(0xfff0, mctrbase + (33 << 2), DENALI_CTL_33_DATA);
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

uint32_t dnc_check_mctr_status(int cdata)
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
		printf("Error: %s single access outside the defined Physical memory space detected\n", me);
		ack |= 0x001;
	}

	if (val & 0x002) {
		printf("Error: %s multiple access outside the defined Physical memory space detected\n", me);
		ack |= 0x002;
	}

#endif

	if (val & 0x004) {
		printf("Error: %s single correctable ECC event detected\n", me);
		ack |= 0x004;
	}

	if (val & 0x008) {
		printf("Error: %s multiple correctable ECC event detected\n", me);
		ack |= 0x008;
	}

	if (val & 0x010) {
		printf("Error: %s single uncorrectable ECC event detected\n", me);
		ack |= 0x010;
	}

	if (val & 0x020) {
		printf("Error: %s multiple uncorrectable ECC event detected\n", me);
		ack |= 0x020;
	}

	if (val & 0xf80) {
		printf("Error: %s error interrupts detected INT_STATUS=%03x\n", me, val & 0xfff);
		ack |= (val & 0xf80);
	}

	if (ack) {
		ack |= dnc_read_csr(0xfff0, mctrbase + (INT_ACK_ADDR << 2));
		dnc_write_csr(0xfff0, mctrbase + (INT_ACK_ADDR << 2), ack);
	}

	return val;
}

static int dnc_initialize_sram(void)
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

			if ((val & 0xc000000) != 0) {
				printf("SRAM clock calibration overflow detected (%08x)\n", val);
				return -1;
			}
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
	} else {
		printf("ASIC revision %d detected, no FTag SRAM\n", dnc_chip_rev);
	}

	return 0;
}

int dnc_init_caches(void)
{
	uint32_t val;
	int cdata, ret = 0;

	for (cdata = 0; cdata < 2; cdata++) {
		const char *name = cdata ? "CData" : "MCTag";
		int mem_size = dimms[cdata].mem_size;

		if (!dnc_asic_mode) {
			/* On FPGA, just check that the phy is initialized */
			val = dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_STATR : H2S_CSR_G4_MCTAG_COM_STATR);

			if (!(val & 0x1000)) {
				printf("%s controller not calibrated (%x)\n", name, val);
				return -1;
			}
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

				printf(" done\n");
			} else {
				/* Controller has already been started, just check for interrupts */
				(void)dnc_check_mctr_status(cdata);
			}
		}

		if ((dnc_asic_mode && dnc_chip_rev >= 2) || !dnc_asic_mode) {
			int tmp = mem_size;

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
				} else {
					printf("Error: Unsupported MCTag/CData combination (%d/%dGB)\n",
					       (1 << dimms[0].mem_size), (1 << dimms[1].mem_size));
					return -1;
				}
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
			if (cdata && mem_size == 0) {
				printf("Error: Unsupported CData size of %dGB\n",
				       (1 << mem_size));
				return -1;
			}

			if (!cdata) {
				max_mem_per_node = 16U << dimms[0].mem_size;
				printf("%dGB MCTag_Size  %dGB Remote Cache  %3dGB Max Coherent Local Memory\n",
				       (1 << dimms[0].mem_size), (1 << dimms[1].mem_size), max_mem_per_node);
				max_mem_per_node = max_mem_per_node << (30 - DRAM_MAP_SHIFT);
			}

			val = dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_CTRLR : H2S_CSR_G4_MCTAG_COM_CTRLR);
			dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_CTRLR : H2S_CSR_G4_MCTAG_COM_CTRLR,
			              (val & ~7) | (1 << 12) | (1 << 7) | ((mem_size & 7) << 4)); /* DRAM_AVAILABLE, FTagBig, MEMSIZE[2:0] */
		}

		/* Initialize DRAM */
		printf("Initializing %dGB %s...\n", 1 << mem_size, name);
		dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_SCRUBBER_ADDR : H2S_CSR_G4_MCTAG_SCRUBBER_ADDR, 0);
		dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_STATR : H2S_CSR_G4_MCTAG_COM_STATR, (1 << 1)); /* Clear INI_FINISHED flag */
		val = dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_MAINTR : H2S_CSR_G4_MCTAG_MAINTR);
		dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_MAINTR : H2S_CSR_G4_MCTAG_MAINTR, val | (1 << 4)); /* Set MEM_INI_ENABLE */

		do {
			udelay(100);
			val = dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_STATR : H2S_CSR_G4_MCTAG_COM_STATR);
		} while (!(val & (1 << 1))); /* Wait for INI_FINISHED flag */

		val = dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_MAINTR : H2S_CSR_G4_MCTAG_MAINTR);
		dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_MAINTR : H2S_CSR_G4_MCTAG_MAINTR, val & ~(1 << 4)); /* Clear MEM_INI_ENABLE */

		if (dimmtest != 0) {
			/* Do DRAM test */
			if (dnc_dimmtest(cdata, dimmtest, &dimms[cdata]) < 0)
				ret = -1;

			/* Check what the interrupt bits say, if we get ECC errors, correctable or not; Abort */
			val = dnc_check_mctr_status(cdata);
			if (val & 0x03c)
				ret = -1;

			/* AOK, re-initialize DRAM after test */
			printf("Re-initializing %dGB %s...\n", 1 << mem_size, name);
			dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_SCRUBBER_ADDR : H2S_CSR_G4_MCTAG_SCRUBBER_ADDR, 0);
			dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_STATR : H2S_CSR_G4_MCTAG_COM_STATR, (1 << 1)); /* Clear INI_FINISHED flag */
			val = dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_MAINTR : H2S_CSR_G4_MCTAG_MAINTR);
			dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_MAINTR : H2S_CSR_G4_MCTAG_MAINTR, val | (1 << 4)); /* Set MEM_INI_ENABLE */

			do {
				udelay(100);
				val = dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_STATR : H2S_CSR_G4_MCTAG_COM_STATR);
			} while (!(val & (1 << 1))); /* Wait for INI_FINISHED flag */

			val = dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_MAINTR : H2S_CSR_G4_MCTAG_MAINTR);
			dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_MAINTR : H2S_CSR_G4_MCTAG_MAINTR, val & ~(1 << 4)); /* Clear MEM_INI_ENABLE */
		}

		if (cdata) {
			printf("Setting RCache size to %dGB\n", 1 << mem_size);
			/* Set the cache size in HReq and MIU */
			val = dnc_read_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL);
			dnc_write_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL, (val & ~(3 << 26)) | ((mem_size - 1) << 26)); /* [27:26] Cache size: 0 - 2GB, 1 - 4GB, 2 - 8GB, 3 - 16GB */
			dnc_write_csr(0xfff0, H2S_CSR_G0_MIU_CACHE_SIZE, mem_size + 1); /* ((2 ^ (N - 1)) * 1GB) */
		}
	}

	/* If ret is different from zero here, it means we should stop */
	if (ret != 0)
		return ret;

	/* Initialize SRAM */
	if (!dnc_asic_mode) { /* Special FPGA considerations for Ftag SRAM */
		printf("FPGA revision %d_%d detected, no FTag SRAM\n", dnc_chip_rev >> 16, dnc_chip_rev & 0xffff);
	} else {
		/* ASIC; initialize SRAM if disable_sram is unset */
		if (!disable_sram) {
			if ((ret = dnc_initialize_sram()) < 0)
				return ret;
		} else {
			printf("No SRAM will be initialized\n");
			dnc_write_csr(0xfff0, H2S_CSR_G2_SRAM_MODE, 0x00000001);  /* Disable SRAM on NC */
		}
	}

	return 0;
}

int cpu_family(uint16_t scinode, uint8_t node)
{
	uint32_t val;
	int fam, model, stepping;
	val = dnc_read_conf(scinode, 0, 24 + node, FUNC3_MISC, 0xfc);
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
	uint64_t tom;
	uint64_t mmio_start;
	uint64_t base, lim;
	int i;
	tom = dnc_rdmsr(MSR_TOPMEM);
	mmio_start = ~0;
	printf("MSR_TOPMEM : %llx\n", tom);

	for (i = 0; i < 8; i++) {
		base = cht_read_conf(0, FUNC1_MAPS, 0x80 + i * 8);
		lim  = cht_read_conf(0, FUNC1_MAPS, 0x84 + i * 8);

		if (!(base & 3))
			continue;

		if (((base & ~0xff) << 8) < mmio_start)
			mmio_start = (base & ~0xff) << 8;

		printf("HT#0 MMIO range %d: %08x - %08x\n",
		       i, base, lim);

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
static void print_phy_gangs(int neigh, int link, char *name, int base)
{
	printf("%s:\n", name);
	printf("- CAD[ 7:0],CTL0,CLK0=0x%08x\n", get_phy_register(neigh, link, base, 0));
	printf("- CAD[15:8],CTL1,CLK1=0x%08x\n", get_phy_register(neigh, link, base + 0x10, 0));
}

static void print_phy_lanes(int neigh, int link, char *name, int base, int clk)
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
	printf("HT#%d L%d Link Control       : 0x%08x\n", neigh, link,
	      cht_read_conf(neigh, FUNC0_HT, 0x84 + link * 0x20));
	printf("HT#%d L%d Link Freq/Revision : 0x%08x\n", neigh, link,
	       cht_read_conf(neigh, FUNC0_HT, 0x88 + link * 0x20));
	printf("HT#%d L%d Link Ext. Control  : 0x%08x\n", neigh, link,
	       cht_read_conf(neigh, 0, 0x170 + link * 4));
	val = get_phy_register(neigh, link, 0xe0, 0); /* Link phy compensation and calibration control 1 */

	uint8_t rtt = (val >> 23) & 0x1f;
	printf("HT#%d L%d Link Phy Settings  : Rtt=%d Ron=%d\n", neigh, link, rtt, (val >> 18) & 0x1f);

	if (rtt == 0) {
		if (workaround_rtt) {
			printf("Warning: Hypertransport interface phy calibration failure\n");
		} else {
			printf("Error: Hypertransport interface phy calibration failure; rebooting in 15s...");
			udelay(15000000);
			reset_cf9(0xa, 0);
		}
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

void probefilter_tokens(int nodes)
{
	int i, j;
	uint32_t val;

	/* Reprogram HT link buffering */
	for (i = 0; i < nodes; i++) {
		for (j = 0; j < 4; j++) {
			val = cht_read_conf(i, FUNC0_HT, 0x98 + j * 0x20);

			/* Probe Filter doesn't affect IO link buffering */
			if ((!(val & 1)) || (val & 4))
				continue;

			val = cht_read_conf(i, FUNC0_HT, 0x170 + i * 4);
			/* Same buffer counts for ganged and unganged */
			val = (8 << 20) | (3 << 18) | (3 << 16) | (4 << 12) | (9 << 8) | (2 << 5) | 8;
			cht_write_conf(i, FUNC0_HT, 0x90 + j * 0x20, val | (1 << 31));
			cht_write_conf(i, FUNC0_HT, 0x94 + j * 0x20, 1 << 16);
		}
	}

	printf("Asserting LDTSTOP# to optimise HT buffer allocation...");
	val = pmio_readb(0x8a);
	pmio_writeb(0x8a, 0xf0);
	pmio_writeb(0x87, 1);
	pmio_writeb(0x8a, val);
	printf("done\n");
}
#endif /* __i386 */

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

	nc_neigh = neigh;
	nc_neigh_link = link;
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

	printf("done\n");

	if (reboot) {
		printf("Rebooting to make new link settings effective...\n");
		reset_cf9(2, nc - 1);
	}

#endif
}

#ifdef __i386
static void disable_atmmode(const int nodes)
{
	uint32_t val;
	uint32_t scrub[8];
	int i;
	val = cht_read_conf(0, FUNC0_HT, 0x68);

	if (!(val & (1 << 12))) {
		if (verbose) printf("ATMModeEn already disabled\n");

		return;
	}

	printf("Disabling ATMMode...");

	/* 1. Disable the L3 and DRAM scrubbers on all nodes in the system:
	   - F3x58[L3Scrub]=00h
	   - F3x58[DramScrub]=00h
	   - F3x5C[ScrubRedirEn]=0 */
	for (i = 0; i <= nodes; i++) {
		/* Fam15h: Accesses to this register must first set F1x10C [DctCfgSel]=0;
		   Accesses to this register with F1x10C [DctCfgSel]=1 are undefined;
		   See erratum 505 */
		cht_write_conf(i, FUNC1_MAPS, 0x10c, 0);
		scrub[i] = cht_read_conf(i, FUNC3_MISC, 0x58);
		cht_write_conf(i, FUNC3_MISC, 0x58, scrub[i] & ~0x1f00001f);
		val = cht_read_conf(i, FUNC3_MISC, 0x5c);
		cht_write_conf(i, FUNC3_MISC, 0x5c, val & ~1);
	}

	/* 2.  Wait 40us for outstanding scrub requests to complete */
	udelay(40);
	/* 3.  Disable all cache activity in the system by setting
	   CR0.CD for all active cores in the system */
	/* 4.  Issue WBINVD on all active cores in the system */
	disable_cache();
#ifdef BROKEN

	/* 5.  Set F3x1C4[L3TagInit]=1 */
	for (i = 0; i <= nodes; i++) {
		val = cht_read_conf(i, FUNC3_MISC, 0x1c4);
		cht_write_conf(i, FUNC3_MISC, 0x1c4, val | (1 << 31));
	}

	/* 6.  Wait for F3x1C4[L3TagInit]=0 */
	for (i = 0; i <= nodes; i++)
		while (cht_read_conf(i, FUNC3_MISC, 0x1c4) & (1 << 31))
			cpu_relax();

#endif

	/* 7.  Set F0x68[ATMModeEn]=0 and F3x1B8[L3ATMModeEn]=0 */
	for (i = 0; i <= nodes; i++) {
		val = cht_read_conf(i, FUNC0_HT, 0x68);
		cht_write_conf(i, FUNC0_HT, 0x68, val & ~(1 << 12));

		val = cht_read_conf(i, FUNC3_MISC, 0x1b8);
		cht_write_conf(i, FUNC3_MISC, 0x1b8, val & ~(1 << 27));
	}

	/* 8.  Enable all cache activity in the system by clearing
	   CR0.CD for all active cores in the system */
	enable_cache();

	/* 9. Restore L3 and DRAM scrubber register values */
	for (i = 0; i <= nodes; i++) {
		/* Fam15h: Accesses to this register must first set F1x10C [DctCfgSel]=0;
		   Accesses to this register with F1x10C [DctCfgSel]=1 are undefined;
		   See erratum 505 */
		cht_write_conf(i, FUNC1_MAPS, 0x10c, 0);
		cht_write_conf(i, FUNC3_MISC, 0x58, scrub[i]);
		val = cht_read_conf(i, FUNC3_MISC, 0x5c);
		cht_write_conf(i, FUNC3_MISC, 0x5c, val | 1);
	}

	printf("done\n");
}

static void disable_probefilter(const int nodes)
{
	uint32_t val;
	uint32_t scrub[8];
	int i;
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
	for (i = 0; i <= nodes; i++) {
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
	for (i = 0; i <= nodes; i++) {
		val = cht_read_conf(i, FUNC3_MISC, 0x1c4);
		cht_write_conf(i, FUNC3_MISC, 0x1c4, val | (1 << 31));
	}

	/* 6.  Wait for F3x1C4[L3TagInit]=0 */
	for (i = 0; i <= nodes; i++)
		while (cht_read_conf(i, FUNC3_MISC, 0x1c4) & (1 << 31))
			cpu_relax();

	/* 7.  Set F3x1D4[PFMode]=00b */
	for (i = 0; i <= nodes; i++) {
		val = cht_read_conf(i, FUNC3_MISC, 0x1d4);
		cht_write_conf(i, FUNC3_MISC, 0x1d4, val & ~3);
	}

	for (i = 0; i <= nodes; i++) {
		val = cht_read_conf(i, FUNC3_MISC, 0x1d4);
	}

	/* 8.  Enable all cache activity in the system by clearing
	   CR0.CD for all active cores in the system */
	enable_cache();
	critical_leave();

	/* 9. Restore L3 and DRAM scrubber register values */
	for (i = 0; i <= nodes; i++) {
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

	uint64_t val = dnc_rdmsr(MSR_APIC_BAR);
	volatile uint32_t *const apic = (void * const)((uint32_t)val & ~0xfff);
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

void enable_probefilter(const int nodes)
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
	for (i = 0; i <= nodes; i++) {
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
		val6 = dnc_rdmsr(MSR_CU_CFG3);
		dnc_wrmsr(MSR_CU_CFG3, val6 | (1ULL << 49));
	}

	/* 4.  Issue WBINVD on all active cores in the system */
	disable_cache();
	/* 5. Enable Probe Filter support */
	val6 = dnc_rdmsr(MSR_CU_CFG2);
	dnc_wrmsr(MSR_CU_CFG2, val6 | (1ULL << 42));

	if (family >= 0x15)
		wake_cores_local(VECTOR_PROBEFILTER_EARLY_f15);
	else
		wake_cores_local(VECTOR_PROBEFILTER_EARLY_f10);

	/* 6. Set F3x1C4[L3TagInit]=1 */
	for (i = 0; i <= nodes; i++) {
		val = cht_read_conf(i, FUNC3_MISC, 0x1c4);
		cht_write_conf(i, FUNC3_MISC, 0x1c4, val | (1 << 31));
	}

	/* 7. Wait for F3x1C4[L3TagInit]=0 */
	for (i = 0; i <= nodes; i++)
		while (cht_read_conf(i, FUNC3_MISC, 0x1c4) & (1 << 31))
			cpu_relax();

	/* 8. Set PF flags depending on cache size */
	for (i = 0; i <= nodes; i++) {
		uint32_t pfctrl = (2 << 2) | (0xf << 12) | (1 << 17) | (1 << 29);

		if ((cht_read_conf(i, FUNC3_MISC, 0x1c4) & 0xffff) == 0xcccc)
			pfctrl |= 3 | (1 << 4) | (1 << 6) | (1 << 8) | (1 << 10) | (2 << 20);
		else
			pfctrl |= 2;

		cht_write_conf(i, FUNC3_MISC, 0x1d4, pfctrl);
	}

	/* 6. Wait for F3x1D4[PFInitDone]=1 */
	for (i = 0; i <= nodes; i++) {
		while ((cht_read_conf(i, FUNC3_MISC, 0x1d4) & (1 << 19)) == 0)
			cpu_relax();
	}

	/* 8.  Enable all cache activity in the system by clearing
	   CR0.CD for all active cores in the system */
	enable_cache();
	wake_cores_local(VECTOR_ENABLE_CACHE);
	critical_leave();

	/* 9. Restore L3 and DRAM scrubber register values */
	for (i = 0; i <= nodes; i++) {
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

static int disable_nc = 0;

static int ht_fabric_find_nc(bool *p_asic_mode, uint32_t *p_chip_rev)
{
#ifndef __i386
	printf("(Only doing HT discovery and reconfig in 32-bit mode)\n");
	return -1;
#else
	int nodes, neigh, link = 0, rt, nc, i;
	uint32_t val;
	val = cht_read_conf(0, FUNC0_HT, 0x60);
	nodes = (val >> 4) & 7;
	bool use = 1;

	for (neigh = 0; neigh <= nodes; neigh++) {
		uint32_t aggr = cht_read_conf(neigh, FUNC0_HT, 0x164);

		for (link = 0; link < 4; link++) {
			val = cht_read_conf(neigh, FUNC0_HT, 0x98 + link * 0x20);

			if ((val & 0x1f) != 0x3)
				continue; /* Not coherent */

			use = 0;

			if (aggr & (0x10000 << link))
				use = 1;

			for (rt = 0; rt <= nodes; rt++) {
				val = cht_read_conf(neigh, FUNC0_HT, 0x40 + rt * 4);

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
		printf("Error: No unrouted coherent links found\n");
		return -1;
	}

	printf("HT#%d L%d is coherent and unrouted\n", neigh, link);

	if (disable_nc) {
		printf("Disabling NC link.\n");
		disable_link(neigh, link);
		return -1;
	}

	nc = nodes + 1;
	/* "neigh" request/response routing, copy bcast values from self */
	val = cht_read_conf(neigh, FUNC0_HT, 0x40 + neigh * 4);
	cht_write_conf(neigh, FUNC0_HT, 0x40 + nc * 4,
	               (val & 0x07fc0000) | (0x402 << link));

	for (i = 0; i <= nodes; i++) {
		val = cht_read_conf(i, FUNC0_HT, 0x68);
		cht_write_conf(i, FUNC0_HT, 0x68, val & ~(1 << 15)); /* LimitCldtCfg */

		if (i == neigh)
			continue;

		/* Route "nc" same as "neigh" for all other nodes */
		val = cht_read_conf(i, FUNC0_HT, 0x40 + neigh * 4);
		cht_write_conf(i, FUNC0_HT, 0x40 + nc * 4, val);
	}

#ifdef __i386
	cht_print(neigh, link);
#endif

	/* Earliest opportunity to test HT link */
	if (ht_testmode & HT_TESTMODE_TEST)
		cht_test(nc, neigh, link);

	val = cht_read_conf(nc, 0, H2S_CSR_F0_DEVICE_VENDOR_ID_REGISTER);

	if (val != 0x06011b47) {
		printf("Error: Unrouted coherent device %08x is not NumaChip\n", val);
		return -1;
	}

	printf("NumaChip found (%08x)\n", val);
	/* Ramp up link speed and width before adding NC to coherent fabric */
	val = cht_read_conf(nc, 0, 0xec);

	if (val == 0) {
		val = cht_read_conf(nc, 0, H2S_CSR_F0_CLASS_CODE_REVISION_ID_REGISTER);
		printf("Doing link calibration of ASIC chip rev %d\n", val & 0xffff);
		ht_optimize_link(nc, val & 0xffff, 1);
		*p_asic_mode = 1;
		*p_chip_rev = val & 0xffff;
	} else {
		printf("Doing link calibration of FPGA chip rev %d_%d\n", val >> 16, val & 0xffff);
		ht_optimize_link(nc, val, 0);
		*p_asic_mode = 0;
		*p_chip_rev = val;
	}

	/* RevB ASIC requires syncflood to be disabled */
	if (*p_asic_mode && *p_chip_rev < 2)
		ht_suppress = 0x3fff; /* Suppress most sync-flood generation */

	if (ht_suppress) {
		printf("Setting HT features (%x)...", ht_suppress);

		for (i = 0; i <= nodes; i++) {
			val = cht_read_conf(i, FUNC3_MISC, 0x44);

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

			cht_write_conf(i, FUNC3_MISC, 0x44, val);
			val = cht_read_conf(i, FUNC3_MISC, 0x180);

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

			/* ChgUcToCeEn: change uncorrectable to correctable for debug purposes */
			if (ht_suppress & 0x4000) val |= 1 << 26;

			cht_write_conf(i, FUNC3_MISC, 0x180, val);

			/* SERR_EN: initiate sync flood when PCIe System Error detected in IOH */
			if (ht_suppress & 0x8000) {
				val = dnc_read_conf(0xfff0, 0, 0, 0, 0x4);
				dnc_write_conf(0xfff0, 0, 0, 0, 0x4, val & ~(1 << 8));
			}

			/* PFErrInt: ProbeFilter error interrupt */
			if (ht_suppress & 0x10000) {
				val = cht_read_conf(i, FUNC3_MISC, 0x1d4);
				cht_write_conf(i, FUNC3_MISC, 0x1d4, val & ~(3 << 22));
			}

			for (int link = 0; link < 4; link++) {
				/* CrcFloodEn: Enable sync flood propagation upon link failure */
				if (ht_suppress & 0x20000) {
					val = cht_read_conf(i, FUNC0_HT, 0x84 + link * 0x20);
					cht_write_conf(i, FUNC0_HT, 0x84 + link * 0x20, val & ~2);
				}

				/* IntType: Link error threshold overflow counter */
				if (ht_suppress & 0x40000) {
					val = cht_read_conf(i, FUNC3_MISC, 0x160 + link * 8);
					assert((val & (1 << 29)) == 0); /* Locked */
					cht_write_conf(i, FUNC3_MISC, 0x160 + link * 8, val & ~(3 << 17));
				}
			}
		}

		printf("done\n");
	}

	/* HT looping testmode useful after link is up at 16-bit 800MHz */
	if (ht_testmode & HT_TESTMODE_LOOP)
		while (1)
			cht_test(nc, neigh, link);

	if (route_only)
		return nc;

	/* On ASIC revB we need to make sure probefilter is disabled */
	if ((*p_asic_mode && (*p_chip_rev < 2)) || force_probefilteroff)
		disable_probefilter(nodes);

	/* On Fam15h disable the Accelerated Transiton to Modified protocol */
	if (family >= 0x15)
		disable_atmmode(nodes);

	printf("Adjusting HT fabric...");
	critical_enter();

	for (i = nodes; i >= 0; i--) {
		uint32_t ltcr, val2;
		/* Disable probes while adjusting */
		ltcr = cht_read_conf(i, FUNC0_HT, 0x68);
		cht_write_conf(i, FUNC0_HT, 0x68,
		               ltcr | (1 << 10) | (1 << 3) | (1 << 2) | (1 << 1) | (1 << 0));
		/* Update "neigh" bcast values for node about to increment fabric size */
		val = cht_read_conf(neigh, FUNC0_HT, 0x40 + i * 4);
		val2 = cht_read_conf(i, FUNC0_HT, 0x60);
		cht_write_conf(neigh, FUNC0_HT, 0x40 + i * 4, val | (0x80000 << link));
		/* FIXME: Race condition observered to cause lockups at this point */
		/* Increase fabric size */
		cht_write_conf(i, FUNC0_HT, 0x60, val2 + (1 << 4));
		/* Reassert LimitCldtCfg */
		cht_write_conf(i, FUNC0_HT, 0x68, ltcr | (1 << 15));
	}

	critical_leave();
	printf("done\n");
	/* reorganize_mmio(nc); */
	return nc;
#endif
}

static int ht_fabric_fixup(bool *p_asic_mode, uint32_t *p_chip_rev)
{
	uint32_t val;
	uint8_t node;
	int dnc_ht_id;

	val = cht_read_conf(0, FUNC0_HT, 0x60);
	dnc_ht_id = (val >> 4) & 7;
	val = cht_read_conf(dnc_ht_id, 0, H2S_CSR_F0_DEVICE_VENDOR_ID_REGISTER);

	if (val == 0x06011b47) {
		printf("NumaChip already present on HT node %d\n", dnc_ht_id);
		/* Chip already found; make sure the desired width/frequency is set */
		val = cht_read_conf(dnc_ht_id, 0, 0xec);

		if (val == 0) {
			val = cht_read_conf(dnc_ht_id, 0, H2S_CSR_F0_CLASS_CODE_REVISION_ID_REGISTER);
			printf("Doing link calibration of ASIC chip rev %d\n", val & 0xffff);
			ht_optimize_link(dnc_ht_id, val & 0xffff, 1);
			*p_asic_mode = 1;
			*p_chip_rev = val & 0xffff;
		} else {
			printf("Doing link calibration of FPGA chip rev %d_%d\n", val >> 16, val & 0xffff);
			ht_optimize_link(dnc_ht_id, val, 0);
			*p_asic_mode = 0;
			*p_chip_rev = val;
		}
	} else {
		dnc_ht_id = ht_fabric_find_nc(p_asic_mode, p_chip_rev);

		if (dnc_ht_id < 0) {
			printf("NumaChip not found\n");
			*p_asic_mode = -1;
			*p_chip_rev = -1;
			return -1;
		}

		printf("NumaChip incorporated as HT node %d\n", dnc_ht_id);
	}

	val = cht_read_conf(dnc_ht_id, 0, H2S_CSR_F0_DEVICE_VENDOR_ID_REGISTER);
	printf("Node #%d F0x00: %x\n", dnc_ht_id, val);
	val = cht_read_conf(0, FUNC0_HT, 0x60);
	cht_write_conf(dnc_ht_id, 0,
	               H2S_CSR_F0_CHTX_NODE_ID,
	               (((val >> 12) & 7) << 24) | /* LkNode */
	               (((val >> 8)  & 7) << 16) | /* SbNode */
	               (dnc_ht_id << 8) | /* NodeCnt */
	               dnc_ht_id); /* NodeId */
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
	for (node = 0; node < dnc_ht_id; node++) {
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
	val = cht_read_conf(dnc_ht_id, 0, H2S_CSR_F0_STATUS_COMMAND_REGISTER);
	printf("Command/Status: %08x\n", val);

	if (val & (1 << 1)) {
		cht_write_conf(dnc_ht_id, 0,
		               H2S_CSR_F0_STATUS_COMMAND_REGISTER, val & ~(1 << 1));
		cht_write_conf(dnc_ht_id, 0,
		               H2S_CSR_F0_BASE_ADDRESS_REGISTER_0, 0);
		cht_write_conf(dnc_ht_id, 0,
		               H2S_CSR_F0_EXPANSION_ROM_BASE_ADDRESS, 0);
	}

	/* Check the expansion rom base address register if this has already been done */
	val = cht_read_conf(dnc_ht_id, 0, H2S_CSR_F0_EXPANSION_ROM_BASE_ADDRESS);

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
		for (node = 0; node < dnc_ht_id; node++)
			mmio_range(0xfff0, node, 8, DEF_DNC_CSR_BASE, DEF_DNC_CSR_LIM, dnc_ht_id);

		printf("Setting CSR_BASE_ADDRESS to %04llx using default address\n", (DNC_CSR_BASE >> 32));
		mem64_write32(DEF_DNC_CSR_BASE | (0xfff0 << 16) | (1 << 15) | H2S_CSR_G3_CSR_BASE_ADDRESS,
		              uint32_tbswap(DNC_CSR_BASE >> 32));
		cht_write_conf(dnc_ht_id, 0,
		               H2S_CSR_F0_EXPANSION_ROM_BASE_ADDRESS,
		               (DNC_CSR_BASE >> 16)); /* Put DNC_CSR_BASE[47:32] in the rom address register offset[31:16] */
#else
		/* XXX: Perhaps we could try default global map in userspace too ? */
		printf("Unable to determine CSR space address\n");
		return -1;
#endif
	}

	printf("Setting CSR and MCFG maps...\n");
	for (node = 0; node < dnc_ht_id; node++) {
		mmio_range_del(0xfff0, node, 8);
		mmio_range(0xfff0, node, 8, DNC_CSR_BASE, DNC_CSR_LIM, dnc_ht_id);
		mmio_range(0xfff0, node, 9, DNC_MCFG_BASE, DNC_MCFG_LIM, dnc_ht_id);
	}

	/* Set MMCFG base register so local NC will forward correctly */
	val = dnc_read_csr(0xfff0, H2S_CSR_G3_MMCFG_BASE);

	if (val != (DNC_MCFG_BASE >> 24)) {
		printf("Setting local MCFG_BASE to %08llx\n", DNC_MCFG_BASE >> 24);
		dnc_write_csr(0xfff0, H2S_CSR_G3_MMCFG_BASE, DNC_MCFG_BASE >> 24);
	}

	cht_write_conf(dnc_ht_id, 0, H2S_CSR_F0_CHTX_LINK_INITIALIZATION_CONTROL, 0);
	cht_write_conf(dnc_ht_id, 0, H2S_CSR_F0_CHTX_ADDITIONAL_LINK_TRANSACTION_CONTROL, 6);
	return dnc_ht_id;
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

static void _pic_reset_ctrl(int val)
{
	dnc_write_csr(0xfff0, H2S_CSR_G1_PIC_RESET_CTRL, val);
	udelay(20000);
	(void)dnc_read_csr(0xfff0, H2S_CSR_G1_PIC_INDIRECT_READ); /* Use a read operation to terminate the current i2c transaction, to avoid a bug in the uC */
	udelay(2000000);
}

static int _is_pic_present(char p_type[16])
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

int adjust_oscillator(char p_type[16], uint32_t osc_setting)
{
	uint32_t val;

	if (enable_relfreq) {
		printf("PHY Relative Frequency Correction mode enabled, not adjusting oscillators\n");
		return 0;
	}

	/* Check if adjusting the frequency is possible */
	if (_is_pic_present(p_type)) {
		if (osc_setting > 2) {
			printf("Invalid oscillator setting %d; skipping\n", osc_setting);
			return 0;
		}

		dnc_write_csr(0xfff0, H2S_CSR_G1_PIC_INDIRECT_READ, 0x40); /* Set the indirect read address register */
		udelay(10000);
		val = dnc_read_csr(0xfff0, H2S_CSR_G1_PIC_INDIRECT_READ);

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
			printf("Oscillator set to %d\n", (val >> 24) & 3);
			/* Trigger a HSS PLL reset */
			_pic_reset_ctrl(1);
		}
	} else
		printf("Oscillator not set\n");

	return 0;
}

struct optargs {
	char label[16];
	int (*handler)(const char *, void *);
	void *userdata;
};

static int parse_string(const char *val, void *stringp)
{
	char **string = (char **)stringp;
	*string = strdup(val);
	assert(*string);
	return 1;
}

static int parse_bool(const char *val, void *voidp)
{
	bool *boolp = (bool *)voidp;

	if (val[0] != '\0') {
		int res = atoi(val);
		assertf(res == !!res, "Boolean option doesn't take value %d", res);
		*boolp = res;
	} else
		*boolp = true;

	return 1;
}

static int parse_int(const char *val, void *intp)
{
	int *int32 = (int *)intp;

	if (val[0] != '\0')
		*int32 = (int)strtol(val, NULL, 0);
	else
		*int32 = 1;

	return 1;
}

static int parse_uint64_t(const char *val, void *intp)
{
	uint64_t *int64 = (uint64_t *)intp;

	if (val[0] != '\0') {
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

	return 1;
}

static int parse_cmdline(const char *cmdline)
{
	static const struct optargs options[] = {
		{"config",	    &parse_string, &config_file_name},/* Config (JSON) file to use */
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
		{"pf.vga-local",    &parse_bool,   &pf_vga_local},    /* Let legacy VGA access route locally */
		{"pf.maxmem",       &parse_uint64_t, &pf_maxmem},       /* Memory per server */
		{"handover-acpi",   &parse_bool,   &handover_acpi},   /* Workaround Linux not being able to handover ACPI */
		{"disable-smm",     &parse_bool,   &disable_smm},     /* Rewrite start of System Management Mode handler to return */
		{"disable-c1e",     &parse_bool,   &disable_c1e},     /* Prevent C1E sleep state entry and LDTSTOP usage */
		{"renumber-bsp",    &parse_int,    &renumber_bsp},
		{"forwarding-mode", &parse_int,    &forwarding_mode},
		{"sync-interval",   &parse_int,    &sync_interval},
		{"enable-relfreq",  &parse_bool,   &enable_relfreq},
		{"singleton",       &parse_bool,   &singleton},       /* Loopback test with cables */
		{"mem-offline",     &parse_bool,   &mem_offline},
		{"trace-buf",       &parse_uint64_t,    &trace_buf_size},
		{"verbose",         &parse_int,    &verbose},
		{"remote-io",       &parse_int,    &remote_io},
		{"boot-wait",       &parse_bool,   &boot_wait},
		{"dimmtest",	    &parse_int,    &dimmtest},        /* Run on-board DIMM self test */
		{"workaround.hreq", &parse_bool,   &workaround_hreq}, /* Enable half HReq buffers; on by default */
		{"workaround.rtt",  &parse_bool,   &workaround_rtt},  /* Prevent failure when HT Rtt calibration fails */
		{"mem-gap",         &parse_uint64_t, &mem_gap},
	};
	char arg[256];
	int lstart, lend, aend, i;

	if (!cmdline)
		return 1;

	printf("Options:");
	lstart = 0;

	while (cmdline[lstart] != '\0') {
		while (cmdline[lstart] != '\0' && cmdline[lstart] == ' ')
			lstart++;

		lend = lstart;

		while (cmdline[lend] != '\0' && cmdline[lend] != ' ' && cmdline[lend] != '=')
			lend++;

		aend = lend;

		while (cmdline[aend] != '\0' && cmdline[aend] != ' ')
			aend++;

		if (lstart == lend)
			break;

		if (lend - lstart < (int)(sizeof(options[0].label))) {
			for (i = 0; i < (int)(sizeof(options) / sizeof(options[0])); i++) {
				if (strncmp(&cmdline[lstart], options[i].label, lend - lstart) == 0) {
					memset(arg, 0, sizeof(arg));

					if (cmdline[lend] == '=')
						lend++;

					if (aend - lend >= (int)(sizeof(arg)))
						strncpy(arg, &cmdline[lend], sizeof(arg) - 1);
					else
						strncpy(arg, &cmdline[lend], aend - lend);

					printf(" %s=%s", options[i].label, arg);

					if (options[i].handler(arg, options[i].userdata) < 0) {
						printf("\n");
						return -1;
					}

					break;
				}
			}

			/* Check if option isn't recognised */
			if (i == (int)(sizeof(options) / sizeof(options[0]))) {
				/* Terminate current arg */
				*strchr(&cmdline[lstart], ' ') = '\0';
				printf("\nError: invalid option '%s'\n", &cmdline[lstart]);
				wait_key();
			}
		}

		lstart = aend;
	}

	printf("\n");
	return 1;
}

static int perform_selftest(int asic_mode, char p_type[16])
{
	int pass, res;
	uint32_t val;
	res = 0;
	printf("Performing internal RAM self test:");

	for (pass = 0; pass < 10 && res == 0; pass++) {
		const uint16_t maxchunk = asic_mode ? 16 : 1; /* On FPGA all these rams are reduced in size */
		int i, chunk;
		/* Test PCII/O ATT */
		printf(" 1");
		dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00000000);

		for (i = 0; i < 256; i++) {
			dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4, i);
		}

		for (i = 0; i < 256; i++) {
			val = dnc_read_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4);

			if (val != (uint32_t)i) {
				res = -1;
				break;
			}
		}

		if (res < 0) break;

		if (asic_mode) {
			/* XXX: MMIO32 ATT has a slightly different layout on FPGA, so skip it for now */
			/* Test MMIO32 ATT */
			printf("2");

			for (chunk = 0; chunk < maxchunk; chunk++) {
				dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00000010 | chunk);

				for (i = 0; i < 256; i++) {
					dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4, (chunk * 256) + i);
				}
			}

			for (chunk = 0; chunk < maxchunk; chunk++) {
				dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00000010 | chunk);

				for (i = 0; i < 256; i++) {
					val = dnc_read_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4);

					if (val != (uint32_t)((chunk * 256) + i)) {
						res = -1;
						break;
					}
				}
			}
		}

		if (res < 0) break;

		/* Test APIC ATT */
		printf("3");

		for (chunk = 0; chunk < maxchunk; chunk++) {
			dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00000020 | chunk);

			for (i = 0; i < 256; i++) {
				dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4, (chunk * 256) + i);
			}
		}

		for (chunk = 0; chunk < maxchunk; chunk++) {
			dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00000020 | chunk);

			for (i = 0; i < 256; i++) {
				val = dnc_read_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4);

				if (val != (uint32_t)((chunk * 256) + i)) {
					res = -1;
					break;
				}
			}
		}

		if (res < 0) break;

		/* Test IntRecNode ATT */
		printf("4");

		for (chunk = 0; chunk < maxchunk; chunk++) {
			dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00000040 | chunk);

			for (i = 0; i < 256; i++) {
				dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4, (chunk * 256) + i);
			}
		}

		for (chunk = 0; chunk < maxchunk; chunk++) {
			dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00000040 | chunk);

			for (i = 0; i < 256; i++) {
				val = dnc_read_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i * 4);

				if (val != (uint32_t)((chunk * 256) + i)) {
					res = -1;
					break;
				}
			}
		}

		if (res < 0) break;

		/* Test SCC routing tables, no readback verify */
		printf("5");

		for (chunk = 0; chunk < maxchunk; chunk++) {
			dnc_write_csr(0xfff0, H2S_CSR_G0_ROUT_TABLE_CHUNK, chunk);

			for (i = 0; i < 16; i++) {
				dnc_write_csr(0xfff0, H2S_CSR_G0_ROUT_BXTBLL00 + i * 4, 0);
				dnc_write_csr(0xfff0, H2S_CSR_G0_ROUT_BLTBL00  + i * 4, 0);
				dnc_write_csr(0xfff0, H2S_CSR_G0_ROUT_BXTBLH00 + i * 4, 0);
			}
		}

		printf("-PASS%d", pass);
	}

	if (asic_mode && res == 0 && _is_pic_present(p_type)) {
		printf("\nPerforming High Speed Serdes self test: ");
		/* Trigger a HSS PLL reset */
		_pic_reset_ctrl(1);
		printf("HSS");

		for (int phy = 0; phy < 6; phy++) {
			/* 1. Check that the HSS PLL has locked */
			val = dnc_read_csr(0xfff0, H2S_CSR_G0_HSSXA_STAT_1 + 0x40 * phy);

			if (!(val & (1 << 8))) {
				res = -1;
				goto pll_err_out;
			}

			printf(" T");
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
				res = -1;
				break;
			}

			/* Run test for 100msec */
			udelay(100000);
			val = dnc_read_csr(0xfff0, H2S_CSR_G0_HSSXA_STAT_1 + 0x40 * phy);

			if (val & 0xf0) {
				res = -1;
				break;
			}

			printf("R");
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
				res = -1;
				break;
			}

			/* Run test for 100msec */
			udelay(100000);
			val = dnc_read_csr(0xfff0, H2S_CSR_G0_HSSXA_STAT_1 + 0x40 * phy);

			if (val & 0xff) {
				res = -1;
				break;
			}

			/* Stop BIST test */
			dnc_write_csr(0xfff0, H2S_CSR_G0_HSSXA_CTR_7 + 0x40 * phy, 0x00f0);
			dnc_write_csr(0xfff0, H2S_CSR_G0_HSSXA_CTR_7 + 0x40 * phy, 0x0000);
			val = dnc_read_csr(0xfff0, H2S_CSR_G0_HSSXA_CTR_1 + 0x40 * phy);
			dnc_write_csr(0xfff0, H2S_CSR_G0_HSSXA_CTR_1 + 0x40 * phy, val & ~(0xf));
			dnc_write_csr(0xfff0, H2S_CSR_G0_HSSXA_CTR_8 + 0x40 * phy, 0x00f0);
			dnc_write_csr(0xfff0, H2S_CSR_G0_HSSXA_CTR_8 + 0x40 * phy, 0x0000);
			printf("%s", _get_linkname(phy));
		}

		/* Trigger a HSS PLL reset */
		_pic_reset_ctrl(1);
	}

	printf("\nSelftest %s\n", (res == 0) ? "passed" : "failed");
	return res;
pll_err_out:
	printf("\n");

	for (pass = 0; pass < 3; pass++) {
		for (int phy = 0; phy < 6; phy++) {
			val = dnc_read_csr(0xfff0, H2S_CSR_G0_HSSXA_STAT_1 + 0x40 * phy);
			printf("HSS%s_STAT_1 = %x\n", _get_linkname(phy), val);
		}

		udelay(1000000);
	}

	return res;
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
	0x0000040f, 0x00000410, 0x00000411, 0x00000412, 0x00000413, 0x00000414, 0x00000415, 0x00000416,
	0x00000417, 0xc0000080, 0xc0000081, 0xc0000082, 0xc0000083, 0xc0000084, 0xc0000100, 0xc0000101,
	0xc0000102, 0xc0000103, 0xc0000408, 0xc0000409, 0xc000040a, 0xc000040b, 0xc000040c, 0xc000040d,
	0xc000040e, 0xc000040f, /* V 0xc0010000, V 0xc0010001, V 0xc0010002, V 0xc0010003, V 0xc0010004, V 0xc0010005,*/
	/* V 0xc0010006, V 0xc0010007,*/ 0xc0010010, 0xc0010015, 0xc0010016, 0xc0010017, 0xc0010018, 0xc0010019,
	0xc001001a, 0xc001001d, 0xc001001f, /* H 0xc0010020,*/ 0xc0010022, /* V 0xc0010030, V 0xc0010031, V 0xc0010032,*/
	/* V 0xc0010033, V 0xc0010034, V 0xc0010035, V 0xc001003e,*/ 0xc0010044, 0xc0010045, 0xc0010046, 0xc0010047,
	0xc0010048, 0xc0010049, 0xc0010050, 0xc0010051, 0xc0010052, 0xc0010053, 0xc0010054, 0xc0010055,
	0xc0010056, /* V 0xc0010058,*/ 0xc0010059, 0xc001005a, 0xc001005b, 0xc001005c, 0xc001005d, 0xc0010060,
	0xc0010061, 0xc0010062, 0xc0010063, /* V 0xc0010064, V 0xc0010065, V 0xc0010066, V 0xc0010067, V 0xc0010068, */
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
		printf("MSR 0x%08x: %016" PRIx64 "\n", msr, dnc_rdmsr(msr));
}

void selftest_late_msrs(void)
{
	unsigned int offset;
	uint32_t msr;
	printf("Checking MSRs for consistency...\n");

	for (offset = 0; (msr = get_msr(offset)) != 0xffffffff; offset++) {
		bool printed = false;
		uint64_t val0 = dnc_rdmsr(msr);

		for (int node = 0; node < dnc_node_count; node++) {
			for (int ht = 0; ht < 8; ht++) {
				for (int i = 0; i < nc_node[node].ht[ht].cores; i++) {
					if ((node == 0) && (ht == 0) && (i == 0))
						continue; /* Skip BSP */
					if (!nc_node[node].ht[ht].cpuid)
						continue;

					uint32_t oldid = nc_node[node].ht[ht].apic_base + i;
					uint32_t apicid = nc_node[node].apic_offset + oldid;
					*REL32(msr_readback) = msr;

#ifdef FIXME
					wake_core_global(apicid, VECTOR_READBACK_MSR);
#else
					/* Use local IPI distribution until global is fixed */
					if (apicid > 254)
						break;

					wake_core_local(apicid, VECTOR_READBACK_MSR);
#endif
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

int dnc_init_bootloader(uint32_t *p_uuid, uint32_t *p_chip_rev, char p_type[16], bool *p_asic_mode, const char *cmdline)
{
	uint32_t uuid, val, chip_rev;
	int i, ht_id;
	bool asic_mode;

	if (parse_cmdline(cmdline) < 0)
		return -1;

	detect_southbridge();

	ht_id = ht_fabric_fixup(&asic_mode, &chip_rev);

	if (verbose > 1)
		dump_northbridge_regs(ht_id);

	/* Indicate immediate jump to next-label (-2) if init-only is also given */
	if ((disable_nc > 0) && init_only)
		return -2;

	/* Indicate immediate jump to next-label (-2) if route_only issued */
	if (route_only)
		return -2;

	if (ht_id < 0)
		return -1;

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

		/* InstallStateS to avoid exclusive state */
		val = cht_read_conf(i, FUNC0_HT, 0x68);
		cht_write_conf(i, FUNC0_HT, 0x68, val | (1 << 23));

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

		/* Linux updates visible Northbridges with IBS LVT offset 1;
		 * this causes inconsistencies so do it here on all Northbridges */
		val = cht_read_conf(i, FUNC3_MISC, 0x1cc);
		cht_write_conf(i, FUNC3_MISC, 0x1cc, (1 << 8) | 1); /* LvtOffset = 1, LvtOffsetVal = 1 */

		/* On Fam15h disable the core prefetch hits as NumaChip doesn't support these */
		if (family >= 0x15) {
			val = cht_read_conf(i, FUNC5_EXTD, 0x88);

			if (!(val & (1 << 9))) {
				if (verbose > 0)
					printf("Setting DisHintInHtMskCnt for node %d\n", i);

				cht_write_conf(i, FUNC5_EXTD, 0x88, val | (1 << 9));
			}
		}
	}

	/* ====================== END ERRATA WORKAROUNDS ====================== */
	uuid = identify_eeprom(p_type);
	printf("UUID: %08X, TYPE: %s\n", uuid, p_type);

	/* Read the SPD info from our DIMMs to see if they are supported */
	for (i = 0; i < 2; i++) {
		if (read_spd_info(p_type, i, &dimms[i]) < 0)
			return -1;
	}

	if (perform_selftest(asic_mode, p_type) < 0)
		return -1;

	/* If init-only parameter is given, stop here and return */
	if (init_only)
		return -2;

	*p_asic_mode = asic_mode;
	*p_chip_rev = chip_rev;
	*p_uuid = uuid;
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

static int _verify_save_id(uint16_t nodeid, int lc)
{
	const char *linkname = _get_linkname(lc);
	uint16_t expected_id = (nodeid | ((lc + 1) << 13));
	uint32_t val;

	if (dnc_raw_read_csr(0xfff1 + lc, LC3_CSR_SAVE_ID, &val) != 0)
		return -1;

	if (val != expected_id) {
		printf("LC3%s SAVE_ID (%04x) doesn't match expected value (%04x)\n",
		       linkname, val, expected_id);
		return -1;
	}

	return 0;
}

static int _check_dim(int dim)
{
	uint16_t linkida = 2 * dim + 0;
	uint16_t linkidb = 2 * dim + 1;
	int ok = 0;
	ok = (dnc_check_phy(linkida) == 0);
	ok &= (dnc_check_phy(linkidb) == 0);

	if (!ok) {
		printf("Errors detected on PHY%s and PHY%s; resetting\n",
		       _get_linkname(linkida), _get_linkname(linkidb));
		/* Counter-rotating rings, reset both phys */
		dnc_reset_phy(linkida);
		dnc_reset_phy(linkidb);
		udelay(1000);
		ok = (dnc_check_phy(linkida) == 0);
		ok &= (dnc_check_phy(linkidb) == 0);

		if (!ok) {
			printf("Errors still present on PHY%s and PHY%s\n",
			       _get_linkname(linkida), _get_linkname(linkidb));
			return -1;
		}
	}

	ok = (dnc_check_lc3(linkida) == 0);
	ok &= (dnc_check_lc3(linkidb) == 0);

	if (!ok) {
		printf("LC3 errors detected on LC3%s and LC3%s; resetting\n",
		       _get_linkname(linkida), _get_linkname(linkidb));
		/* Counter-rotating rings, reset both phys */
		dnc_reset_phy(linkida);
		dnc_reset_phy(linkidb);
		udelay(1000);
		ok = (dnc_check_phy(linkida) == 0);
		ok &= (dnc_check_phy(linkidb) == 0);
		ok &= (dnc_check_lc3(linkida) == 0);
		ok &= (dnc_check_lc3(linkidb) == 0);

		if (!ok) {
			printf("LC3 reset not successful\n");
			return -1;
		}
	}

	return ok ? 0 : -1;
}

#ifdef UNUSED
static int shortest(uint8_t dim, uint16_t src, uint16_t dst)
{
	/* Extract positions on ring */
	int src2 = (src >> (dim * 4)) & 0xf;
	int dst2 = (dst >> (dim * 4)) & 0xf;
	int len = (dim == 0) ? cfg_fabric.x_size : (dim == 1) ? cfg_fabric.y_size : cfg_fabric.z_size;
	assert(len);
	int forward = ((len - src2) + dst2) % len;
	int backward = ((src2 + (len - dst2)) + len) % len;

	if (forward == backward)
		return src2 & 1; /* Load balance */

	return backward < forward;
}
#endif

static enum node_state setup_fabric(struct node_info *info)
{
	int i;
	uint8_t lc;
	dnc_write_csr(0xfff0, H2S_CSR_G0_NODE_IDS, info->sciid << 16);
	memset(shadow_ltbl, 0, sizeof(shadow_ltbl));
	memset(shadow_rtbll, 0, sizeof(shadow_rtbll));
	memset(shadow_rtblm, 0, sizeof(shadow_rtblm));
	memset(shadow_rtblh, 0, sizeof(shadow_rtblh));
#ifdef UNUSED
	printf("Using %s path routing\n", cfg_fabric.strict ? "shortest" : "unoptimised");
#endif

	if (cfg_fabric.x_size > 0)
		_add_route(info->sciid, 0, 1); /* Self route via LC3XA */
	else if (cfg_fabric.y_size > 0)
		_add_route(info->sciid, 0, 3); /* Self route via LC3YA */
	else
		_add_route(info->sciid, 0, 5); /* Self route via LC3ZA */

	/* Make sure responses get back to SCC */
	for (lc = 1; lc <= 6; lc++) {
		_add_route(info->sciid, lc, 0);
	}

	for (i = 0; i < cfg_nodes; i++) {
		uint16_t src = info->sciid;
		uint16_t dst = cfg_nodelist[i].sciid;
		uint8_t dim = 0;
		uint8_t out;

		if (src == dst)
			continue;

		while ((src ^ dst) & ~0xf) {
			dim++;
			src >>= 4;
			dst >>= 4;
		}

		out = dim * 2 + 1;
		out += ((src ^ dst) & 0x1); /* Load balance pairs */

#ifdef UNUSED
		if (cfg_fabric.strict)
			/* SCI IDs correspond to position on the rings */
			out += shortest(dim, info->sciid, cfg_nodelist[i].sciid);
		else
			/* SCI IDs may not correspond; load-balance route */
			out += src & 1;
#endif

		printf("Routing from %03x -> %03x on dim %d (lc %d)\n",
		       info->sciid, cfg_nodelist[i].sciid, dim, out);
		_add_route(cfg_nodelist[i].sciid, 0, out);

		for (lc = 1; lc <= 6; lc++) {
			/* Don't touch packets already on correct dim */
			if ((lc - 1) / 2 != dim) {
				_add_route(cfg_nodelist[i].sciid, lc, out);
			}
		}
	}

	save_scc_routing(shadow_rtbll[0], shadow_rtblm[0], shadow_rtblh[0]);
	/* Make sure all necessary links are up and working */
	int res = 1;

	if (cfg_fabric.x_size > 0) {
		if (_check_dim(0) != 0)
			return RSP_FABRIC_NOT_READY;

		res = (0 == dnc_init_lc3(info->sciid, 0, dnc_asic_mode ? 16 : 1,
		                         shadow_rtbll[1], shadow_rtblm[1],
		                         shadow_rtblh[1], shadow_ltbl[1])) && res;
		res = (0 == dnc_init_lc3(info->sciid, 1, dnc_asic_mode ? 16 : 1,
		                         shadow_rtbll[2], shadow_rtblm[2],
		                         shadow_rtblh[2], shadow_ltbl[2])) && res;
	}

	if (cfg_fabric.y_size > 0) {
		if (_check_dim(1) != 0)
			return RSP_FABRIC_NOT_READY;

		res = (0 == dnc_init_lc3(info->sciid, 2, dnc_asic_mode ? 16 : 1,
		                         shadow_rtbll[3], shadow_rtblm[3],
		                         shadow_rtblh[3], shadow_ltbl[3])) && res;
		res = (0 == dnc_init_lc3(info->sciid, 3, dnc_asic_mode ? 16 : 1,
		                         shadow_rtbll[4], shadow_rtblm[4],
		                         shadow_rtblh[4], shadow_ltbl[4])) && res;
	}

	if (cfg_fabric.z_size > 0) {
		if (_check_dim(2) != 0)
			return RSP_FABRIC_NOT_READY;

		res = (0 == dnc_init_lc3(info->sciid, 4, dnc_asic_mode ? 16 : 1,
		                         shadow_rtbll[5], shadow_rtblm[5],
		                         shadow_rtblh[5], shadow_ltbl[5])) && res;
		res = (0 == dnc_init_lc3(info->sciid, 5, dnc_asic_mode ? 16 : 1,
		                         shadow_rtbll[6], shadow_rtblm[6],
		                         shadow_rtblh[6], shadow_ltbl[6])) && res;
	}

	printf("Done with fabric setup\n");
	return (res) ? RSP_FABRIC_READY : RSP_FABRIC_NOT_READY;
}

static enum node_state validate_fabric(struct node_info *info, struct part_info *part)
{
	int res = 1;

	if (!dnc_check_fabric(info))
		return RSP_FABRIC_NOT_OK;

	/* Builder is checking that it can access all other nodes via CSR */
	if (part->builder == info->sciid) {
		printf("Validating fabric");

		for (int iter = 0; res && iter < 10; iter++) {
			printf(".");

			for (int i = 1; i < cfg_nodes; i++) {
				uint16_t node = cfg_nodelist[i].sciid;
				res = (0 == dnc_raw_write_csr(node, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00000020)) && res; /* Select APIC ATT */

				for (int j = 0; res && j < 16; j++) {
					uint32_t val;
					res = (0 == dnc_raw_read_csr(node, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + j * 4, &val)) && res;
				}
			}

			udelay(1000);
		}

		printf("done\n");
	}

	return (res) ? RSP_FABRIC_OK : RSP_FABRIC_NOT_OK;
}

int dnc_check_fabric(struct node_info *info)
{
	int res = 1;

	if (cfg_fabric.x_size > 0) {
		if (_check_dim(0) < 0)
			return 0;

		if (_verify_save_id(info->sciid, 0) != 0) {
			res = (0 == dnc_init_lc3(info->sciid, 0, dnc_asic_mode ? 16 : 1,
			                         shadow_rtbll[1], shadow_rtblm[1],
			                         shadow_rtblh[1], shadow_ltbl[1])) && res;
		}

		if (_verify_save_id(info->sciid, 1) != 0) {
			res = (0 == dnc_init_lc3(info->sciid, 1, dnc_asic_mode ? 16 : 1,
			                         shadow_rtbll[2], shadow_rtblm[2],
			                         shadow_rtblh[2], shadow_ltbl[2])) && res;
		}
	}

	if (cfg_fabric.y_size > 0) {
		if (_check_dim(1) < 0)
			return 0;

		if (_verify_save_id(info->sciid, 2) != 0) {
			res = (0 == dnc_init_lc3(info->sciid, 2, dnc_asic_mode ? 16 : 1,
			                         shadow_rtbll[3], shadow_rtblm[3]
			                         , shadow_rtblh[3], shadow_ltbl[3])) && res;
		}

		if (_verify_save_id(info->sciid, 3) != 0) {
			res = (0 == dnc_init_lc3(info->sciid, 3, dnc_asic_mode ? 16 : 1,
			                         shadow_rtbll[4], shadow_rtblm[4],
			                         shadow_rtblh[4], shadow_ltbl[4])) && res;
		}
	}

	if (cfg_fabric.z_size > 0) {
		if (_check_dim(2) < 0)
			return 0;

		if (_verify_save_id(info->sciid, 4) != 0) {
			res = (0 == dnc_init_lc3(info->sciid, 4, dnc_asic_mode ? 16 : 1,
			                         shadow_rtbll[5], shadow_rtblm[5],
			                         shadow_rtblh[5], shadow_ltbl[5])) && res;
		}

		if (_verify_save_id(info->sciid, 5) != 0) {
			res = (0 == dnc_init_lc3(info->sciid, 5, dnc_asic_mode ? 16 : 1,
			                         shadow_rtbll[6], shadow_rtblm[6],
			                         shadow_rtblh[6], shadow_ltbl[6])) && res;
		}
	}

	return res;
}

static enum node_state enter_reset(struct node_info *info __attribute__((unused)))
{
	printf("Entering reset\n");

	if (0 && _is_pic_present(dnc_card_type)) {
		int tries = 0;

		/* Reset already held?  Toggle reset logic to ensure reset
		 * reverts to know state */
		while ((dnc_read_csr(0xfff0, H2S_CSR_G0_HSSXA_STAT_1) & (1 << 8)) == 0) {
			if (tries == 0) {
				printf("HSSXA_STAT_1 is zero, toggling reset...\n");
				_pic_reset_ctrl(2);
			}

			printf("Waiting for HSSXA_STAT_1 to leave zero (try %d)...\n", tries);

			if (tries++ > 16)
				return RSP_PHY_NOT_TRAINED;

			udelay(1000);
		}

		/* Hold reset */
		_pic_reset_ctrl(2);
		tries = 0;

		while (dnc_read_csr(0xfff0, H2S_CSR_G0_HSSXA_STAT_1) & (1 << 8)) {
			if (tries == 8) {
				printf("HSSXA_STAT_1 is still one, toggling reset...\n");
				_pic_reset_ctrl(2);
			}

			printf("Waiting for HSSXA_STAT_1 to leave one (try %d)...\n", tries);

			if (tries++ > 16)
				return RSP_PHY_NOT_TRAINED;

			udelay(10000);
		}
	} else {
		/* No external reset control, simply reset all phys to start training sequence */
		for (int i = 0; i < 6; i++)
			dnc_reset_phy(i);
	}

	printf("In reset\n");
	return RSP_RESET_ACTIVE;
}

static int phy_check_status(int phy)
{
	uint32_t val;
	val = dnc_read_csr(0xfff0, H2S_CSR_G0_PHYXA_ELOG + 0x40 * phy);

	if (val & 0xf0) {
		if (verbose)
			printf("Warning: Issuing fabric phy reset due to PHY%s_ELOG 0x%x\n", _get_linkname(phy) , val);

		/* Clock compensation error, try forced retraining */
		dnc_reset_phy(phy);
		return 1 << phy;
	}

	/* Other errors */
	if (val > 0)
		return 1 << phy;

	val = dnc_read_csr(0xfff0, H2S_CSR_G0_PHYXA_LINK_STAT + 0x40 * phy);

	if (verbose & (val != 0x1fff))
		printf("Warning: Fabric phy PHY%s_LINK_STAT 0x%x\n", _get_linkname(phy) , val);

	return (val != 0x1fff) << phy;
}

static void phy_print_error(int mask)
{
	int i;
	printf("Error: Fabric phy training failure - check cables to ports");

	for (i = 0; i < 6; i++)
		if (mask & (1 << i))
			printf(" %s", _get_linkname(i));

	printf("\n");
}

static enum node_state release_reset(struct node_info *info __attribute__((unused)))
{
	int pending, i;
	printf("Releasing reset...");

	if (0 && _is_pic_present(dnc_card_type)) {
		/* Release reset */
		_pic_reset_ctrl(2);
		i = 0;

		while ((dnc_read_csr(0xfff0, H2S_CSR_G0_HSSXA_STAT_1) & (1 << 8)) == 0) {
			if (i++ > 20)
				return RSP_PHY_NOT_TRAINED;

			udelay(200);
		}
	} else {
		/* No external reset control, check that the PLL has locked */
		if ((dnc_read_csr(0xfff0, H2S_CSR_G0_HSSXA_STAT_1) & (1 << 8)) == 0) {
			printf("HSSXA_STAT_1 is zero, PLL not locked ???...\n");
			return RSP_PHY_NOT_TRAINED;
		}
	}

	/* Verify that all relevant PHYs are training */
	i = 0;

	while (1) {
		pending = 0;

		if (cfg_fabric.x_size > 0) {
			pending |= phy_check_status(0);
			pending |= phy_check_status(1);
		}

		if (cfg_fabric.y_size > 0) {
			pending |= phy_check_status(2);
			pending |= phy_check_status(3);
		}

		if (cfg_fabric.z_size > 0) {
			pending |= phy_check_status(4);
			pending |= phy_check_status(5);
		}

		if (!pending) {
			printf("done\n");
			return RSP_PHY_TRAINED;
		}

		if (i++ > 10) {
			phy_print_error(pending);
			return RSP_PHY_NOT_TRAINED;
		}

		udelay(500);
	}
}

static int lc_check_status(int lc, int dimidx)
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
			pending += lc_check_status(0, info->sciid & 0x00f);
			pending += lc_check_status(1, info->sciid & 0x00f);
		}

		if (cfg_fabric.y_size > 0) {
			pending += lc_check_status(2, info->sciid & 0x0f0);
			pending += lc_check_status(3, info->sciid & 0x0f0);
		}

		if (cfg_fabric.z_size > 0) {
			pending += lc_check_status(4, info->sciid & 0xf00);
			pending += lc_check_status(5, info->sciid & 0xf00);
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
	case CMD_ENTER_RESET:
		*rstate = enter_reset(info);
		return 1;
	case CMD_RELEASE_RESET:
		udelay(2000);
		*rstate = release_reset(info);
		return 1;
	case CMD_VALIDATE_RINGS:
		*rstate = validate_rings(info);
		return 1;
	case CMD_SETUP_FABRIC:
		*rstate = setup_fabric(info);
		udelay(2000);
		return 1;
	case CMD_VALIDATE_FABRIC:
		*rstate = validate_fabric(info, part);
		return 1;
	default:
		return 0;
	}
}

void wait_for_master(struct node_info *info, struct part_info *part)
{
	struct state_bcast rsp, cmd;
	int count, backoff;
	int go_ahead = 0;
	uint32_t last_cmd = ~0;
	uint32_t builduuid = ~0;
	int handle;
	int i;
	handle = udp_open();
	rsp.state = RSP_SLAVE_READY;
	rsp.uuid  = info->uuid;
	rsp.sciid = info->sciid;
	rsp.tid   = 0;

	for (i = 0; i < cfg_nodes; i++)
		if (cfg_nodelist[i].sciid == part->builder)
			builduuid = cfg_nodelist[i].uuid;

	count = 0;
	backoff = 1;

	while (!go_ahead) {
		if (++count >= backoff) {
			if (name_matching)
				printf("Broadcasting state: %s (sciid 0x%03x, tid %d)\n",
				       node_state_name[rsp.state], rsp.sciid, rsp.tid);
			else
				printf("Broadcasting state: %s (sciid 0x%03x, uuid %08X, tid %d)\n",
				       node_state_name[rsp.state], rsp.sciid, rsp.uuid, rsp.tid);
			udp_broadcast_state(handle, &rsp, sizeof(rsp));
			udelay(100 * backoff);

			if (backoff < 32)
				backoff = backoff * 2;

			count = 0;
		}

		int len;

		/* In order to avoid jamming, broadcast own status at least
		 * once every 2*cfg_nodes packet seen */
		for (i = 0; i < 2 * cfg_nodes; i++) {
			len = udp_read_state(handle, &cmd, sizeof(cmd));

			if (!len)
				break;

			if (len != sizeof(cmd))
				continue;

			/* printf("Got cmd packet (state %d, sciid %03x, uuid %08X, tid %d)\n",
			 *       cmd.state, cmd.sciid, cmd.uuid, cmd.tid); */
			if ((!name_matching && (cmd.uuid == builduuid)) ||
			    ( name_matching && (cmd.sciid == part->builder))) {
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
					udp_broadcast_state(handle, &cmd, sizeof(cmd));
					go_ahead = 1;
					break;
				}
			}
		}
	}
}

