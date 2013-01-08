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

#include <stdio.h>
#include <unistd.h>

#include "dnc-regs.h"
#include "dnc-defs.h"
#include "dnc-bootloader.h"
#include "dnc-commonlib.h"
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

int lirq_nest = 0;

//#define DEBUG(...) printf(__VA_ARGS__)
#define DEBUG(...) do { } while (0)

#define WATCHDOG_BASE (uint32_t *)0xfec000f0 /* AMD recommended */

static volatile uint32_t *watchdog_ctl = WATCHDOG_BASE;
static volatile uint32_t *watchdog_timer = WATCHDOG_BASE + 1;

int ht_testmode;
int cht_config_use_extd_addressing = 0;

uint64_t dnc_csr_base = DEF_DNC_CSR_BASE;
uint64_t dnc_csr_lim = DEF_DNC_CSR_LIM;

void pmio_writeb(uint16_t offset, uint8_t val)
{
    /* Write offset and value in single 16-bit write */
    outw(offset | val << 8, PMIO_PORT);
}

void pmio_writel(uint16_t offset, uint32_t val)
{
    unsigned int i;

    for (i = 0; i < sizeof(val); i++)
	pmio_writeb(offset + i, val >> (i * 8));
}

uint8_t pmio_readb(uint16_t offset)
{
    outb(offset, PMIO_PORT /* PMIO index */);
    return inb(PMIO_PORT + 1 /* PMIO data */);
}

uint16_t pmio_reads(uint16_t offset)
{
    unsigned int i;
    uint16_t val = 0;

    for (i = 0; i < sizeof(val); i++)
	val |= pmio_readb(offset + i) << (i * 8);

    return val;
}

uint32_t pmio_readl(uint16_t offset)
{
    unsigned int i;
    uint32_t val = 0;

    for (i = 0; i < sizeof(val); i++)
	val |= pmio_readb(offset + i) << (i * 8);

    return val;
}

void pmio_setb(uint16_t offset, uint8_t mask)
{
    uint8_t val = pmio_readb(offset) | mask;
    pmio_writeb(offset, val);
}

void pmio_clearb(uint16_t offset, uint8_t mask)
{
    uint8_t val = pmio_readb(offset) & ~mask;
    pmio_writeb(offset, val);
}

void pmio_setl(uint16_t offset, uint32_t mask)
{
    uint32_t val = pmio_readl(offset) | mask;
    pmio_writel(offset, val);
}

void pmio_clearl(uint16_t offset, uint32_t mask)
{
    uint32_t val = pmio_readb(offset) & ~mask;
    pmio_writel(offset, val);
}

uint32_t ioh_nbmiscind_read(uint16_t node, uint8_t reg) {
    dnc_write_conf(node, 0, 0, 0, 0x60, reg);
    return dnc_read_conf(node, 0, 0, 0, 0x64);
}

void ioh_nbmiscind_write(uint16_t node, uint8_t reg, uint32_t val) {
    dnc_write_conf(node, 0, 0, 0, 0x60, reg | 0x80);
    dnc_write_conf(node, 0, 0, 0, 0x64, val);
}

uint32_t ioh_htiu_read(uint16_t node, uint8_t reg) {
    dnc_write_conf(node, 0, 0, 0, 0x94, reg);
    return dnc_read_conf(node, 0, 0, 0, 0x98);
}

void ioh_htiu_write(uint16_t node, uint8_t reg, uint32_t val) {
    dnc_write_conf(node, 0, 0, 0, 0x94, reg | 0x100);
    dnc_write_conf(node, 0, 0, 0, 0x98, val);
}

static inline void watchdog_run(unsigned int counter)
{
    *watchdog_ctl = 0x81; /* WatchDogRunStopB | WatchDogTrigger */
    *watchdog_timer = counter; /* in centiseconds */
    *watchdog_ctl = 0x81;
}

static inline void watchdog_stop(void)
{
    *watchdog_ctl = 0;
}

void watchdog_setup(void)
{
    /* FIXME: These register offsets are correct for SP5100, but not for (eg) SB810 */

    /* Enable watchdog timer */
    pmio_clearb(0x69, 1);

    /* Enable watchdog decode */
    uint32_t val2 = dnc_read_conf(0xfff0, 0, 20, 0, 0x41);
    dnc_write_conf(0xfff0, 0, 20, FUNC0_HT, 0x41, val2 | (1 << 3));

    /* Write watchdog base address */
    pmio_writel(0x6c, (unsigned int)watchdog_ctl);
}

void reset_cf9(int mode, int last)
{
    int i;

    /* Ensure last lines were sent from management controller */
    msleep(1000);

    for (i = 0; i <= last; i++) {
	uint32_t val = cht_read_conf(i, FUNC0_HT, 0x6c);
	cht_write_conf(i, FUNC0_HT, 0x6c, val | 0x20); /* BiosRstDet */
    }
    outb(mode, 0xcf9);
    outb(mode | 4, 0xcf9);
}

static uint32_t _read_config(uint8_t bus, uint8_t dev, uint8_t func, uint16_t reg)
{
    uint32_t ret;
    DEBUG("pci:%02x:%02x.%x %03x -> ",
	  bus, dev, func, reg);
    cli();
    outl(PCI_EXT_CONF(bus, ((dev<<3)|func), reg), PCI_CONF_SEL);
    ret = inl(PCI_CONF_DATA);
    sti();
    DEBUG("%08x\n", ret);
    return ret;
}

static void _write_config(uint8_t bus, uint8_t dev, uint8_t func, uint16_t reg, uint32_t val)
{
    DEBUG("pci:%02x:%02x.%x %03x <- %08x",
	  bus, dev, func, reg, val);
    cli();
    outl(PCI_EXT_CONF(bus, ((dev<<3)|func), reg), PCI_CONF_SEL);
    outl(val, PCI_CONF_DATA);
    sti();
    DEBUG("\n");
}

uint32_t cht_read_conf(uint8_t node, uint8_t func, uint16_t reg)
{
    uint32_t ret;
    DEBUG("HT#%d F%xx%03x -> ",
	  node, func, reg);
    cli();
    outl(HT_REG(node, func, reg), PCI_CONF_SEL);
    ret = inl(PCI_CONF_DATA);
    sti();
    DEBUG("%08x\n", ret);
    return ret;
}

void cht_write_conf(uint8_t node, uint8_t func, uint16_t reg, uint32_t val)
{
    DEBUG("HT#%d F%xx%03x <- %08x",
	  node, func, reg, val);
    cli();
    outl(HT_REG(node, func, reg), PCI_CONF_SEL);
    outl(val, PCI_CONF_DATA);
    sti();
    DEBUG("\n");
}

/* Check for link instability */
static int cht_error(int node, int link)
{
    uint32_t status = cht_read_conf(node, FUNC0_HT, 0x84 + link * 0x20);
    return status & ((3 << 8) | (1 << 4)); /* CrcErr, LinkFail */
}

void cht_test(uint8_t node, int neigh, int neigh_link)
{
    static int loop = 0, errors = 0;
    uint32_t base;
    int i;

    if (ht_testmode & HT_TESTMODE_WATCHDOG) {
	printf("Testing HT link (5s timeout)...");
	watchdog_run(500); /* Reset if read hangs due to unstable link */
    } else
	printf("Testing HT link...");

    if (ht_testmode & HT_TESTMODE_LOOP)
	printf("loop %d ", loop++);

    base = cht_read_conf(node, 1, H2S_CSR_F1_MMIO_BASE_ADDRESS_REGISTERS);

    for (i = 0; i < 2000000; i++) {
	uint32_t h = i;
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	h &= ~0xfc; /* Clear read-only bits */

	cht_write_conf(node, 1, H2S_CSR_F1_MMIO_BASE_ADDRESS_REGISTERS, h);
	if (cht_read_conf(node, 1, H2S_CSR_F1_MMIO_BASE_ADDRESS_REGISTERS) != h)
	    errors++;
    }

    /* Restore previous value */
    cht_write_conf(node, 1, H2S_CSR_F1_MMIO_BASE_ADDRESS_REGISTERS, base);

    if (ht_testmode & HT_TESTMODE_WATCHDOG)
	watchdog_stop();

    if (!(ht_testmode & HT_TESTMODE_LOOP) && (errors || cht_error(neigh, neigh_link))) {
	printf("%d link errors while reading; resetting system...\n", errors);
	/* Cold reset, since warm doesn't always reset the link enough */
	reset_cf9(0xa, neigh);
    }

    printf("%d errors\n", errors);
}

#ifdef UNUSED
uint32_t cht_read_conf_nc(uint8_t node, uint8_t func, int neigh, int neigh_link, uint16_t reg)
{
    uint32_t ret;
    int reboot;

    if (ht_testmode & HT_TESTMODE_WATCHDOG)
	watchdog_run(100); /* 1s timeout if read hangs due to unstable link */

    ret = cht_read_conf(node, func, reg);

    if (ht_testmode & HT_TESTMODE_WATCHDOG)
	watchdog_stop();

    /* Only check for Target Abort if unable to check link */
    if (neigh == -1 || neigh_link == -1)
	reboot = ret == 0xffffffff;
    else {
	reboot = cht_error(neigh, neigh_link);
	if (!reboot && ret == 0xffffffff) {
	    printf("Warning: undetected link error (HT%d F%dx%02x read 0xffffffff)\n",
		   node, func, reg);
	    reboot = 1;
	}
    }

    if (reboot) {
	printf("Link error while reading; resetting system...\n");
	/* Cold reset, since warm doesn't always reset the link enough */
	reset_cf9(0xa, neigh);
    }

    return ret;
}

void cht_write_conf_nc(uint8_t node, uint8_t func, int neigh, int neigh_link, uint16_t reg, uint32_t val)
{
    if (ht_testmode & HT_TESTMODE_WATCHDOG)
	watchdog_run(100); /* 1s timeout if write hangs due to unstable link */

    cht_write_conf(node, func, reg, val);

    if (ht_testmode & HT_TESTMODE_WATCHDOG)
	watchdog_stop();

    /* Only check for Target Abort if unable to check link */
    if (neigh != -1 && neigh_link != -1 && cht_error(neigh, neigh_link)) {
	printf("Link error while writing; resetting system...\n");
	/* Cold reset, since warm doesn't always reset the link enough */
	reset_cf9(0xa, neigh);
    }
}
#endif

/* Since we use FS to access these areas, the address needs to be in canonical form (sign extended from bit47) */
#define canonicalize(a) (((a) & (1ULL<<47)) ? ((a) | (0xffffULL<<48)) : (a))

#define setup_fs(addr) do {                                             \
        asm volatile("mov %%ds, %%ax\n\tmov %%ax, %%fs" ::: "eax");     \
        asm volatile("wrmsr"                                            \
                     : /* No output */                                  \
                     : "A"(canonicalize(addr)), "c"(MSR_FS_BASE));      \
    } while(0)

uint32_t mem64_read32(uint64_t addr)
{
    uint32_t ret;
    cli();
    setup_fs(addr);
    asm volatile("mov %%fs:(0), %%eax" : "=a"(ret));
    sti();
    return ret;
}

void mem64_write32(uint64_t addr, uint32_t val)
{
    cli();
    setup_fs(addr);
    asm volatile("mov %0, %%fs:(0)" :: "a"(val));
    sti();
}

uint16_t mem64_read16(uint64_t addr)
{
    uint16_t ret;
    cli();
    setup_fs(addr);
    asm volatile("movw %%fs:(0), %%ax" : "=a"(ret));
    sti();
    return ret;
}

void mem64_write16(uint64_t addr, uint16_t val)
{
    cli();
    setup_fs(addr);
    asm volatile("movw %0, %%fs:(0)" :: "a"(val));
    sti();
}

uint8_t mem64_read8(uint64_t addr)
{
    uint8_t ret;
    cli();
    setup_fs(addr);
    asm volatile("movb %%fs:(0), %%al" : "=a"(ret));
    sti();
    return ret;
}

void mem64_write8(uint64_t addr, uint8_t val)
{
    cli();
    setup_fs(addr);
    asm volatile("movb %0, %%fs:(0)" :: "a"(val));
    sti();
}

uint32_t dnc_read_csr(uint32_t node, uint16_t csr)
{
    uint32_t val;
    DEBUG("SCI%03x:csr%04x :  ", node, csr);
    val = uint32_tbswap(mem64_read32(DNC_CSR_BASE | (node << 16) | 0x8000 | csr));
    DEBUG("%08x\n", val);
    return val;
}

void dnc_write_csr(uint32_t node, uint16_t csr, uint32_t val)
{
    DEBUG("SCI%03x:csr%04x <- %08x", node, csr, val);
    mem64_write32(DNC_CSR_BASE | (node << 16) | 0x8000 | csr, uint32_tbswap(val));
    DEBUG("\n");
}


uint32_t dnc_read_csr_geo(uint32_t node, uint8_t bid, uint16_t csr)
{
    if (csr >= 0x800) {
	printf("Error: dnc_write_csr_geo read from unsupported range: "
	       "%04x#%d @%x\n",
	       node, bid, csr);
	return 0xffffffff;
    }

    return uint32_tbswap(mem64_read32(DNC_CSR_BASE | (node << 16) | (bid << 11) | csr));
}

void dnc_write_csr_geo(uint32_t node, uint8_t bid, uint16_t csr, uint32_t val)
{
    if (csr >= 0x800) {
	printf("Error: dnc_write_csr_geo write to unsupported range: "
	       "%04x#%d @%x, %08x\n",
	       node, bid, csr, val);
	return;
    }

    mem64_write32(DNC_CSR_BASE | (node << 16) | (bid << 11) | csr, uint32_tbswap(val));
}


uint32_t dnc_read_conf(uint16_t node, uint8_t bus, uint8_t device, uint8_t func, uint16_t reg)
{
    uint32_t val;
    if (node == 0xfff0) {
        val = _read_config(bus, device, func, reg);
    } else {
        DEBUG("SCI%03x:dev%02x:%02x F%xx%03x:  ",
              node, bus, device, func, reg);
        val = mem64_read32(DNC_MCFG_BASE | ((uint64_t)node << 28) | 
                           PCI_MMIO_CONF(bus, device, func, reg));
        DEBUG("%08x\n", val);
    }
    return val;
}

void dnc_write_conf(uint16_t node, uint8_t bus, uint8_t device, uint8_t func, uint16_t reg, uint32_t val)
{
    if (node == 0xfff0) {
        _write_config(bus, device, func, reg, val);
    } else {
        DEBUG("SCI%03x:dev%02x:%02x F%xx%03x <- %08x",
              node, bus, device, func, reg, val);
        mem64_write32(DNC_MCFG_BASE | ((uint64_t)node << 28) | 
                      PCI_MMIO_CONF(bus, device, func, reg), val);
        DEBUG("\n");
    }
}

uint64_t dnc_rdmsr(uint32_t msr)
{
    union {
        uint32_t dw[2];
        uint64_t qw;
    } val;
    asm volatile("rdmsr" : "=d"(val.dw[1]), "=a"(val.dw[0]) : "c"(msr));
    return val.qw;
}


void dnc_wrmsr(uint32_t msr, uint64_t v)
{
    union {
        uint32_t dw[2];
        uint64_t qw;
    } val;
    val.qw = v;
    asm volatile("wrmsr" :: "d"(val.dw[1]), "a"(val.dw[0]), "c"(msr));
}
