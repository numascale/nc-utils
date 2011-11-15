// $Id:$
// This source code including any derived information including but
// not limited by net-lists, fpga bit-streams, and object files is the
// confidential and proprietary property of
// 
// Numascale AS
// Enebakkveien 302A
// NO-1188 Oslo
// Norway
// 
// Any unauthorized use, reproduction or transfer of the information
// provided herein is strictly prohibited.
// 
// Copyright © 2008-2011
// Numascale AS Oslo, Norway. 
// All Rights Reserved.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "dnc-regs.h"
#include "dnc-types.h"
#include "dnc-access.h"
#include "dnc-fabric.h"
#include "dnc-config.h"

#include "dnc-commonlib.h"

extern unsigned char sleep(unsigned int msec);

// -------------------------------------------------------------------------

char *config_file_name = "nc-config/fabric.json";
char *next_label = "menu.c32";
char *microcode_path = "";
int sync_mode = 1;
int init_only = 0;
int route_only = 0;
int enable_nbmce = 0;
int enable_nbwdt = 0;
int enable_selftest = 1;
int force_probefilteroff = 0;
int force_ganged = 0;
int disable_smm = 0;
int renumber_bsp = 0;
int forwarding_mode = 3; /* 0=store-and-forward, 1-2=intermediate, 3=full cut-through */

// Structs to hold DIMM configuration from SPD readout.

struct dimm_config {
    u8 addr_pins;
    u8 column_size;
    u8 cs_map;
    u8 width;
    int mem_size; // Size of DIMM in GByte powers of 2
};

int nc_neigh = -1, nc_neigh_link = -1;
static struct dimm_config dimms[2]; // 0 - MCTag, 1 - CData

static int read_spd_info(int cdata, struct dimm_config *dimm)
{
    u8 addr_bits;
    u16 spd_addr = cdata ? 1 : 0;
    u32 reg;
    u8 mdata[20];

    reg = dnc_read_csr(0xfff0, (1<<12) + (spd_addr<<8) +  0); // Read SPD location 0, 1, 2, 3
    if (((reg >> 8) & 0xff) != 0x08) {
	printf("ERROR: Couldn't find a DDR2 SDRAM memory module attached to the %s memory controller\n",
	       cdata ? "CData" : "MCTag");
	return -1;
    }
    dimm->addr_pins = 16 - (reg & 0xf); // Number of Row address bits (max 16)
    addr_bits = (reg & 0xf);
    
    reg = dnc_read_csr(0xfff0, (1<<12) + (spd_addr<<8) +  4); // Read SPD location 4, 5, 6, 7
    dimm->column_size = 13 - ((reg >> 24) & 0xf); // Number of Column address bits (max 13)
    dimm->cs_map = ((reg >> 16) & 1) ? 3 : 1; // Single or Dual rank
    addr_bits = addr_bits + ((reg >> 24) & 0xf) + ((reg >> 16) & 1);

    reg = dnc_read_csr(0xfff0, (1<<12) + (spd_addr<<8) +  8); // Read SPD location 8, 9, 10, 11
    if (!(reg & 2)) {
	printf("ERROR: Unsupported non-ECC %s DIMM!\n", cdata ? "CData" : "MCTag");
	return -1;
    }

    reg = dnc_read_csr(0xfff0, (1<<12) + (spd_addr<<8) + 12); // Read SPD location 12, 13, 14, 15
    dimm->width = (reg >> 8) & 0xff;
    if ((dimm->width != 4) && (dimm->width != 8)) {
	printf("ERROR: Unsupported %s SDRAM width %d!\n", cdata ? "CData" : "MCTag", dimm->width);
	return -1;
    }
    
    reg = dnc_read_csr(0xfff0, (1<<12) + (spd_addr<<8) + 20); // Read SPD location 20, 21, 22, 23
    if (!(reg & 0x11000000)) {
	printf("ERROR: Unsupported non-Registered %s DIMM!\n", cdata ? "CData" : "MCTag");
	return -1;
    }

    ((u32 *)mdata)[0] = u32bswap(dnc_read_csr(0xfff0, (1<<12) + (spd_addr<<8) + 72)); // Read SPD location 72, 73, 74, 75
    ((u32 *)mdata)[1] = u32bswap(dnc_read_csr(0xfff0, (1<<12) + (spd_addr<<8) + 76)); // Read SPD location 76, 77, 78, 79
    ((u32 *)mdata)[2] = u32bswap(dnc_read_csr(0xfff0, (1<<12) + (spd_addr<<8) + 80)); // Read SPD location 80, 81, 82, 83
    ((u32 *)mdata)[3] = u32bswap(dnc_read_csr(0xfff0, (1<<12) + (spd_addr<<8) + 84)); // Read SPD location 84, 85, 86, 87
    ((u32 *)mdata)[4] = u32bswap(dnc_read_csr(0xfff0, (1<<12) + (spd_addr<<8) + 88)); // Read SPD location 88, 89, 90, 91

    mdata[19] = 0;
    printf("%s is a %s module (x%d, %d MByte)\n", cdata ? "CData" : "MCTag", &mdata[1], dimm->width, 1<<(addr_bits - 14));

    switch (addr_bits) {
	case 28: dimm->mem_size = 4; break; // 16G
	case 27: dimm->mem_size = 3; break; //  8G
	case 26: dimm->mem_size = 2; break; //  4G
	case 25: dimm->mem_size = 1; break; //  2G
	case 24: dimm->mem_size = 0; break; //  1G
	default: dimm->mem_size = 0; printf("ERROR: Unsupported %s DIMM SIZE (%d MByte)\n",
					    cdata ? "CData" : "MCTag", 1<<(addr_bits - 14)); return -1;
    }

    return 0;
}

#include "../interface/mctr_define_register_C.h"
#include "../interface/regconfig_200_cl4_bl4_genericrdimm.h"

static void _denali_mctr_reset(int cdata, struct dimm_config *dimm)
{
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(  0<<2), DENALI_CTL_00_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(  1<<2), DENALI_CTL_01_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(  2<<2), DENALI_CTL_02_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(  3<<2), DENALI_CTL_03_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(  4<<2), DENALI_CTL_04_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(  5<<2), DENALI_CTL_05_DATA);
    
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(  7<<2), DENALI_CTL_07_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(  8<<2), DENALI_CTL_08_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(  9<<2), DENALI_CTL_09_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 10<<2), DENALI_CTL_10_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 11<<2), DENALI_CTL_11_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 12<<2),
		  (DENALI_CTL_12_DATA & ~(3<<16)) | ((dimm->width == 8 ? 1 : 0)<<16));
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 13<<2),
                  (DENALI_CTL_13_DATA & ~(0x7<<16)) | ((dimm->addr_pins & 0x7)<<16));
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 14<<2),
                  (DENALI_CTL_14_DATA & ~(0x7<<16)) | ((dimm->column_size & 0x7)<<16));
    
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 16<<2), DENALI_CTL_16_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 17<<2), DENALI_CTL_17_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 18<<2), DENALI_CTL_18_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 19<<2),
                  (DENALI_CTL_19_DATA & ~(0x3<<24)) | ((dimm->cs_map & 0x3)<<24));
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 20<<2), DENALI_CTL_20_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 21<<2), DENALI_CTL_21_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 22<<2), DENALI_CTL_22_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 23<<2), DENALI_CTL_23_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 24<<2), DENALI_CTL_24_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 25<<2), DENALI_CTL_25_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 26<<2), DENALI_CTL_26_DATA);
    
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 28<<2), DENALI_CTL_28_DATA);
    
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 30<<2), DENALI_CTL_30_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 31<<2), DENALI_CTL_31_DATA);

    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 33<<2), DENALI_CTL_33_DATA);
    
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 35<<2), DENALI_CTL_35_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 36<<2), DENALI_CTL_36_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 37<<2), DENALI_CTL_37_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 38<<2), DENALI_CTL_38_DATA);

    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 46<<2), DENALI_CTL_46_DATA);
    
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 48<<2), DENALI_CTL_48_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 49<<2), DENALI_CTL_49_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 50<<2), DENALI_CTL_50_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 51<<2), DENALI_CTL_51_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 52<<2), DENALI_CTL_52_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 53<<2), DENALI_CTL_53_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 54<<2), DENALI_CTL_54_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 55<<2), DENALI_CTL_55_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 56<<2), DENALI_CTL_56_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 57<<2), DENALI_CTL_57_DATA);
    
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 59<<2), DENALI_CTL_59_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 60<<2), DENALI_CTL_60_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 61<<2), DENALI_CTL_61_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 62<<2), DENALI_CTL_62_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 63<<2), DENALI_CTL_63_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 64<<2), DENALI_CTL_64_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 65<<2), DENALI_CTL_65_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 66<<2), DENALI_CTL_66_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 67<<2), DENALI_CTL_67_DATA);
    
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 69<<2), DENALI_CTL_69_DATA);
    
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 94<<2), DENALI_CTL_94_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 95<<2), DENALI_CTL_95_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 96<<2), DENALI_CTL_96_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 97<<2), DENALI_CTL_97_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 98<<2), DENALI_CTL_98_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+( 99<<2), DENALI_CTL_99_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(100<<2), DENALI_CTL_100_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(101<<2), DENALI_CTL_101_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(102<<2), DENALI_CTL_102_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(103<<2), DENALI_CTL_103_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(104<<2), DENALI_CTL_104_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(105<<2), DENALI_CTL_105_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(106<<2), DENALI_CTL_106_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(107<<2), DENALI_CTL_107_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(108<<2), DENALI_CTL_108_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(109<<2), DENALI_CTL_109_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(110<<2), DENALI_CTL_110_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(111<<2), DENALI_CTL_111_DATA);
    
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(184<<2), DENALI_CTL_184_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(185<<2), DENALI_CTL_185_DATA);
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(186<<2), DENALI_CTL_186_DATA);
    
    dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(188<<2), DENALI_CTL_188_DATA);
}

u32 dnc_check_mctr_status(int cdata)
{
    u32 val;
    u32 ack = 0;
    const char *me = cdata ? "CData" : "MCTag";

    if (!dnc_asic_mode)
        return 0;
    
    val = dnc_read_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(INT_STATUS_ADDR<<2));
    if (val & 0x001) {
        printf("ERROR: %s Single access outside the defined Physical memory space\n", me);
        ack |= 0x001;
    }
    if (val & 0x002) {
        printf("ERROR: %s Multiple access outside the defined Physical memory space\n", me);
        ack |= 0x002;
    }
    if (val & 0x004) {
        printf("ERROR: %s Single correctable ECC event detected\n", me);
        ack |= 0x004;
    }
    if (val & 0x008) {
        printf("ERROR: %s Multiple correctable ECC event detected\n", me);
        ack |= 0x008;
    }
    if (val & 0x010) {
        printf("ERROR: %s Single uncorrectable ECC event detected\n", me);
        ack |= 0x010;
    }
    if (val & 0x020) {
        printf("ERROR: %s Multiple uncorrectable ECC event detected\n", me);
        ack |= 0x020;
    }
    if (val & 0xf80) {
        printf("ERROR: %s error interrupts detected INT_STATUS = %03x\n", me, val & 0xfff);
        ack |= (val & 0xf80);
    }
    
    if (ack) {
        ack |= dnc_read_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(INT_ACK_ADDR<<2));
        dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(INT_ACK_ADDR<<2), ack);
    }
    
    tsc_wait(100);

    return val;
}

int dnc_init_caches(void) {
    u32 val;
    int cdata;

    for (cdata = 0; cdata < 2; cdata++) {
        char *name = cdata ? "CData" : "MCTag";
        int mem_size = dimms[cdata].mem_size;
        
        if (!dnc_asic_mode) {
            // On FPGA, just check that the PHY is initialized
            val = dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_STATR : H2S_CSR_G4_MCTAG_COM_STATR);
            if (!(val & 0x1000)) {
                printf("%s Controller not calibrated (%x)\n", name, val);
                return -1;
            }
        } else {
            val = dnc_read_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00) + (START_ADDR<<2)); // Read the Denali START parameter
            if (((val >> START_OFFSET) & 1) == 0) {
                printf("RESET and Load Denali Controller, Denali PHY and external DDR2 Ram (MODE- EMODE-Register)\n");
                _denali_mctr_reset(cdata, &dimms[cdata]);
                printf("START Denali Controller\n");
                val = dnc_read_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(START_ADDR<<2));
                dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(START_ADDR<<2), val | (1<<START_OFFSET));
                
                printf("Polling the Denali Interrupt Status Register\n");
                do {
                    val = dnc_check_mctr_status(cdata);
                } while (!(val & 0x40));
                
                // Reset DRAM initialization complete interrupt
                val = dnc_read_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(INT_ACK_ADDR<<2));
                dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(INT_ACK_ADDR<<2), val | 0x40);
                
                // Mask DRAM initialization complete interrupt
                // Mask the out-of-bounds interrupts on CData since they happen all the time
                val = dnc_read_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(INT_MASK_ADDR<<2));
                dnc_write_csr(0xfff0, (cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00)+(INT_MASK_ADDR<<2),
                              val | ((cdata ? 0x43 : 0x40)<<INT_MASK_OFFSET));
                
                val = dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_ERROR_STATR : H2S_CSR_G4_MCTAG_ERROR_STATR);
                if (!(val & 0x20)) {
                    printf("ERROR: No Denali interrupt seen by H2S (%08x)\n", val);
                    return -1;
                }
                dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_ERROR_STATR : H2S_CSR_G4_MCTAG_ERROR_STATR, val); // Clear Error status
            } else {
                val = dnc_check_mctr_status(cdata);
            }
            printf("Initialization of Denali controller done\n");
        }

	if ((dnc_asic_mode && dnc_chip_rev >= 2) ||
	    (!dnc_asic_mode && ((dnc_chip_rev>>16) >= 6251))) {
	    int tmp = mem_size;

	    // New Rev2 MCTag setting for supporting larger local memory.

	    if (!cdata) {
		if (dnc_asic_mode || (!dnc_asic_mode && ((dnc_chip_rev>>16) >= 6254))) {
		    if        (dimms[0].mem_size == 0 && dimms[1].mem_size == 1) { // 1GB MCTag_Size  2GB Remote Cache   24GB Coherent Local Memory
			tmp = 0;
		    } else if (dimms[0].mem_size == 0 && dimms[1].mem_size == 2) { // 1GB MCTag_Size  4GB Remote Cache   16GB Coherent Local Memory
			tmp = 1;
		    } else if (dimms[0].mem_size == 1 && dimms[1].mem_size == 1) { // 2GB MCTag_Size  2GB Remote Cache   56GB Coherent Local Memory
			tmp = 2;
		    } else if (dimms[0].mem_size == 1 && dimms[1].mem_size == 2) { // 2GB MCTag_Size  4GB Remote Cache   48GB Coherent Local Memory
			tmp = 3;
		    } else if (dimms[0].mem_size == 2 && dimms[1].mem_size == 2) { // 4GB MCTag_Size  4GB Remote Cache  112GB Coherent Local Memory
			tmp = 4;
		    } else if (dimms[0].mem_size == 2 && dimms[1].mem_size == 3) { // 4GB MCTag_Size  8GB Remote Cache   96GB Coherent Local Memory
			tmp = 5;
		    } else if (dimms[0].mem_size == 3 && dimms[1].mem_size == 2) { // 8GB MCTag_Size  4GB Remote Cache  240GB Coherent Local Memory
			tmp = 6;
		    } else if (dimms[0].mem_size == 3 && dimms[1].mem_size == 3) { // 8GB MCTag_Size  8GB Remote Cache  224GB Coherent Local Memory
			tmp = 7;
		    } else {
			printf("ERROR: Unsupported MCTag/CData combination (%d/%d GByte)\n",
			       (1<<dimms[0].mem_size), (1<<dimms[1].mem_size));
			return -1;
		    }
		}

		printf("%dGB MCTag_Size  %dGB Remote Cache  %3dGB Max Coherent Local Memory\n",
		       (1<<dimms[0].mem_size), (1<<dimms[1].mem_size), (1U<<(5+dimms[0].mem_size)) - (1U<<(2+dimms[1].mem_size)));
	    }

	    val = dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_CTRLR : H2S_CSR_G4_MCTAG_COM_CTRLR);
	    dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_CTRLR : H2S_CSR_G4_MCTAG_COM_CTRLR,
			  (val & ~7) | (1<<12) | ((tmp & 7)<<4)); // DRAM_AVAILABLE, MEMSIZE[2:0]
	} else { // Older than Rev2
	    // On Rev1 and older there's a direct relationship between the MTag size and RCache size.

            if (cdata && mem_size == 0) {
                printf("ERROR: Unsupported CData SIZE (%d GByte)\n",
                       (1<<mem_size));
                return -1;
            }

	    val = dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_CTRLR : H2S_CSR_G4_MCTAG_COM_CTRLR);
	    dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_CTRLR : H2S_CSR_G4_MCTAG_COM_CTRLR,
			  (val & ~7) | (1<<12) | (1<<7) | ((mem_size & 7)<<4)); // DRAM_AVAILABLE, FTagBig, MEMSIZE[2:0]
	}

        // Initialize DRAM
        printf("Initializing %s (%d GByte)\n", cdata ? "CData" : "MCTag", 1<<mem_size);
        val = dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_MAINTR : H2S_CSR_G4_MCTAG_MAINTR);
        dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_MAINTR : H2S_CSR_G4_MCTAG_MAINTR, val | (1<<4));
        do {
            tsc_wait(100);
            val = dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_STATR : H2S_CSR_G4_MCTAG_COM_STATR);
        } while (!(val & 2));
        val = dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_MAINTR : H2S_CSR_G4_MCTAG_MAINTR);
        dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_MAINTR : H2S_CSR_G4_MCTAG_MAINTR, val & ~(1<<4));

        if (cdata) {
            printf("Setting RCache size to %d GByte\n", 1<<mem_size);
            // Set the cache size in HReq and MIU
            val = dnc_read_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL);
            dnc_write_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL, (val & ~(3<<26)) | ((mem_size-1)<<26)); // [27:26] Cache size: 0 - 2GB, 1 - 4GB, 2 - 8GB, 3 - 16GB
            dnc_write_csr(0xfff0, H2S_CSR_G0_MIU_CACHE_SIZE, mem_size+1); //  ((2^(N-1))*1 Gbyte)
        }
    }

    // Special FPGA considerations for Ftag SRAM
    if (!dnc_asic_mode)  {
	if ((dnc_chip_rev>>16) < 6233) {
	    printf("Disabling FTag SRAM\n");
	    dnc_write_csr(0xfff0, H2S_CSR_G2_SRAM_MODE, 0x00000001);
	} else {
	    printf("FPGA Revision %d_%d detected, no FTag SRAM\n", dnc_chip_rev>>16, dnc_chip_rev&0xffff);
	}
    } else {
        // ASIC
	if (dnc_chip_rev < 2) {
	    val = dnc_read_csr(0xfff0, H2S_CSR_G2_DDL_STATUS);
	    if (((val >> 24) & 0xff) != 0x3f)
		printf("Waiting for SRAM clock calibration!\n");
	    while (((val >> 24) & 0xff) != 0x3f) {
		tsc_wait(100);
		if ((val & 0xc000000) != 0) {
		    printf("SRAM clock calibration overflow detected (%08x)!\n", val);
		    return -1;
		}
	    }
	    printf("SRAM clock calibration complete!\n");
	    // Zero out FTag SRAM
	    dnc_write_csr(0xfff0, H2S_CSR_G2_SRAM_MODE, 0x00000002);
	    while (1) { 
		tsc_wait(100);
		val = dnc_read_csr(0xfff0, H2S_CSR_G2_FTAG_STATUS);
		if ((val & 1) == 0)
		    break;
	    }
	    printf("Enabling FTag SRAM\n");
	    dnc_write_csr(0xfff0, H2S_CSR_G2_SRAM_MODE, 0x00000000);
	} else {
	    printf("ASIC Revision %d detected, no FTag SRAM\n", dnc_chip_rev);
	}
    }

    return 0;
}

int cpu_family(u16 scinode, u8 node)
{
    u32 val;
    int family, model, stepping;

    val = dnc_read_conf(scinode, 0, 24+node, NB_FUNC_MISC, 0xfc);

    family = ((val >> 20) & 0xf) + ((val >> 8) & 0xf);
    model = ((val >> 12) & 0xf0) | ((val >> 4) & 0xf);
    stepping = val & 0xf;

    return (family << 16) | (model << 8) | stepping;
}

void add_extd_mmio_maps(u16 scinode, u8 node, u8 idx, u64 start, u64 end, u8 dest)
{
    u32 val;

    if ((cpu_family(scinode, node) >> 16) < 0x15) {
	u64 mask;

	mask = 0;
	start = start >> 27;
	end   = end >> 27;
	while ((start | mask) != (end | mask))
	    mask = (mask << 1) | 1;

	/* Make sure CHtExtAddrEn, ApicExtId and ApicExtBrdCst are enabled */
	val = dnc_read_conf(scinode, 0, 24+node, NB_FUNC_HT, 0x68);
	dnc_write_conf(scinode, 0, 24+node, NB_FUNC_HT, 0x68,
		       val | (1<<25) | (1<<18) | (1<<17));

	/* Set ExtMmioMapAddSel granularity to 128M */
	val = dnc_read_conf(scinode, 0, 24+node,  NB_FUNC_HT, 0x168);
	dnc_write_conf(scinode, 0, 24+node, NB_FUNC_HT, 0x168, (val & ~0x300) | 0x200);

	/* printf("[%03x#%d] Adding Extd MMIO map #%d %016llx-%016llx (%08llx/%08llx) to HT#%d\n",
	   scinode, node, idx, start, end, start, mask, dest); */
	dnc_write_conf(scinode, 0, 24+node, NB_FUNC_MAPS, 0x110, (2 << 28) | idx);
	dnc_write_conf(scinode, 0, 24+node, NB_FUNC_MAPS, 0x114, (start << 8) | dest);
	dnc_write_conf(scinode, 0, 24+node, NB_FUNC_MAPS, 0x110, (3 << 28) | idx);
	dnc_write_conf(scinode, 0, 24+node, NB_FUNC_MAPS, 0x114, (mask << 8) | 1);
    }
    else {
	/* From family 15h, the Extd MMIO maps are deprecated in favor
	 * of extending the legacy MMIO maps with a "base/limit high"
	 * register set.  To avoid trampling over existing mappings,
	 * use the (also new) 9-12 mapping entries when invoked here. */
	if (idx > 4) {
	    printf("add_extd_mmio_maps: index (%d) out of range for family 15h!\n", idx);
	    return;
	}
	printf("[%03x#%d] Adding MMIO map #%d %016llx-%016llx to HT#%d\n",
	       scinode, node, idx+8, start, end, dest);
	dnc_write_conf(scinode, 0, 24+node, NB_FUNC_MAPS, 0x1a0 + idx * 8, 0);

	dnc_write_conf(scinode, 0, 24+node, NB_FUNC_MAPS, 0x1c0 + idx * 4,
		       ((end >> 40) << 16) | (start >> 40));
	dnc_write_conf(scinode, 0, 24+node, NB_FUNC_MAPS, 0x1a4 + idx * 8,
		       ((end >> 16) << 8) | dest);
	dnc_write_conf(scinode, 0, 24+node, NB_FUNC_MAPS, 0x1a0 + idx * 8,
		       ((start >> 16) << 8) | 3);
    }
}

void del_extd_mmio_maps(u16 scinode, u8 node, u8 idx)
{
    u32 val;

    /* Make sure CHtExtAddrEn, ApicExtId and ApicExtBrdCst are enabled */
    val = dnc_read_conf(scinode, 0, 24+node, NB_FUNC_HT, 0x68);
    dnc_write_conf(scinode, 0, 24+node, NB_FUNC_HT, 0x68,
                   val | (1<<25) | (1<<18) | (1<<17));

    /* printf("[%03x#%d] Removing Extd MMIO map #%d\n", scinode, node, idx); */
    dnc_write_conf(scinode, 0, 24+node, NB_FUNC_MAPS, 0x110, (2 << 28) | idx);
    dnc_write_conf(scinode, 0, 24+node, NB_FUNC_MAPS, 0x114, 0);
    dnc_write_conf(scinode, 0, 24+node, NB_FUNC_MAPS, 0x110, (3 << 28) | idx);
    dnc_write_conf(scinode, 0, 24+node, NB_FUNC_MAPS, 0x114, 0);
}

#ifdef __i386
static u32 get_phy_register(int node, int link, int idx, int direct)
{
    int base = 0x180 + link * 8;
    int i;
    u32 reg;
    cht_write_config(node, 4, base, idx | (direct << 29));
    for (i = 0; i < 1000; i++) {
        reg = cht_read_config(node, 4, base);
        if (reg & 0x80000000)
            return cht_read_config(node, 4, base + 4);
    }
    printf("Read from phy register HT#%d F4x%x idx %x did not complete!\n",
           node, base, idx);
    return 0;
}
#endif /* __i386 */

#ifdef UNUSED
static void set_phy_register(int node, int link, int idx, int direct, u32 val)
{

    int base = 0x180 + link * 8;
    int i;
    u32 reg;
    cht_write_config(node, 4, base + 4, val);
    cht_write_config(node, 4, base, idx | (direct << 29) | (1 << 30));
    for (i = 0; i < 1000; i++) {
        reg = cht_read_config(node, 4, base);
        if (reg & 0x80000000)
            return;
    }
    printf("Write to phy register HT#%d F4x%x idx %x did not complete!\n",
           node, base, idx);
}

static void reorganize_mmio(int nc)
{
    /* Stub for now */
    u64 tom;
    u64 mmio_start;
    u64 base, lim;
    int i;
    tom = dnc_rdmsr(MSR_TOPMEM);
    mmio_start = ~0;
    printf("MSR_TOPMEM : %llx\n", tom);
    for (i = 0; i < 8; i++) {
	base = cht_read_config(0, 1, 0x80 + i * 8);
	lim  = cht_read_config(0, 1, 0x84 + i * 8);
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

#ifdef __i386
static void print_phy_gangs(char *name, int base)
{
    printf("%s:\n", name);
    printf("- CAD[ 7:0],CTL0,CLK0=%08Xh\n", get_phy_register(nc_neigh, nc_neigh_link, base, 0));
    printf("- CAD[15:8],CTL1,CLK1=%08Xh\n", get_phy_register(nc_neigh, nc_neigh_link, base + 0x10, 0));
}

static void print_phy_lanes(char *name, int base, int clk)
{
    int i;

    printf("%s:\n", name);
    for (i = 0; i <= 15; i++)
	printf("- CAD[%2d]=%08Xh\n", i, get_phy_register(nc_neigh, nc_neigh_link, base + i * 0x80, 1));
    printf("- CTL[ 0]=%08Xh\n", get_phy_register(nc_neigh, nc_neigh_link, base + 16 * 0x80, 1));
    if (clk)
	printf("- CLK[ 0]=%08Xh\n", get_phy_register(nc_neigh, nc_neigh_link, base + 17 * 0x80, 1));
    printf("- CTL[ 1]=%08Xh\n", get_phy_register(nc_neigh, nc_neigh_link, base + 18 * 0x80, 1));
    if (clk)
	printf("- CLK[ 1]=%08Xh\n", get_phy_register(nc_neigh, nc_neigh_link, base + 19 * 0x80, 1));
}

static void cht_print(void)
{
    print_phy_gangs("link phy impedance", 0xc0);
    print_phy_gangs("link phy receiver loop filter", 0xc1);
    print_phy_gangs("link phy timing margin", 0xc3);
    print_phy_gangs("link phy DFE and DFR control", 0xc4);
    print_phy_gangs("link phy transmit control", 0xc5);
    print_phy_gangs("link FIFO read pointer optimisation", 0xcf);

    printf("link phy compensation and calibration control 1: %08Xh\n", get_phy_register(nc_neigh, nc_neigh_link, 0xe0, 0));
    printf("link phy PLL control: %08Xh\n", get_phy_register(nc_neigh, nc_neigh_link, 0xe3, 0));
    printf("link BIST control: %08Xh\n", get_phy_register(nc_neigh, nc_neigh_link, 0x100, 0));

    print_phy_lanes("link phy DFE and DFR control", 0x4006, 0);
    print_phy_lanes("link phy DLL control", 0x400a, 0);
    print_phy_lanes("link phy transmit control", 0x6000, 1);
    printf("link phy transmit clock phase control:\n- CLKOUT[ 0]=%08Xh\n- CLKOUT[ 1]=%08Xh\n", get_phy_register(nc_neigh, nc_neigh_link, 0x6884, 1), get_phy_register(nc_neigh, nc_neigh_link, 0x6984, 1));

/* these cause hangs
    print_phy_lanes("link phy receiver DLL control and test 5", 0x400f, 0);
    print_phy_lanes("link phy receiver process control", 0x4011, 0);
    print_phy_lanes("link phy tx deemphasis and margin test control", 0x600c, 1); */
}
#endif /* __i386 */

static void ht_optimize_link(int nc, int rev, int asic_mode)
{
#ifndef __i386
    printf("(Only doing HT reconfig in 32-bit mode.)\n");
    return;
#else
    int reboot = 0;
    int ganged;
    int neigh;
    int link;
    int next, i;
    u32 rqrt, val;

    /* Start looking from node 0 */
    neigh = 0;
    while (1) {
	next = 0;
	rqrt = cht_read_config(neigh, NB_FUNC_HT, 0x40 + 4 * nc) & 0x1f;
	/* Look for other CPUs routed on same link as NC */
	for (i = 0; i < nc; i++) {
	    if (rqrt == (cht_read_config(neigh, NB_FUNC_HT, 0x40 + 4 * i) & 0x1f)) {
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

    ganged = cht_read_config(neigh, 0, 0x170 + link * 4) & 1;
    
    printf("Found link to NC on HT#%d L%d (%s)\n", neigh, link,
           ganged ? "GANGED" : "UNGANGED");

    val = cht_read_config(neigh, NB_FUNC_HT, 0x84 + link * 0x20);
    printf("HT#%d L%d Link Control       : %08x\n", neigh, link, val);
    val = cht_read_config(neigh, NB_FUNC_HT, 0x88 + link * 0x20);
    printf("HT#%d L%d Link Freq/Revision : %08x\n", neigh, link, val);
    val = cht_read_config(neigh, 0, 0x170 + link * 4);
    printf("HT#%d L%d Link Ext. Control  : %08x\n", neigh, link, val);
    val = get_phy_register(nc_neigh, nc_neigh_link, 0xe0, 0); /* link phy compensation and calibration control 1 */
    printf("HT#%d L%d Link Phy Settings  : RTT=%02x RON=%02x\n", neigh, link, (val >> 23) & 0x1f, (val >> 18) & 0x1f);

    printf("Checking width/freq ");

    /* Set T0Time to max */
    val = cht_read_config(neigh, NB_FUNC_HT, 0x16c);
    cht_write_config(neigh, NB_FUNC_HT, 0x16c, (val & ~0x3f) | 0x3a);

    /* Make sure link towards NC is ganged, disable LS2En */
    /* XXX: Why do we alter this, optimally the link should be detected as
       ganged anyway if we set our CTL[1] terminations correctly ?? */
    if (force_ganged) {
	val = cht_read_config(neigh, 0, 0x170 + link * 4);
	cht_write_config(neigh, 0, 0x170 + link * 4, (val & ~0x100) | 1);
    }

    /* For ASIC revision 1 and later, optimize width (16b) */
    /* For FPGA revision 6022 and later, optimize width (16b) */
    val = cht_read_config(neigh, NB_FUNC_HT, 0x84 + link * 0x20);
    if (force_ganged ||
	(ganged && ((val >> 16) == 0x11) && ((asic_mode && rev >= 1))))
    {
	printf("*");
	tsc_wait(50);
	val = cht_read_config(neigh, NB_FUNC_HT, 0x84 + link * 0x20);
	printf(".");
	if ((val >> 24) != 0x11) {
	    printf("<CPU width>");
	    tsc_wait(50);
	    cht_write_config(neigh, NB_FUNC_HT, 0x84 + link * 0x20,
			     (val & 0x00ffffff) | 0x11000000);
	    reboot = 1;
	}
	tsc_wait(50);
	printf(".");
	val = cht_read_config_nc(nc, 0, neigh, link,
                                 H2S_CSR_F0_LINK_CONTROL_REGISTER);
	printf(".");
	if ((val >> 24) != 0x11) {
	    printf("<NC width>");
	    tsc_wait(50);
	    cht_write_config_nc(nc, 0, neigh, link,
                                H2S_CSR_F0_LINK_CONTROL_REGISTER,
                                (val & 0x00ffffff) | 0x11000000);
	    reboot = 1;
	}
	printf(".");
    }

    /* On ASIC optimize link frequency (800MHz) */
    if (asic_mode) {
        printf("+");
        tsc_wait(50);
        val = cht_read_config(neigh, NB_FUNC_HT, 0x88 + link * 0x20);
        printf(".");
        if (((val >> 8) & 0xf) != 0x5) {
            printf("<CPU freq>");
            tsc_wait(50);
            cht_write_config(neigh, NB_FUNC_HT, 0x88 + link * 0x20,
                             (val & ~0xf00) | 0x500);
            reboot = 1;
        }
        tsc_wait(50);
        printf(".");
        val = cht_read_config_nc(nc, 0, neigh, link,
                                 H2S_CSR_F0_LINK_FREQUENCY_REVISION_REGISTER);
        printf(".");
        if (((val >> 8) & 0xf) != 0x5) {
            printf("<NC freq>");
            tsc_wait(50);
            cht_write_config_nc(nc, 0, neigh, link,
                                H2S_CSR_F0_LINK_FREQUENCY_REVISION_REGISTER,
                                (val & ~0xf00) | 0x500);
            reboot = 1;
        }
    }
    printf(". done.\n");

    if (ht_testmode & HT_TESTMODE_PRINT) {
	cht_print();
    }

    if (reboot) {
	printf("Rebooting to make new link settings effective...\n");
	reset_cf9(2, nc - 1);
    }
#endif
}

#ifdef __i386
static void disable_probefilter(int nodes)
{
    u32 val;
    u32 scrub[8];
    int i;
    
    val = cht_read_config(0, NB_FUNC_MISC, 0x1d4);
    /* Probe filter not active? */
    if ((val & 3) == 0)
	return;

    printf("HT#0 F3x1d4: %08x\n", val);
    printf("Probe filter active; disabling (%d)...\n", nodes);

    /* 1. Disable the L3 and DRAM scrubbers on all nodes in the system:
       - F3x58[L3Scrub]=00h.
       - F3x58[DramScrub]=00h.
       - F3x5C[ScrubRedirEn]=0. */
    for (i = 0; i <= nodes; i++) {
	scrub[i] = cht_read_config(i, NB_FUNC_MISC, 0x58);
	cht_write_config(i, NB_FUNC_MISC, 0x58, scrub[i] & ~0x1f00001f);
	val = cht_read_config(i, NB_FUNC_MISC, 0x5c);
	cht_write_config(i, NB_FUNC_MISC, 0x5c, val & ~1);
    }

    /* 2.  Wait 40us for outstanding scrub requests to complete. */
    printf(".");
	
    /* 3.  Disable all cache activity in the system by setting
       CR0.CD for all active cores in the system. */
    disable_cache();
    /* 4.  Issue WBINVD on all active cores in the system. */
    asm volatile("wbinvd" ::: "memory");
    printf(".");

    /* 5.  Set F3x1C4[L3TagInit]=1. */
    for (i = 0; i <= nodes; i++) {
	val = cht_read_config(i, NB_FUNC_MISC, 0x1c4);
	cht_write_config(i, NB_FUNC_MISC, 0x1c4, val | (1 << 31));
    }
    printf(".");

    /* 6.  Wait for F3x1C4[L3TagInit]=0. */
    for (i = 0; i <= nodes; i++) {
	do {
	    val = cht_read_config(i, NB_FUNC_MISC, 0x1c4);
	    printf("<%d>", i);
	} while (val & (1 << 31));
    }
    printf(".");

    /* 7.  Set F3x1D4[PFMode]=00b. */
    for (i = 0; i <= nodes; i++) {
	val = cht_read_config(i, NB_FUNC_MISC, 0x1d4);
	cht_write_config(i, NB_FUNC_MISC, 0x1d4, val & ~3);
    }
    printf(".");

    for (i = 0; i <= nodes; i++) {
	val = cht_read_config(i, NB_FUNC_MISC, 0x1d4);
	printf("<%d:%08x>", i, val);
    }
    printf(".");

    /* 8.  Enable all cache activity in the system by clearing
       CR0.CD for all active cores in the system. */
    enable_cache();
    printf(".");

    /* 9. Restore L3 and DRAM scrubber register values. */
    for (i = 0; i <= nodes; i++) {
	cht_write_config(i, NB_FUNC_MISC, 0x58, scrub[i]);
	val = cht_read_config(i, NB_FUNC_MISC, 0x5c);
	cht_write_config(i, NB_FUNC_MISC, 0x5c, val | 1);
    }
    printf("*\n");
}

static void disable_link(int node, int link)
{
    u32 val;
    val = cht_read_config(node, NB_FUNC_HT, 0x16c);
    cht_write_config(node, NB_FUNC_HT, 0x16c, val & ~(1<<8));
    printf("HT#%d F0x16c: %08x\n", node, cht_read_config(node, NB_FUNC_HT, 0x16c));
    printf("HT#%d F0x%02x: %08x\n", node, 0x84 + 0x20 * link,
	   cht_read_config(node, NB_FUNC_HT, 0x84 + 0x20 * link));
    val = cht_read_config(node, NB_FUNC_HT, 0x84 + 0x20 * link);
    cht_write_config(node, NB_FUNC_HT, 0x84 + 0x20 * link, val | 0xc0);
    printf("HT#%d F0x%02x: %08x\n", node, 0x84 + 0x20 * link,
	   cht_read_config(node, NB_FUNC_HT, 0x84 + 0x20 * link));
}
#endif /* __i386 */

int disable_nc = 0;

static int ht_fabric_find_nc(int *p_asic_mode, u32 *p_chip_rev)
{
#ifndef __i386
    printf("(Only doing HT discovery and reconfig in 32-bit mode.)\n");
    return -1;
#else
    int nodes, neigh, link, rt, use, nc, i;
    u32 val;

    val = cht_read_config(0, NB_FUNC_HT, 0x60);
    nodes = (val >> 4) & 7;

    /* Disable HT sync flood for harmless (?) error conditions */
    for (i = 0; i <= nodes; i++) {
	val = cht_read_config(i, NB_FUNC_MISC, 0x44);
	cht_write_config(i, NB_FUNC_MISC, 0x44, val &
			 ~((1 << 30) | (1 << 21) | (1 << 20)
			   | (1 << 4) | (1 << 3) | (1 << 2)));

	val = cht_read_config(i, NB_FUNC_MISC, 0x180);
	cht_write_config(i, NB_FUNC_MISC, 0x180, val &
			 ~((1 << 22) | (1 << 21) | (1 << 20) 
			   | (1 << 9) | (1 << 8) | (1 << 7)
			   | (1 << 6) | (1 << 1)));
    }

    use = 1;
    for (neigh = 0; neigh <= nodes; neigh++) {
        u32 aggr = cht_read_config(neigh, NB_FUNC_HT, 0x164);
        for (link = 0; link < 4; link++) {
            val = cht_read_config(neigh, NB_FUNC_HT, 0x98 + link * 0x20);
            if ((val & 0x1f) != 0x3)
                continue; /* Not coherent */
            use = 0;
            if (aggr & (0x10000 << link))
                use = 1;
            for (rt = 0; rt <= nodes; rt++) {
                val = cht_read_config(neigh, NB_FUNC_HT, 0x40 + rt * 4);
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
        printf("No unrouted coherent links found.\n");
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
    val = cht_read_config(neigh, NB_FUNC_HT, 0x40 + neigh * 4);
    cht_write_config(neigh, NB_FUNC_HT, 0x40 + nc * 4, 
                     (val & 0x07fc0000) | (0x402 << link));

    for (i = 0; i <= nodes; i++) {
        val = cht_read_config(i, NB_FUNC_HT, 0x68);
        cht_write_config(i, NB_FUNC_HT, 0x68, val & ~(1 << 15)); /* LimitCldtCfg */

        if (i == neigh)
            continue;
        /* Route "nc" same as "neigh" for all other nodes */
        val = cht_read_config(i, NB_FUNC_HT, 0x40 + neigh * 4);
        cht_write_config(i, NB_FUNC_HT, 0x40 + nc * 4, val);
    }

    /* earliest opportunity to test HT link */
    if (ht_testmode & HT_TESTMODE_TEST)
	cht_test(nc, 0, neigh, link, H2S_CSR_F0_DEVICE_VENDOR_ID_REGISTER, 0x06011b47);

    val = cht_read_config_nc(nc, 0, neigh, link,
                             H2S_CSR_F0_DEVICE_VENDOR_ID_REGISTER);
    if (val != 0x06011b47) {
	printf("Unrouted coherent device found is not NumaChip: %08x.\n", val);
        return -1;
    }

    printf("NumaChip found (%08x).\n", val);

    if (route_only)
	return nc;

    /* Ramp up link speed and width before adding NC to coherent fabric */
    tsc_wait(50);
    val = cht_read_config_nc(nc, 0, neigh, link, 0xec);
    if (val == 0) {
        tsc_wait(50);
        val = cht_read_config_nc(nc, 0, neigh, link,
                                 H2S_CSR_F0_CLASS_CODE_REVISION_ID_REGISTER);
        printf("Doing link calibration of ASIC chip, rev %d.\n", val & 0xffff);
        ht_optimize_link(nc, val & 0xffff, 1);
        *p_asic_mode = 1;
        *p_chip_rev = val & 0xffff;
    } else {
        printf("Doing link calibration of FPGA chip, rev %d_%d.\n", val>>16, val & 0xffff);
        ht_optimize_link(nc, val, 0);
        *p_asic_mode = 0;
        *p_chip_rev = val;
    }

    if ((*p_asic_mode && (*p_chip_rev < 2)) || force_probefilteroff) {
	disable_probefilter(nodes);
    }
    
    disable_cache();

    for (i = nodes; i >= 0; i--) {
        u32 ltcr;
        /* Disable probes while adjusting */
        ltcr = cht_read_config(i, NB_FUNC_HT, 0x68);
        cht_write_config(i, NB_FUNC_HT, 0x68,
                         ltcr | (1 << 10) | (1 << 3) | (1 << 2) | (1 << 1) | (1 << 0));

        /* Update "neigh" bcast values for node about to increment fabric size */
        val = cht_read_config(neigh, NB_FUNC_HT, 0x40 + i * 4);
        cht_write_config(neigh, NB_FUNC_HT, 0x40 + i * 4, val | (0x80000 << link));

        /* Increase fabric size */
        val = cht_read_config(i, NB_FUNC_HT, 0x60);
        cht_write_config(i, NB_FUNC_HT, 0x60, val + (1<<4));

        /* Reassert LimitCldtCfg */
        cht_write_config(i, NB_FUNC_HT, 0x68,
                         ltcr | (1 << 15));
    }

    enable_cache();
    // reorganize_mmio(nc);

    printf("Done!\n");

    return nc;
#endif
}

static int ht_fabric_fixup(int *p_asic_mode, u32 *p_chip_rev)
{
    u32 val;
    u64 rval;
    u8 node;
    int dnc_ht_id;

    /* Set EnableCf8ExtCfg */
    rval = dnc_rdmsr(MSR_NB_CFG);
    rval = rval | (1ULL << 46);
    dnc_wrmsr(MSR_NB_CFG, rval);

    val = cht_read_config(0, NB_FUNC_HT, 0x60);
    printf("Node #0 F0x60: %x\n", val);
    dnc_ht_id = (val >> 4) & 7;

    val = cht_read_config_nc(dnc_ht_id, 0, nc_neigh, nc_neigh_link,
                             H2S_CSR_F0_DEVICE_VENDOR_ID_REGISTER);
    if (val == 0x06011b47) {
        printf("NumaChip already present on HT node %d\n", dnc_ht_id);

        // Chip already found, just make sure we run the desired with/frequency
        val = cht_read_config_nc(dnc_ht_id, 0, nc_neigh, nc_neigh_link, 0xec);
        if (val == 0) {
            val = cht_read_config_nc(dnc_ht_id, 0, nc_neigh, nc_neigh_link,
                                     H2S_CSR_F0_CLASS_CODE_REVISION_ID_REGISTER);
            printf("Doing link calibration of ASIC chip, rev %d.\n", val & 0xffff);
            ht_optimize_link(dnc_ht_id, val & 0xffff, 1);
            *p_asic_mode = 1;
            *p_chip_rev = val & 0xffff;
        } else {
            printf("Doing link calibration of FPGA chip, rev %d_%d.\n", val>>16, val & 0xffff);
            ht_optimize_link(dnc_ht_id, val, 0);
            *p_asic_mode = 0;
            *p_chip_rev = val;
        }
    }
    else {
        dnc_ht_id = ht_fabric_find_nc(p_asic_mode, p_chip_rev);
        if (dnc_ht_id < 0) {
            printf("NumaChip not found.\n");
            return -1;
        }
        printf("NumaChip incorporated as HT node %d\n", dnc_ht_id);
    }   
    val = cht_read_config_nc(dnc_ht_id, 0, nc_neigh, nc_neigh_link,
                             H2S_CSR_F0_DEVICE_VENDOR_ID_REGISTER);
    printf("Node #%d F0x00: %x\n", dnc_ht_id, val);

    val = cht_read_config(0, NB_FUNC_HT, 0x60);
    cht_write_config_nc(dnc_ht_id, 0, nc_neigh, nc_neigh_link,
                        H2S_CSR_F0_CHTX_NODE_ID,
                        (((val >> 12) & 7) << 24) | /* LkNode */
                        (((val >> 8)  & 7) << 16) | /* SbNode */
                        (dnc_ht_id << 8) | /* NodeCnt */
                        dnc_ht_id); /* NodeId */

    /* XXX: Adjust global CSR_BASE to 46 bits in order to access it in Linux */
    DNC_CSR_BASE = 0x3fff00000000ULL;
    DNC_CSR_LIM = 0x3fffffffffffULL;

    /* Check if BIOS has assigned a BAR0, if so clear it */
    val = cht_read_config_nc(dnc_ht_id, 0, nc_neigh, nc_neigh_link,
                             H2S_CSR_F0_STATUS_COMMAND_REGISTER);
    printf("Command/Status: %08x\n", val);
    if (val & (1 << 1)) {
        cht_write_config_nc(dnc_ht_id, 0, nc_neigh, nc_neigh_link,
                            H2S_CSR_F0_STATUS_COMMAND_REGISTER, val & ~(1 << 1));
        cht_write_config_nc(dnc_ht_id, 0, nc_neigh, nc_neigh_link,
                            H2S_CSR_F0_BASE_ADDRESS_REGISTER_0, 0);
        cht_write_config_nc(dnc_ht_id, 0, nc_neigh, nc_neigh_link,
                            H2S_CSR_F0_EXPANSION_ROM_BASE_ADDRESS, 0);
    }

    /* Check the expansion rom base address register if this has already been done. */
    val = cht_read_config_nc(dnc_ht_id, 0, nc_neigh, nc_neigh_link,
                             H2S_CSR_F0_EXPANSION_ROM_BASE_ADDRESS);
    if (val != 0 && !(val & 1)) {
        if ((val & 0xffff0000) != (DNC_CSR_BASE >> 16)) {
            printf("Mismatching CSR space hi addresses, warm-reset needed! (%04x != %04x).\n",
                   (u32)(val >> 16), (u32)(DNC_CSR_BASE >> 32));
            return -1;
        }
    }
    else {
#ifdef __i386
        /* Bootloader mode, modify CSR_BASE_ADDRESS through the default global maps,
         * and set this value in expansion rom base address register. */
        for (node = 0; node < dnc_ht_id; node++) {
            printf("Setting default CSR maps for node %d\n", node);
            add_extd_mmio_maps(0xfff0, node, 0, DEF_DNC_CSR_BASE, DEF_DNC_CSR_LIM, dnc_ht_id);
        }
        printf("Setting CSR_BASE_ADDRESS to %04llx using default address\n", (DNC_CSR_BASE >> 32));
        mem64_write32(DEF_DNC_CSR_BASE | (0xfff0 << 16) | (1<<15) | H2S_CSR_G3_CSR_BASE_ADDRESS,
                      u32bswap(DNC_CSR_BASE >> 32));
        cht_write_config_nc(dnc_ht_id, 0, nc_neigh, nc_neigh_link,
                            H2S_CSR_F0_EXPANSION_ROM_BASE_ADDRESS,
                            (DNC_CSR_BASE >> 16)); // Put DNC_CSR_BASE[47:32] in the rom address register offset[31:16]
#else
        /* XXX: perhaps we could try default global map in userspace too ? */
        printf("Unable to determine CSR space address.\n");
        return -1;
#endif
    }

    for (node = 0; node < dnc_ht_id; node++) {
        printf("Setting CSR and MCFG maps for node %d\n", node);
        add_extd_mmio_maps(0xfff0, node, 0, DNC_CSR_BASE, DNC_CSR_LIM, dnc_ht_id);
        add_extd_mmio_maps(0xfff0, node, 1, DNC_MCFG_BASE, DNC_MCFG_LIM, dnc_ht_id);
    }

    cht_write_config_nc(dnc_ht_id, 0, nc_neigh, nc_neigh_link,
                        H2S_CSR_F0_CHTX_LINK_INITIALIZATION_CONTROL, 0);
    cht_write_config_nc(dnc_ht_id, 0, nc_neigh, nc_neigh_link,
                        H2S_CSR_F0_CHTX_ADDITIONAL_LINK_TRANSACTION_CONTROL, 6);

    return dnc_ht_id;
}

#define SPI_INSTR_WRSR  0x01
#define SPI_INSTR_WRITE 0x02
#define SPI_INSTR_READ  0x03
#define SPI_INSTR_WRDI  0x04
#define SPI_INSTR_RDSR  0x05
#define SPI_INSTR_WREN  0x06

static void set_eeprom_instruction(u16 node, u32 instr)
{
    u32 reg;

    dnc_write_csr(node, H2S_CSR_G3_SPI_INSTRUCTION_AND_STATUS, instr);
    reg = 0x100;
    while (reg & 0x100) {
        reg = dnc_read_csr(node, H2S_CSR_G3_SPI_INSTRUCTION_AND_STATUS);
        if (reg & 0x100) tsc_wait(100);
    }
}

static u32 read_eeprom_dword(u16 node, u32 addr) {
    set_eeprom_instruction(node, SPI_INSTR_WRDI);
    tsc_wait(100);
    set_eeprom_instruction(node, (addr << 16) | SPI_INSTR_READ);
    return dnc_read_csr(node, H2S_CSR_G3_SPI_READ_WRITE_DATA);
}

static u32 identify_eeprom(u16 node, char type[16], u8 *osc_setting)
{
    u32 reg;
    int i;

    for (i = 0; i < 4; i++) {
        reg = read_eeprom_dword(node, 0xffc0 + i*4);
        memcpy(&type[i*4], &reg, 4);
    }
    type[15] = '\0';

    reg = read_eeprom_dword(node, 0xffbc);
    *osc_setting = (reg >> 24) & 0x3;
    
    return read_eeprom_dword(node, 0xfffc);
}

static void _pic_reset_ctrl(int val)
{
    dnc_write_csr(0xfff0, H2S_CSR_G1_PIC_RESET_CTRL, val);
    tsc_wait(500);
    (void)dnc_read_csr(0xfff0, H2S_CSR_G1_PIC_INDIRECT_READ); // Use a read operation to terminate the current i2c transaction, to avoid a bug in the uC
}

static int adjust_oscillator(char *type, u8 osc_setting)
{
    u32 val;
    
    /* Check if we have the possibility to actually
       adjust the frequency.
    */
    if ((strncmp("313001CC", type, 8) == 0) ||
        (strncmp("313001cc", type, 8) == 0) ||
        (strncmp("313001CD", type, 8) == 0) ||
	(strncmp("N313001CDA", type, 10) == 0) ||
	(strncmp("N313001CDB", type, 10) == 0))
    {
        if (osc_setting > 2) {
            printf("Invalid Oscillator setting read from eeprom (%d), skipping\n", osc_setting);
            return 0;
        }
        dnc_write_csr(0xfff0, H2S_CSR_G1_PIC_INDIRECT_READ, 0x40); // Set the indirect read address register
        tsc_wait(500);
        val = dnc_read_csr(0xfff0, H2S_CSR_G1_PIC_INDIRECT_READ);
        // On the RevC cards the micro controller isn't quite fast enough
        // to send bit7 of every byte correctly on the I2C bus when reading.
        // The bits we care about is in bit[1:0] of the high order byte.
        printf("Current oscillator setting : %d\n", (val>>24) & 3);
        if (((val>>24) & 3) != osc_setting) {
            // Write directly to the frequency selct register
            val = 0x0000b0e9 | (osc_setting<<24);
            printf("Writing new oscillator setting : %d\n", osc_setting);
            dnc_write_csr(0xfff0, H2S_CSR_G1_PIC_INDIRECT_READ + 0x40, val);
            // Wait for the new frequency to settle
            tsc_wait(500);
            // Trigger a HSS PLL reset
            _pic_reset_ctrl(1);
            tsc_wait(500);
        }
    }
        
    return 0;
}

struct optargs {
    char label[16];
    int (*handler)(const char *, void *);
    void *userdata;
};

static int parse_string(const char *val, void *stringp)
{
    *(char **)stringp = strdup(val);
    return 1;
}

static int parse_int(const char *val, void *intp)
{
    if (val[0] != '\0')
	*(int *)intp = atoi(val);
    else
	*(int *)intp = 1;
    return 1;
}

extern char *gitlog_dnc_bootloader_sha;
extern char *gitlog_dnc_bootloader_diff;
static int print_git_log(const char *val __attribute__((unused)),
			 void *data __attribute__((unused)))
{
    printf("Git HEAD: %s\n", gitlog_dnc_bootloader_sha);
    if (strlen(gitlog_dnc_bootloader_diff) > 0) {
	printf("-----\n%s-----\n", gitlog_dnc_bootloader_diff);
    }
    else {
	printf("(unmodified)\n");
    }
    return 1;
}

static int parse_cmdline(const char *cmdline) 
{
    static struct optargs options[] = {
        {"config",	&parse_string, &config_file_name},
        {"next-label",	&parse_string, &next_label},
        {"sync-mode",	&parse_int, &sync_mode},
        {"init-only",	&parse_int, &init_only},
        {"route-only",	&parse_int, &route_only},
        {"disable-nc",	&parse_int, &disable_nc},
        {"enablenbmce",	&parse_int, &enable_nbmce},
        {"enablenbwdt",	&parse_int, &enable_nbwdt},
        {"self-test",   &parse_int, &enable_selftest},
        {"microcode",	&parse_string, &microcode_path},
        {"ht-testmode",	&parse_int, &ht_testmode},
	{"disable-pf",  &parse_int, &force_probefilteroff},
        {"force-ganged", &parse_int, &force_ganged},
        {"disable-smm", &parse_int, &disable_smm},
        {"renumber-bsp", &parse_int, &renumber_bsp},
        {"forwarding-mode", &parse_int, &forwarding_mode},
        {"print-git-log", &print_git_log, NULL},
    };
    char arg[256];
    int lstart, lend, aend, i;

    if (!cmdline)
        return 1;

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
            for (i = 0; i < (int)(sizeof(options)/sizeof(options[0])); i++) {
                if (strncmp(&cmdline[lstart], options[i].label, lend - lstart) == 0) {
                    memset(arg, 0, sizeof(arg));
                    if (cmdline[lend] == '=')
                        lend++;
                    if (aend - lend >= (int)(sizeof(arg)))
                        strncpy(arg, &cmdline[lend], sizeof(arg) - 1);
                    else
                        strncpy(arg, &cmdline[lend], aend - lend);
                           
                    if (options[i].handler(arg, options[i].userdata) < 0)
                        return -1;
                    break;
                }
            }
        }

        lstart = aend;
    }

    return 1;
}

static int perform_selftest(int asic_mode)
{
    int pass, res;
    u32 val;

    res = 0;
    printf("Performing self test");

    for (pass=0; pass<10 && res==0; pass++) {
        const u16 maxchunk = asic_mode ? 16 : 1; // On FPGA all these rams are reduced in size
        int i, chunk;

        // Test PCII/O ATT
        printf(".");
        dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00000000);
        for (i = 0; i < 256; i++) {
            dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i*4, i);
        }
        for (i = 0; i < 256; i++) {
            val = dnc_read_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i*4);
            if (val != (u32)i) {
                res = -1;
                break;
            }
        }

        if (res < 0) break;

        if (asic_mode) {
            // XXX: MMIO32 ATT has a slightly different layout on FPGA, so we just skip it for now..
            // Test MMIO32 ATT
            printf(".");
            for (chunk = 0; chunk < maxchunk; chunk++) {
                dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00000010 | chunk);
                for (i = 0; i < 256; i++) {
                    dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i*4, (chunk*256) + i);
                }
            }
            for (chunk = 0; chunk < maxchunk; chunk++) {
                dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00000010 | chunk);
                for (i = 0; i < 256; i++) {
                    val = dnc_read_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i*4);
                    if (val != (u32)((chunk*256) + i)) {
                        res = -1;
                        break;
                    }
                }
            }
        }

        if (res < 0) break;

        // Test APIC ATT
        printf(".");
        for (chunk = 0; chunk < maxchunk; chunk++) {
            dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00000020 | chunk);
            for (i = 0; i < 256; i++) {
                dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i*4, (chunk*256) + i);
            }
        }
        for (chunk = 0; chunk < maxchunk; chunk++) {
            dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00000020 | chunk);
            for (i = 0; i < 256; i++) {
                val = dnc_read_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i*4);
                if (val != (u32)((chunk*256) + i)) {
                    res = -1;
                    break;
                }
            }
        }

        if (res < 0) break;

        // Test IntRecNode ATT
        printf(".");
        for (chunk = 0; chunk < maxchunk; chunk++) {
            dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00000040 | chunk);
            for (i = 0; i < 256; i++) {
                dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i*4, (chunk*256) + i);
            }
        }
        for (chunk = 0; chunk < maxchunk; chunk++) {
            dnc_write_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT, 0x00000040 | chunk);
            for (i = 0; i < 256; i++) {
                val = dnc_read_csr(0xfff0, H2S_CSR_G3_NC_ATT_MAP_SELECT_0 + i*4);
                if (val != (u32)((chunk*256) + i)) {
                    res = -1;
                    break;
                }
            }
        }

        if (res < 0) break;
        
        // Test SCC routing tables, no readback verify
        printf(".");
        for (chunk = 0; chunk < maxchunk; chunk++) {
            dnc_write_csr(0xfff0, H2S_CSR_G0_ROUT_TABLE_CHUNK, chunk);
            for (i = 0; i < 16; i++) {
                dnc_write_csr(0xfff0, H2S_CSR_G0_ROUT_BXTBLL00 + i*4, 0);
                dnc_write_csr(0xfff0, H2S_CSR_G0_ROUT_BLTBL00  + i*4, 0);
                dnc_write_csr(0xfff0, H2S_CSR_G0_ROUT_BXTBLH00 + i*4, 0);
            }
        }

        printf("PASS%d", pass);
    }
    printf("\nSelftest %s\n", (res==0) ? "PASSED" : "FAILED");

    return res;
}

int dnc_init_bootloader(u32 *p_uuid, int *p_asic_mode, int *p_chip_rev, const char *cmdline)
{
    u32 uuid, val, chip_rev;
    u8 osc_setting;
    char type[16];
    int ht_id = -1;
    struct node_info *info;
    struct part_info *part;
    int i, asic_mode;
    
    if (parse_cmdline(cmdline) < 0)
        return -1;

#ifdef __i386
    watchdog_setup();
#endif

    ht_id = ht_fabric_fixup(&asic_mode, &chip_rev);

    /* Indicate immediate jump to next-label (-2) if init-only is also given. */
    if ((disable_nc > 0) && (init_only > 0))
        return -2;

    if (route_only > 0)
        return -2;

    if (ht_id < 0)
        return -1;

    /* ====================== START ERRATA WORKAROUNDS ====================== */

    if (asic_mode && (chip_rev == 0)) {
        // ERRATA #N9: Reduce the number of HReq buffers to 2 (29:28), HPrb buffers to 2 (15:14)
        // and SReq buffers to 4 (7:4), to take off some pressure on the response channel.
        // ERRATA #N20: Covered by the above, HReq uses different TransIDs (29:28) than HPrb (15:14)
        val = dnc_read_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL);
        dnc_write_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL, (val & ~(0x1fUL)) | (1<<4) | (1<<3) | (1<<2) | (1<<0));
        dnc_write_csr(0xfff0, H2S_CSR_G3_HPRB_CTRL, 0x3fff0000);
        dnc_write_csr(0xfff0, H2S_CSR_G3_SREQ_CTRL, 0x000f0000);
    } else if (asic_mode && (chip_rev == 1)) {
        // ERRATA #N20: Disable buffer 0-15 to ensure that HPrb and HReq have different
        // transIDs (HPrb: 0-15, HReq: 16-30)
        val = dnc_read_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL);
        dnc_write_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL, (val & ~(0x1fUL)) | (1<<4));
    }

    // Set LockControl=0 to enable HT Lock functionality and avoid data corruption on split transactions.
    // Also set LockDelay to 7.
    // We can do this now since we've disabled HT Locking on non-split transactions, even for high contention
    // cases (ref Errata #N28).
    val = dnc_read_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL);
    dnc_write_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL, (val & ~((3<<24) | (7<<13))) | (3<<24) | (7<<13));

    // Since our microcode now supports remote owner state, we disable the
    // error responses on shared probes to GONE cache lines.
    val = dnc_read_csr(0xfff0, H2S_CSR_G3_HPRB_CTRL);
    dnc_write_csr(0xfff0, H2S_CSR_G3_HPRB_CTRL, val | (1<<1)); // disableErrorResponse=1
    
    if (asic_mode && (chip_rev < 2)) {
	// Unknown ERRATA: Disable the Early CData read. It causes data corruption.
	val = dnc_read_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL);
	dnc_write_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL, (val & ~(3<<6)) | (3<<6));
	// Unknown ERRATA: Disable CTag cache.
	val = dnc_read_csr(0xfff0, H2S_CSR_G4_MCTAG_MAINTR);
	dnc_write_csr(0xfff0, H2S_CSR_G4_MCTAG_MAINTR, val & ~(1<<2));
	// Unknown ERRATA: Reduce the HPrb/FTag pipleline (M_PREF_MODE bit[7:4]) to avoid hang situations.
	val = dnc_read_csr(0xfff0, H2S_CSR_G2_M_PREF_MODE);
	dnc_write_csr(0xfff0, H2S_CSR_G2_M_PREF_MODE, (val & ~(0xf<<4)) | (1<<4));
    } else { // ASIC Rev2 and FPGA
        val = dnc_read_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL);
        dnc_write_csr(0xfff0, H2S_CSR_G3_HREQ_CTRL, val | (1<<19)); // Enable WeakOrdering
    }

    if (asic_mode && (chip_rev < 2)) {
	// ERRATA #N17: Disable SCC Context #0 to avoid false baPiuTHit assertions when buffer #0 is active.
	// ERRATA #N23: Disable 8 SCC Contexts (0-7) to avoid SPrb filling up completely and thus asserting
	// pmInhRemoteReq and causing the pre-grant of pmGrant to de-assert which causes issues when both
	// BIU and MIU sends requests to an almost full SReq module.
	// Also disable the MIB timer completely (bit6) for now (debugging purposes).
	val = dnc_read_csr(0xfff0, H2S_CSR_G0_MIB_IBC);
	dnc_write_csr(0xfff0, H2S_CSR_G0_MIB_IBC, val | (0x00ff << 8) | (1 << 6));
    } else { // ASIC Rev2 and FPGA
	// ERRATA #N37: On FPGA now we have an RTL fix in SCC buffers to throttle the amount of
	// local memory requests to accept (bit[27:24] in MIB_IBC). SReq has 4 buffers available
	// on FGPA, but cannot accept more than coherent requests total (1 reserved for
	// non-coherent transactions). We need atleast 1 free buffer to receive the retried
	// coherent requests to ensure forward progress on probes.
	// Also disable the MIB timer completely (bit6) for now (debugging purposes).
	val = dnc_read_csr(0xfff0, H2S_CSR_G0_MIB_IBC);
	dnc_write_csr(0xfff0, H2S_CSR_G0_MIB_IBC, val | (1 << 6));
    }

    for (i = 0; i < ht_id; i++) {
        // XXX: Disable Northbridge WatchDog timer to avoid false error situations in these debug times..
        val = cht_read_config(i, NB_FUNC_MISC, 0x44);
        val = val & ~((1<<6)|(1<<8)); // Clear both bits
        if (!enable_nbmce) val |= (1<<6);
        if (!enable_nbwdt) val |= (1<<8);
        cht_write_config(i, NB_FUNC_MISC, 0x44, val);

        // XXX: Disable DRAM scrubbing in case we want to do HT-Tracing
        val = cht_read_config(i, NB_FUNC_MISC, 0x58);
        if (val & 0x1f) {
            printf("Disabling DRAM scrubber on HT#%d (F3x58=%08x)\n", i, val);
            cht_write_config(i, NB_FUNC_MISC, 0x58, val & ~0x1f);
        }

	if (asic_mode && (chip_rev < 2)) {
	    // InstallStateS to avoid exclusive state
	    val = cht_read_config(i, NB_FUNC_HT, 0x68);
	    cht_write_config(i, NB_FUNC_HT, 0x68, val | (1<<23));

	    // ERRATA #N26: Disable Write-bursting in the MCT to avoid a MCT "caching" effect on CPU writes (VicBlk)
	    // which have bad side-effects with NumaChip in certain scenarios.
	    val = cht_read_config(i, NB_FUNC_DRAM, 0x11c);
	    cht_write_config(i, NB_FUNC_DRAM, 0x11c, val | (0x1f<<2));

	}
	// ERRATA #N27: Disable Coherent Prefetch Probes (Query probes) as NumaChip don't handle them correctly
	val = cht_read_config(i, NB_FUNC_DRAM, 0x1b0);
	cht_write_config(i, NB_FUNC_DRAM, 0x1b0, val & ~(7<<8)); // CohPrefPrbLimit=000b

	// XXX: In case Traffic distribution is enabled on 2 socket systems, we
	// need to disable it for Directed Probes. Ref email to AMD dated 4/28/2010.
	val = cht_read_config(i, NB_FUNC_HT, 0x164);
	cht_write_config(i, NB_FUNC_HT, 0x164, val & ~0x1); /* Disable Traffic distribution for requests */
    }
    
    /* ====================== END ERRATA WORKAROUNDS ====================== */
    
    uuid = identify_eeprom(0xfff0, type, &osc_setting);
    printf("UUID: %06d, TYPE: %s\n", uuid, type);

    // Read the SPD info from our DIMMs to see if they are supported.
    for (i = 0; i < 2; i++) {
        if (read_spd_info(i, &dimms[i]) < 0)
            return -1;
    }

    if (!asic_mode && ((chip_rev>>16) < 6251)) {
	// On FPGA cards we only have 1 GB MCTag and 2GB CData
	dimms[0].mem_size = 0;
	dimms[1].mem_size = 1;
    }

    if (enable_selftest > 0) {
        if (perform_selftest(asic_mode) < 0)
            return -1;
    }

#ifdef __i386
    if (ht_testmode & HT_TESTMODE_PRINT)
	cht_print();
#endif
     
    /* If init-only parameter is given, we stop here and return */
    if (init_only > 0)
        return -2;

    if (read_config_file(config_file_name) < 0)
        return -1;

    info = get_node_config(uuid);
    if (!info)
        return -1;
    printf("Node: <%s> uuid: %d, sciid: 0x%03x, partition: %d, osc: %d\n",
           info->desc, info->uuid, info->sciid, info->partition, info->osc);
    part = get_partition_config(info->partition);
    if (!part)
        return -1;

    printf("Partition master: 0x%03x; builder: 0x%03x\n", part->master, part->builder);
    printf("Fabric dimensions: x: %d, y: %x, z: %d\n",
           cfg_fabric.x_size, cfg_fabric.y_size, cfg_fabric.z_size);

    for (i = 0; i < cfg_nodes; i++) {
        if (cfg_nodelist[i].uuid == uuid)
            continue;
        printf("Remote node: <%s> uuid: %d, sciid: 0x%03x, partition: %d, osc: %d\n",
               cfg_nodelist[i].desc,
               cfg_nodelist[i].uuid,
               cfg_nodelist[i].sciid,
               cfg_nodelist[i].partition,
               cfg_nodelist[i].osc);
    }
    
    if (info->osc < 3)
        osc_setting = info->osc;

    if (adjust_oscillator(type, osc_setting) < 0)
        return -1;

    *p_asic_mode = asic_mode;
    *p_chip_rev = chip_rev;
    *p_uuid = uuid;
    return ht_id;
}

static void save_scc_routing(u16 rtbll[], u16 rtblm[], u16 rtblh[])
{
    u16 chunk, offs;
    u16 maxchunk = dnc_asic_mode ? 16 : 1;

    printf("Setting routing table on SCC...\n");
    
    for (chunk = 0; chunk < maxchunk; chunk++) {
	printf(".");
        dnc_write_csr(0xfff0, H2S_CSR_G0_ROUT_TABLE_CHUNK, chunk);
        for (offs = 0; offs < 16; offs++) {
            // SCC
            dnc_write_csr(0xfff0, H2S_CSR_G0_ROUT_BXTBLL00 + (offs<<2), rtbll[(chunk<<4)+offs]);
            dnc_write_csr(0xfff0, H2S_CSR_G0_ROUT_BLTBL00  + (offs<<2), rtblm[(chunk<<4)+offs]);
            dnc_write_csr(0xfff0, H2S_CSR_G0_ROUT_BXTBLH00 + (offs<<2), rtblh[(chunk<<4)+offs]);
        }
    }
    printf("\n");
}

u16 shadow_rtbll[7][256];
u16 shadow_rtblm[7][256];
u16 shadow_rtblh[7][256];
u16 shadow_ltbl[7][256];

/* Add route on "bxbarid" towards "dest" over "link". */
static void _add_route(u16 dest, u8 bxbarid, u8 link)
{
    u16 offs = (dest >> 4) & 0xff;
    u16 mask = 1<<(dest & 0xf);

    if (bxbarid > 0) shadow_ltbl[bxbarid][offs] |= mask;
    shadow_rtbll[bxbarid][offs] |= ((link & 1) ? mask : 0);
    shadow_rtblm[bxbarid][offs] |= ((link & 2) ? mask : 0);
    shadow_rtblh[bxbarid][offs] |= ((link & 4) ? mask : 0);
//    printf("add_route: ltbl[%d][%02x] = %04x\n", bxbarid, offs, shadow_ltbl[bxbarid][offs]);
//    printf("add_route: rtbll[%d][%02x] = %04x\n", bxbarid, offs, shadow_rtbll[bxbarid][offs]);
//    printf("add_route: rtblm[%d][%02x] = %04x\n", bxbarid, offs, shadow_rtblm[bxbarid][offs]);
//    printf("add_route: rtblh[%d][%02x] = %04x\n", bxbarid, offs, shadow_rtblh[bxbarid][offs]);
}

void test_route(u8 bxbarid, u16 dest)
{
    u16 offs = (dest >> 4) & 0xff;
    u16 mask = 1<<(dest & 0xf);
    u8 out = 0;
    
    printf("Testing route on bxbarid %d to target ID %04x (offs=%02x, mask=%04x)\n", bxbarid, dest, offs, mask);
//    printf("ltbl[%d][%02x] = %04x\n", bxbarid, offs, shadow_ltbl[bxbarid][offs]);
//    printf("rtbll[%d][%02x] = %04x\n", bxbarid, offs, shadow_rtbll[bxbarid][offs]);
//    printf("rtblm[%d][%02x] = %04x\n", bxbarid, offs, shadow_rtblm[bxbarid][offs]);
//    printf("rtblh[%d][%02x] = %04x\n", bxbarid, offs, shadow_rtblh[bxbarid][offs]);
    if (bxbarid > 0 && (shadow_ltbl[bxbarid][offs] & mask)) {
        printf("bxbarid %d will pick up packet\n", bxbarid);
        out += ((shadow_rtbll[bxbarid][offs] & mask) >> (dest & 0xf))*1;
        out += ((shadow_rtblm[bxbarid][offs] & mask) >> (dest & 0xf))*2;
        out += ((shadow_rtblh[bxbarid][offs] & mask) >> (dest & 0xf))*4;
        printf("packet will be routed to bxbarid %d\n", out);
    } else if (bxbarid == 0) {
        out += ((shadow_rtbll[bxbarid][offs] & mask) >> (dest & 0xf))*1;
        out += ((shadow_rtblm[bxbarid][offs] & mask) >> (dest & 0xf))*2;
        out += ((shadow_rtblh[bxbarid][offs] & mask) >> (dest & 0xf))*4;
        printf("packet will be routed to bxbarid %d\n", out);
    }   
}

static int _verify_save_id(u16 nodeid, int lc)
{
    const char *linkname = _get_linkname(lc);
    u16 expected_id = (nodeid | ((lc+1) << 13));
    u32 val;

    if (dnc_raw_read_csr(0xfff1 + lc, LC3_CSR_SAVE_ID, &val) != 0)
        return -1;

    if (val != expected_id) {
        printf("LC3%s SAVE_ID (%04x) is not matching expected value (%04x)!\n",
               linkname, val, expected_id);
        return -1;
    }

    return 0;
}

static int _check_dim(int dim)
{
    u16 linkida = 2*dim + 0;
    u16 linkidb = 2*dim + 1;
    int ok = 0;

    ok = (dnc_check_phy(linkida) == 0);
    ok &= (dnc_check_phy(linkidb) == 0);
    
    if (!ok) {
        printf("PHY errors on PHY%s and PHY%s, issuing PHY reset!\n",
               _get_linkname(linkida), _get_linkname(linkidb));
        // Counter rotating rings, reset both PHYs
        dnc_reset_phy(linkida); dnc_reset_phy(linkidb);
        tsc_wait(1000);

	ok = (dnc_check_phy(linkida) == 0);
	ok &= (dnc_check_phy(linkidb) == 0);
	if (!ok) {
	    printf("PHY errors still present on PHY%s and PHY%s!\n",
		   _get_linkname(linkida), _get_linkname(linkidb));
	    return -1;
	}
    }

    ok = (dnc_check_lc3(linkida) == 0);
    ok &= (dnc_check_lc3(linkidb) == 0);

    if (!ok) {
        printf("LC3 errors on LC3%s and LC3%s, issuing PHY reset!\n",
               _get_linkname(linkida), _get_linkname(linkidb));
        // Counter rotating rings, reset both PHYs
        dnc_reset_phy(linkida); dnc_reset_phy(linkidb);
        tsc_wait(1000);

	ok = (dnc_check_phy(linkida) == 0);
	ok &= (dnc_check_phy(linkidb) == 0);
	ok &= (dnc_check_lc3(linkida) == 0);
	ok &= (dnc_check_lc3(linkidb) == 0);
        if (!ok) {
	    printf("LC3 reset not successful!\n");
	    return -1;
	}
    }

    return ok ? 0 : -1;
}

int shortest(u8 dim, u16 src, u16 dst) {
    /* extract positions on ring */
    int src2 = (src >> (dim * 4)) & 0xf;
    int dst2 = (dst >> (dim * 4)) & 0xf;
    int len = (dim == 0) ? cfg_fabric.x_size : (dim == 1) ? cfg_fabric.y_size : cfg_fabric.z_size;
    int forward = ((len - src2) + dst2) % len;
    int backward = ((src2 + (len - dst2)) + len) % len;

    if (forward == backward)
	return src2 & 1; /* load balance */
    return backward < forward;
}

int dnc_setup_fabric(struct node_info *info)
{
    int i;
    u8 lc;
    
    dnc_write_csr(0xfff0, H2S_CSR_G0_NODE_IDS, info->sciid << 16);

    memset(shadow_ltbl, 0, sizeof(shadow_ltbl));
    memset(shadow_rtbll, 0, sizeof(shadow_rtbll));
    memset(shadow_rtblm, 0, sizeof(shadow_rtblm));
    memset(shadow_rtblh, 0, sizeof(shadow_rtblh));

    printf("Using %s path routing\n", cfg_fabric.strict ? "shortest" : "unoptimised");

    if (cfg_fabric.x_size > 0)
        _add_route(info->sciid, 0, 1); // Self route via LC3XA.
    else if (cfg_fabric.y_size > 0)
        _add_route(info->sciid, 0, 3); // Self route via LC3YA.
    else
        _add_route(info->sciid, 0, 5); // Self route via LC3ZA.

    // Make sure responses get back to SCC
    for (lc = 1; lc <= 6; lc++) {
        _add_route(info->sciid, lc, 0);
    }
    
    for (i = 0; i < cfg_nodes; i++) {
        u16 src = info->sciid;
        u16 dst = cfg_nodelist[i].sciid;
        u8 dim = 0;
        u8 out;

        if (src == dst)
            continue;

        while ((src ^ dst) & ~0xf) {
            dim++;
            src >>= 4;
            dst >>= 4;
        }
        out = dim * 2 + 1;

	if (cfg_fabric.strict)
	    /* SCI IDs correspond to position on the rings */
	    out += shortest(dim, info->sciid, cfg_nodelist[i].sciid);
	else
	    /* SCI IDs may not correspond; load-balance route */
	    out += src & 1;

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
    
    // Make sure all necessary links are up and working
    if (cfg_fabric.x_size > 0) {
        if (_check_dim(0) != 0)
	    return -1;
        dnc_init_lc3(info->sciid, 0, dnc_asic_mode ? 16 : 1,
                     shadow_rtbll[1], shadow_rtblm[1], shadow_rtblh[1], shadow_ltbl[1]);
        dnc_init_lc3(info->sciid, 1, dnc_asic_mode ? 16 : 1,
                     shadow_rtbll[2], shadow_rtblm[2], shadow_rtblh[2], shadow_ltbl[2]);
    }
    if (cfg_fabric.y_size > 0) {
        if (_check_dim(1) != 0)
	    return -1;
        dnc_init_lc3(info->sciid, 2, dnc_asic_mode ? 16 : 1,
                     shadow_rtbll[3], shadow_rtblm[3], shadow_rtblh[3], shadow_ltbl[3]);
        dnc_init_lc3(info->sciid, 3, dnc_asic_mode ? 16 : 1,
                     shadow_rtbll[4], shadow_rtblm[4], shadow_rtblh[4], shadow_ltbl[4]);
    }
    if (cfg_fabric.z_size > 0) {
        if (_check_dim(2) != 0)
	    return -1;
        dnc_init_lc3(info->sciid, 4, dnc_asic_mode ? 16 : 1,
                     shadow_rtbll[5], shadow_rtblm[5], shadow_rtblh[5], shadow_ltbl[5]);
        dnc_init_lc3(info->sciid, 5, dnc_asic_mode ? 16 : 1,
                     shadow_rtbll[6], shadow_rtblm[6], shadow_rtblh[6], shadow_ltbl[6]);
    }

    printf("Done with fabric setup\n");

    return 0;
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

enum node_state enter_reset(struct node_info *info)
{
    int tries = 0;
    u32 val;
    printf("Entering reset.\n");

    if (dnc_asic_mode && dnc_chip_rev >= 1) {
	/* Reset already held?  Toggle reset logic to ensure reset
	 * reverts to know state */
	while ((dnc_read_csr(0xfff0, H2S_CSR_G0_HSSXA_STAT_1) & (1<<8)) == 0) {
	    if (tries == 0) {
		printf("HSSXA_STAT_1 is zero, toggling reset...\n");
		_pic_reset_ctrl(2);
		tsc_wait(1000);
	    }
	    /* printf("Waiting for HSSXA_STAT_1 to leave zero (try %d)...\n", tries); */
	    tsc_wait(200);
	    if (tries++ > 16)
		tries = 0;
	}

	tsc_wait(200);

	/* Hold reset */
	_pic_reset_ctrl(2);
	tsc_wait(200);
	tries = 0;
	while (((val = dnc_read_csr(0xfff0, H2S_CSR_G0_HSSXA_STAT_1)) & (1<<8)) != 0) {
	    /* printf("Waiting for HSSXA_STAT_1 to go to zero (%08x) (try %d)...\n",
	       val, tries); */
	    tsc_wait(200);
	    if (tries++ > 16)
		return enter_reset(info);
	}
    } else {
	// No external reset control, simply reset all PHYs to start training sequence
        dnc_reset_phy(0); dnc_reset_phy(1);
        dnc_reset_phy(2); dnc_reset_phy(3);
        dnc_reset_phy(4); dnc_reset_phy(5);
    }
    
    printf("In reset\n");
    return RSP_RESET_ACTIVE;
}

static int phy_check_status(int phy)
{
    u32 val;
    tsc_wait(200);
    val = dnc_read_csr(0xfff0, H2S_CSR_G0_PHYXA_ELOG + 0x40 * phy);
    tsc_wait(200);
    if (val & 0xf0) {
	/* Clock compensation error, try forced retraining */
	dnc_reset_phy(phy);
	return 1;
    }
    /* Other errors */
    if (val > 0)
	return 1 << phy;

    val = dnc_read_csr(0xfff0, H2S_CSR_G0_PHYXA_LINK_STAT + 0x40 * phy);
    return  (val != 0x1fff) << phy;
}

static void phy_print_error(int mask)
{
    int i;

    printf("PHY training failure - check cables to ports");

    for (i = 0; i < 6; i++)
	if (mask & (1 << i))
	    printf(" %s", _get_linkname(i));

    printf("\n");
}


enum node_state release_reset(struct node_info *info __attribute__((unused)))
{
    int pending, i;
    printf("Releasing reset.\n");

    if (dnc_asic_mode && dnc_chip_rev >= 1) {
	/* Release reset */
	_pic_reset_ctrl(2);
	tsc_wait(200);
	i = 0;
	while ((dnc_read_csr(0xfff0, H2S_CSR_G0_HSSXA_STAT_1) & (1<<8)) == 0) {
	    if (i++ > 20)
		return RSP_PHY_NOT_TRAINED;
	    tsc_wait(200);
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
	    printf("Done.\n");
	    return RSP_PHY_TRAINED;
	}
	if (i++ > 10) {
	    phy_print_error(pending);
	    return RSP_PHY_NOT_TRAINED;
	}
	tsc_wait(200);
    }
}			

static int lc_check_status(int lc, int dimidx)
{
    if (dnc_check_lc3(lc) == 0)
	return 0;

    tsc_wait(200);
    /* Only initiate resets from one of the ring nodes */
    if (dimidx == 0) {
	printf("Initiating reset on LC%s\n", _get_linkname(lc));
	dnc_reset_lc3(lc);
    }
    return 1;
}

enum node_state validate_rings(struct node_info *info)
{
    int pending, i;

    printf("Validating rings.\n");
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
	tsc_wait(200);
    }
}

int handle_command(enum node_state cstate, enum node_state *rstate, 
		   struct node_info *info,
		   struct part_info *part __attribute__((unused)))
{
    switch (cstate) {
    case CMD_ENTER_RESET:
	*rstate = enter_reset(info);
	return 1;
    case CMD_RELEASE_RESET:
	tsc_wait(2000);
	*rstate = release_reset(info);
	return 1;
    case CMD_VALIDATE_RINGS:
	*rstate = validate_rings(info);
	return 1;
    case CMD_SETUP_FABRIC:
	*rstate = (dnc_setup_fabric(info) == 0) ?
	    RSP_FABRIC_READY : RSP_FABRIC_NOT_READY;
	tsc_wait(2000);
	return 1;
    case CMD_VALIDATE_FABRIC:
	*rstate = dnc_check_fabric(info) ?
	    RSP_FABRIC_OK : RSP_FABRIC_NOT_OK;
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
    u32 last_cmd = ~0;
    u32 builduuid = ~0;
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
	    printf("Broadcasting state: %d, sciid: %03x, uuid = %d, tid = %d\n",
                   rsp.state, rsp.sciid, rsp.uuid, rsp.tid);
	    udp_broadcast_state(handle, &rsp, sizeof(rsp));
	    tsc_wait(100 * backoff);
	    if (backoff < 32)
		backoff = backoff * 2;
	    count = 0;
	}

	int len;
	/* In order to avoid jamming, broadcast own status at least
	 * once every 2*cfg_nodes packet seen. */
	for (i = 0; i < 2*cfg_nodes; i++) {
	    len = udp_read_state(handle, &cmd, sizeof(cmd));
	    if (!len)
		break;
	    if (len != sizeof(cmd))
		continue;

            //printf("Got cmd packet. state = %d, sciid = %03x, uuid = %d, tid = %d\n",
            //       cmd.state, cmd.sciid, cmd.uuid, cmd.tid);
	    if (cmd.uuid == builduuid) {
		if (cmd.tid == last_cmd) {
		    // Ignoring seen command
		    continue;
		}
		last_cmd = cmd.tid;
		count = 0;
		backoff = 1;
		if (handle_command(cmd.state, &rsp.state, info, part)) {
		    rsp.tid = cmd.tid;
		}
		else if (cmd.state == CMD_CONTINUE) {
		    printf("Master signalled go-ahead.\n");
		    /* Belts and suspenders: slaves re-broadcast go-ahead command */
		    udp_broadcast_state(handle, &cmd, sizeof(cmd));
		    go_ahead = 1;
		    break;
		}
	    }
	}
    }
}

