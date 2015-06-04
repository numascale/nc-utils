# r9

import os, sys, ctypes, subprocess, struct, mmap, time, errno

error_fstat = {
	11: 'Internal BIU bus packet discard',
	10: 'Internal MIU bus packet discard',
	 9: 'Internal PIU bus packet discard',
	 8: 'B-Link packet parity error',
	 3: 'SCC request split timeout',
	 2: 'Error response',
	 1: 'SCC coherence error',
	 0: 'SCC coherence processor stack overflow',
}

error_nfstat = {
#	24: 'Raw packet response again',
	23: 'Interrupt again',
	22: 'Performance counter 1 set again',
	21: 'Performance counter 2 set again',
	20: 'SCI line locking again',
	19: 'GCM memory address miss again',
	16: 'B-Link device needs attention',
#	 8: 'Raw packet response'
	 7: 'Interrupt target',
	 6: 'Performance counter 1 set',
	 5: 'Performance counter 2 set',
	 4: 'SCI line locking',
	 3: 'GCM memory address miss',
	 0: 'B-Link device needs attention',
}

cdata_error_statr = {
	5: 'Denali controller sum interrupt',
	4: 'RSC error in read queue',
	3: 'RHN error in read queue',
	2: 'Correctable error on port1',
	1: 'Correctable error on port0',
	0: 'Uncorrectable error',
}

databahn_int_status = {
	11: 'DLL unlock condition',
	10: 'Read DQS gate error',
	 9: 'ODT and CAS latecy 3 programmed',
	 8: 'Unaligned write access',
	 5: 'Multiple uncorrectable ECC events',
	 4: 'Single uncorrectable ECC event',
	 3: 'Multiple correctable ECC events',
	 2: 'Single correctable ECC event',
	 1: 'Multiple accesses outside address space',
	 0: 'Single access outside address space',
}

error_status = {
	3: 'MC Tag Interrupt',
	2: 'MC Tag Error',
	1: 'CData Interrupt',
	0: 'CData Error',
}

def pin(core):
	_libc = ctypes.CDLL('libc.so.6', use_errno=True)
	_sched_setaffinity = _libc.sched_setaffinity
	_sched_setaffinity.argtypes = [ctypes.c_int, ctypes.c_ulong, ctypes.POINTER(ctypes.c_ulong)]
	_c_ulong_size = ctypes.sizeof(ctypes.c_ulong)

	cpuset = ctypes.c_ulong(1 << core)

	if _sched_setaffinity(0, _c_ulong_size, cpuset) < 0:
		err = ctypes.get_errno()
		raise OSError(err, os.strerror(err))

class Node:
	def write(self, bus, dev, fun, reg, val):
		numachip.mmcfg_raw_write(self.sci, bus, dev, fun, reg, val)

	def read(self, bus, dev, fun, reg):
		return numachip.mmcfg_raw_read(self.sci, bus, dev, fun, reg)

	def __init__(self, sci):
		self.sci = sci
		# assume NC is top HT
		self.nc_ht = (numachip.mmcfg_raw_read(sci, 0, 0x18, 0, 0x60) >> 4) & 7
		self.nbs = []

		# assume NBs below
		for ht in range(self.nc_ht):
			self.nbs.append(Northbridge(self.sci, ht))

	def log(self, ht, msg):
		# check for first error for HT
		if ht not in self.errors:
			self.errstr += '%03x#%u ' % (self.sci, ht)
			self.errors.append(ht)
		else:
			self.errstr += '      '

		self.errstr += msg

	def ncensure(self, reg, mask, expect, bits=None):
		val = numachip.csr_raw_read(self.sci, numachip.regs[reg]) & mask
		if val != expect:
			msg = '%s:%08x' % (reg, val)

			if bits:
				strings = []
				for bit, string in bits.iteritems():
					if (val ^ expect) & (1 << bit):
						strings.append(string)

				if strings:
					msg += ' (' + ', '.join(strings) + ')'

			self.log(self.nc_ht, msg)

	def nbensure(self, ht, reg, expect):
		reg = Northbridge.regs[reg]
		val = numachip.mmcfg_raw_read(self.sci, 0, 0x18 + ht, reg >> 12, reg & 0xfff)
		if val != expect:
			self.log(ht, '%s:%08x' % (reg, val))

	def mces(self, ht):
		statreg = Northbridge.regs['MC STATUS']
		status = numachip.mmcfg_raw_read(self.sci, 0, 0x18 + ht, statreg >> 12, statreg & 0xfff)
		status |= numachip.mmcfg_raw_read(self.sci, 0, 0x18 + ht, statreg >> 12, (statreg & 0xfff) + 4) << 32

#		status = 0xdc4540004d080813
		if not status & (1 << 63):
			return

		addrreg = Northbridge.regs['MC ADDR']
		addr = numachip.mmcfg_raw_read(self.sci, 0, 0x18 + ht, addrreg >> 12, addrreg & 0xfff)
		addr |= numachip.mmcfg_raw_read(self.sci , 0, 0x18 + ht, addrreg >> 12, (addrreg & 0xfff) + 4) << 32

#		addr = 0x00000018076cc000
		self.log(ht, self.nbs[ht].mce(status, addr))

	def state(self):
		try:
			val = numachip.csr_raw_read(self.sci, numachip.regs['SEQ_INFO'])
		except (Numachip.TimeoutException, Numachip.HTErrorException, Numachip.BlockedException):
			return

		return val & 0x3ff

	def check(self):
		self.errors = []
		self.errstr = ''

		try:
			ht = self.nc_ht

			if not numachip.csr_raw_read(self.sci, numachip.regs['ATT_INDEX']):
				self.log(ht, 'rebooted')
				return

			self.ncensure('CDATA_ERROR_STATR', 0xffffffff, 0x00000000, cdata_error_statr)
			self.ncensure('MCTAG_ERROR_STATR', 0xffffffff, 0x00000000)
			self.ncensure('ERROR_STATUS', 0xffffffff, 0x00000000, error_status)
			self.ncensure('ERROR_NFSTAT', 0xffffffff, 0x01000100, error_nfstat)
			self.ncensure('ERROR_FSTAT', 0xffffffff, 0x00000000, error_fstat)
			self.ncensure('MCTAG_INT_STATUS', 0x0000effc, 0x00000000, databahn_int_status)
			self.ncensure('CDATA_INT_STATUS', 0x0000effc, 0x00000000, databahn_int_status)

			if platform.oemn.size_x:
				self.ncensure('HSSXA_STAT_1', 0xffffffff, 0x00000100)
				self.ncensure('HSSXB_STAT_1', 0xffffffff, 0x00000100)
				self.ncensure('PHYXA_ELOG', 0xffffffff, 0x00000000)
				self.ncensure('PHYXB_ELOG', 0xffffffff, 0x00000000)
				self.ncensure('PHYXA_LINK_STAT', 0xffffffff, 0x00001fff)
				self.ncensure('PHYXB_LINK_STAT', 0xffffffff, 0x00001fff)

			if platform.oemn.size_y:
				self.ncensure('HSSYA_STAT_1', 0xffffffff, 0x00000100)
				self.ncensure('HSSYB_STAT_1', 0xffffffff, 0x00000100)
				self.ncensure('PHYYA_ELOG', 0xffffffff, 0x00000000)
				self.ncensure('PHYZB_ELOG', 0xffffffff, 0x00000000)
				self.ncensure('PHYYA_LINK_STAT', 0xffffffff, 0x00001fff)
				self.ncensure('PHYYB_LINK_STAT', 0xffffffff, 0x00001fff)

			if platform.oemn.size_z:
				self.ncensure('HSSZA_STAT_1', 0xffffffff, 0x00000100)
				self.ncensure('HSSZB_STAT_1', 0xffffffff, 0x00000100)
				self.ncensure('PHYZA_ELOG', 0xffffffff, 0x00000000)
				self.ncensure('PHYZB_ELOG', 0xffffffff, 0x00000000)
				self.ncensure('PHYZA_LINK_STAT', 0xffffffff, 0x00001fff)
				self.ncensure('PHYZB_LINK_STAT', 0xffffffff, 0x00001fff)

			if not options.lean:
				for ht in range(self.nc_ht):
					self.mces(ht)
					self.nbensure(ht, 'DRAM HOT', 0x00000000)

		except Numachip.TimeoutException:
			self.log(ht, 'timeout')
		except Numachip.HTErrorException:
			self.log(ht, 'HT error')
		except Numachip.BlockedException:
			self.log(ht, 'blocked')

		return self.errstr

class Platform:
	class OEMN(ctypes.Structure):
		_pack_ = 1
		_fields_  = (
			# ACPI fields
			('acpi_sig',        ctypes.c_char * 4),
			('acpi_len',        ctypes.c_uint),
			('acpi_rev',        ctypes.c_ubyte),
			('acpi_checksum',   ctypes.c_ubyte),
			('acpi_oemid',      ctypes.c_char * 6),
			('acpi_oemtableid', ctypes.c_char * 8),
			('acpi_oemrev',     ctypes.c_uint),
			('acpi_creatorid',  ctypes.c_char * 4),
			('acpi_creatorrev', ctypes.c_uint),

			# fixed fields
			('numachip_rev',    ctypes.c_ubyte, 4),
			('size_x',          ctypes.c_ubyte, 4),
			('size_y',          ctypes.c_ubyte, 4),
			('size_z',          ctypes.c_ubyte, 4),
			('northbridges',    ctypes.c_uint, 3),
			('neigh_ht',        ctypes.c_uint, 3),
			('neigh_link',      ctypes.c_uint, 2),
			('symmetric',       ctypes.c_uint, 1),
			('renumbering',     ctypes.c_uint, 1),
			('remote_io',       ctypes.c_uint, 1),
			('observer',        ctypes.c_uint, 1),
			('cores',           ctypes.c_uint, 8),
			# warning: alignment occurs at type boundaries
		)

		def __init__(self):
			try:
				with open('/sys/firmware/acpi/tables/OEMN', 'rb') as f:
					f.readinto(self)
				assert self.acpi_len >= 41
				self.standalone = False
			except IOError as e:
				if e.errno != errno.ENOENT:
					raise
				self.size_x = 2
				self.size_y = 0
				self.size_z = 0
				self.standalone = True

			if options.verbose > 0:
				print 'ACPI: sig=%s len=%d rev=%d check=%d oemid=%s oemtableid=%s oemrev=%x creatorid=%s creatorrev=%d' % (self.acpi_sig[0:3], self.acpi_len, self.acpi_rev, self.acpi_checksum, self.acpi_oemid[0:5], self.acpi_oemtableid[0:7], self.acpi_oemrev, self.acpi_creatorid[0:3], self.acpi_creatorrev)
				print 'data: numachip_rev=%d size=%d,%d,%d northbridges=%d neigh_ht=%d neigh_link=%d symmetric=%d renumbering=%d' % (self.numachip_rev, self.size_x, self.size_y, self.size_z, self.northbridges, self.neigh_ht, self.neigh_link, self.symmetric, self.renumbering)

	class PCI:
		def __init__(self, processor):
			mcfg_base = processor.rdmsr(processor.MCFG_BASE) & ~0x3f
			fd = os.open('/dev/mem', os.O_RDWR)
			self.mcfg = mmap.mmap(fd, 256 << 20, mmap.MAP_SHARED, mmap.PROT_READ | mmap.PROT_WRITE, 0, mcfg_base)
			os.close(fd)

		def conf_readl(self, bus, dev, fn, reg):
			addr = (bus << 20) | (dev << 15) | (fn << 12) | reg
			return struct.unpack('=L', self.mcfg[addr:addr + 4])[0]

		def conf_writel(self, bus, dev, fn, reg, val):
			addr = (bus << 20) | (dev << 15) | (fn << 12) | reg
			self.mcfg[addr:addr + 4] = struct.pack('=L', val)

	class Processor:
		MCFG_BASE = 0xc0010058

		def __init__(self):
			# catch failure and attempt msr module load
			try:
				self.fd = os.open('/dev/cpu/0/msr', os.O_RDWR)
			except OSError as e:
				if e.errno != errno.ENOENT:
					raise

				subprocess.call(('modprobe', 'msr'))
				self.fd = os.open('/dev/cpu/0/msr', os.O_RDWR)

		def rdmsr(self, msr):
			os.lseek(self.fd, msr, os.SEEK_SET)
			return struct.unpack('=Q', os.read(self.fd, 8))[0]

		def wrmsr(self, msr, val):
			os.lseek(self.fd, msr, os.SEEK_SET)
			os.write(self.fd, struct.pack('=Q', val))

	def __init__(self):
		if os.getuid():
			raise SystemExit('root access required')

		self.oemn = self.OEMN()
		self.processor = self.Processor()
		self.pci = self.PCI(self.processor)

class Northbridge:
	regs = {
		'LINK 0 RETRIES': 0x0130,
		'LINK 1 RETRIES': 0x0134,
		'LINK 2 RETRIES': 0x0138,
		'LINK 3 RETRIES': 0x013c,
		'DRAM HOT':       0x02ac,
		'MC STATUS':      0x3048,
		'MC ADDR':        0x3050,
		'ONLINE SPARE':   0x30b0,
		'L3C ERRORS':     0x3170,
		'DRAM ERRORS':    0x3160,
		'LINK ERRORS':    0x3168,
	}

	timeout = ('no-timeout', 'timeout')
	cache = ('reserved', 'L1', 'L2', 'L3/HT')
	transtype = ('instruction', 'data', 'generic')
	memtranstype = ('generic', 'read', 'write', 'data read', 'data write', 'instruction fetch', 'prefetch', 'evict', 'probe')
	participation = ('requestor', 'responder', 'observer', 'generic')
	memio = ('memory', 'reserved', 'IO', 'generic')

	modes = (
		None, 'CRC error', 'sync error', 'mst abort', 'tgt abort',
		'GART error', 'RMW error', 'WDT error', 'ECC error', None,
		'link data error', 'protocol error', 'NB Array error',
		'DRAM parity error', 'link retry', 'GART table walk data error',
		None, None, None, None, None, None, None, None, None,
		'compute unit data error', None, None, 'L3 cache data error',
		'L3 cache tag error', 'L3 cache LRU error', 'probe filter error',
	 )

	proterrtype = {
		int('00000', 2): 'Link: SRQ Read Response without matching request',
		int('00001', 2): 'Link: Probe Response without matching request',
		int('00010', 2): 'Link: TgtDone without matching request',
		int('00011', 2): 'Link: TgtStart without matching request',
		int('00100', 3): 'Link: Command buffer overflow',
		int('00101', 2): 'Link: Data buffer overflow',
		int('00110', 2): 'Link: Link retry packet count acknowledge overflow',
		int('00111', 2): 'Link: Data command in the middle of a data transfer',
		int('01000', 2): 'Link: Link address extension command followed by a packet other than a command with address',
		int('01001', 2): 'Link: A specific coherent-only packet from a CPU was issued to an IO link',
		int('01010', 2): 'Link: A command with invalid encoding was received',
		int('01011', 2): 'Link: Link CTL deassertion occurred when a data phase was not pending',
		int('10000', 2): 'L3: Request gets multiple hits in L3',
		int('10001', 2): 'L3: Probe access gets multiple hits in L3',
		int('10010', 2): 'L3: Request queue overflow',
		int('10011', 2): 'L3: WrVicBlk hit incompatible L3 state',
		int('10100', 2): 'L3: ClVicBlk hit incompatible L3 state',
		int('11000', 2): 'PF: Directed probe miss',
		int('11001', 2): 'PF: Directed probe clean hit',
		int('11010', 2): 'PF: VicBlkM hit inconsistent directory state "O|S"',
		int('11011', 2): 'PF: VicBlkE hit inconsistent directory state "O|S"',
		int('11101', 2): 'PF: L3 lookup response without a matching PFQ entry',
		int('11110', 2): 'PF: L3 update data read request without a matching PFQ entry',
	}

	def __init__(self, sci, ht):
		self.sci = sci
		self.ht = ht

	def mce(self, status, addr):
		desc = self.modes[(status >> 16) & 0x1f]
		if desc == 'protocol error':
			desc += ' "' + self.proterrtype[(addr >> 1) & 0x1f] + '"\n  '

		if status & 0xfff0 == 0x0010:
			desc += ' level=GART-TLB'
			desc += ' ' + self.transtype[(status >> 2) & 3]
			desc += ' ' + self.cache[status & 3]
		elif status & 0xff00 == 0x0100:
			desc += ' level=cache'
			desc += ' memtranstype=' + self.memtranstype[(status >> 4) & 15]
			desc += ' transtype=' + self.transtype[(status >> 2) & 3]
			desc += ' cache=' + self.cache[status & 3]
		elif status & 0xf800 == 0x0800:
			desc += ' level=bus'
			desc += ' participation=' + self.participation[(status >> 10) & 3]
			desc += ' ' + self.timeout[(status >> 8) & 1]
			desc += ' memtranstype=' + self.memtranstype[(status >> 4) & 15]
			desc += ' mem/io=' + self.memio[(status >> 2) & 3]
			desc += ' cache=' + self.cache[status & 3]
		else:
			desc += 'unexpected ErrorCode 0x%x' % status & 0xfff

		if status & (1 << 61):
			desc += ' uncorrected'
		else:
			desc += ' corrected'

		if status & (1 << 62):
			desc += ' overflow'

		if status & (1 << 59):
			desc += ' thresholded'

		desc += ' link='
		for l in range(4):
			if status & (1 << (36 + l)):
				desc += str(l)
		desc += '.' + str((status >> 41) & 1)

		if status & (1 << 58):
			desc += ' addr=0x%x' % addr

		if status & (1 << 57):
			desc += ' corrupted'

		if status & (1 << 56):
			desc += ' core=%u' % ((status >> 32) & 0xf)

		if status & (1 << 40):
			desc += ' scrub'

		return desc

class Numachip:
	G0_NODE_IDS                                = 0x0008
	G0_RAW_CONTROL                             = 0x0c50
	G0_RAW_INDEX                               = 0x0c54
	G0_RAW_ENTRY_LO                            = 0x0c58
	G0_RAW_ENTRY_HI                            = 0x0c5c
	G3_MMCFG_BASE                              = 0x3010
	G3_HT_NODEID                               = 0x3024
	G3_SELECT_COUNTER                          = 0x3f78
	G3_COMPARE_AND_MASK_OF_COUNTER_0           = 0x3fa0
	G3_PERFORMANCE_COUNTER_0_40_BIT_UPPER_BITS = 0x3fc0
	G3_PERFORMANCE_COUNTER_0_40_BIT_LOWER_BITS = 0x3fc4
	H2S_CSR_G0_SEQ_INDEX                       = 0x0c20
	H2S_CSR_G0_WCS_ENTRY                       = 0x0c24

	UCODE_CALL                                 = 0x0018029b
	UCODE_CALLRET                              = 0x00140184
	UCODE_DELAY                                = 0x0043c00a
	UCODE_DELAYRET                             = 0x001a0000
	UCODE_CALLS                                = 248
	UCODE_DELAYS                               = 248

	ucode = [
		0x0055f320, 0x0051f220, 0x0014001a, 0x0051f02c,
		0x00140010, 0x00140015, 0x00140024, 0x00140024,
		0x00140024, 0x00140024, 0x0013000c, 0x0071c2a6,
		0x0071c0a6, 0x00124000, 0x005df320, 0x0055f02e,
		0x00102012, 0x0055f026, 0x00110014, 0x00614680,
		0x0055f826, 0x00102017, 0x0051f026, 0x00110019,
		0x00614480, 0x0051f826, 0x0010401c, 0x0055f02c,
		0x005df02c, 0x0071c2ac, 0x00130020, 0x0079c280,
		0x0079c080, 0x00130023, 0x005df000, 0x0059f000,
		0x001c8000, 0x00160800, 0x00121000, 0x00260602,
		0x00260e06, 0x00360a04, 0x0000002f, 0x00110000,
		0x00101024, 0x00260904, 0x00260d06, 0x00614b00,
		0x00100433, 0x002e0102, 0x003e0502, 0x00140000,
		0x00000039, 0x00110000, 0x00101024, 0x00260904,
		0x00000c3d, 0x00614680, 0x00100433, 0x002e0104,
		0x003e0504, 0x00614280, 0x00100400, 0x00360d04,
		0x00000000, 0x00121000, 0x00110000, 0x00240804,
		0x00614280, 0x00100400, 0x00240404, 0x00340c04,
		0x0000000d, 0x0012100d, 0x0011000d, 0x0012404e,
		0x00611780, 0x0010040d, 0x001d1400, 0x00240400,
		0x00240c00, 0x00340800, 0x0055f320, 0x000a8058,
		0x000ab05d, 0x0003b05f, 0x00039060, 0x0030e860,
		0x000a185b, 0x00208162, 0x00303128, 0x00201938,
		0x00300d62, 0x00209138, 0x0030b128, 0x0031b128,
		0x00319138, 0x00039068, 0x0003b068, 0x00028069,
		0x0002e869, 0x00020869, 0x00020c69, 0x0055f320,
		0x00614900, 0x00120424, 0x002990c8, 0x0029b0d8,
		0x00288040, 0x00280804, 0x0028e874, 0x00380c06,
		0x00021881, 0x00023081, 0x00029024, 0x0002b024,
		0x0003b07a, 0x0003907a, 0x0020e860, 0x00288160,
		0x00280960, 0x00380d60, 0x00614480, 0x00281900,
		0x00283100, 0x00289100, 0x0028b100, 0x0029b100,
		0x00399100, 0x00615800, 0x00615800, 0x0014007a,
		0x0002189a, 0x0002309a, 0x00029024, 0x0002b024,
		0x0003b08e, 0x0003908e, 0x0020e860, 0x00100498,
		0x00288162, 0x00380d62, 0x00614900, 0x00100496,
		0x00281938, 0x00283128, 0x00289138, 0x0028b128,
		0x0029b128, 0x00399138, 0x002819c8, 0x003831d8,
		0x00280940, 0x00380d04, 0x00615800, 0x00615800,
		0x0014008e, 0x000290a3, 0x000390a3, 0x0002b0a3,
		0x0003b0a3, 0x0020e860, 0x00308060, 0x00614080,
		0x00209000, 0x0020b000, 0x00219000, 0x0031b000,
		0x000270ad, 0x000250ad, 0x0020c460, 0x00204860,
		0x00304c60, 0x00614080, 0x00207000, 0x00305000,
		0x00120401, 0x00207348, 0x00305358, 0x00120401,
		0x002073c8, 0x003053d8, 0x00120401, 0x0020730c,
		0x00305318, 0x00120401, 0x00207238, 0x00305228,
		0x00120801, 0x000a50c1, 0x000ab0c4, 0x0003b0c4,
		0x0032b65a, 0x00223218, 0x00224a24, 0x00325228,
		0x0022b258, 0x0023b2d8, 0x00328a44, 0x00120801,
		0x000a50cc, 0x000ab0cf, 0x0003b0cf, 0x0030b44a,
		0x0020300c, 0x00204862, 0x00305038, 0x0020b048,
		0x0021b0c8, 0x00308846, 0x001300df, 0x000008dc,
		0x000004d7, 0x00000cd8, 0x0071c3a0, 0x0071c320,
		0x001d2da0, 0x0043c04c, 0x001040d8, 0x0071cea0,
		0x001d2d22, 0x001d1400, 0x001400f3, 0x000200eb,
		0x000218e9, 0x000290ea, 0x000230ea, 0x0002b0ea,
		0x0003b0ea, 0x000390ea, 0x000250ea, 0x000270ea,
		0x0071c0ac, 0x0071c1a0, 0x0071c120, 0x001c2020,
		0x0044d042, 0x001004ec, 0x001040fc, 0x00050100,
		0x00050510, 0x00050d05, 0x001c2922, 0x0043c042,
		0x001040f3, 0x0004e8fe, 0x0011011b, 0x001208fa,
		0x00615800, 0x001400f7, 0x001c2018, 0x0071cd20,
		0x0044d00c, 0x001400ec, 0x001c0800, 0x001400f3,
		0x00120903, 0x00615800, 0x00140100, 0x001c2148,
		0x0071cd20, 0x001c290a, 0x0043c042, 0x00104106,
		0x0004e90e, 0x0012090c, 0x00615800, 0x00140109,
		0x001c20d8, 0x0071cd20, 0x001c0800, 0x00140106,
		0x001c290a, 0x0043c002, 0x00104111, 0x0004e919,
		0x00120917, 0x00615800, 0x00140114, 0x001c2058,
		0x0071cd20, 0x001c0800, 0x00140111, 0x001d3806,
		0x0071cea0, 0x0013012e, 0x00000528, 0x00000d22,
		0x00000922, 0x0071c3a0, 0x001d2d22, 0x001d3404,
		0x0018018d, 0x00120824, 0x001c3000, 0x0071cfa0,
		0x001d2d0e, 0x001d3404, 0x001803e9, 0x00120824,
		0x001c3000, 0x0071c3a0, 0x0002013a, 0x00029148,
		0x00039140, 0x0002b15d, 0x0003b14e, 0x0002316e,
		0x00025172, 0x00027139, 0x00021938, 0x0071c0ac,
		0x0071c1a0, 0x001803e0, 0x00180183, 0x0012093e,
		0x00615800, 0x0014013b, 0x001c200c, 0x0071cda0,
		0x001c204a, 0x004cd010, 0x00100544, 0x0012414c,
		0x00180394, 0x00180183, 0x001c200c, 0x0071cda0,
		0x001c204a, 0x004cd010, 0x00100562, 0x00104162,
		0x001c200c, 0x0071c1a0, 0x001c205a, 0x004cd010,
		0x0002b557, 0x00100553, 0x00124166, 0x00180394,
		0x00180183, 0x001c200c, 0x0071cda0, 0x00100559,
		0x0012416e, 0x001803b7, 0x00180183, 0x001c200c,
		0x0071cda0, 0x001c205a, 0x004cd010, 0x0002b568,
		0x00100562, 0x00124166, 0x0018039c, 0x00180183,
		0x001c200c, 0x0071cda0, 0x001c200c, 0x0071c1a0,
		0x0010056a, 0x0012416e, 0x001803bf, 0x00180183,
		0x001c200c, 0x0071cda0, 0x001c202e, 0x001803f5,
		0x001c200c, 0x0071c1a0, 0x001803d3, 0x00180183,
		0x001c200c, 0x0071cda0, 0x00180178, 0x00160000,
		0x00021ba4, 0x0002939c, 0x0002b3bf, 0x000233c7,
		0x00039394, 0x0003b3b7, 0x000253d3, 0x000273e0,
		0x00020182, 0x00140024, 0x001a0000, 0x001c2020,
		0x0044d046, 0x00100584, 0x0012418a, 0x00050da1,
		0x0044d00c, 0x00140184, 0x0005019d, 0x0005059e,
		0x001c2922, 0x0043c04e, 0x0010418d, 0x0004e998,
		0x001c202e, 0x0006a19a, 0x0007a19a, 0x0006219a,
		0x0006819c, 0x0007819c, 0x0006019c, 0x00140024,
		0x001c0800, 0x0014018d, 0x001c0800, 0x001803f5,
		0x001e200c, 0x001e210c, 0x001c290e, 0x001803e9,
		0x001e200c, 0x001c0900, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x0018029b, 0x0018029b,
		0x0018029b, 0x0018029b, 0x00140184, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x0043c00a,
		0x0043c00a, 0x0043c00a, 0x0043c00a, 0x001a0000,
		0x00615480, 0x001c2040, 0x004cd814, 0x0002c39b,
		0x0002c7e2, 0x00100796, 0x00104396, 0x001e2000,
		0x00615080, 0x001c2040, 0x004cd014, 0x0002c3a3,
		0x0002c7e2, 0x0010079e, 0x0010439e, 0x001e2000,
		0x00615480, 0x001c2004, 0x004cd814, 0x0002c3b6,
		0x0002c7e2, 0x000283b4, 0x001007a6, 0x001243b6,
		0x001c2006, 0x004cd010, 0x0002c3b6, 0x0002c7e2,
		0x001007ad, 0x001243a5, 0x00020b95, 0x001403ad,
		0x00100796, 0x00104396, 0x001e2000, 0x00615480,
		0x001c2044, 0x0043c010, 0x00028fbd, 0x001043b9,
		0x001403cd, 0x001c2040, 0x00140396, 0x00615080,
		0x001c2044, 0x0043c01a, 0x00028fc5, 0x001043c1,
		0x001403cd, 0x001c2040, 0x0014039e, 0x00615480,
		0x001c2044, 0x0043c01e, 0x00028fa5, 0x001043c9,
		0x001403cd, 0x001c2074, 0x004ed200, 0x0002c3d2,
		0x001007ce, 0x001043ce, 0x001e2000, 0x00615080,
		0x001c2024, 0x0043c018, 0x0002c3df, 0x0002c7de,
		0x001043d5, 0x001c2026, 0x004bc20c, 0x0002c3df,
		0x001043da, 0x001e2000, 0x001403e2, 0x001e2000,
		0x00615080, 0x001c2062, 0x004bc00e, 0x0002c3e8,
		0x00054024, 0x00041c24, 0x001043e2, 0x00100424,
		0x001e2000, 0x0043c008, 0x001043e9, 0x00100424,
		0x0006a3f2, 0x0007a3f2, 0x000683f4, 0x000783f4,
		0x001c0800, 0x001403e9, 0x001c0800, 0x001403f5,
		0x001a0000, 0x0043c014, 0x00040c24, 0x001043f5,
		0x00100424, 0x000643fb, 0x001a0000, 0x001c0800,
		0x001403f5]

	regs = {
		'STATE_CLEAR':         0x0000,
		'PHYXA_LINK_STAT':     0x0a00,
		'PHYXA_ELOG':          0x0a08,
		'HSSXA_STAT_1':        0x0a30,
		'PHYXB_LINK_STAT':     0x0a40,
		'PHYXB_ELOG':          0x0a48,
		'HSSXB_STAT_1':        0x0a70,
		'PHYYA_LINK_STAT':     0x0a80,
		'PHYYA_ELOG':          0x0a88,
		'HSSYA_STAT_1':        0x0ab0,
		'PHYYB_LINK_STAT':     0x0ac0,
		'PHYYB_ELOG':          0x0ac8,
		'HSSYB_STAT_1':        0x0af0,
		'PHYZA_LINK_STAT':     0x0b00,
		'PHYZA_ELOG':          0x0b08,
		'HSSZA_STAT_1':        0x0b30,
		'PHYZB_LINK_STAT':     0x0b40,
		'PHYZB_ELOG':          0x0b48,
		'HSSZB_STAT_1':        0x0b70,
		'SEQ_INFO':            0x0c34,
		'ERROR_FSTAT':         0x0d10,
		'ERROR_NFSTAT':        0x0d20,
		'ATT_INDEX':           0x0dc0,
		'ERROR_STATUS':        0x3424,
		'MCTAG_INT_STATUS':    0x4084,
		'MCTAG_ERROR_STATR':   0x470c,
		'CDATA_ERROR_STATR':   0x4f0c,
		'CDATA_INT_STATUS':    0x4884,
	}

	events = (
		{'name': 'scc-reqs', 'counter': 0, 'event': 2, 'mask': 2, 'limit': 100000, 'units': 'req/s'},
	)
	delay_poll = 4E-6 # 4us
	max_tries = 5000

	class NumachipException(Exception):
		pass

	class HTErrorException(NumachipException):
		pass

	class AddressDecodeException(NumachipException):
		pass

	class TimeoutException(NumachipException):
		pass

	class BlockedException(NumachipException):
		pass

	def csr_read(self, reg):
		val = struct.unpack('>L', self.lcsr[reg:reg + 4])[0]
		if options.verbose > 1:
			print 'csr_read  %03x = %08x' % (reg, val)
		return val

	def csr_write(self, reg, val):
		if options.verbose > 1:
			print 'csr_write %03x = %08x' % (reg, val)
		self.lcsr[reg:reg + 4] = struct.pack('>L', val)

	def __init__(self):
		self.standalone = platform.oemn.standalone
		p = platform.processor
		mcfg_base = p.rdmsr(p.MCFG_BASE) & ~0x3f

		fd = os.open('/dev/mem', os.O_RDWR)
		if not self.standalone:
			self.lcsr = mmap.mmap(fd, 0x6000, mmap.MAP_SHARED, mmap.PROT_READ | mmap.PROT_WRITE, 0, 0x3ffffff08000)
		self.mcfg = mmap.mmap(fd, 256 << 20, mmap.MAP_SHARED, mmap.PROT_READ | mmap.PROT_WRITE, 0, mcfg_base)
		os.close(fd)

		if not self.standalone:
			self.own = self.csr_read(self.G0_NODE_IDS) >> 16
			# ensure raw engine is in ready state
			self.csr_write(self.G0_RAW_CONTROL, 1 << 12)

	def __del__(self):
		# skip if not initialised
		if not hasattr(self, 'f'):
			return

		self.lcsr.close()
		self.mcfg.close()

	def raw_entry_read(self):
		# caller should set index and autoinc
		lo = self.csr_read(self.G0_RAW_ENTRY_LO)
		hi = self.csr_read(self.G0_RAW_ENTRY_HI)
		return (lo << 32) | hi

	def raw_entry_write(self, entry):
		# caller should set index and autoinc
		self.csr_write(self.G0_RAW_ENTRY_LO, entry >> 32)
		self.csr_write(self.G0_RAW_ENTRY_HI, entry & 0xffffffff)

	def raw_write(self, sci, addr, val):
		# raw engine unstable on local card
		assert sci != 0xfff0 and sci != self.own

		cmd = (addr & 0xc) | 0x13 # writesb

		tries1 = 0
		while True:
			self.csr_write(self.G0_RAW_INDEX, 1 << 31)
			self.raw_entry_write((0x1f << 48) | ((sci & 0xffff) << 32) | (cmd << 16) | self.own)
			self.raw_entry_write((0x1f << 48) | addr)
			self.raw_entry_write((val << 32) | val)
			self.raw_entry_write((val << 32) | val)
			self.csr_write(self.G0_RAW_CONTROL, 1 << 2) # start

			tries2 = 0
			while True:
				time.sleep(self.delay_poll)
				ctrl = self.csr_read(self.G0_RAW_CONTROL)
				if (ctrl & 0xc00) == 0: # finished
					break

				tries2 += 1
				if tries2 > self.max_tries:
					# reset so ready for next time
					self.csr_write(self.G0_RAW_CONTROL, 1 << 12) # reset
					raise self.TimeoutException()

			# check response length
			rlen = ((ctrl >> 5) & 0xf)
			if rlen != 2 and rlen != 4:
				raise Exception('write response length %d received' % rlen)

			self.csr_write(self.G0_RAW_INDEX, 1 | (1 << 31))
			val = (self.raw_entry_read() >> 44) & 0xf
			if val == 0:
				# RESP_NORMAL; write successful
				break

			# should only receive a 'conflict' response
			if val != 4: # RESP_CONFLICT
				raise Exception('write response status %d received' % val)

			tries1 += 1
			if tries1 > self.max_tries:
				raise self.BlockedException()

	def raw_read(self, sci, addr):
		# raw engine unstable on itself
		assert sci != 0xfff0 and sci != self.own

		cmd = (addr & 0xc) | 0x3 # readsb

		tries1 = 0
		while True:
			self.csr_write(self.G0_RAW_INDEX, 0 | (1 << 31))
			self.raw_entry_write((0x1f << 48) | ((sci & 0xffff) << 32) | (cmd << 16) | self.own)
			self.raw_entry_write((0x1f << 48) | addr)
			self.csr_write(self.G0_RAW_CONTROL, 1 << 1) # start

			tries2 = 0
			while True:
				time.sleep(self.delay_poll)
				ctrl = self.csr_read(self.G0_RAW_CONTROL)
				if (ctrl & 0xc00) == 0: # finished
					break

				tries2 += 1
				sys.stdout.flush()
				if tries2 > self.max_tries:
					# reset so ready for next time
					self.csr_write(self.G0_RAW_CONTROL, 1 << 12) # reset
					raise self.TimeoutException()

			# check response length
			length = (ctrl >> 5) & 0xf
			if length == 4:
				# got data
				break
			assert length == 2
			# 'conflict' response is expected
			self.csr_write(self.G0_RAW_INDEX, 1 | (1 << 31))
			val = (self.raw_entry_read() >> 44) & 0xf

			if val in (6, 7): # RESP_TYPE (HT Error or NXA bit set), RESP_ADDRESS (address decode error or NXA)
				raise self.HTErrorException()

			if val != 4: # RESP_CONFLICT
				# retry
				break

			tries1 += 1
			if tries1 > self.max_tries:
				raise self.BlockedException()

		index = addr & 0xc
		# raw index auto-incremented from above
		if index == 0x0:
			val = self.raw_entry_read() >> 32
		elif index == 0x4:
			val = self.raw_entry_read() & 0xffffffff
		elif index == 0x8:
			self.csr_write(self.G0_RAW_INDEX, 3)
			val = self.raw_entry_read() >> 32
		elif index == 0xc:
			self.csr_write(self.G0_RAW_INDEX, 3)
			val = self.raw_entry_read() & 0xffffffff

		return val

	def csr_raw_read(self, sci, reg):
		if options.verbose:
			print 'read %03x:%x' % (sci, reg)
		if sci == 0xfff0 or sci == self.own:
			return self.csr_read(reg)

		return self.raw_read(sci, (0xfffff << 28) | (reg & 0x7ffc))

	def csr_raw_write(self, sci, reg, val):
		if options.verbose:
			print 'write %03x:%x' % (sci, reg)
		if sci == 0xfff0 or sci == self.own:
			self.csr_write(reg, val)
			return

		self.raw_write(sci, (0xfffff << 28) | (reg & 0x7ffc), val)

	def mmcfg_raw_read(self, sci, bus, dev, func, reg):
		if sci == 0xfff0 or sci == self.own:
			addr = (bus << 20) | (dev << 15) | (func << 12) | reg
			return struct.unpack('=L', self.mcfg[addr:addr + 4])[0]

		val = self.raw_read(sci, (0x3f0000 << 24) | (bus << 20) | (dev << 15) | (func << 12) | reg)
		return bswap(val)

	def mmcfg_raw_write(self, sci, bus, dev, func, reg, val):
		if sci == 0xfff0 or sci == self.own:
			addr = (bus << 20) | (dev << 15) | (func << 12) | reg
			self.mcfg[addr:addr + 4] = struct.pack('=L', val)
			return

		self.raw_write(sci, (0x3f0000 << 24) | (bus << 20) | (dev << 15) | (func << 12) | reg, bswap(val))

	def washdelay(self, sci, calls, delays):
		assert calls*delays <= self.UCODE_CALLS*self.UCODE_DELAYS, 'Washdelay value too large'

		# copy so modifications are discarded
		ucode2 = list(self.ucode)

		counter = 0
		for i in range(len(ucode2)):
			if ucode2[i] == self.UCODE_DELAY:
				counter += 1
				if counter > delays:
					ucode2[i] = self.UCODE_DELAYRET
					break
		assert counter

		counter = 0
		for i in range(len(ucode2)):
			if ucode2[i] == self.UCODE_CALL:
				counter += 1
				if counter > calls:
					ucode2[i] = self.UCODE_CALLRET
					break
		assert counter

		for i in range(len(self.ucode)):
			self.csr_raw_write(sci, self.H2S_CSR_G0_SEQ_INDEX, i)
			self.csr_raw_write(sci, self.H2S_CSR_G0_WCS_ENTRY, ucode2[i])

		return calls * delays

def bswap(val):
	s = struct.pack('>L', val)
	return struct.unpack('<L', s)[0]

def en(val, bit):
	if (val >> bit) & 1:
		return '+'
	return '-'
