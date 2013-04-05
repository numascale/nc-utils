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
#include "dnc-commonlib.h"

#include "../interface/mctr_define_register_C.h"

static void maint_wrmem(int cdata, uint32_t addr, uint32_t data)
{
	dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_MEMORY_WDATA : H2S_CSR_G4_MCTAG_MEMORY_WDATA, data);    /* Set MEMORY_WDATA */
	dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_MEMORY_ADDR : H2S_CSR_G4_MCTAG_MEMORY_ADDR, addr);      /* Set MEMORY_ADDR */
	uint32_t reg = dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_CTRLR : H2S_CSR_G4_MCTAG_COM_CTRLR);  /* Read COM_CTRLR */
	dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_CTRLR : H2S_CSR_G4_MCTAG_COM_CTRLR, reg | 1);       /* COM_CTRLR.CSR_MEM_WRMASK_ACC=0, COM_CTRLR.CSR_MEM_RD_ACC=0, COM_CTRLR.CSR_MEM_ACC=1 */
	dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_STATR : H2S_CSR_G4_MCTAG_COM_STATR, 1);             /* Reset COM_STATR.CSR_MEM_ACC_DONE to start access */

	while (!(dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_STATR : H2S_CSR_G4_MCTAG_COM_STATR) & 1))
		udelay(10000);

	dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_CTRLR : H2S_CSR_G4_MCTAG_COM_CTRLR, reg & ~7);
}

static void maint_rdmem(int cdata, uint32_t addr, uint32_t *data)
{
	uint32_t reg;
	dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_MEMORY_ADDR : H2S_CSR_G4_MCTAG_MEMORY_ADDR, addr);      /* Set MEMORY_ADDR */
	reg = dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_CTRLR : H2S_CSR_G4_MCTAG_COM_CTRLR);           /* Read COM_CTRLR */
	dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_CTRLR : H2S_CSR_G4_MCTAG_COM_CTRLR, reg | 3);       /* COM_CTRLR.CSR_MEM_WRMASK_ACC=0, COM_CTRLR.CSR_MEM_RD_ACC=1, COM_CTRLR.CSR_MEM_ACC=1 */
	dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_STATR : H2S_CSR_G4_MCTAG_COM_STATR, 1);             /* Reset COM_STATR.CSR_MEM_ACC_DONE to start access */

	while (!(dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_STATR : H2S_CSR_G4_MCTAG_COM_STATR) & 1))
		udelay(10000);

	*data = dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_MEMORY_RDATA : H2S_CSR_G4_MCTAG_MEMORY_RDATA);
	dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_CTRLR : H2S_CSR_G4_MCTAG_COM_CTRLR, reg & ~7);
}

static int maint_rdmem_chk(int cdata, uint32_t addr, uint32_t chk)
{
	uint32_t data;
	maint_rdmem(cdata, addr, &data);
	if (data != chk) {
		error("Memory read data check failed at address 0x%08x (data:%08x != chk:%08x)", addr, data, chk);
		return -1;
	}

	return 0;
}

static void dnc_dimmtest_start(const int cdata, struct dimm_config *const dimm, const int segment)
{
	uint32_t reg, memsize;
	uint64_t start_addr;
	const int denalibase = cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00;

	if (segment == -1)
		start_addr = 0;
	else
		start_addr = (uint64_t)segment << DRAM_SEGMENT_SHIFT;

	/* De-assert DRAM_AVAILABLE so we don't mess with the user ports when running BIST */
	reg = dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_CTRLR : H2S_CSR_G4_MCTAG_COM_CTRLR);
	dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_CTRLR : H2S_CSR_G4_MCTAG_COM_CTRLR, reg & ~(1 << 12));

	dnc_write_csr(0xfff0, denalibase + (BIST_START_ADDRESS_ADDR << 2), start_addr & 0xffffffff);
	dnc_write_csr(0xfff0, denalibase + ((BIST_START_ADDRESS_ADDR + 1) << 2), start_addr >> 32);
	dnc_write_csr(0xfff0, denalibase + (BIST_DATA_MASK_ADDR << 2), 0);
	dnc_write_csr(0xfff0, denalibase + ((BIST_DATA_MASK_ADDR + 1) << 2), 0);
	reg = dnc_read_csr(0xfff0, denalibase + (ADDR_SPACE_ADDR << 2));

	if (segment == -1)
		memsize = dimm->mem_size + 30;
	else
		memsize = DRAM_SEGMENT_SHIFT; /* 256MB; ~15s runtime */

	dnc_write_csr(0xfff0, denalibase + (ADDR_SPACE_ADDR << 2), (reg & ~0x3f) | memsize);
	reg = dnc_read_csr(0xfff0, denalibase + (BIST_ADDR_CHECK_ADDR << 2));
	dnc_write_csr(0xfff0, denalibase + (BIST_ADDR_CHECK_ADDR << 2), reg | (1 << BIST_ADDR_CHECK_OFFSET));
	reg = dnc_read_csr(0xfff0, denalibase + (BIST_DATA_CHECK_ADDR << 2));
	dnc_write_csr(0xfff0, denalibase + (BIST_DATA_CHECK_ADDR << 2), reg | (1 << BIST_DATA_CHECK_OFFSET));

	/* Denali docs states that there should be at least 10 clocks of idle before asserting the BIST go bit */
	udelay(10000);

	/* Start BIST test */
	reg = dnc_read_csr(0xfff0, denalibase + (BIST_GO_ADDR << 2));
	dnc_write_csr(0xfff0, denalibase + (BIST_GO_ADDR << 2), reg | (1 << BIST_GO_OFFSET));
}

static void dnc_dimmtest_wait(const int cdata)
{
	uint32_t reg;
	const int denalibase = cdata ? H2S_CSR_G4_CDATA_DENALI_CTL_00 : H2S_CSR_G4_MCTAG_DENALI_CTL_00;

	/* Wait for it to signal completion */
	do {
		udelay(10000);
		reg = dnc_read_csr(0xfff0, denalibase + (INT_STATUS_ADDR << 2));
	}  while (!(reg & (1 << 7)));

	/* Reset BIST complete interrupt */
	reg = dnc_read_csr(0xfff0, denalibase + (INT_ACK_ADDR << 2));
	dnc_write_csr(0xfff0, denalibase + (INT_ACK_ADDR << 2), reg | (1 << 7));

	reg = dnc_read_csr(0xfff0, denalibase + (BIST_RESULT_ADDR << 2));
	int result = (reg >> BIST_RESULT_OFFSET) & ((1 << BIST_RESULT_WIDTH) - 1);
	if (!(result & 1)) {
		error("failed checking data:");
		for (int i = 0; i < 4; i++) {
			reg = dnc_read_csr(0xfff0, denalibase + ((BIST_FAIL_DATA_ADDR + i) << 2));
			printf("- fail data #%d: %08x\n", i, reg);
		}
		for (int i = 0; i < 4; i++) {
			reg = dnc_read_csr(0xfff0, denalibase + ((BIST_EXP_DATA_ADDR + i) << 2));
			printf("- exp data #%d: %08x\n", i, reg);
		}
	}

	if (!(result & 2)) {
		error("failed checking address lines:");
		for (int i = 0; i < 2; i++) {
			reg = dnc_read_csr(0xfff0, denalibase + ((BIST_FAIL_ADDR_ADDR + i) << 2));
			printf("- fail addr #%d: %08x\n", i, reg);
		}
	}

	if ((result & 3) != 3)
		fatal("Memory errors detected on DIMM %d", cdata);

	/* Reset BIST Go bit */
	reg = dnc_read_csr(0xfff0, denalibase + ((BIST_GO_ADDR) << 2));
	dnc_write_csr(0xfff0, denalibase + ((BIST_GO_ADDR) << 2), reg & ~(1 << BIST_GO_OFFSET));
	udelay(10000);

	/* Re-assert DRAM_AVAILABLE */
	reg = dnc_read_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_CTRLR : H2S_CSR_G4_MCTAG_COM_CTRLR);
	dnc_write_csr(0xfff0, cdata ? H2S_CSR_G4_CDATA_COM_CTRLR : H2S_CSR_G4_MCTAG_COM_CTRLR, reg | (1 << 12));
}

static void dnc_dimmtest_final(const int cdata, struct dimm_config *const dimm)
{
	uint32_t tempa, tempb;
	int i, tmp_cnt;
	const int passes = 15;

	printf("Test 2a: MAINT Memory Access incremental housenumbers at low addresses...");
	tempa = 0;
	tempb = 0x11111111;
	tmp_cnt = 0;
	for (i = 0; i < passes; i++) {
		maint_wrmem(cdata, tempa, tempb);
		if (maint_rdmem_chk(cdata, tempa, tempb) < 0)
			fatal("failed during pass %d", tmp_cnt);

		tempa++;
		tempb += 0x11111111;
		tmp_cnt++;
	}

	tempa = 0;
	tempb = 0x11111111;
	for (i = 0; i < passes; i++) {
		if (maint_rdmem_chk(cdata, tempa, tempb) < 0)
			fatal("failed during pass %d", tmp_cnt);

		tempa++;
		tempb += 0x11111111;
		tmp_cnt++;
	}
	printf("passed\n");

	printf("Test 2b: MAINT Memory Access incremental address position...");
	tmp_cnt = 0;
	int loops = dimm->mem_size + 29;

	for (i = 0; i < loops; i++) {
		tempa = i ? 1 << (i - 1) : 0;
		tempb = 1 << i;
		maint_wrmem(cdata, tempa, tempb);
		if (maint_rdmem_chk(cdata, tempa, tempb) < 0)
			fatal("failed during pass %d", tmp_cnt);

		tmp_cnt++;

		for (int j = 0; j < i; j++) {
			tempa = j ? 1 << (j - 1) : 0;
			tempb = 1 << j;
			if (maint_rdmem_chk(cdata, tempa, tempb) < 0)
				fatal("failed during readback pass %d (echo %d - %d)", tmp_cnt, i, j);
		}
	}
	printf("passed\n");

	tempa = 1 << (dimm->mem_size + 25);
	tempb = 0xc396a578;
	tmp_cnt = 0;
	printf("Test 2c: MAINT Memory Access incremental housenumbers at high addresses...");
	for (i = 0; i < passes; i++) {
		maint_wrmem(cdata, tempa, tempb);
		if (maint_rdmem_chk(cdata, tempa, tempb) < 0)
			fatal("failed during pass %d", tmp_cnt);

		tempa++;
		tempb += 0x11111111;
		tmp_cnt++;
	}

	tempa = 1 << (dimm->mem_size + 25);
	tempb = 0xc396a578;
	for (i = 0; i < passes; i++) {
		if (maint_rdmem_chk(cdata, tempa, tempb) < 0)
			fatal("failed during pass %d", tmp_cnt);

		tempa++;
		tempb += 0x11111111;
		tmp_cnt++;
	}
	printf("passed\n");

	printf("Test 3: MAINT Memory Access random patterns, write then read...");
	tmp_cnt = 0;
	for (i = 0; i < passes; i++) {
		tempa = random();
		tempa = tempa & ((1 << (dimm->mem_size + 26)) - 1);
		tempb = random();

		maint_wrmem(cdata, tempa, tempb);
		if (maint_rdmem_chk(cdata, tempa, tempb) < 0)
			fatal("failed during pass %d", tmp_cnt);

		tmp_cnt++;
	}
	printf("passed\n");
}

void dnc_dimmtest(const int testmask, struct dimm_config *const dimm)
{
	if (!testmask) {
		warning("Skipping memory testing");
		return;
	}

	int cdata, hash = rtc_read(RTC_SECONDS);
	if (testmask == 2) {
		printf("Testing all memory segments...");
		for (cdata = 0; cdata < 2; cdata++)
			dnc_dimmtest_start(cdata, dimm, -1);
	} else {
		printf("Testing memory segments");
		for (cdata = 0; cdata < 2; cdata++) {
			int segments = 1 << (31 + dimm->mem_size - DRAM_SEGMENT_SHIFT);
			int segment = hash % segments; /* Caveat: Non-uniform */
			printf(" %d/%d", segment + 1, segments);
			dnc_dimmtest_start(cdata, dimm, segment);
		}
		printf("...");
	}

	for (cdata = 0; cdata < 2; cdata++)
		dnc_dimmtest_wait(cdata);
	printf("passed\n");

	for (cdata = 0; cdata < 2; cdata++)
		dnc_dimmtest_final(cdata, dimm);

	for (cdata = 0; cdata < 2; cdata++) {
		uint32_t val = dnc_check_mctr_status(cdata);
		assertf(!(val & 0x03c), "ECC errors detected in DIMM %d", cdata);
	}

	/* Reinitialise DRAM to clear it */
	dnc_dram_initialise();
}
