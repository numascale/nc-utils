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

#include <sys/io.h>
#include "dnc-types.h"

#define HT_TESTMODE_PRINT	1
#define HT_TESTMODE_TEST	2
#define HT_TESTMODE_WATCHDOG	4
#define HT_TESTMODE_LOOP	8

#define DNC_MCFG_BASE 0x3f0000000000ULL
#define DNC_MCFG_LIM  0x3ffeffffffffULL

extern u64 dnc_csr_base;
extern u64 dnc_csr_lim;

#define DNC_CSR_BASE (dnc_csr_base)
#define DNC_CSR_LIM (dnc_csr_lim)
#define DEF_DNC_CSR_BASE 0xffff00000000ULL
#define DEF_DNC_CSR_LIM  0xffffffffffffULL

#define PMIO_PORT 0xcd6

extern int lirq_nest;
#define cli() if (lirq_nest++ == 0) { asm volatile("cli"); }
#define sti() if (--lirq_nest == 0) { asm volatile("sti"); }
#define cpu_relax() asm volatile("pause" ::: "memory")

extern int cht_config_use_extd_addressing;
extern int ht_testmode;

static inline u64 rdtscll(void)
{
    u64 val;
    asm volatile ("rdtsc" : "=A" (val));
    return val;
}

static inline u32 u32bswap(u32 val)
{
    asm volatile("bswap %0" : "+r"(val));
    return val;
}

void pmio_writeb(u16 offset, u8 val);
void pmio_writel(u16 offset, u32 val);
u8 pmio_readb(u16 offset);
u16 pmio_reads(u16 offset);
u32 pmio_readl(u16 offset);
void pmio_setb(u16 offset, u8 val);
void pmio_clearb(u16 offset, u8 val);
void pmio_setl(u16 offset, u32 val);
void pmio_clearl(u16 offset, u32 val);
void watchdog_setup(void);
void reset_cf9(int mode, int last);
void cht_test(u8 node, int neigh, int neigh_link);
u32  cht_read_config(u8 node, u8 func, u16 reg);
void cht_write_config(u8 node, u8 func, u16 reg, u32 val);
u32 cht_read_config_nc(u8 node, u8 func, int neigh, int neigh_link, u16 reg);
void cht_write_config_nc(u8 node, u8 func, int neigh, int neigh_link, u16 reg, u32 val);
u32  mem64_read32(u64 addr);
void mem64_write32(u64 addr, u32 val);
u16  mem64_read16(u64 addr);
void mem64_write16(u64 addr, u16 val);
u8   mem64_read8(u64 addr);
void mem64_write8(u64 addr, u8 val);
u32  dnc_read_csr(u32 node, u16 csr);
void dnc_write_csr(u32 node, u16 csr, u32 val);
u32  dnc_read_csr_geo(u32 node, u8 bid, u16 csr);
void dnc_write_csr_geo(u32 node, u8 bid, u16 csr, u32 val);
u32  dnc_read_conf(u16 node, u8 bus, u8 device, u8 func, u16 reg);
void dnc_write_conf(u16 node, u8 bus, u8 device, u8 func, u16 reg, u32 val);
u64 dnc_rdmsr(u32 msr);
void dnc_wrmsr(u32 msr, u64 v);

#endif
