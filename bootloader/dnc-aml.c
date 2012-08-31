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
#include <inttypes.h>
#include <assert.h>

#include "dnc-aml.h"
#include "dnc-bootloader.h"

static char *buf_start = NULL, *buf_curr = NULL;

#define CHECKSUM	0x00 /* Not checked */
#define NAME		0x08
#define BYTE		0x0a
#define DWORD		0x0c
#define QWORD		0x0e
#define SCOPE		0x10
#define BUFFER		0x11
#define PACKAGE		0x12
#define METHOD		0x14
#define RETURN		0xa4
#define ENDTAG		0x79
#define EISAID		0x41d0
#define DEVICE		0x5b82

#define ResourceProducer 1
#define ResourceConsumer 0
#define Fixed 1
#define NotFixed 0
#define PosDecode 1
#define SubDecode 0

static inline void emitb(const char val)
{
    *buf_curr = val;
    buf_curr += sizeof(val);
}

static inline void emitw(const uint16_t val)
{
    *(uint16_t *)buf_curr = val;
    buf_curr += sizeof(val);
}

static inline void emitq(const uint64_t val)
{
    *(uint64_t *)buf_curr = val;
    buf_curr += sizeof(val);
}

static inline void emits(const char *str)
{
    strcpy(buf_curr, str);
    buf_curr += strlen(str);
}

static void aml_device(const char *format, ...)
{
    va_list args;
    char name[16];

    va_start(args, format);
    vsprintf(name, format, args);
    va_end(args);

    emitw(DEVICE);
    emits(name);
}

static void aml_name(const char *name)
{
    emitb(NAME);
    emits(name);
}

static void aml_eisaid(const uint16_t id)
{
    emitw(EISAID);
    emitw(id);
}

static void aml_method(const char *name, const int params)
{
    emitb(METHOD);
    emitb(SCOPE);
    emits(name);
    emitb(params);
}

static void aml_returnq(const uint64_t val)
{
    emitb(RETURN);
    emitq(val);
}

static void aml_resource(bool rusage, bool mintype, bool maxtype, bool decode,
	uint16_t gran, uint16_t min, uint16_t max, uint16_t trans, uint16_t len)
{
    char *pkg_start = buf_curr;
    emitb(BUFFER);
    char *pkglen = buf_curr;
    emitb(0x00); /* Encoded package length byte, updated later */
    char *buf_start = buf_curr;
    emitb(0x0a);
    char *bufsize = buf_curr;
    emitb(0x00); /* Buffer size byte, updated later */
    emitw(0x880d);
    emitw(0x0002);
    emitw(rusage | (decode << 1) | (maxtype << 2) | (mintype << 3));
    emitw(gran);
    emitw(min);
    emitw(max);
    emitw(trans);
    emitw(len);
    emitb(ENDTAG);

    /* Write length */
    *pkglen = buf_curr - pkg_start;
    *bufsize = buf_curr - buf_start;

    emitb(CHECKSUM);
}

void acpi_update_ssdt(void)
{
    buf_start = malloc(4096);
    assert(buf_start);
    buf_curr = buf_start;

    for (int node = 0; node < dnc_node_count; node++) {
	uint64_t addr = 0x3f0000000000 | (node << 26);

	/* Device (\_SB.PCI1) */
	aml_device("._SB_PCI%d", node);

	/* Name (_HID, EisaId("PNP0A08")) */
	aml_name("_HID");
	aml_eisaid(0x0a08);

	/* Name (_CID, EisaId("PNP0A03")) */
	aml_name("_CID");
	aml_eisaid(0x0a03);

	/* Name (_ADR, 0x00000000) */
	aml_name("_ADR");
	emitb(0x00);

	/* Name (_UID, 0) */
	aml_name("_UID");
	emitb(0x00);

	/* Name (_BBN, 0x00) */
	aml_name("_BBN");
	emitb(0x00);

	/* Name (_SEG, 0x01) */
	aml_name("_SEG");
	emitb(0x01);

	/* Name (_CRS, ResourceTemplate() {
	     WordBusNumber(
	       ResourceProducer, MinFixed, MaxFixed, PosDecode,
	         0x0000, 0x0000, 0x00FF, 0x0000, 0x0100)}) */
	aml_name("_CRS");
	aml_resource(ResourceProducer, Fixed, Fixed, PosDecode,
	    0x0000, 0x0000, 0x00ff, 0x0000, 0x0100);

	aml_method("_CBA", 0);		/* Method (_CBA, 0) */
	aml_returnq(addr);		/* Return (0x3f0100000000) */
    }

    free(buf_start);
}

