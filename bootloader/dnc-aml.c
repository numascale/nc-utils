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
protected:
	static const uint8_t ExtOpPrefix = 0x5b;
	static const uint8_t BytePrefix = 0x0a;
	static const uint8_t WordPrefix = 0x0b;
	static const uint8_t DWordPrefix = 0x0c;
	static const uint8_t QWordPrefix = 0x0e;
	static const uint8_t BufferOp = 0x11;
	static const uint16_t EndTag = 0x0079;
	int offset, bufsize;
public:
	Vector<Container *> children;
	unsigned char *buf;
	enum ResourceUsage {ResourceProducer, ResourceConsumer};
	enum MinType {MinNotFixed, MinFixed};
	enum MaxType {MaxNotFixed, MaxFixed};
	enum RangeType {NonISAOnly = 1, ISAOnly, EntireRange};
	enum Decode {PosDecode, SubDecode};
	enum Cacheability {Uncacheable, Cacheable, CacheableWC, Prefetchable};
	enum ReadWriteType {ReadOnly, ReadWrite};
	enum ResourceType {ResourceTypeMemory, ResourceTypeIO, ResourceTypeBus};
	enum Serialisation {NotSerialised, Serialised};

	Container(void): offset(0), bufsize(0), buf(NULL) {}

	virtual ~Container(void) {
		free(buf);
	}

	void ensure(const int need) {
		if (offset + need <= bufsize)
			return;

		bufsize += 1024;
		buf = (unsigned char *)realloc((void *)buf, bufsize);
		assert(buf);
	}

	void pack(const uint8_t val) {
		const int need = sizeof(val);
		ensure(need);
		*(uint8_t *)&buf[offset] = val;
		offset += need;
	}

	void pack(const uint16_t val) {
		const int need = sizeof(val);
		ensure(need);
		*(uint16_t *)&buf[offset] = val;
		offset += need;
	}

	void pack(const uint32_t val) {
		const int need = sizeof(val);
		ensure(need);
		*(uint32_t *)&buf[offset] = val;
		offset += need;
	}

	void pack(const uint64_t val) {
		const int need = sizeof(val);
		ensure(need);
		*(uint64_t *)&buf[offset] = val;
		offset += need;
	}

	void pack(const char *str) {
		const int need = strlen(str);
		assert(need >= 4);
		ensure(need);
		strncpy((char *)&buf[offset], str, need);
		offset += strlen(str);
	}

	int lenlen(int l) {
		if (l < 64)
			return 1;

		l >>= 4;
		int i = 1;

		while (l) {
			i++;
			l >>= 8;
		}
		return i;
	}

	void pack_length(int l) {
		if (l < 64) {
			pack((uint8_t)l);
			return;
		}

		unsigned char pkglen = offset;
		/* First octet stores 4 LSBs */
		pack((uint8_t)(l & 0xf));
		l >>= 4;

		while (l) {
			pack((uint8_t)l);
			l >>= 8;
		}

		/* Compute and store additional octets in 7:6 of first octet */
		uint8_t octets = offset - pkglen - 1;
		assert(octets < 4);
		buf[pkglen] |= octets << 6;
	}

	virtual int build(void) {
		int l = 0;

		for (unsigned i = 0; i < children.used; i++)
			l += children.elements[i]->build();

		return l;
	}

	void insert(void) {
		for (unsigned int i = 0; i < children.used; i++) {
			ensure(children.elements[i]->offset);
			memcpy((void *)&buf[offset], (const void *)&children.elements[i]->buf[0], children.elements[i]->offset);
			offset += children.elements[i]->offset;
		}
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
	  gran(_gran), mina(_mina), maxa(_maxa), trans(_trans), size(_size) {
		assert(_mina);
		assert(_maxa);
	}

	int build(void) {
		assert(children.used == 0);

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

		return offset;
	}
};

class QWordMemory: public Container {
	const ResourceUsage usage;
	const MinType mint;
	const MaxType maxt;
	const Cacheability cache;
	const ReadWriteType access;
	const uint64_t gran, mina, maxa, trans, size;
public:
	QWordMemory(const ResourceUsage _usage, const MinType _mint, const MaxType _maxt,
	  const Cacheability _cache, const ReadWriteType _access, const uint64_t _gran, const uint64_t _mina,
	  const uint64_t _maxa, const uint64_t _trans, const uint64_t _size):
	  usage(_usage), mint(_mint), maxt(_maxt), cache(_cache), access(_access),
	  gran(_gran), mina(_mina), maxa(_maxa), trans(_trans), size(_size) {
		assert(_mina);
		assert(_maxa);
	}

	int build(void) {
		assert(children.used == 0);

		pack((uint8_t)0x8a);
		pack((uint16_t)0x002b); /* Minimum length (43) */
		pack((uint8_t)ResourceTypeMemory);
		pack((uint8_t)((usage << 1) | (mint << 2) | (maxt << 3)));
		pack((uint8_t)(access | (cache << 1))); /* Type-specific flags */
		pack(gran);
		pack(mina);
		pack(maxa);
		pack(trans);
		pack(size);

		return offset;
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

	int build(void) {
		assert(children.used == 0);

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

		return offset;
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

	int build(void) {
		assert(children.used == 0);

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

		return offset;
	}
};

class Package: public Container {
public:
	int build(void) {
		pack(BufferOp);
		int l = 4 + Container::build();
		pack_length(lenlen(l) + l);
		pack(BytePrefix);
		pack((uint8_t)(l - 2));
		Container::insert();
		pack(EndTag);

		return offset;
	}
};

class Constant: public Container {
	const uint64_t val;
public:
	Constant(const uint64_t _val): val(_val) {}

	int build(void) {
		assert(children.used == 0);

		if (val <= 1) {
			pack((uint8_t)val);
			return 1;
		}

		if (val <= 0xff) {
			pack(BytePrefix);
			pack((uint8_t)val);
			return 2;
		}

		if (val <= 0xffff) {
			pack(WordPrefix);
			pack((uint16_t)val);
			return 3;
		}

		if (val <= 0xffffffff) {
			pack(DWordPrefix);
			pack((uint32_t)val);
			return 5;
		}

		pack(QWordPrefix);
		pack(val);
		return 9;
	}
};

class Method: public Container {
	static const uint8_t MethodOp = 0x14;
	char name[5];
public:
	Method(const char *_name, const int, const Serialisation) {
		strncpy(name, _name, sizeof(name));
	}

	int build(void) {
		assert(children.used > 0);

		pack(MethodOp);
		int l = strlen(name) + 1 + Container::build();
		pack_length(lenlen(l) + l);
		pack(name);
		pack((uint8_t)0); /* Field flags/number of args */
		Container::insert();

		return offset;
	}
};

class EisaId: public Container {
	static const uint16_t EISAID = 0xd041;
	const uint16_t id;
public:
	static const uint16_t PNP0A08 = 0x0a08;
	static const uint16_t PNP0A03 = 0x0a03;

	EisaId(const uint16_t _id): id(_id) {}

	int build(void) {
		assert(children.used == 0);

		pack(DWordPrefix);
		pack(EISAID);
		pack(N16(id));

		return offset;
	}
};

class Name: public Container {
	static const uint8_t NameOp = 0x08;
	char name[5];
public:
	Name(const char *_name, Container *c) {
		strncpy(name, _name, sizeof(name));
		children.add(c);
	};

	Name(const char *_name) {
		strncpy(name, _name, sizeof(name));
	}

	int build(void) {
		assert(children.used == 1);

		pack(NameOp);
		pack(name);
		Container::build();
		Container::insert();

		return offset;
	}
};

/* DefReturn := ReturnOp ArgObject */
/* ArgObject := TermArg => DataRefObject */
class Return: public Container {
	static const uint8_t ReturnOp = 0xa4;
public:
	Return(Container *c) {
		children.add(c);
	}

	int build(void) {
		assert(children.used == 1);

		pack(ReturnOp);
		Container::build();
		Container::insert();

		return offset;
	}
};

/* DefDevice := DeviceOp PkgLength NameString ObjectList */
/* DeviceOp := ExtOpPrefix 0x82 */
/* ExtOpPrefix := 0x5b */
class Device: public Container {
	static const uint8_t DeviceOp = 0x82;
	char name[5];
public:
	Device(const char *_name) {
		strncpy(name, _name, sizeof(name));
	}

	int build(void) {
		pack(ExtOpPrefix);
		pack(DeviceOp);
		int l = Container::build() + strlen(name);
		pack_length(lenlen(l) + l);
		pack(name);
		Container::insert();

		return offset;
	}
};

class Scope: public Container {
	static const uint8_t ScopeOp = 0x10;
	char name[16];
public:
	Scope(const char *_name) {
		strncpy(name, _name, sizeof(name));
	}

	int build(void) {
		pack(ScopeOp);
		int l = Container::build() + strnlen(name, sizeof(name));
		pack_length(l + lenlen(l));
		pack(name);
		Container::insert();
		return offset;
	}
};

unsigned char *remote_aml(uint32_t *len)
{
	Container *sb = new Scope("\\_SB_");

	const unsigned nnodes = min(dnc_node_count, AML_MAXNODES);
	node_info_t *node = &nodes[0];

	/* For the master's PCI bus, add a proximity object */
	Container *rbus = new Scope("PCI0");
	rbus->children.add(new Name("_PXM", new Constant(node->ht[node->bsp_ht].pdom)));
	sb->children.add(rbus);

	for (unsigned n = 1; n < nnodes; n++) {
		char name[5];
		node = &nodes[n];
		snprintf(name, sizeof(name), "X%03u", n);

		Container *bus = new Device(name);

		bus->children.add(new Name("_HID", new EisaId(EisaId::PNP0A08)));
		bus->children.add(new Name("_CID", new EisaId(EisaId::PNP0A03)));
		bus->children.add(new Name("_UID", new Constant(0x80 + n)));
		bus->children.add(new Name("_BBN", new Constant(0x00)));
		bus->children.add(new Name("_SEG", new Constant(n)));
		bus->children.add(new Name("_ADR", new Constant(0x00)));
		bus->children.add(new Name("_PXM", new Constant(node->ht[node->bsp_ht].pdom)));

		Container *package = new Package();

		package->children.add(new WordBusNumber(
		  WordBusNumber::ResourceProducer,
		  WordBusNumber::MinFixed,
		  WordBusNumber::MaxFixed,
		  WordBusNumber::PosDecode,
		  0x0000,
		  0x0000,
		  0x00FF,
		  0x0000,
		  0x0100));

		if (node->io_limit > node->io_base)
			package->children.add(new DWordIO(
			  DWordIO::ResourceProducer,
			  DWordIO::MinFixed,
			  DWordIO::MaxFixed,
			  DWordIO::PosDecode,
			  DWordIO::EntireRange,
			  0x000000,
			  node->io_base,
			  node->io_limit,
			  0x000000,
			  node->io_limit - node->io_base + 1));

		if (node->mmio32_limit > node->mmio32_base)
			package->children.add(new DWordMemory(
			  DWordMemory::ResourceProducer,
			  DWordMemory::MinFixed,
			  DWordMemory::MaxFixed,
			  DWordMemory::Cacheable,
			  DWordMemory::ReadWrite,
			  0x00000000,
			  node->mmio32_base,
			  node->mmio32_limit,
			  0x00000000,
			  node->mmio32_limit - node->mmio32_base + 1));

		if (node->mmio64_limit > node->mmio64_base)
			package->children.add(new QWordMemory(
			  QWordMemory::ResourceProducer,
			  QWordMemory::MinFixed,
			  QWordMemory::MaxFixed,
			  QWordMemory::Prefetchable,
			  QWordMemory::ReadWrite,
			  0x00000000,
			  node->mmio64_base,
			  node->mmio64_limit,
			  0x00000000,
			  node->mmio64_limit - node->mmio64_base + 1));

		bus->children.add(new Name("_CRS", package));

		Container *method = new Method("_CBA", 0, Method::NotSerialised);
		const uint64_t config = DNC_MCFG_BASE | ((uint64_t)node->sci << 28);
		Container *passed = new Return(new Constant(config));

		method->children.add(passed);
		bus->children.add(method);

		sb->children.add(bus);
	}

	*len = sb->build();
	sb->insert();
	return sb->buf;
}
