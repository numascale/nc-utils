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
protected:
	int nchildren;
	static const int buflen = 16384;
	static const uint8_t ExtOpPrefix = 0x5b;
	static const uint8_t BytePrefix = 0x0a;
	static const uint8_t WordPrefix = 0x0b;
	static const uint8_t DWordPrefix = 0x0c;
	static const uint8_t QWordPrefix = 0x0e;
	static const uint8_t BufferOp = 0x11;
	static const uint16_t EndTag = 0x0079;
public:
	static unsigned char *buf, *pos;

	enum ResourceUsage {ResourceProducer, ResourceConsumer};
	enum MinType {MinNotFixed, MinFixed};
	enum MaxType {MaxNotFixed, MaxFixed};
	enum RangeType {NonISAOnly = 1, ISAOnly, EntireRange};
	enum Decode {PosDecode, SubDecode};
	enum Cacheability {Uncacheable, Cacheable, CacheableWC, CacheablePrefetch};
	enum ReadWriteType {ReadOnly, ReadWrite};
	enum ResourceType {ResourceTypeMemory, ResourceTypeIO, ResourceTypeBus};

	Container(void): nchildren(0) {}

	virtual ~Container(void) {
		while (nchildren) {
			delete children[nchildren - 1];
			nchildren--;
		}
	}

	void child(Container *c) {
		children[nchildren++] = c;
		assert(nchildren < maxchildren);
	}

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

	virtual int len(void) {
		int n = 0;

		for (int i = 0; i < nchildren; i++)
			n += children[i]->len();

		return n;
	}

	void pack_length(size_t l) {
		if (l < 64) {
			pack((uint8_t)l);
			return;
		}

		unsigned char *pkglen = pos;
		/* First octet stores 4 LSBs */
		pack((uint8_t)(l & 0xf));
		l >>= 4;

		while (l) {
			pack((uint8_t)l);
			l >>= 8;
		}

		/* Compute and store additional octets in 7:6 of first octet */
		uint8_t octets = pos - pkglen - 1;
		assert(octets < 4);
		*pkglen |= octets << 6;
	}

	virtual void emit(void) {
		for (int i = 0; i < nchildren; i++)
			children[i]->emit();
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

	int len(void) {
		return pos - buf;
	}
};

class DWordMemory: public Container {
	const ResourceUsage usage;
	const MinType mint;
	const MaxType maxt;
	const Cacheability cache;
	const ReadWriteType access;
	const uint32_t gran, mina, maxa, trans, size;
public:
	DWordMemory(const ResourceUsage _usage, const MinType _mint, const MaxType _maxt,
	  const Cacheability _cache, const ReadWriteType _access, const uint32_t _gran, const uint32_t _mina,
	  const uint32_t _maxa, const uint32_t _trans, const uint32_t _size):
	  usage(_usage), mint(_mint), maxt(_maxt), cache(_cache), access(_access),
	  gran(_gran), mina(_mina), maxa(_maxa), trans(_trans), size(_size) {}

	int len(void) {
		return Container::len() + 26;
	}

	void emit(void) {
		assert(nchildren == 0);

		pack((uint8_t)0x87);
		pack((uint16_t)0x0017); /* Minimum length (23) */
		pack((uint8_t)ResourceTypeMemory);
		pack((uint8_t)((usage << 1) | (mint << 2) | (maxt << 3)));
		pack((uint8_t)(access | (cache << 1))); /* Type-specific flags */
		pack(gran);
		pack(mina);
		pack(maxa);
		pack(trans);
		pack(size);
	}
};

class DWordIO: public Container {
	const ResourceUsage usage;
	const MinType mint;
	const MaxType maxt;
	const Decode decode;
	const RangeType ranget;
	const uint32_t gran, mina, maxa, trans, size;
public:
	DWordIO(const ResourceUsage _usage, const MinType _mint, const MaxType _maxt,
	  const Decode _decode, const RangeType _ranget, const uint32_t _gran, const uint32_t _mina,
	  const uint32_t _maxa, const uint32_t _trans, const uint32_t _size):
	  usage(_usage), mint(_mint), maxt(_maxt), decode(_decode), ranget(_ranget),
	  gran(_gran), mina(_mina), maxa(_maxa), trans(_trans), size(_size) {}

	int len(void) {
		return Container::len() + 26;
	}

	void emit(void) {
		assert(nchildren == 0);

		pack((uint8_t)0x87);
		pack((uint16_t)0x0017); /* Minimum length (23) */
		pack((uint8_t)ResourceTypeIO);
		pack((uint8_t)((decode << 1) | (mint << 2) | (maxt << 3)));
		pack((uint8_t)ranget); /* Type-specific flags */
		pack(gran);
		pack(mina);
		pack(maxa);
		pack(trans);
		pack(size);
	}
};

class WordBusNumber: public Container {
	const ResourceUsage usage;
	const MinType mint;
	const MaxType maxt;
	const Decode decode;
	const uint16_t gran, mina, maxa, trans, size;
public:
	WordBusNumber(const ResourceUsage _usage, const MinType _mint, const MaxType _maxt,
	  const Decode _decode, const uint16_t _gran, const uint16_t _mina, const uint16_t _maxa,
	  const uint16_t _trans, const uint16_t _size):
	  usage(_usage), mint(_mint), maxt(_maxt), decode(_decode), gran(_gran),
	  mina(_mina), maxa(_maxa), trans(_trans), size(_size) {}

	int len(void) {
		return 16;
	}

	void emit(void) {
		assert(nchildren == 0);

		pack((uint8_t)0x88);
		pack((uint16_t)0x000d); /* Minimum length (13) */
		pack((uint8_t)ResourceTypeBus);
		pack((uint8_t)((decode << 1) | (mint << 2) | (maxt << 3)));
		pack((uint8_t)0); /* Type-specific flags; 0 for bus */
		pack(gran);
		pack(mina);
		pack(maxa);
		pack(trans);
		pack(size);
	}
};

class Package: public Container {
public:
	int len(void) {
		return Container::len() + 2 + 4;
	}

	void emit(void) {
		assert(nchildren > 0);

		pack(BufferOp);
		pack_length(len());
		pack((uint16_t)0x2c0a); /* FIXME */
		Container::emit();
		pack(EndTag);
	}
};

class Constant: public Container {
	const uint64_t val;
public:
	Constant(const uint64_t _val): val(_val) {}

	int len(void) {
		if (val <= 1)
			return 1;

		if (val <= 0xff)
			return 2;

		if (val <= 0xffff)
			return 3;

		if (val <= 0xffffffff)
			return 5;

		return 9;
	}

	void emit(void) {
		assert(nchildren == 0);

		if (val <= 1) {
			pack((uint8_t)val);
			return;
		}

		if (val <= 0xff) {
			pack(BytePrefix);
			pack((uint8_t)val);
			return;
		}

		if (val <= 0xffff) {
			pack(WordPrefix);
			pack((uint16_t)val);
			return;
		}

		if (val <= 0xffffffff) {
			pack(DWordPrefix);
			pack((uint32_t)val);
			return;
		}

		pack(QWordPrefix);
		pack(val);
	}
};

class Method: public Container {
	static const uint8_t MethodOp = 0x14;
	const char *name;
public:
	enum Serialisation {NotSerialised, Serialised};

	Method(const char *_name, const int, const Serialisation): name(_name) {}

	int len(void) {
		return Container::len() + strlen(name) + 2;
	}

	void emit(void) {
		assert(nchildren > 0);

		pack(MethodOp);
		pack_length(len());
		pack(name);
		pack((uint8_t)0); /* Field flags */

		Container::emit();
	}
};

class EisaId: public Container {
	static const uint16_t EISAID = 0xd041;
	const uint16_t id;
public:
	EisaId(const uint16_t _id): id(_id) {}

	int len(void) {
		return Container::len() + 5;
	}

	void emit(void) {
		assert(nchildren == 0);

		pack(DWordPrefix);
		pack(EISAID);
		pack(N16(id));
	}
};

class Name: public Container {
	static const uint8_t NameOp = 0x08;
	const char *name;
public:
	Name(const char *_name, Container *c): name(_name) {
		child(c);
	};

	Name(const char *_name): name(_name) {}

	int len(void) {
		return Container::len() + strlen(name);
	}

	void emit(void) {
		assert(nchildren > 0);

		pack(NameOp);
		pack(name);

		Container::emit();
	}
};

/* DefReturn := ReturnOp ArgObject */
/* ArgObject := TermArg => DataRefObject */
class Return: public Container {
	static const uint8_t ReturnOp = 0xa4;
public:
	Return(Container *c) {
		child(c);
	}

	int len(void) {
		return Container::len() + 1;
	}

	void emit(void) {
		assert(nchildren > 0);
		pack(ReturnOp);
		Container::emit();
	}
};

/* DefDevice := DeviceOp PkgLength NameString ObjectList */
/* DeviceOp := ExtOpPrefix 0x82 */
/* ExtOpPrefix := 0x5b */
class Device: public Container {
	static const uint8_t DeviceOp = 0x82;
	static const uint16_t PNP0A08 = 0x0a08;
	static const uint16_t PNP0A03 = 0x0a03;
	char name[4];
public:
	Device(const char *_name, const int node) {
		strncpy(name, _name, sizeof(name));
		child(new Name("_HID", new EisaId(PNP0A08)));
		child(new Name("_CID", new EisaId(PNP0A03)));
		child(new Name("_ADR", new Constant(0x00)));
		child(new Name("_UID", new Constant(0x00)));
		child(new Name("_BBN", new Constant(0x00)));
		child(new Name("_SEG", new Constant(node)));

		Container *package = new Package();

		package->child(new WordBusNumber(
		  WordBusNumber::ResourceProducer,
		  WordBusNumber::MinFixed,
		  WordBusNumber::MaxFixed,
		  WordBusNumber::PosDecode,
		  0x0000,
		  0x0000,
		  0x00FF,
		  0x0000,
		  0x0100));

		package->child(new DWordIO(
		  DWordIO::ResourceProducer,
		  DWordIO::MinFixed,
		  DWordIO::MaxFixed,
		  DWordIO::PosDecode,
		  DWordIO::EntireRange,
		  0x000000,
		  0xf00000 + (node - 1) * 0x010000,
		  0xf0ffff + (node - 1) * 0x010000,
		  0x000000,
		  0x010000));

		package->child(new DWordMemory(
		  DWordIO::ResourceProducer,
		  DWordIO::MinFixed,
		  DWordIO::MaxFixed,
		  DWordIO::Cacheable,
		  DWordIO::ReadWrite,
		  0x000000,
		  nodes[node].mmio32_base,
		  nodes[node].mmio32_limit,
		  0x000000,
		  nodes[node].mmio32_limit - nodes[node].mmio32_base + 1));

		child(new Name("_CRS", package));

		Container *method = new Method("_CBA", 0, Method::NotSerialised);
		const uint64_t config = DNC_MCFG_BASE | ((uint64_t)node << 32);
		Container *passed = new Return(new Constant(config));
		method->child(passed);
		child(method);
	}

	int len(void) {
		return Container::len() + strlen(name) + 11;
	}

	void emit(void) {
		assert(nchildren > 0);

		pack(ExtOpPrefix);
		pack(DeviceOp);
		pack_length(len());
		pack(name);

		Container::emit();
	}
};

class Scope: public Container {
	static const uint8_t ScopeOp = 0x10;
	const char *name;
public:
	Scope(const char *_name): name(_name) {}

	int len(void) {
		return Container::len() + strlen(name) + 6;
	}

	void emit(void) {
		assert(nchildren > 0);

		pack(ScopeOp);
		pack_length(len());
		pack(name);

		Container::emit();
	}
};

unsigned char *Container::buf = NULL, *Container::pos = NULL;

unsigned char *remote_aml(uint32_t *len)
{
	AML ssdt = AML();
	Container *sb = new Scope("\\_SB_");

	for (int node = 1; node < dnc_node_count; node++) {
		char name[5];
		snprintf(name, sizeof(name), "R%03X", node);

		Container *device = new Device(name, node);
		sb->child(device);
	}

	ssdt.child(sb);
	ssdt.emit();

	*len = ssdt.len();
	return ssdt.buf;
}

