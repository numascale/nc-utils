import ctypes, os, subprocess, struct, mmap, time, sys, errno

verbose = 1

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
			# warning: alignment occurs at type boundaries
		)

		def __init__(self):
			try:
				with open('/sys/firmware/acpi/tables/OEMN', 'rb') as f:
					f.readinto(self)
				assert self.acpi_len >= 52
			except IOError as e:
				if e.errno != 2:
					raise
				self.size_x = 2
				self.size_y = 4
				self.size_z = 0

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
			try:
				return struct.unpack('=Q', os.read(self.fd, 8))[0]
			except OSError as e:
				if e.errno != 5: # Input/Output error
					raise
				raise SystemExit('MSR 0x%08x not support on this architecture' % msr)

		def wrmsr(self, msr, val):
			os.lseek(self.fd, msr, os.SEEK_SET)
			os.write(self.fd, struct.pack('=Q', val))

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
	H2S_CSR_G0_NODE_IDS       = (0 << 12) | 0x008
	H2S_CSR_G0_RAW_CONTROL    = (0 << 12) | 0xc50
	H2S_CSR_G0_RAW_INDEX      = (0 << 12) | 0xc54
	H2S_CSR_G0_RAW_ENTRY_LO   = (0 << 12) | 0xc58
	H2S_CSR_G0_RAW_ENTRY_HI   = (0 << 12) | 0xc5c
	H2S_CSR_G3_MMCFG_BASE     = (3 << 12) | 0x010
	H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_0 = (3 << 12) | 0xfa0
	H2S_CSR_G3_PERFORMANCE_COUNTER_0_40_BIT_UPPER_BITS = (3 << 12) | 0xfc0
	H2S_CSR_G3_PERFORMANCE_COUNTER_0_40_BIT_LOWER_BITS = (3 << 12) | 0xfc4

	events = (
		{'name': 'scc-reqs', 'counter': 0, 'event': 2, 'mask': 2, 'limit': 100000, 'units': 'req/s'},
	)
	delay_poll = 1E-3 # 1ms
	max_tries = 1000

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

	def csr_read(self, sci, reg):
		addr = (sci << 16) | (1 << 15) | reg # non-geo access
		if verbose > 1:
			print 'csr_read(0x%x) =' % (0x3fff00000000 | addr),
			sys.stdout.flush()

		try:
			val = struct.unpack('>L', self.csr[addr:addr + 4])[0]
		except struct.error:
			raise SystemExit('fatal attempted access outside CSR space at offset 0x%x' % reg)

		if verbose > 1:
			print '0x%08x' % val
		return val

	def csr_write(self, sci, reg, val):
		addr = (sci << 16) | (1 << 15) | reg # non-geo access
		if verbose > 1:
			print 'csr_write(0x%x) = 0x%08x' % (0x3fff00000000 | addr, val)
		self.csr[addr:addr + 4] = struct.pack('>L', val)

	def __init__(self, platform):
		self.platform = platform

		fd = os.open('/dev/mem', os.O_RDWR)
		self.csr = mmap.mmap(fd, 0x100000000, mmap.MAP_SHARED, mmap.PROT_READ | mmap.PROT_WRITE, 0, 0x3fff00000000)
		os.close(fd)

		self.own = self.csr_read(0xfff0, self.H2S_CSR_G0_NODE_IDS) >> 16

		# ensure raw engine is in ready state
		self.csr_write(0xfff0, self.H2S_CSR_G0_RAW_CONTROL, 1 << 12)

	def __del__(self):
		# skip if not initialised
		if not hasattr(self, 'f'):
			return

		self.csr.close()

	def raw_entry_read(self, index):
		self.csr_write(0xfff0, self.H2S_CSR_G0_RAW_INDEX, index)
		lo = self.csr_read(0xfff0, self.H2S_CSR_G0_RAW_ENTRY_LO)
		hi = self.csr_read(0xfff0, self.H2S_CSR_G0_RAW_ENTRY_HI)
		return (lo << 32) | hi

	def raw_entry_write_first(self, entry):
		self.csr_write(0xfff0, self.H2S_CSR_G0_RAW_INDEX, 1 << 31) # first entry, autoinc
		self.csr_write(0xfff0, self.H2S_CSR_G0_RAW_ENTRY_LO, entry >> 32)
		self.csr_write(0xfff0, self.H2S_CSR_G0_RAW_ENTRY_HI, entry & 0xffffffff)

	def raw_entry_write(self, entry):
		self.csr_write(0xfff0, self.H2S_CSR_G0_RAW_ENTRY_LO, entry >> 32)
		self.csr_write(0xfff0, self.H2S_CSR_G0_RAW_ENTRY_HI, entry & 0xffffffff)

	def raw_write(self, sci, addr, val):
		cmd = (addr & 0xc) | 0x13 # writesb

		tries1 = 0
		while True:
			self.raw_entry_write_first((0x1f << 48) | ((sci & 0xffff) << 32) | (cmd << 16) | self.own)
			self.raw_entry_write((0x1f << 48) | addr)
			self.raw_entry_write((val << 32) | val)
			self.raw_entry_write((val << 32) | val)
			self.csr_write(0xfff0, self.H2S_CSR_G0_RAW_CONTROL, 1 << 2) # start

			tries2 = 0
			while True:
				time.sleep(self.delay_poll)
				ctrl = self.csr_read(0xfff0, self.H2S_CSR_G0_RAW_CONTROL)
				if (ctrl & 0xc00) == 0: # finished
					break

				tries2 += 1
				if tries2 > self.max_tries:
					# reset so ready for next time
					self.csr_write(0xfff0, self.H2S_CSR_G0_RAW_CONTROL, 1 << 12) # reset
					raise self.TimeoutException()

			# check response length
			assert ((ctrl >> 5) & 0xf) == 2

			val = (self.raw_entry_read(1) >> 44) & 0xf
			if val == 0:
				# RESP_NORMAL; write successful
				break

			# should only receive a 'conflict' response
			if val != 4: # RESP_CONFLICT
				raise Exception('write: response status %d received' % val)

			tries1 += 1
			if tries1 > self.max_tries:
				raise self.BlockedException()

	def raw_read(self, sci, addr):
		cmd = (addr & 0xc) | 0x3 # readsb

		tries1 = 0
		while True:
			self.raw_entry_write_first((0x1f << 48) | ((sci & 0xffff) << 32) | (cmd << 16) | self.own)
			self.raw_entry_write((0x1f << 48) | addr)
			self.csr_write(0xfff0, self.H2S_CSR_G0_RAW_CONTROL, 1 << 1) # start

			tries2 = 0
			while True:
				time.sleep(self.delay_poll)
				ctrl = self.csr_read(0xfff0, self.H2S_CSR_G0_RAW_CONTROL)
				if (ctrl & 0xc00) == 0: # finished
					break

				tries2 += 1
				sys.stdout.flush()
				if tries2 > self.max_tries:
					# reset so ready for next time
					self.csr_write(0xfff0, self.H2S_CSR_G0_RAW_CONTROL, 1 << 12) # reset
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
				raise Exception('read: response status %d received' % val)

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

	def bswap(self, val):
		s = struct.pack('>L', val)
		return struct.unpack('<L', s)[0]

	def mmcfg_raw_read(self, sci, bus, dev, fn, reg):
		if sci == 0xfff0:
			return self.platform.pci.readl(bus, dev, fn, reg)

		val = self.raw_read(sci, (0x3f0000 << 24) | (bus << 20) | (dev << 15) | (fn << 12) | reg)
		return self.bswap(val)

	def mmcfg_raw_write(self, sci, bus, dev, fn, reg, val):
		if sci == 0xfff0:
			self.platform.pci.writel(bus, dev, fn, reg, val)

		self.raw_write(sci, (0x3f0000 << 24) | (bus << 20) | (dev << 15) | (fn << 12) | reg, self.bswap(val))

