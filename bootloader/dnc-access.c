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

#include <stdio.h>

#include "dnc-access.h"

#define PCI_CONF_SEL 0xcf8
#define PCI_CONF_DATA 0xcfc

#define PCI_EXT_CONF(bus, devfn, reg)                           \
    (0x80000000 | (((reg) & 0xF00) << 16) | ((bus) << 16)       \
     | ((devfn) << 8) | ((reg) & 0xFC))
#define PCI_MMIO_CONF(bus, device, func, reg)                   \
    (((bus) << 20) | ((device) << 15) | ((func) << 12) | (reg))

#define HT_REG(node, func, reg)				\
    (cht_config_use_extd_addressing ?			\
     PCI_EXT_CONF(255, ((((node))<<3)|(func)), (reg)) :	\
     PCI_EXT_CONF(0, (((24+(node))<<3)|(func)), (reg)))

#define cli() asm volatile("cli")
#define sti() asm volatile("sti")

//#define DEBUG(...) printf(__VA_ARGS__)
#define DEBUG(...) do { } while (0)

int cht_config_use_extd_addressing = 0;

u64 dnc_csr_base = DEF_DNC_CSR_BASE;
u64 dnc_csr_lim = DEF_DNC_CSR_LIM;

extern void sleep(unsigned int);

void reset_cf9(int mode, int last)
{
    int i;

    /* ensure last lines were sent from management controller */
    sleep(1000);

    for (i = 0; i <= last; i++) {
	u32 val = cht_read_config(i, NB_FUNC_HT, 0x6c);
	cht_write_config(i, NB_FUNC_HT, 0x6c, val | 0x20); /* BiosRstDet */
    }
    outb(mode, 0xcf9);
    outb(mode | 4, 0xcf9);
}

static u32 _read_config(u8 bus, u8 dev, u8 func, u16 reg)
{
    u32 ret;
    cli();
    outl(PCI_EXT_CONF(bus, ((dev<<3)|func), reg), PCI_CONF_SEL);
    ret = inl(PCI_CONF_DATA);
    sti();
    DEBUG("pci:%02x:%02x.%x %03x -> %08x\n",
	  bus, dev, func, reg, ret);
    return ret;
}

static void _write_config(u8 bus, u8 dev, u8 func, u16 reg, u32 val)
{
    DEBUG("pci:%02x:%02x.%x %03x <- %08x\n",
	  bus, dev, func, reg, val);
    cli();
    outl(PCI_EXT_CONF(bus, ((dev<<3)|func), reg), PCI_CONF_SEL);
    outl(val, PCI_CONF_DATA);
    sti();
}

u32 cht_read_config(u8 node, u8 func, u16 reg)
{
    u32 ret;
    cli();
    outl(HT_REG(node, func, reg), PCI_CONF_SEL);
    ret = inl(PCI_CONF_DATA);
    sti();
    DEBUG("HT#%d F%xx%03x -> %08x\n",
	  node, func, reg, ret);
    return ret;
}

void cht_write_config(u8 node, u8 func, u16 reg, u32 val)
{
    DEBUG("HT#%d F%xx%03x <- %08x\n",
	  node, func, reg, val);
    cli();
    outl(HT_REG(node, func, reg), PCI_CONF_SEL);
    outl(val, PCI_CONF_DATA);
    sti();
}

/* check for link instability */
static int link_error(int node, int link)
{
    u32 status = cht_read_config(node, NB_FUNC_HT, 0x84 + link * 0x20);
    return status & ((3 << 8) | (1 << 4)); /* CrcErr, LinkFail */
}

u32 cht_read_config_nc(u8 node, u8 func, int neigh, int neigh_link, u16 reg)
{
    u32 ret;
    int reboot;

    ret = cht_read_config(node, func, reg);

    /* only check for Target Abort if unable to check link */
    if (neigh == -1 || neigh_link == -1)
	reboot = ret == 0xffffffff;
    else {
	reboot = link_error(neigh, neigh_link);
	if (!reboot && ret == 0xffffffff) {
	    printf("Warning: undetected link error (read 0xffffffff)\n");
	    reboot = 1;
	}
    }

    if (reboot) {
	printf("Link error while reading; resetting system...\n");
	reset_cf9(2, neigh);
    }

    return ret;
}

void cht_write_config_nc(u8 node, u8 func, int neigh, int neigh_link, u16 reg, u32 val)
{
    cht_write_config(node, func, reg, val);

    /* only check for Target Abort if unable to check link */
    if (neigh != -1 && neigh_link != -1 && link_error(neigh, neigh_link)) {
	printf("Link error while writing; resetting system...\n");
	reset_cf9(2, neigh);
    }
}

// Since we use FS to access these areas, the address needs to be in canonical form (sign extended from bit47).
#define canonicalize(a) (((a) & (1ULL<<47)) ? ((a) | (0xffffULL<<48)) : (a))

#define setup_fs(addr) do {                                             \
        asm volatile("mov %%ds, %%ax\n\tmov %%ax, %%fs" ::: "eax");     \
        asm volatile("wrmsr"                                            \
                     : /* no output */                                  \
                     : "A"(canonicalize(addr)), "c"(MSR_FS_BASE));      \
    } while(0)

u32 mem64_read32(u64 addr)
{
    u32 ret;
    cli();
    setup_fs(addr);
    asm volatile("mov %%fs:(0), %%eax" : "=a"(ret));
    sti();
    return ret;
}

void mem64_write32(u64 addr, u32 val)
{
    cli();
    setup_fs(addr);
    asm volatile("mov %0, %%fs:(0)" :: "a"(val));
    sti();
}

u16 mem64_read16(u64 addr)
{
    u16 ret;
    cli();
    setup_fs(addr);
    asm volatile("movw %%fs:(0), %%ax" : "=a"(ret));
    sti();
    return ret;
}

void mem64_write16(u64 addr, u16 val)
{
    cli();
    setup_fs(addr);
    asm volatile("movw %0, %%fs:(0)" :: "a"(val));
    sti();
}

u8 mem64_read8(u64 addr)
{
    u8 ret;
    cli();
    setup_fs(addr);
    asm volatile("movb %%fs:(0), %%al" : "=a"(ret));
    sti();
    return ret;
}

void mem64_write8(u64 addr, u8 val)
{
    cli();
    setup_fs(addr);
    asm volatile("movb %0, %%fs:(0)" :: "a"(val));
    sti();
}

u32 dnc_read_csr(u32 node, u16 csr)
{
    u32 val;
    val = u32bswap(mem64_read32(DNC_CSR_BASE | (node << 16) | 0x8000 | csr));
    DEBUG("sci%04x:csr%04x :  %08x\n", node, csr, val);
    return val;
}

void dnc_write_csr(u32 node, u16 csr, u32 val)
{
    DEBUG("sci%04x:csr%04x <- %08x\n", node, csr, val);
    mem64_write32(DNC_CSR_BASE | (node << 16) | 0x8000 | csr, u32bswap(val));
}


u32 dnc_read_csr_geo(u32 node, u8 bid, u16 csr)
{
    if (csr >= 0x800) {
	printf("*** dnc_write_csr_geo: read from unsupported range: "
	       "%04x#%d @%x\n",
	       node, bid, csr);
	return 0xffffffff;
    }

    return u32bswap(mem64_read32(DNC_CSR_BASE | (node << 16) | (bid << 11) | csr));
}

void dnc_write_csr_geo(u32 node, u8 bid, u16 csr, u32 val)
{
    if (csr >= 0x800) {
	printf("*** dnc_write_csr_geo: write to unsupported range: "
	       "%04x#%d @%x, %08x\n",
	       node, bid, csr, val);
	return;
    }

    mem64_write32(DNC_CSR_BASE | (node << 16) | (bid << 11) | csr, u32bswap(val));
}


u32 dnc_read_conf(u16 node, u8 bus, u8 device, u8 func, u16 reg)
{
    u32 val;
    if (node == 0xfff0) {
        val = _read_config(bus, device, func, reg);
    } else {
        val = mem64_read32(DNC_MCFG_BASE | ((u64)node << 28) | 
                           PCI_MMIO_CONF(bus, device, func, reg));
        DEBUG("sci%04x:dev%02x:%02x F%xx%03x :  %08x\n",
              node, bus, device, func, reg, val);
    }
    return val;
}

void dnc_write_conf(u16 node, u8 bus, u8 device, u8 func, u16 reg, u32 val)
{
    if (node == 0xfff0) {
        _write_config(bus, device, func, reg, val);
    } else {
        DEBUG("sci%04x:dev%02x:%02x F%xx%03x <- %08x\n",
              node, bus, device, func, reg, val);
        mem64_write32(DNC_MCFG_BASE | ((u64)node << 28) | 
                      PCI_MMIO_CONF(bus, device, func, reg), val);
    }
}

u64 dnc_rdmsr(u32 msr)
{
    union {
        u32 dw[2];
        u64 qw;
    } val;
    asm volatile("rdmsr" : "=d"(val.dw[1]), "=a"(val.dw[0]) : "c"(msr));
    return val.qw;
}


void dnc_wrmsr(u32 msr, u64 v)
{
    union {
        u32 dw[2];
        u64 qw;
    } val;
    val.qw = v;
    asm volatile("wrmsr" :: "d"(val.dw[1]), "a"(val.dw[0]), "c"(msr));
}
