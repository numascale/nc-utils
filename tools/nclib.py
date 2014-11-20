# r3

import ctypes, os, subprocess, struct, mmap, time, sys, errno

verbose = 0

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
				if e.errno != 2:
					raise
				self.size_x = 2
				self.size_y = 0
				self.size_z = 0
				self.standalone = True

			if verbose > 0:
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
				if e.errno != 2:
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

class Cpuset:
	def __init__(self, nodesperserver, corespercpu):
		self.nodesperserver = nodesperserver
		self.corespercpu = corespercpu
		self.used = []
		if not os.path.isdir('/dev/cpuset'):
			raise SystemExit('need to mount /dev/cpuset')

		self.libc = ctypes.CDLL('libc.so.6')

	def bind(self, name, server):
		path = '/dev/cpuset/' + name
#		print 'server=%d' % server

		try:
			os.mkdir(path)
		except OSError as e:
			if e.errno != errno.EEXIST:
				raise e

		with open(path + '/cpus', 'w') as f:
			start = server * self.corespercpu * self.nodesperserver
			end = (server + 1) * self.corespercpu * self.nodesperserver - 1
#			print 'start=%d end=%d' % (start, end)
			f.write('%d-%d' % (start, end))

		with open(path + '/mems', 'w') as f:
			start = server * self.nodesperserver
			end = (server + 1) * self.nodesperserver - 1
			f.write('%d-%d' % (start, end))

		with open(path + '/tasks', 'w') as f:
			pid = self.libc.syscall(186) # gettid
			f.write('%d' % pid)

		self.used.append(name)

	def __del__(self):
		for elem in self.used:
			os.rmdir('/dev/cpuset/' + elem)

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
			desc += 'core=%u' % (status >> 32) & 0xf

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

	regs = {
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
		'ERROR_STATUS':        0x3424,
		'MCTAG_INT_STATUS':    0x4084,
		'MCTAG_ERROR_STATR':   0x470c,
		'CDATA_ERROR_STATR':   0x4f0c,
		'CDATA_INT_STATUS':    0x4884,
	}

	events = (
		{'name': 'scc-reqs', 'counter': 0, 'event': 2, 'mask': 2, 'limit': 100000, 'units': 'req/s'},
	)
	delay_poll = 2E-6 # 1us
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
		if verbose > 1:
			print 'csr_read  %03x = %08x' % (reg, val)
		return val

	def csr_write(self, reg, val):
		if verbose > 1:
			print 'csr_write %03x = %08x' % (reg, val)
		self.lcsr[reg:reg + 4] = struct.pack('>L', val)

	def __init__(self, platform):
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

	def raw_entry_read(self, index):
		self.csr_write(self.G0_RAW_INDEX, index)
		lo = self.csr_read(self.G0_RAW_ENTRY_LO)
		hi = self.csr_read(self.G0_RAW_ENTRY_HI)
		return (lo << 32) | hi

	def raw_entry_write(self, index, entry):
		self.csr_write(self.G0_RAW_INDEX, index)
		self.csr_write(self.G0_RAW_ENTRY_LO, entry >> 32)
		self.csr_write(self.G0_RAW_ENTRY_HI, entry & 0xffffffff)

	def raw_write(self, sci, addr, val):
		cmd = (addr & 0xc) | 0x13 # writesb

		tries1 = 0
		while True:
			self.raw_entry_write(0, (0x1f << 48) | ((sci & 0xffff) << 32) | (cmd << 16) | self.own)
			self.raw_entry_write(1, (0x1f << 48) | addr)
			self.raw_entry_write(2, (val << 32) | val)
			self.raw_entry_write(3, (val << 32) | val)
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
			if rlen != 2:
				print 'warning: write response length %d unexpected' % rlen
				tries1 += 1
				continue

			val = (self.raw_entry_read(1) >> 44) & 0xf
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
		cmd = (addr & 0xc) | 0x3 # readsb

		tries1 = 0
		while True:
			self.raw_entry_write(0, (0x1f << 48) | ((sci & 0xffff) << 32) | (cmd << 16) | self.own)
			self.raw_entry_write(1, (0x1f << 48) | addr)
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
			val = (self.raw_entry_read(1) >> 44) & 0xf

			if val in (6, 7): # RESP_TYPE (HT Error or NXA bit set), RESP_ADDRESS (address decode error or NXA)
				raise self.HTErrorException()

			if val != 4: # RESP_CONFLICT
				raise Exception('read response status %d received' % val)

			tries1 += 1
			if tries1 > self.max_tries:
				raise self.BlockedException()

		index = addr & 0xc
		if index == 0x0:
			val = self.raw_entry_read(2) >> 32
		elif index == 0x4:
			val = self.raw_entry_read(2) & 0xffffffff
		elif index == 0x8:
			val = self.raw_entry_read(3) >> 32
		elif index == 0xc:
			val = self.raw_entry_read(3) & 0xffffffff

		return val

	def csr_raw_read(self, sci, reg):
		return self.raw_read(sci, (0xfffff << 28) | (reg & 0x7ffc))

	def csr_raw_write(self, sci, reg, val):
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

def bswap(val):
	s = struct.pack('>L', val)
	return struct.unpack('<L', s)[0]

def en(val, bit):
	if (val >> bit) & 1:
		return '+'
	return '-'
