# r2

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

		self.libc = ctypes.CDLL("libc.so.6")

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

class Numachip:
	G0_NODE_IDS      = (0 << 12) | 0x008
	G0_RAW_CONTROL   = (0 << 12) | 0xc50
	G0_RAW_INDEX     = (0 << 12) | 0xc54
	G0_RAW_ENTRY_LO  = (0 << 12) | 0xc58
	G0_RAW_ENTRY_HI  = (0 << 12) | 0xc5c
	G3_MMCFG_BASE    = (3 << 12) | 0x010
	G3_HT_NODEID     = (3 << 12) | 0x024
	G3_SELECT_COUNTER = (3 << 12) | 0xf78
	G3_COMPARE_AND_MASK_OF_COUNTER_0 = (3 << 12) | 0xfa0
	G3_PERFORMANCE_COUNTER_0_40_BIT_UPPER_BITS = (3 << 12) | 0xfc0
	G3_PERFORMANCE_COUNTER_0_40_BIT_LOWER_BITS = (3 << 12) | 0xfc4
	events = (
		{'name': 'scc-reqs', 'counter': 0, 'event': 2, 'mask': 2, 'limit': 100000, 'units': 'req/s'},
	)
	delay_poll = 1E-6 # 1us
	max_tries = 400

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
