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
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <inttypes.h>

#include "dnc-aml.h"
#include "dnc-commonlib.h"
#include "dnc-bootloader.h"

#define BUF_LIM		4096

#define CHECKSUM	(uint8_t)0x00 /* Not checked */
#define NAME		(uint8_t)0x08
#define BYTE		(uint8_t)0x0a
#define WORD		(uint8_t)0x0b
#define DWORD		(uint8_t)0x0c
#define QWORD		(uint8_t)0x0e
#define SCOPE		(uint8_t)0x10
#define BUFFER		(uint8_t)0x11
#define PACKAGE		(uint8_t)0x12
#define METHOD		(uint8_t)0x14
#define RETURN		(uint8_t)0xa4
#define ENDTAG		(uint8_t)0x79
#define MARKER		(uint8_t)0xff
#define EISAID		(uint16_t)0xd041
#define DEVICE		(uint16_t)0x825b

#define TypeMemory	0
#define TypeIO		1
#define TypeBus		2
#define ResourceProducer 1
#define ResourceConsumer 0
#define Fixed		1
#define NotFixed	0
#define SubDecode	1
#define PosDecode	0

#define AML_ADDR_BITS	48
#define NAME_MAX	16
#define BLOCK_MAX	(1ULL << (64 - AML_ADDR_BITS))

#define emit(p, x) do { \
    *(typeof(x) *)(p) = (x); \
    (p) += sizeof(x); \
  } while (0)

#define N16(x) (uint16_t)( \
    (((x) & 0xff00) >> 8) | \
    (((x) & 0x00ff) << 8))
#define N32(x) (uint32_t)( \
    (((x) & 0xff000000) >> 24) | \
    (((x) & 0x00ff0000) >> 8) | \
    (((x) & 0x0000ff00) << 8) | \
    ((x) & 0x000000ff) << 24))
#define N64(x) (uint64_t)( \
    (((x) & 0x00000000000000ffULL) << 56) | \
    (((x) & 0x000000000000ff00ULL) << 40) | \
    (((x) & 0x0000000000ff0000ULL) << 24) | \
    (((x) & 0x00000000ff000000ULL) <<  8) | \
    (((x) & 0x000000ff00000000ULL) >>  8) | \
    (((x) & 0x0000ff0000000000ULL) >> 24) | \
    (((x) & 0x00ff000000000000ULL) >> 40) | \
    (((x) & 0xff00000000000000ULL) >> 56))

typedef uint64_t ptrlen_t;
typedef unsigned char aml;
typedef ptrlen_t (*aml_func)(ptrlen_t);

static inline aml *PTR(ptrlen_t val)
{
	return (aml *)(uint32_t)(val & ((1ULL << AML_ADDR_BITS) - 1));
}

static inline int LEN(ptrlen_t val)
{
	return val >> AML_ADDR_BITS;
}

static inline uint64_t ptrlen(aml *start, aml *end)
{
	uint64_t mask = (uint32_t)start;
	assert(mask < (1ULL << AML_ADDR_BITS));
	return mask | (uint64_t)(end - start) << AML_ADDR_BITS;
}

static inline void aml_str(aml **block, const char *str)
{
    strcpy(*(char **)block, str);
    *block += strlen(str);
}

static void aml_length(aml **block, uint32_t len)
{
    if (len < 64) {
	emit(*block, (uint8_t)len);
	return;
    }

    unsigned char *pkglen = *block;

    /* First octet stores 4 LSbs */
    emit(*block, (uint8_t)(len & 0xf));
    len >>= 4;

    while (len) {
	emit(*block, (uint8_t)len);
	len >>= 8;
    }

    /* Compute and store additional octets in 7:6 of first octet */
    uint8_t octets = *block - pkglen - 1;
    assert(octets < 4);
    *pkglen |= octets << 6;
}

static void aml_name(aml **block, const char *name)
{
    emit(*block, NAME);
    aml_str(block, name);
}

static void aml_eisaid(aml **block, const uint16_t id)
{
    emit(*block, DWORD);
    emit(*block, EISAID);
    emit(*block, N16(id));
}

static void aml_constant(aml **block, uint64_t val)
{
    if (val <= 1) {
	emit(*block, (uint8_t)val);
    } else if (val <= 0xff) {
	emit(*block, BYTE);
	emit(*block, (uint8_t)val);
    } else if (val <= 0xffff) {
	emit(*block, WORD);
	emit(*block, (uint16_t)val);
    } else if (val <= 0xffffffff) {
	emit(*block, DWORD);
	emit(*block, (uint32_t)val);
    } else {
	emit(*block, QWORD);
	emit(*block, val);
    }
}

static void aml_return(aml **block, uint64_t val)
{
    emit(*block, RETURN);
    aml_constant(block, val);
}

static ptrlen_t aml_cba(const uint64_t addr)
{
    aml *block_start = malloc(BLOCK_MAX);
    assert(block_start);
    aml *block = block_start;

    aml_return(&block, addr);

    return ptrlen(block_start, block);
}

static void aml_resource(aml **block, uint8_t type, bool mintype, bool maxtype, bool decode,
	uint16_t gran, uint16_t min, uint16_t max, uint16_t trans, uint16_t len)
{
    aml *pkg_start = *block;
    emit(*block, BUFFER);
    aml *pkglen = *block;
    emit(*block, MARKER); /* PackageLength, updated later */
    aml *buf_start = *block;
    emit(*block, (uint8_t)0x0a); /* BufferSize */
    aml *bufsize = *block;
    emit(*block, (uint8_t)0); /* Buffer size byte, updated later */
    emit(*block, (uint8_t)0x88);
    emit(*block, (uint16_t)0x000d); /* Minimum length (13) */
    emit(*block, type);
    emit(*block, (uint8_t)((decode << 1) | (mintype << 2) | (maxtype << 3)));
    emit(*block, (uint8_t)0); /* Type-specific flags; 0 for bus */
    emit(*block, gran);
    emit(*block, min);
    emit(*block, max);
    emit(*block, trans);
    emit(*block, len);
    emit(*block, ENDTAG);

    /* Write length */
    *pkglen = *block - pkg_start;
    *bufsize = *block - buf_start - 1;

    emit(*block, CHECKSUM);
}

static void aml_method(aml **outer, const char *name, ptrlen_t block)
{
    aml *inner = PTR(block);
    uint32_t len = LEN(block);

    emit(*outer, METHOD);
    aml_length(outer, len + strlen(name) + 2);
    strcpy(*(char **)outer, name);
    *outer += strlen(name);

    emit(*outer, (uint8_t)0); /* Field flags */

    memcpy(*(char **)outer, inner, len);
    *outer += len;
    free(inner);
}

static ptrlen_t aml_pci(uint16_t node) {
    aml *block_start = malloc(BLOCK_MAX);
    assert(block_start);
    aml *block = block_start;

    /* Name (_HID, EisaId("PNP0A08")) */
    aml_name(&block, "_HID");
    aml_eisaid(&block, 0x0a08);

    /* Name (_HID, EisaId("PNP0A03")) */
    aml_name(&block, "_CID");
    aml_eisaid(&block, 0x0a03);

    /* Name (_ADR, 0x00000000) */
    aml_name(&block, "_ADR");
    aml_constant(&block, 0);

    /* Name (_UID, 0) */
    aml_name(&block, "_UID");
    aml_constant(&block, 0);

    /* Name (_BBN, 0x00) */
    aml_name(&block, "_BBN");
    aml_constant(&block, 0);

    /* Name (_SEG, 0x01) */
    aml_name(&block, "_SEG");
    aml_constant(&block, node);

/* Name (_CRS, ResourceTemplate() {
     WordBusNumber(
       ResourceProducer, MinFixed, MaxFixed, PosDecode,
         0x0000, 0x0000, 0x00FF, 0x0000, 0x0100)}) */
    aml_name(&block, "_CRS");
    aml_resource(&block, TypeBus, Fixed, Fixed, PosDecode,
	0x0000, 0x0000, 0x00ff, 0x0000, 0x0100);

    uint64_t addr = 0x3f0000000000 | ((uint64_t)node << 32);
    aml_method(&block, "_CBA", aml_cba(addr));

    return ptrlen(block_start, block);
}

static void aml_device(aml **outer, ptrlen_t block, const char *format, ...)
{
    emit(*outer, DEVICE);
    char name[16];

    /* Emit formatted name */
    va_list args;
    va_start(args, format);
    int chars = vsprintf(name, format, args);
    va_end(args);

    aml *inner = PTR(block);
    uint32_t len = LEN(block);

    aml_length(outer, len + chars + 2);
    memcpy(*outer, name, chars);
    *outer += chars;

    memcpy(*outer, inner, len);
    *outer += len;
    free(inner);
}

static ptrlen_t aml_systembus(void)
{
    aml *block_start = malloc(BLOCK_MAX);
    assert(block_start);
    aml *block = block_start;

    for (uint16_t node = 1; node < dnc_node_count; node++)
	aml_device(&block, aml_pci(node), "P%03d", node);

    return ptrlen(block_start, block);
}

static ptrlen_t aml_scope(const char *name, ptrlen_t block)
{
    aml *outer_start = malloc(BLOCK_MAX);
    assert(outer_start);
    aml *outer = outer_start;

    aml *inner = PTR(block);
    uint32_t len = LEN(block);

    emit(outer, SCOPE);
    aml_length(&outer, len + strlen(name) + 2);
    strcpy((char *)outer, name);
    outer += strlen(name);

    memcpy(outer, inner, len);
    outer += len;
    free(inner);

    return ptrlen(outer_start, outer);
}

unsigned char *remote_aml(uint32_t *len)
{
    ptrlen_t block = aml_scope("\\_SB_", aml_systembus());
    *len = LEN(block);
    return PTR(block);
}

