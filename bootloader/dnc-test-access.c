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

#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "dnc-access.h"

#define MCFG_LEN (1ULL<<28)  /* MMCFG space is 256MB per node */
#define CSR_LEN  (1ULL<<16)  /* CSR space is 64KB per node */

#define PCI_MMIO_CONF(bus, device, func, reg)                   \
    (((bus) << 20) | ((device) << 15) | ((func) << 12) | (reg))

int cht_config_use_extd_addressing = 0;
int lirq_nest = 0;

uint64_t dnc_csr_base = DEF_DNC_CSR_BASE;
uint64_t dnc_csr_lim = DEF_DNC_CSR_LIM;

static int devmemfd = -1;
static inline int getdevmemfd(void)
{
	if (devmemfd < 0)  {
		devmemfd = open("/dev/mem", O_RDWR);
	}

	return devmemfd;
}

static int cfgfd[32][8] = {
	{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}
};

static int get_config_space(uint8_t bus, uint8_t device, uint8_t func)
{
	char cfgpath[256];

	if (bus != 0)
		return -1;

	if (device > 31)
		return -1;

	if (func > 7)
		return -1;

	if (cfgfd[device][func] > 0)
		return cfgfd[device][func];

	snprintf(cfgpath, 255,
	         "/sys/devices/pci0000:%02x/0000:%02x:%02x.%d/config",
	         bus, bus, device, func);
	cfgpath[255] = '\0';
	cfgfd[device][func] = open(cfgpath, O_RDWR);

	if (cfgfd[device][func] < 0) {
		fprintf(stderr, "*** Unable to open <%s>\n", cfgpath);
		return -1;
	}

	return cfgfd[device][func];
}

static uint32_t _read_config(uint8_t bus, uint8_t dev, uint8_t func, uint16_t reg)
{
	int cfg;
	uint32_t val;
	cfg = get_config_space(bus, dev, func);

	if (cfg <= 0)
		return 0xffffffff;

	if (pread64(cfg, &val, 4, reg) != 4)
		return 0xffffffff;

	return val;
}

static void _write_config(uint8_t bus, uint8_t dev, uint8_t func, uint16_t reg, uint32_t val)
{
	int cfg;
	cfg = get_config_space(bus, dev, func);

	if (cfg <= 0)
		return;

	assert(pwrite64(cfg, &val, 4, reg) == 4);
}

uint32_t cht_read_conf(uint8_t node, uint8_t func, uint16_t reg)
{
	return _read_config(0, node + 24, func, reg);
}

void cht_write_conf(uint8_t node, uint8_t func, uint16_t reg, uint32_t val)
{
	_write_config(0, node + 24, func, reg, val);
}

#if !defined(PAGE_LEN)
#define PAGE_LEN 0x1000ULL
#endif

static void *_map_mem64(uint64_t addr, uint64_t len)
{
	static int memfd = -1;
	static struct {
		uint64_t addr;
		uint64_t len;
		char *mem;
	} maps[16];
	static unsigned int map_next = 0, map_cur = 0;
	char *mem = NULL;
	unsigned int i;

	if (!len)
		len = PAGE_LEN;

	if (memfd < 0) {
		memfd = open("/dev/mem", O_RDWR);

		if (memfd < 0) {
			fprintf(stderr, "Unable to open </dev/mem>\n");
			exit(-1);
		}
	}

	for (i = 0; i < map_next; i++) {
		/* XXX: need better overlap check with other maps */
		if ((maps[i].addr == addr) && (maps[i].len == len)) {
			mem = maps[i].mem;
			break;
		}
	}

	if (!mem) {
		if (map_next < sizeof(maps) / sizeof(maps[0])) {
			i = map_next++;
		} else {
			i = map_cur;
			map_cur = (map_cur + 1) % 16;
			munmap(maps[i].mem, maps[i].len);
		}

		mem = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, memfd,
		           addr & ~(PAGE_LEN - 1));

		if (mem == MAP_FAILED) {
			fprintf(stderr, "Mapping failed! %d\n", errno);
			exit(0);
		}

		maps[i].addr = addr;
		maps[i].len  = len;
		maps[i].mem  = mem;
	}

	return mem + (addr & 0xfff);
}

uint32_t mem64_read32(uint64_t addr)
{
	uint32_t ret;
	asm volatile("mov (%%rdx), %%eax" : "=a"(ret) : "d"(_map_mem64(addr, 0)));
	return ret;
}

void mem64_write32(uint64_t addr, uint32_t val)
{
	asm volatile("mov %%eax, (%%rdx)" :: "a"(val), "d"(_map_mem64(addr, 0)));
}

uint32_t dnc_read_csr(uint32_t node, uint16_t csr)
{
	return uint32_tbswap(mem64_read32(DNC_CSR_BASE | (node << 16) | 0x8000 | csr));
}

void dnc_write_csr(uint32_t node, uint16_t csr, uint32_t val)
{
	mem64_write32(DNC_CSR_BASE | (node << 16) | 0x8000 | csr, uint32_tbswap(val));
}


uint32_t dnc_read_csr_geo(uint32_t node, uint8_t bid, uint16_t csr)
{
	if (csr >= 0x800) {
		printf("*** dnc_write_csr_geo: read from unsupported range: "
		       "%04x#%d @%x\n",
		       node, bid, csr);
		return 0xffffffff;
	}

	return uint32_tbswap(mem64_read32(DNC_CSR_BASE | (node << 16) | (bid << 11) | csr));
}

void dnc_write_csr_geo(uint32_t node, uint8_t bid, uint16_t csr, uint32_t val)
{
	if (csr >= 0x800) {
		printf("*** dnc_write_csr_geo: write to unsupported range: "
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
		val = mem64_read32(DNC_MCFG_BASE | ((uint64_t)node << 28) |
		                   PCI_MMIO_CONF(bus, device, func, reg));
	}

	return val;
}

void dnc_write_conf(uint16_t node, uint8_t bus, uint8_t device, uint8_t func, uint16_t reg, uint32_t val)
{
	if (node == 0xfff0) {
		_write_config(bus, device, func, reg, val);
	} else {
		mem64_write32(DNC_MCFG_BASE | ((uint64_t)node << 28) |
		              PCI_MMIO_CONF(bus, device, func, reg), val);
	}
}

void pmio_writeb(uint16_t offset, uint8_t val)
{
}

void pmio_writel(uint16_t offset, uint32_t val)
{
}

uint8_t pmio_readb(uint16_t offset)
{
	return 0xff;
}

uint32_t pmio_readl(uint16_t offset)
{
	return 0xffffffff;
}

void pmio_setb(uint16_t offset, uint8_t val)
{
}

void pmio_clearb(uint16_t offset, uint8_t val)
{
}

void pmio_setl(uint16_t offset, uint32_t val)
{
}

void pmio_clearl(uint16_t offset, uint32_t val)
{
}

uint64_t rdmsr(uint32_t msr)
{
	int fd;
	uint64_t val;
	fd = open("/dev/cpu/0/msr", O_RDWR);

	if (fd < 0) {
		fprintf(stderr, "*** Unable to open </dev/cpu/0/msr>\n");
		return 0xffffffffffffffffULL;
	}

	if (pread64(fd, &val, 8, msr) != 8) {
		fprintf(stderr, "*** Unable to read msr.\n");
		close(fd);
		return 0xffffffffffffffffULL;
	}

	close(fd);
	return val;
}


void wrmsr(uint32_t msr, uint64_t v)
{
	int fd;
	fd = open("/dev/cpu/0/msr", O_RDWR);

	if (fd < 0) {
		fprintf(stderr, "*** Unable to open </dev/cpu/0/msr>\n");
		return;
	}

	if (pwrite64(fd, &v, 8, msr) != 8) {
		fprintf(stderr, "*** Unable to write msr.\n");
	}

	close(fd);
}

#if defined(TEST_ACCESS)

#define MSR_MTRR_DEFAULT_MTYPE 0x000002ff
#define MSR_IORR_BASE0         0xc0010016
#define MSR_IORR_BASE1         0xc0010018
#define MSR_IORR_MASK0         0xc0010017
#define MSR_IORR_MASK1         0xc0010019
#define MSR_TOPMEM             0xc001001a
#define MSR_TOPMEM2            0xc001001d

int main(int argc, char **argv)
{
	uint64_t val;
	val = rdmsr(MSR_MTRR_DEFAULT_MTYPE);
	printf("MTRR_DEFAULT_MTYPE : %llx\n", val);
	val = rdmsr(MSR_IORR_BASE0);
	printf("MSR_IORR_BASE0 : %llx\n", val);
	val = rdmsr(MSR_IORR_BASE1);
	printf("MSR_IORR_BASE1 : %llx\n", val);
	val = rdmsr(MSR_IORR_MASK0);
	printf("MSR_IORR_MASK0 : %llx\n", val);
	val = rdmsr(MSR_IORR_MASK1);
	printf("MSR_IORR_MASK1 : %llx\n", val);
	val = rdmsr(MSR_TOPMEM);
	printf("MSR_TOPMEM : %llx\n", val);
	val = rdmsr(MSR_TOPMEM2);
	printf("MSR_TOPMEM2 : %llx\n", val);
	return 0;
}
#endif
