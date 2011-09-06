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

#ifndef __DNC_ACCESS
#define __DNC_ACCESS 1

#include <sys/io.h>
#include "dnc-types.h"

#define HT_TESTMODE_PRINT	1
#define HT_TESTMODE_TEST	2
#define HT_TESTMODE_WATCHDOG	4

#define DNC_MCFG_BASE 0x3f0000000000ULL
#define DNC_MCFG_LIM  0x3ffeffffffffULL

extern u64 dnc_csr_base;
extern u64 dnc_csr_lim;

#define DNC_CSR_BASE (dnc_csr_base)
#define DNC_CSR_LIM (dnc_csr_lim)
#define DEF_DNC_CSR_BASE 0xffff00000000ULL
#define DEF_DNC_CSR_LIM  0xffffffffffffULL

extern int cht_config_use_extd_addressing;
extern int ht_testmode;

static inline u32 u32bswap(u32 val)
{
    asm volatile("bswap %0" : "+r"(val));
    return val;
}

void watchdog_setup(void);
void reset_cf9(int mode, int last);
void cht_test(u8 node, u8 func, int neigh, int neigh_link, u16 reg, u32 expect);
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
