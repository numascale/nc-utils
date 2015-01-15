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

#include "dnc-types.h"
#include "dnc-defs.h"

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

#define RTC_SECONDS		0
#define RTC_MINUTES		2
#define RTC_HOURS		4
#define RTC_DAY_OF_WEEK		6
#define RTC_DAY         7
#define RTC_MONTH		8
#define RTC_YEAR		9
#define RTC_SETTINGS    11

extern int lirq_nest;
#define cli() if (lirq_nest++ == 0) { asm volatile("cli"); }
#define sti() if (--lirq_nest == 0) { asm volatile("sti"); }
/* Since we use FS to access these areas, the address needs to be in canonical form (sign extended from bit47) */
#define canonicalize(a) (((a) & (1ULL << 47)) ? ((a) | (0xffffULL << 48)) : (a))

#define setup_fs(addr) do {                                             \
        asm volatile("mov %%ds, %%ax\n\tmov %%ax, %%fs" ::: "eax");     \
        asm volatile("wrmsr"                                            \
                     : /* No output */                                  \
                     : "A"(canonicalize(addr)), "c"(MSR_FS_BASE));      \
    } while(0)

extern int cht_config_use_extd_addressing;
extern int ht_testmode;

checked static inline uint64_t rdtscll(void)
{
	uint64_t val;
	/* rdtscp doesn't work on Fam10h, so use mfence to serialise */
	asm volatile("mfence; rdtsc" : "=A"(val));
	return val;
}

checked static inline uint32_t uint32_tbswap(uint32_t val)
{
	asm volatile("bswap %0" : "+r"(val));
	return val;
}

static inline uint64_t mem64_read64(const uint64_t addr)
{
	uint64_t val;
	cli();
	setup_fs(addr);
	asm volatile("movq %%fs:(0), %%mm0; movq %%mm0, (%0)" : :"r"(&val) :"memory");
	sti();
	return val;
}

static inline uint32_t mem64_read32(const uint64_t addr)
{
	uint32_t ret;
	cli();
	setup_fs(addr);
	asm volatile("mov %%fs:(0), %%eax" : "=a"(ret));
	sti();
	return ret;
}

static inline void mem64_write64(const uint64_t addr, const uint64_t val)
{
	cli();
	setup_fs(addr);
	asm volatile("movq (%0), %%mm0; movq %%mm0, %%fs:(0)" : :"r"(&val) :"memory");
	sti();
}

static inline void mem64_write32(const uint64_t addr, const uint32_t val)
{
	cli();
	setup_fs(addr);
	asm volatile("mov %0, %%fs:(0)" :: "a"(val));
	sti();
}

static inline uint16_t mem64_read16(const uint64_t addr)
{
	uint16_t ret;
	cli();
	setup_fs(addr);
	asm volatile("movw %%fs:(0), %%ax" : "=a"(ret));
	sti();
	return ret;
}

static inline void mem64_write16(const uint64_t addr, const uint16_t val)
{
	cli();
	setup_fs(addr);
	asm volatile("movw %0, %%fs:(0)" :: "a"(val));
	sti();
}

static inline uint8_t mem64_read8(const uint64_t addr)
{
	uint8_t ret;
	cli();
	setup_fs(addr);
	asm volatile("movb %%fs:(0), %%al" : "=a"(ret));
	sti();
	return ret;
}

static inline void mem64_write8(const uint64_t addr, const uint8_t val)
{
	cli();
	setup_fs(addr);
	asm volatile("movb %0, %%fs:(0)" :: "a"(val));
	sti();
}

void dump(const void *addr, const unsigned len);
checked uint8_t rtc_read(const int addr);
void pmio_write8(uint16_t offset, uint8_t val);
void pmio_write32(uint16_t offset, uint32_t val);
checked uint8_t pmio_read8(uint16_t offset);
checked uint16_t pmio_read16(uint16_t offset);
checked uint32_t pmio_read32(uint16_t offset);
void pmio_setb(uint16_t offset, uint8_t val);
void pmio_clearb(uint16_t offset, uint8_t val);
void pmio_setl(uint16_t offset, uint32_t val);
void pmio_clearl(uint16_t offset, uint32_t val);
checked uint32_t ioh_nbmiscind_read(const sci_t sci, const uint8_t reg);
void ioh_nbmiscind_write(const sci_t sci, const uint8_t reg, const uint32_t val);
uint32_t ioh_nbpcieind_read(const sci_t sci, const uint8_t core, const uint8_t reg);
void ioh_nbpcieind_write(const sci_t sci, const uint8_t core, const uint8_t reg, const uint32_t val);
checked uint32_t ioh_htiu_read(const sci_t sci, uint8_t reg);
void ioh_htiu_write(const sci_t sci, uint8_t reg, uint32_t val);
checked uint32_t ioh_ioapicind_read(const uint16_t sci, const uint8_t reg);
void ioh_ioapicind_write(const uint16_t sci, const uint8_t reg, const uint32_t val);
void watchdog_setup(void);
void watchdog_run(const unsigned counter);
void watchdog_stop(void);
void reset_cf9(int mode, int last);
void cht_test(uint8_t node, int neigh, int neigh_link);
checked uint32_t cht_read_conf(uint8_t node, uint8_t func, uint16_t reg);
void cht_write_conf(uint8_t node, uint8_t func, uint16_t reg, uint32_t val);
checked uint32_t cht_read_conf_nc(uint8_t node, uint8_t func, int neigh, int neigh_link, uint16_t reg);
void cht_write_conf_nc(uint8_t node, uint8_t func, int neigh, int neigh_link, uint16_t reg, uint32_t val);
uint64_t mem64_read64(const uint64_t addr);
uint32_t mem64_read32(const uint64_t addr);
void mem64_write64(const uint64_t addr, const uint64_t val);
void mem64_write32(const uint64_t addr, const uint32_t val);
checked uint16_t mem64_read16(const uint64_t addr);
void mem64_write16(const uint64_t addr, const uint16_t val);
checked uint8_t mem64_read8(const uint64_t addr);
void mem64_write8(const uint64_t addr, const uint8_t val);
uint32_t dnc_read_csr(uint32_t node, uint16_t csr);
void dnc_write_csr(uint32_t node, uint16_t csr, uint32_t val);
checked uint32_t dnc_read_csr_geo(uint32_t node, uint8_t bid, uint16_t csr);
void dnc_write_csr_geo(uint32_t node, uint8_t bid, uint16_t csr, uint32_t val);
checked uint32_t dnc_read_conf(const sci_t sci, const uint8_t bus, const uint8_t device, const uint8_t func, const uint16_t reg);
checked uint64_t dnc_read_conf64(const sci_t sci, const uint8_t bus, const uint8_t device, const uint8_t func, const uint16_t reg);
void dnc_write_conf(const sci_t sci, const uint8_t bus, const uint8_t device, const uint8_t func, const uint16_t reg, const uint32_t val);
void dnc_write_conf64_split(const sci_t sci, const uint8_t bus, const uint8_t device, const uint8_t func, const uint16_t reg, const uint64_t val);
checked uint64_t rdmsr(const uint32_t msr);
void wrmsr(const uint32_t msr, const uint64_t v);
void *zalloc_persist(const size_t size);

#endif
