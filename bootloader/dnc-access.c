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

#define HT_REG(node, func, reg)									\
    (cht_config_use_extd_addressing ?							\
     PCI_EXT_CONF(255, ((((node)) << 3) | (func)), (reg)) :		\
     PCI_EXT_CONF(0, (((24 + (node)) << 3) | (func)), (reg)))

int lirq_nest = 0;

/* #define DEBUG(...) printf(__VA_ARGS__) */
#define DEBUG(...) do { } while (0)
#define WATCHDOG_BASE (uint32_t *)0xfec000f0 /* AMD recommended */

/* Pre-OS and OS may overwrite areas under this */
#define MALLOC_SAFE_START (64 << 20)

static volatile uint32_t *watchdog_base = 0;

int ht_testmode;
int cht_config_use_extd_addressing = 0;

uint64_t dnc_csr_base = DEF_DNC_CSR_BASE;
uint64_t dnc_csr_lim = DEF_DNC_CSR_LIM;

void dump(const void *addr, const unsigned len)
{
	const unsigned char *addr2 = (const unsigned char *)addr;
	unsigned i = 0;

	while (i < len) {
		for (int j = 0; j < 8 && (i + j) < len; j++)
			printf(" %02x", addr2[i + j]);
		i += 8;
		printf("\n");
	}
}

uint8_t rtc_read(const int addr)
{
	outb(addr, 0x70);
	uint8_t val = inb(0x71);

	/* Convert from BCD if needed */
	outb(RTC_SETTINGS, 0x70);
	uint8_t settings = inb(0x71);
	if (!(settings & 4))
		return (val & 0xf) + (val / 16) * 10;

	return val;
}

uint8_t pmio_read8(uint16_t offset)
{
	outb(offset, PMIO_PORT /* PMIO index */);
	return inb(PMIO_PORT + 1 /* PMIO data */);
}

void pmio_write8(uint16_t offset, uint8_t val)
{
	/* Write offset and value in single 16-bit write */
	outw(offset | val << 8, PMIO_PORT);

	if (verbose > 2) {
		uint8_t val2 = pmio_read8(offset);

		if (val2 != val)
			warning("PMIO reg 0x%02x readback (0x%02x) differs from write (0x%02x)", offset, val2, val);
	}
}

uint16_t pmio_read16(uint16_t offset)
{
	unsigned int i;
	uint16_t val = 0;

	for (i = 0; i < sizeof(val); i++)
		val |= pmio_read8(offset + i) << (i * 8);

	return val;
}

uint32_t pmio_read32(uint16_t offset)
{
	unsigned int i;
	uint32_t val = 0;

	for (i = 0; i < sizeof(val); i++)
		val |= pmio_read8(offset + i) << (i * 8);

	return val;
}

void pmio_write32(uint16_t offset, uint32_t val)
{
	unsigned int i;

	for (i = 0; i < sizeof(val); i++)
		pmio_write8(offset + i, val >> (i * 8));
}

void pmio_set8(uint16_t offset, uint8_t mask)
{
	uint8_t val = pmio_read8(offset) | mask;
	pmio_write8(offset, val);
}

void pmio_clear8(uint16_t offset, uint8_t mask)
{
	uint8_t val = pmio_read8(offset) & ~mask;
	pmio_write8(offset, val);
}

void pmio_set32(uint16_t offset, uint32_t mask)
{
	uint32_t val = pmio_read32(offset) | mask;
	pmio_write32(offset, val);
}

void pmio_clear32(uint16_t offset, uint32_t mask)
{
	uint32_t val = pmio_read8(offset) & ~mask;
	pmio_write32(offset, val);
}

uint32_t ioh_nbmiscind_read(const sci_t sci, const uint8_t reg)
{
	dnc_write_conf(sci, 0, 0, 0, 0x60, reg);
	return dnc_read_conf(sci, 0, 0, 0, 0x64);
}

void ioh_nbmiscind_write(const sci_t sci, const uint8_t reg, const uint32_t val)
{
	dnc_write_conf(sci, 0, 0, 0, 0x60, reg | 0x80);
	dnc_write_conf(sci, 0, 0, 0, 0x64, val);

	if (verbose > 2) {
		uint32_t val2 = ioh_nbmiscind_read(sci, reg);

		if (val2 != val)
			warning("IOH NBMISCIND reg 0x%02x readback (0x%08x) differs from write (0x%08x)", reg, val2, val);
	}
}

uint32_t ioh_nbpcieind_read(const sci_t sci, const uint8_t core, const uint8_t reg)
{
	dnc_write_conf(sci, 0, 0, 0, 0xe0, reg | ((uint32_t)core << 16));
	return dnc_read_conf(sci, 0, 0, 0, 0xe4);
}

void ioh_nbpcieind_write(const sci_t sci, const uint8_t core, const uint8_t reg, const uint32_t val)
{
	dnc_write_conf(sci, 0, 0, 0, 0xe0, reg | ((uint32_t)core << 16));
	dnc_write_conf(sci, 0, 0, 0, 0xe4, val);
}

uint32_t ioh_htiu_read(const sci_t sci, uint8_t reg)
{
	dnc_write_conf(sci, 0, 0, 0, 0x94, reg);
	return dnc_read_conf(sci, 0, 0, 0, 0x98);
}

void ioh_htiu_write(const sci_t sci, uint8_t reg, uint32_t val)
{
	dnc_write_conf(sci, 0, 0, 0, 0x94, reg | 0x100);
	dnc_write_conf(sci, 0, 0, 0, 0x98, val);

	if (verbose > 2) {
		uint32_t val2 = ioh_htiu_read(sci, reg);

		if (val2 != val)
			warning("IOH HTIU reg 0x%02x readback (0x%08x) differs from write (0x%08x)", reg, val2, val);
	}
}

uint32_t ioh_ioapicind_read(const uint16_t sci, const uint8_t reg)
{
	dnc_write_conf(sci, 0, 0, 0, 0xf8, reg);
	return dnc_read_conf(sci, 0, 0, 0, 0xfc);
}

void ioh_ioapicind_write(const uint16_t sci, const uint8_t reg, const uint32_t val)
{
	dnc_write_conf(sci, 0, 0, 0, 0xf8, reg);
	dnc_write_conf(sci, 0, 0, 0, 0xfc, val);
}

static void watchdog_write(const uint8_t reg, const uint32_t val)
{
	if (verbose >= 2)
		printf("watchdog[%u] -> 0x%08x\n", reg, val);
	assert(watchdog_base);
	watchdog_base[reg] = val;
}

void watchdog_run(const unsigned counter)
{
	watchdog_write(0, 0x81); /* WatchDogRunStopB | WatchDogTrigger */
	watchdog_write(1, counter); /* in centiseconds */
	watchdog_write(0, 0x81);
}

void watchdog_stop(void)
{
	watchdog_write(0, 0);
}

void watchdog_setup(void)
{
	/* Enable watchdog timer */
	pmio_clear8(0x69, 1);
	/* Enable watchdog decode */
	uint32_t val2 = dnc_read_conf(0xfff0, 0, 20, 0, 0x41);
	dnc_write_conf(0xfff0, 0, 20, 0, 0x41, val2 | (1 << 3));

	watchdog_base = WATCHDOG_BASE;
	/* Write watchdog base address */
	pmio_write32(0x6c, (unsigned int)watchdog_base);
}

void reset_cf9(int mode, int last)
{
	int i;

	for (i = 0; i <= last; i++) {
		uint32_t val = cht_read_conf(i, FUNC0_HT, 0x6c);
		cht_write_conf(i, FUNC0_HT, 0x6c, val | 0x20); /* BiosRstDet */
	}

	/* Ensure console drains */
	udelay(1000000);

	outb(mode, 0xcf9);
	outb(mode | 4, 0xcf9);
}

static uint32_t _read_config(uint8_t bus, uint8_t dev, uint8_t func, uint16_t reg)
{
	uint32_t ret;
	DEBUG("pci:%02x:%02x.%x %03x -> ", bus, dev, func, reg);
	cli();
	outl(PCI_EXT_CONF(bus, ((dev << 3) | func), reg), PCI_CONF_SEL);
	ret = inl(PCI_CONF_DATA);
	sti();
	DEBUG("%08x\n", ret);
	return ret;
}

static void _write_config(uint8_t bus, uint8_t dev, uint8_t func, uint16_t reg, uint32_t val)
{
	DEBUG("pci:%02x:%02x.%x %03x <- %08x", bus, dev, func, reg, val);
	cli();
	outl(PCI_EXT_CONF(bus, ((dev << 3) | func), reg), PCI_CONF_SEL);
	outl(val, PCI_CONF_DATA);
	sti();
	DEBUG("\n");

	if (verbose > 2) {
		uint32_t val2 = _read_config(bus, dev, func, reg);

		if (val2 != val)
			warning("Config %02x:%02x.%x reg 0x%x readback (0x%08x) differs from write (0x%08x)", bus, dev, func, reg, val2, val);
	}
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

	if (verbose > 2) {
		/* Ignore link phy offset */
		if (func == 4 && reg == 0x190)
			return;

		uint32_t val2 = cht_read_conf(node, func, reg);

		if (val2 != val)
			warning("Coherent config HT%xF%xx%X readback (0x%08x) differs from write (0x%08x)", node, func, reg, val2, val);
	}
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
	bool reboot;

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
			warning("Undetected link error (HT%d F%dx%02x read 0xffffffff)",
			       node, func, reg);
			reboot = 1;
		}
	}

	if (reboot)
		fatal_reboot("Link error while reading");

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

uint32_t dnc_read_conf(const sci_t sci, const uint8_t bus, const uint8_t device, const uint8_t func, const uint16_t reg)
{
	if (sci == 0xfff0)
		return _read_config(bus, device, func, reg);

	DEBUG("SCI%03x:dev%02x:%02x F%xx%03x:  ", sci, bus, device, func, reg);
	uint32_t val = mem64_read32(DNC_MCFG_BASE | ((uint64_t)sci << 28) | PCI_MMIO_CONF(bus, device, func, reg));
	DEBUG("%08x\n", val);

	return val;
}

uint64_t dnc_read_conf64(const sci_t sci, const uint8_t bus, const uint8_t device, const uint8_t func, const uint16_t reg)
{
	uint64_t addr = DNC_MCFG_BASE | ((uint64_t)sci << 28) | PCI_MMIO_CONF(bus, device, func, reg);

	/* Processor will issue two 4-byte non-coherent reads */
	if (sci == 0xfff0 || sci == nodes[0].sci)
		return mem64_read64(addr);

	/* Use discrete reads for non-local access for now */
	return ((uint64_t)mem64_read32(addr + 4) << 32) | mem64_read32(addr);
}

void dnc_write_conf(const sci_t sci, const uint8_t bus, const uint8_t device, const uint8_t func, const uint16_t reg, const uint32_t val)
{
	if (sci == 0xfff0) {
		_write_config(bus, device, func, reg, val);
		return;
	}

	DEBUG("SCI%03x:dev%02x:%02x F%xx%03x <- %08x", sci, bus, device, func, reg, val);
	mem64_write32(DNC_MCFG_BASE | ((uint64_t)sci << 28) | PCI_MMIO_CONF(bus, device, func, reg), val);
	DEBUG("\n");
}

void dnc_write_conf64_split(const sci_t sci, const uint8_t bus, const uint8_t device, const uint8_t func, const uint16_t reg, const uint64_t val)
{
	uint64_t addr = DNC_MCFG_BASE | ((uint64_t)sci << 28) | PCI_MMIO_CONF(bus, device, func, reg);

	/* Processor does not support single 64-bit transaction */
	mem64_write32(addr, val);
	mem64_write32(addr + 4, val >> 32);
}

uint64_t rdmsr(const uint32_t msr)
{
	union {
		uint32_t dw[2];
		uint64_t qw;
	} val;
	asm volatile("rdmsr" : "=d"(val.dw[1]), "=a"(val.dw[0]) : "c"(msr));
	return val.qw;
}


void wrmsr(const uint32_t msr, const uint64_t v)
{
	union {
		uint32_t dw[2];
		uint64_t qw;
	} val;
	val.qw = v;
	asm volatile("wrmsr" :: "d"(val.dw[1]), "a"(val.dw[0]), "c"(msr));

	if (verbose > 2) {
		uint64_t v2 = rdmsr(msr);

		if (v2 != v)
			warning("MSR 0x%08x readback (0x%016llx) differs from write (0x%016llx)", msr, v2, v);
	}
}

/* Allocate memory at top near ACPI area to avoid conflicts */
void *zalloc_persist(const size_t size)
{
	void *allocs[32]; /* Enough for 4GB of 128MB allocations */
	int nallocs = 0;

	/* Allocate 128MB blocks until exhaustion */
	while ((allocs[nallocs] = malloc(128 << 20)))
		nallocs++;

	/* Free last one guaranteeing space for allocation */
	free(allocs[nallocs--]);

	/* Allocate requested size */
	void *ptr = zalloc(size);
	assert(ptr);

	/* Free remaining allocations */
	while (nallocs)
		free(allocs[nallocs--]);

	return ptr;
}
