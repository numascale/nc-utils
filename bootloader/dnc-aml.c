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

#include "dnc-access.h"
#include "dnc-aml.h"
#include "dnc-commonlib.h"
#include "dnc-bootloader.h"

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

class Container {
	static const int maxchildren = 12;
	Container *children[maxchildren];
	int nchildren;
protected:
	static const int buflen = 16384;
	static const uint8_t BYTE = 0x0a;
	static const uint8_t WORD = 0x0b;
	static const uint8_t DWORD = 0x0c;
	static const uint8_t QWORD = 0x0e;
public:
	static unsigned char *buf, *pos;

	enum ResourceUsage {ResourceConsumer, ResourceProducer};
	enum MinType {MinNotFixed, MinFixed};
	enum MaxType {MaxNotFixed, MaxFixed};
	enum Decode {PosDecode, SubDecode};
	enum Cacheability {Cacheable, Uncacheable};
	enum MemAccess {ReadWrite};

	Container(void): nchildren(0) {}

	~Container(void) {
		while (nchildren) {
			delete children[nchildren - 1];
			nchildren--;
		}
	}

	void child(Container *c) {
		children[nchildren++] = c;
		assert(nchildren < maxchildren);
	}

	void emit(void) {} /* Overridden in inheriters */

	void pack(const uint8_t val) {
		*(uint8_t *)pos = val;
		pos += sizeof(val);
		assert(pos < buf + buflen);
	}

	void pack(const uint16_t val) {
		*(uint16_t *)pos = val;
		pos += sizeof(val);
		assert(pos < buf + buflen);
	}

	void pack(const uint32_t val) {
		*(uint32_t *)pos = val;
		pos += sizeof(val);
		assert(pos < buf + buflen);
	}

	void pack(const uint64_t val) {
		*(uint64_t *)pos = val;
		pos += sizeof(val);
		assert(pos < buf + buflen);
	}

	void pack(const char *str) {
		strcpy((char *)pos, str);
		pos += strlen(str);
		assert(pos < buf + buflen);
	}

	void pack_length(size_t len) {
		if (len < 64) {
			pack((uint8_t)len);
			return;
		}

		unsigned char *pkglen = pos;
		/* First octet stores 4 LSBs */
		pack((uint8_t)(len & 0xf));
		len >>= 4;

		while (len) {
			pack((uint8_t)len);
			len >>= 8;
		}

		/* Compute and store additional octets in 7:6 of first octet */
		uint8_t octets = pos - pkglen - 1;
		assert(octets < 4);
		*pkglen |= octets << 6;
	}

	void pack_children(void) {
		for (int i = 0; i < nchildren; i++)
			children[i]->emit();
	}

	uint32_t used(void) {
		return pos - buf;
	}
};

class AML: public Container {
public:
	AML(void) {
		/* Freed outside this class */
		buf = (unsigned char *)malloc(buflen);
		assert(buf);
		pos = buf;
	}
};

class WordBusNumber: public Container {
	static const uint8_t ENDTAG = 0x79;
	static const uint8_t CHECKSUM = 0x00; /* Not checked */
	static const uint8_t BUFFER = 0x11;
	static const uint8_t MARKER = 0xff;

	const ResourceUsage resource;
	const MinType mint;
	const MaxType maxt;
	const Decode decode;
	const uint32_t gran, mina, maxa, trans, len;
public:
	WordBusNumber(const ResourceUsage _resource, const MinType _mint, const MaxType _maxt,
	  const Decode _decode, const uint32_t _gran, const uint32_t _mina, const uint32_t _maxa,
	  const uint32_t _trans, const uint32_t _len):
	  resource(_resource), mint(_mint), maxt(_maxt), decode(_decode), gran(_gran),
	  mina(_mina), maxa(_maxa), trans(_trans), len(_len) {}

	void emit(void) {
		unsigned char *pkg_start = pos;
		pack(BUFFER);
		unsigned char *pkglen = pos;
		pack(MARKER); /* PackageLength, updated later */
		unsigned char *buf_start = pos;
		pack((uint8_t)0x0a); /* BufferSize */
		unsigned char *bufsize = pos;
		pack((uint8_t)0); /* Buffer size byte, updated later */
		pack((uint8_t)0x88);
		pack((uint16_t)0x000d); /* Minimum length (13) */
		pack((uint8_t)resource);
		pack((uint8_t)((decode << 1) | (mint << 2) | (maxt << 3)));
		pack((uint8_t)0); /* Type-specific flags; 0 for bus */
		pack(gran);
		pack(mina);
		pack(maxa);
		pack(trans);
		pack(len);
		pack(ENDTAG);

		/* Write length */
		*pkglen = pos - pkg_start;
		*bufsize = pos - buf_start - 1;
		pack(CHECKSUM);
	}
};

class Constant: public Container {
	const uint64_t val;
public:
	Constant(const uint64_t _val): val(_val) {}

	void emit(void) {
		if (val <= 1) {
			pack((uint8_t)val);
			return;
		}

		if (val <= 0xff) {
			pack(BYTE);
			pack((uint8_t)val);
		}

		if (val <= 0xffff) {
			pack(WORD);
			pack((uint16_t)val);
		}

		if (val <= 0xffffffff) {
			pack(DWORD);
			pack((uint32_t)val);
		}

		pack(QWORD);
		pack(val);
	}
};

class Method: public Container {
	static const uint8_t METHOD = 0x14;
	const char *name;
public:
	enum Serialisation {NotSerialised, Serialised};

	Method(const char *_name, const int, const Serialisation): name(_name) {}

	void emit(void) {
		pack(METHOD);
		pack_length(strlen(name));
		pack(name);
		pack((uint8_t)0); /* Field flags */
		pack_children();
	}
};

class EisaId: public Container {
	static const uint16_t EISAID = 0xd041;
	const uint16_t id;
public:
	EisaId(const uint16_t _id): id(_id) {}

	void emit(void) {
		pack(DWORD);
		pack(EISAID);
		pack(N16(id));
	}
};

class Name: public Container {
	const char *name;
public:
	Name(const char *_name, Container *c): name(_name) {
		child(c);
	};

	void emit(void) {
		pack(name);
	}
};

class Return: public Container {
	static const uint8_t RETURN = 0xa4;
public:
	Return(Container *c) {
		child(c);
	}

	void emit(void) {
		pack(RETURN);
	}
};

class Device: public Container {
	static const uint16_t DEVICE = 0x825b;
	static const uint16_t PNP0A08 = 0x0a08;
	static const uint16_t PNP0A03 = 0x0a03;
	const char *name;
public:
	Device(const char *_name, const int node): name(_name) {
		child(new Name("_HID", new EisaId(PNP0A08)));
		child(new Name("_CID", new EisaId(PNP0A03)));
		child(new Name("_ADR", new Constant(0x00)));
		child(new Name("_UID", new Constant(0x00)));
		child(new Name("_BBN", new Constant(0x00)));
		child(new Name("_SEG", new Constant(node)));
		child(new Name("_CRS", new WordBusNumber(
		  WordBusNumber::ResourceProducer,
		  WordBusNumber::MinFixed,
		  WordBusNumber::MaxFixed,
		  WordBusNumber::PosDecode,
		  0x0000,
		  0x0000,
		  0x00FF,
		  0x0000,
		  0x1000)));

		Container *method = new Method("_CBA", 0, Method::NotSerialised);
		const uint64_t config = DNC_MCFG_BASE | ((uint64_t)node << 32);
		Container *passed = new Return(new Constant(config));
		method->child(passed);
		child(method);
	}

	void emit(void) {
		pack(DEVICE);
		pack_length(strlen(name));
		pack(name);

		pack_children();
	}
};

class Scope: public Container {
	static const uint8_t SCOPE = 0x10;
	const char *name;
public:
	Scope(const char *_name): name(_name) {}

	void emit(void) {
		pack(SCOPE);
		pack_length(strlen(name));
		pack(name);
	}
};

unsigned char *Container::buf = NULL, *Container::pos = NULL;

unsigned char *remote_aml(uint32_t *len)
{
	AML sdst = AML();
	Container *sb = new Scope("\\_SB_");

	for (int node = 1; node < dnc_node_count; node++) {
		char name[4];
		snprintf(name, sizeof(name), "R%03X", node);

		Container *device = new Device(name, node);
		sb->child(device);
	}

	sdst.child(sb);

	*len = sdst.used();
	return sdst.buf;
}

