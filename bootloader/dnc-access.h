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

#ifndef __DNC_ACCESS
#define __DNC_ACCESS 1

#include <inttypes.h>
#include <sys/io.h>

#define HT_TESTMODE_PRINT	1
#define HT_TESTMODE_TEST	2
#define HT_TESTMODE_WATCHDOG	4
#define HT_TESTMODE_LOOP	8

#define DNC_MCFG_BASE 0x3f0000000000ULL
#define DNC_MCFG_LIM  0x3ffeffffffffULL

extern uint64_t dnc_csr_base;
extern uint64_t dnc_csr_lim;

#define DNC_CSR_BASE (dnc_csr_base)
#define DNC_CSR_LIM (dnc_csr_lim)
#define DEF_DNC_CSR_BASE 0xffff00000000ULL
#define DEF_DNC_CSR_LIM  0xffffffffffffULL
#define HT_BASE 0xfd00000000ULL
#define HT_LIMIT 0x10000000000ULL

#define PMIO_PORT		0xcd6

#define SR56X0_HTIU_TOM2LO	0x30
#define SR56X0_HTIU_TOM2HI	0x31
#define SR56X0_MISC_TOM		0x16
#define SR56X0_MISC_TOM3	0x4e

extern int lirq_nest;
#define cli() if (lirq_nest++ == 0) { asm volatile("cli"); }
#define sti() if (--lirq_nest == 0) { asm volatile("sti"); }

extern int cht_config_use_extd_addressing;
extern int ht_testmode;

static inline uint64_t rdtscll(void)
{
	uint64_t val;
	/* rdtscp doesn't work on Fam10h, so use mfence to serialise */
	asm volatile("mfence; rdtsc" : "=A"(val));
	return val;
}

static inline uint32_t uint32_tbswap(uint32_t val)
{
	asm volatile("bswap %0" : "+r"(val));
	return val;
}

void pmio_writeb(uint16_t offset, uint8_t val);
void pmio_writel(uint16_t offset, uint32_t val);
uint8_t pmio_readb(uint16_t offset);
uint16_t pmio_reads(uint16_t offset);
uint32_t pmio_readl(uint16_t offset);
void pmio_setb(uint16_t offset, uint8_t val);
void pmio_clearb(uint16_t offset, uint8_t val);
void pmio_setl(uint16_t offset, uint32_t val);
void pmio_clearl(uint16_t offset, uint32_t val);
uint32_t ioh_nbmiscind_read(uint16_t node, uint8_t reg);
void ioh_nbmiscind_write(uint16_t node, uint8_t reg, uint32_t val);
uint32_t ioh_htiu_read(uint16_t node, uint8_t reg);
void ioh_htiu_write(uint16_t node, uint8_t reg, uint32_t val);
void watchdog_setup(void);
void reset_cf9(int mode, int last);
void cht_test(uint8_t node, int neigh, int neigh_link);
uint32_t  cht_read_conf(uint8_t node, uint8_t func, uint16_t reg);
void cht_write_conf(uint8_t node, uint8_t func, uint16_t reg, uint32_t val);
uint32_t cht_read_conf_nc(uint8_t node, uint8_t func, int neigh, int neigh_link, uint16_t reg);
void cht_write_conf_nc(uint8_t node, uint8_t func, int neigh, int neigh_link, uint16_t reg, uint32_t val);
uint32_t  mem64_read32(uint64_t addr);
void mem64_write32(uint64_t addr, uint32_t val);
uint16_t  mem64_read16(uint64_t addr);
void mem64_write16(uint64_t addr, uint16_t val);
uint8_t   mem64_read8(uint64_t addr);
void mem64_write8(uint64_t addr, uint8_t val);
uint32_t  dnc_read_csr(uint32_t node, uint16_t csr);
void dnc_write_csr(uint32_t node, uint16_t csr, uint32_t val);
uint32_t  dnc_read_csr_geo(uint32_t node, uint8_t bid, uint16_t csr);
void dnc_write_csr_geo(uint32_t node, uint8_t bid, uint16_t csr, uint32_t val);
uint32_t  dnc_read_conf(uint16_t node, uint8_t bus, uint8_t device, uint8_t func, uint16_t reg);
void dnc_write_conf(uint16_t node, uint8_t bus, uint8_t device, uint8_t func, uint16_t reg, uint32_t val);
uint64_t dnc_rdmsr(uint32_t msr);
void dnc_wrmsr(uint32_t msr, uint64_t v);

#endif
