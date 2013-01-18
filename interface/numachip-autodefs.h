#undef H2S_CSR_F0_DEVICE_VENDOR_ID_REGISTER
#define H2S_CSR_F0_DEVICE_VENDOR_ID_REGISTER (0x00)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_device_vendor_id_register {
	unsigned _vendor_id: 16;
	unsigned _device_id: 16;
};
#endif
#undef H2S_CSR_F0_STATUS_COMMAND_REGISTER
#define H2S_CSR_F0_STATUS_COMMAND_REGISTER (0x04)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_status_command_register {
	unsigned : 1;
	unsigned _memory_space_enable: 1;
	unsigned : 18;
	unsigned _capabilities_list_present: 1;
	unsigned : 11;
};
#endif
#undef H2S_CSR_F0_CLASS_CODE_REVISION_ID_REGISTER
#define H2S_CSR_F0_CLASS_CODE_REVISION_ID_REGISTER (0x08)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_class_code_revision_id_register {
	unsigned : 32;
};
#endif
#undef H2S_CSR_F0_HEADER_TYPE_REGISTER
#define H2S_CSR_F0_HEADER_TYPE_REGISTER (0x0C)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_header_type_register {
	unsigned : 32;
};
#endif
#undef H2S_CSR_F0_BASE_ADDRESS_REGISTER_0
#define H2S_CSR_F0_BASE_ADDRESS_REGISTER_0 (0x10)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_base_address_register_0 {
	unsigned : 20;
	unsigned _address: 12;
};
#endif
#undef H2S_CSR_F0_BASE_ADDRESS_REGISTER_1
#define H2S_CSR_F0_BASE_ADDRESS_REGISTER_1 (0x14)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_base_address_register_1 {
	unsigned : 32;
};
#endif
#undef H2S_CSR_F0_BASE_ADDRESS_REGISTER_2
#define H2S_CSR_F0_BASE_ADDRESS_REGISTER_2 (0x18)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_base_address_register_2 {
	unsigned : 32;
};
#endif
#undef H2S_CSR_F0_BASE_ADDRESS_REGISTER_3
#define H2S_CSR_F0_BASE_ADDRESS_REGISTER_3 (0x1C)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_base_address_register_3 {
	unsigned : 32;
};
#endif
#undef H2S_CSR_F0_BASE_ADDRESS_REGISTER_4
#define H2S_CSR_F0_BASE_ADDRESS_REGISTER_4 (0x20)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_base_address_register_4 {
	unsigned : 32;
};
#endif
#undef H2S_CSR_F0_BASE_ADDRESS_REGISTER_5
#define H2S_CSR_F0_BASE_ADDRESS_REGISTER_5 (0x24)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_base_address_register_5 {
	unsigned : 32;
};
#endif
#undef H2S_CSR_F0_CARDBUS_CIS_POINTER
#define H2S_CSR_F0_CARDBUS_CIS_POINTER (0x28)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_cardbus_cis_pointer {
	unsigned : 32;
};
#endif
#undef H2S_CSR_F0_SUB_SYSTEM_ID_VENDOR_ID_REGISTER
#define H2S_CSR_F0_SUB_SYSTEM_ID_VENDOR_ID_REGISTER (0x2C)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_sub_system_id_vendor_id_register {
	unsigned : 32;
};
#endif
#undef H2S_CSR_F0_EXPANSION_ROM_BASE_ADDRESS
#define H2S_CSR_F0_EXPANSION_ROM_BASE_ADDRESS (0x30)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_expansion_rom_base_address {
	unsigned _rom: 1;
	unsigned : 15;
	unsigned _base: 16;
};
#endif
#undef H2S_CSR_F0_CAPABILITIES_POINTER_REGISTER
#define H2S_CSR_F0_CAPABILITIES_POINTER_REGISTER (0x34)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_capabilities_pointer_register {
	unsigned : 32;
};
#endif
#undef H2S_CSR_F0_RESERVED
#define H2S_CSR_F0_RESERVED (0x38)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_reserved {
	unsigned : 32;
};
#endif
#undef H2S_CSR_F0_MAX_LATENCY_MIN_GNT_INT_PIN_INT_LINE_REGISTER
#define H2S_CSR_F0_MAX_LATENCY_MIN_GNT_INT_PIN_INT_LINE_REGISTER (0x3C)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_max_latency_min_gnt_int_pin_int_line_register {
	unsigned : 32;
};
#endif
#undef H2S_CSR_F0_LINK_CAPABILITY_HEADER
#define H2S_CSR_F0_LINK_CAPABILITY_HEADER (0x80)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_link_capability_header {
	unsigned _capability: 8;
	unsigned : 8;
	unsigned _feature: 8;
	unsigned : 8;
};
#endif
#undef H2S_CSR_F0_LINK_CONTROL_REGISTER
#define H2S_CSR_F0_LINK_CONTROL_REGISTER (0x84)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_link_control_register {
	unsigned : 1;
	unsigned _crcflooden: 1;
	unsigned : 1;
	unsigned _crcforceerr: 1;
	unsigned : 1;
	unsigned _initcomplete: 1;
	unsigned : 2;
	unsigned : 2;
	unsigned : 2;
	unsigned _isocen: 1;
	unsigned _ldtstoptrien: 1;
	unsigned _extctl: 1;
	unsigned _addr64biten: 1;
	unsigned _maxwidthin: 3;
	unsigned : 1;
	unsigned _maxwidthout: 3;
	unsigned : 1;
	unsigned _widthin: 3;
	unsigned : 1;
	unsigned _widthout: 3;
	unsigned : 1;
};
#endif
#undef H2S_CSR_F0_LINK_FREQUENCY_REVISION_REGISTER
#define H2S_CSR_F0_LINK_FREQUENCY_REVISION_REGISTER (0x88)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_link_frequency_revision_register {
	unsigned _revision: 8;
	unsigned _linkfreq: 4;
	unsigned : 4;
	unsigned _lnkfreqcap: 16;
};
#endif
#undef H2S_CSR_F0_LINK_FEATURE_CAPABILITY_REGISTER
#define H2S_CSR_F0_LINK_FEATURE_CAPABILITY_REGISTER (0x8C)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_link_feature_capability_register {
	unsigned _isocmode: 1;
	unsigned _ldtstopmode: 1;
	unsigned _crctstmode: 1;
	unsigned _extctlrqd: 1;
	unsigned _64bitaddr: 1;
	unsigned _unitidreorderdis: 1;
	unsigned : 2;
	unsigned _extregset: 1;
	unsigned _upstrcfgcap: 1;
	unsigned : 22;
};
#endif
#undef H2S_CSR_F0_CHTX_COHERENT_LINK_CAPABILITY_HEADER
#define H2S_CSR_F0_CHTX_COHERENT_LINK_CAPABILITY_HEADER (0xA4)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_chtx_coherent_link_capability_header {
	unsigned _capability: 8;
	unsigned : 8;
	unsigned _feature: 8;
	unsigned : 8;
};
#endif
#undef H2S_CSR_F0_LINK_BASE_CHANNEL_BUFFER_COUNT_REGISTER
#define H2S_CSR_F0_LINK_BASE_CHANNEL_BUFFER_COUNT_REGISTER (0xA8)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_link_base_channel_buffer_count_register {
	unsigned _npreqcmd: 5;
	unsigned _preq: 3;
	unsigned _rspcmd: 4;
	unsigned _probecmd: 4;
	unsigned _npreqdata: 2;
	unsigned _rspdata: 2;
	unsigned _freecmd: 5;
	unsigned _freedata: 3;
	unsigned : 3;
	unsigned _lockbc: 1;
};
#endif
#undef H2S_CSR_F0_LINK_ISOCHRONOUS_CHANNEL_BUFFER_COUNT_REGISTERS
#define H2S_CSR_F0_LINK_ISOCHRONOUS_CHANNEL_BUFFER_COUNT_REGISTERS (0xAC)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_link_isochronous_channel_buffer_count_registers {
	unsigned : 16;
	unsigned _isocnpreqcmd: 3;
	unsigned _isocpreq: 3;
	unsigned _isocrspcmd: 3;
	unsigned _isocnpreqdata: 2;
	unsigned _isocrspdata: 2;
	unsigned : 3;
};
#endif
#undef H2S_CSR_F0_LINK_TYPE_REGISTERS
#define H2S_CSR_F0_LINK_TYPE_REGISTERS (0xB0)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_link_type_registers {
	unsigned _linkcon: 1;
	unsigned _initcomplete: 1;
	unsigned _noncoherent: 1;
	unsigned : 1;
	unsigned _linkconpend: 1;
	unsigned : 27;
};
#endif
#undef H2S_CSR_F0_CHTX_COHERENCY_CAPABILITY_HEADER
#define H2S_CSR_F0_CHTX_COHERENCY_CAPABILITY_HEADER (0xC4)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_chtx_coherency_capability_header {
	unsigned _capability: 8;
	unsigned : 8;
	unsigned _feature: 8;
	unsigned : 8;
};
#endif
#undef H2S_CSR_F0_CHTX_NODE_ID
#define H2S_CSR_F0_CHTX_NODE_ID (0xC8)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_chtx_node_id {
	unsigned _nodeid: 3;
	unsigned : 5;
	unsigned _nodecnt: 3;
	unsigned : 5;
	unsigned _sbnode: 3;
	unsigned : 5;
	unsigned _lknode: 3;
	unsigned : 5;
};
#endif
#undef H2S_CSR_F0_CHTX_CPU_COUNT
#define H2S_CSR_F0_CHTX_CPU_COUNT (0xCC)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_chtx_cpu_count {
	unsigned _cpucnt: 16;
	unsigned : 16;
};
#endif
#undef H2S_CSR_F0_CHTX_UNIT_ID
#define H2S_CSR_F0_CHTX_UNIT_ID (0xD0)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_chtx_unit_id {
	unsigned _cpuunitid: 2;
	unsigned : 2;
	unsigned _mctunitid: 2;
	unsigned _hbunitid: 2;
	unsigned : 4;
	unsigned _cpuunitwritable: 1;
	unsigned _mctunitwritable: 1;
	unsigned _hbunitwritable: 1;
	unsigned : 17;
};
#endif
#undef H2S_CSR_F0_CHTX_LINK_TRANSACTION_CONTROL
#define H2S_CSR_F0_CHTX_LINK_TRANSACTION_CONTROL (0xD4)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_chtx_link_transaction_control {
	unsigned : 4;
	unsigned _dismts: 1;
	unsigned : 3;
	unsigned _dispmemc: 1;
	unsigned _disrmtpmemc: 1;
	unsigned : 1;
	unsigned _rwsppasspw: 1;
	unsigned : 5;
	unsigned _apicextbrdcst: 1;
	unsigned _apicextid: 1;
	unsigned _apicextspur: 1;
	unsigned _seqidsrcnodeen: 1;
	unsigned : 2;
	unsigned _installstates: 1;
	unsigned _disprefmodeen: 1;
	unsigned : 7;
};
#endif
#undef H2S_CSR_F0_CHTX_LINK_INITIALIZATION_CONTROL
#define H2S_CSR_F0_CHTX_LINK_INITIALIZATION_CONTROL (0xD8)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_chtx_link_initialization_control {
	unsigned _routetbldis: 1;
	unsigned _deflnk: 3;
	unsigned : 4;
	unsigned _defsublnk: 1;
	unsigned : 23;
};
#endif
#undef H2S_CSR_F0_CHTX_ADDITIONAL_LINK_TRANSACTION_CONTROL
#define H2S_CSR_F0_CHTX_ADDITIONAL_LINK_TRANSACTION_CONTROL (0xDC)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_chtx_additional_link_transaction_control {
	unsigned : 1;
	unsigned : 1;
	unsigned _dataordercap: 1;
	unsigned _chtextnodeen: 1;
	unsigned _chtextnodecfgen: 1;
	unsigned : 4;
	unsigned : 1;
	unsigned : 6;
	unsigned _chtextaddrcap: 1;
	unsigned _chtextnodecap: 1;
	unsigned _bussegmentcap: 1;
	unsigned : 1;
	unsigned : 12;
};
#endif
#undef H2S_CSR_F0_ROUTING_TABLE_CAPABILITY_HEADER
#define H2S_CSR_F0_ROUTING_TABLE_CAPABILITY_HEADER (0xE8)
#ifndef __ASSEMBLER__
struct dnc_csr_f0_routing_table_capability_header {
	unsigned _capability: 8;
	unsigned : 8;
	unsigned _feature: 8;
	unsigned : 8;
};
#endif
#undef H2S_CSR_F1_DEVICE_VENDOR_ID_REGISTER
#define H2S_CSR_F1_DEVICE_VENDOR_ID_REGISTER (0x00)
#ifndef __ASSEMBLER__
struct dnc_csr_f1_device_vendor_id_register {
	unsigned _vendor_id: 16;
	unsigned _device_id: 16;
};
#endif
#undef H2S_CSR_F1_STATUS_COMMAND_REGISTER
#define H2S_CSR_F1_STATUS_COMMAND_REGISTER (0x04)
#ifndef __ASSEMBLER__
struct dnc_csr_f1_status_command_register {
	unsigned : 1;
	unsigned _memory_space_enable: 1;
	unsigned : 18;
	unsigned _capabilities_list_present: 1;
	unsigned : 11;
};
#endif
#undef H2S_CSR_F1_CLASS_CODE_REVISION_ID_REGISTER
#define H2S_CSR_F1_CLASS_CODE_REVISION_ID_REGISTER (0x08)
#ifndef __ASSEMBLER__
struct dnc_csr_f1_class_code_revision_id_register {
	unsigned : 32;
};
#endif
#undef H2S_CSR_F1_HEADER_TYPE_REGISTER
#define H2S_CSR_F1_HEADER_TYPE_REGISTER (0x0C)
#ifndef __ASSEMBLER__
struct dnc_csr_f1_header_type_register {
	unsigned : 32;
};
#endif
#undef H2S_CSR_F1_BASE_ADDRESS_REGISTER_0
#define H2S_CSR_F1_BASE_ADDRESS_REGISTER_0 (0x10)
#ifndef __ASSEMBLER__
struct dnc_csr_f1_base_address_register_0 {
	unsigned : 32;
};
#endif
#undef H2S_CSR_F1_BASE_ADDRESS_REGISTER_1
#define H2S_CSR_F1_BASE_ADDRESS_REGISTER_1 (0x14)
#ifndef __ASSEMBLER__
struct dnc_csr_f1_base_address_register_1 {
	unsigned : 32;
};
#endif
#undef H2S_CSR_F1_BASE_ADDRESS_REGISTER_2
#define H2S_CSR_F1_BASE_ADDRESS_REGISTER_2 (0x18)
#ifndef __ASSEMBLER__
struct dnc_csr_f1_base_address_register_2 {
	unsigned : 32;
};
#endif
#undef H2S_CSR_F1_BASE_ADDRESS_REGISTER_3
#define H2S_CSR_F1_BASE_ADDRESS_REGISTER_3 (0x1C)
#ifndef __ASSEMBLER__
struct dnc_csr_f1_base_address_register_3 {
	unsigned : 32;
};
#endif
#undef H2S_CSR_F1_BASE_ADDRESS_REGISTER_4
#define H2S_CSR_F1_BASE_ADDRESS_REGISTER_4 (0x20)
#ifndef __ASSEMBLER__
struct dnc_csr_f1_base_address_register_4 {
	unsigned : 32;
};
#endif
#undef H2S_CSR_F1_BASE_ADDRESS_REGISTER_5
#define H2S_CSR_F1_BASE_ADDRESS_REGISTER_5 (0x24)
#ifndef __ASSEMBLER__
struct dnc_csr_f1_base_address_register_5 {
	unsigned : 32;
};
#endif
#undef H2S_CSR_F1_CARDBUS_CIS_POINTER
#define H2S_CSR_F1_CARDBUS_CIS_POINTER (0x28)
#ifndef __ASSEMBLER__
struct dnc_csr_f1_cardbus_cis_pointer {
	unsigned : 32;
};
#endif
#undef H2S_CSR_F1_SUB_SYSTEM_ID_VENDOR_ID_REGISTER
#define H2S_CSR_F1_SUB_SYSTEM_ID_VENDOR_ID_REGISTER (0x2C)
#ifndef __ASSEMBLER__
struct dnc_csr_f1_sub_system_id_vendor_id_register {
	unsigned : 32;
};
#endif
#undef H2S_CSR_F1_EXPANSION_ROM_BASE_ADDRESS
#define H2S_CSR_F1_EXPANSION_ROM_BASE_ADDRESS (0x30)
#ifndef __ASSEMBLER__
struct dnc_csr_f1_expansion_rom_base_address {
	unsigned : 32;
};
#endif
#undef H2S_CSR_F1_CAPABILITIES_POINTER_REGISTER
#define H2S_CSR_F1_CAPABILITIES_POINTER_REGISTER (0x34)
#ifndef __ASSEMBLER__
struct dnc_csr_f1_capabilities_pointer_register {
	unsigned : 32;
};
#endif
#undef H2S_CSR_F1_RESERVED
#define H2S_CSR_F1_RESERVED (0x38)
#ifndef __ASSEMBLER__
struct dnc_csr_f1_reserved {
	unsigned : 32;
};
#endif
#undef H2S_CSR_F1_MAX_LATENCY_MIN_GNT_INT_PIN_INT_LINE_REGISTER
#define H2S_CSR_F1_MAX_LATENCY_MIN_GNT_INT_PIN_INT_LINE_REGISTER (0x3C)
#ifndef __ASSEMBLER__
struct dnc_csr_f1_max_latency_min_gnt_int_pin_int_line_register {
	unsigned : 32;
};
#endif
#undef H2S_CSR_F1_RESOURCE_MAPPING_CAPABILITY_HEADER
#define H2S_CSR_F1_RESOURCE_MAPPING_CAPABILITY_HEADER (0x40)
#ifndef __ASSEMBLER__
struct dnc_csr_f1_resource_mapping_capability_header {
	unsigned _capability: 8;
	unsigned : 8;
	unsigned : 3;
	unsigned : 5;
	unsigned : 8;
};
#endif
#undef H2S_CSR_F1_RESOURCE_MAPPING_ENTRY_INDEX
#define H2S_CSR_F1_RESOURCE_MAPPING_ENTRY_INDEX (0x44)
#ifndef __ASSEMBLER__
struct dnc_csr_f1_resource_mapping_entry_index {
	unsigned _entry: 3;
	unsigned : 29;
};
#endif
#undef H2S_CSR_F1_DRAM_BASE_ADDRESS_REGISTERS
#define H2S_CSR_F1_DRAM_BASE_ADDRESS_REGISTERS (0x48)
#ifndef __ASSEMBLER__
struct dnc_csr_f1_dram_base_address_registers {
	unsigned _readenable: 1;
	unsigned _writeenable: 1;
	unsigned : 6;
	unsigned _drambase_47_24: 24;
};
#endif
#undef H2S_CSR_F1_DRAM_LIMIT_ADDRESS_REGISTERS
#define H2S_CSR_F1_DRAM_LIMIT_ADDRESS_REGISTERS (0x4C)
#ifndef __ASSEMBLER__
struct dnc_csr_f1_dram_limit_address_registers {
	unsigned _dstnode: 3;
	unsigned : 5;
	unsigned _dramlimit_47_24: 24;
};
#endif
#undef H2S_CSR_F1_MMIO_BASE_ADDRESS_REGISTERS
#define H2S_CSR_F1_MMIO_BASE_ADDRESS_REGISTERS (0x50)
#ifndef __ASSEMBLER__
struct dnc_csr_f1_mmio_base_address_registers {
	unsigned _readenable: 1;
	unsigned _writeenable: 1;
	unsigned : 6;
	unsigned _mmiobase_39_16: 24;
};
#endif
#undef H2S_CSR_F1_MMIO_LIMIT_ADDRESS_REGISTERS
#define H2S_CSR_F1_MMIO_LIMIT_ADDRESS_REGISTERS (0x54)
#ifndef __ASSEMBLER__
struct dnc_csr_f1_mmio_limit_address_registers {
	unsigned _dstnode: 3;
	unsigned : 4;
	unsigned _nonposted: 1;
	unsigned _mmiolimit_39_16: 24;
};
#endif
#undef H2S_CSR_F1_EXT_D_MMIO_ADDRESS_BASE_REGISTERS
#define H2S_CSR_F1_EXT_D_MMIO_ADDRESS_BASE_REGISTERS (0x58)
#ifndef __ASSEMBLER__
struct dnc_csr_f1_ext_d_mmio_address_base_registers {
	unsigned _dstnode: 3;
	unsigned : 2;
	unsigned _hypertransport: 1;
	unsigned : 2;
	unsigned _mmiomapbase_20_0: 21;
	unsigned _address_39_36: 1;
	unsigned _address_43_40: 1;
	unsigned _address_47_44: 1;
};
#endif
#undef H2S_CSR_F1_EXT_D_MMIO_ADDRESS_MASK_REGISTERS
#define H2S_CSR_F1_EXT_D_MMIO_ADDRESS_MASK_REGISTERS (0x5C)
#ifndef __ASSEMBLER__
struct dnc_csr_f1_ext_d_mmio_address_mask_registers {
	unsigned _mmiomapen: 1;
	unsigned : 7;
	unsigned _mmiomapmask_20_0: 21;
	unsigned : 3;
};
#endif
#undef H2S_CSR_F1_IO_SPACE_BASE_ADDRESS_REGISTERS
#define H2S_CSR_F1_IO_SPACE_BASE_ADDRESS_REGISTERS (0x60)
#ifndef __ASSEMBLER__
struct dnc_csr_f1_io_space_base_address_registers {
	unsigned _readenable: 1;
	unsigned _writeenable: 1;
	unsigned _vgaenable: 1;
	unsigned _isaenable: 1;
	unsigned : 8;
	unsigned _iobase_24_12: 13;
	unsigned : 7;
};
#endif
#undef H2S_CSR_F1_IO_SPACE_LIMIT_ADDRESS_REGISTERS
#define H2S_CSR_F1_IO_SPACE_LIMIT_ADDRESS_REGISTERS (0x64)
#ifndef __ASSEMBLER__
struct dnc_csr_f1_io_space_limit_address_registers {
	unsigned _dstnode: 3;
	unsigned : 9;
	unsigned _iolimit_24_12: 13;
	unsigned : 7;
};
#endif
#undef H2S_CSR_F1_CONFIGURATION_MAP_REGISTERS
#define H2S_CSR_F1_CONFIGURATION_MAP_REGISTERS (0x68)
#ifndef __ASSEMBLER__
struct dnc_csr_f1_configuration_map_registers {
	unsigned _readenable: 1;
	unsigned _writeenable: 1;
	unsigned _devcmpen: 1;
	unsigned : 1;
	unsigned _dstnode: 3;
	unsigned : 9;
	unsigned _busnumbase_7_0: 8;
	unsigned _busnumlimit_7_0: 8;
};
#endif
#undef H2S_CSR_G0_PHYXA_LINK_STAT
#define H2S_CSR_G0_PHYXA_LINK_STAT (0xA00+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_phyxa_link_stat {
	unsigned _linkup: 1;
	unsigned _ts1ok: 4;
	unsigned _symbalign: 4;
	unsigned _rxsigdet: 4;
	unsigned _remctr: 3;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_PHYXA_LINK_CTR
#define H2S_CSR_G0_PHYXA_LINK_CTR (0xA04+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_phyxa_link_ctr {
	unsigned _genremctr: 3;
	unsigned _sci_loopback: 1;
	unsigned _wb_source_sel: 2;
	unsigned _reset_lc3core: 1;
	unsigned _reset_hss: 1;
	unsigned _hss_rxsigdet_bypass: 1;
	unsigned _wb_rcv_trx_sel: 1;
	unsigned : 22;
};
#endif
#undef H2S_CSR_G0_PHYXA_ELOG
#define H2S_CSR_G0_PHYXA_ELOG (0xA08+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_phyxa_elog {
	unsigned _10to8_dec_err: 4;
	unsigned _clk_cmp_buf_err: 4;
	unsigned _flg_code_err: 1;
	unsigned _sync_pt_timout: 1;
	unsigned : 22;
};
#endif
#undef H2S_CSR_G0_HSSXA_CTR_1
#define H2S_CSR_G0_HSSXA_CTR_1 (0xA0C+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssxa_ctr_1 {
	unsigned _fddatawrapd: 4;
	unsigned _rxeqd_1_0: 8;
	unsigned _txdrvmoded: 4;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSXA_CTR_2
#define H2S_CSR_G0_HSSXA_CTR_2 (0xA10+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssxa_ctr_2 {
	unsigned _rxphsdnd: 4;
	unsigned _rxphslockd: 4;
	unsigned _rxphsupd: 4;
	unsigned : 20;
};
#endif
#undef H2S_CSR_G0_HSSXA_CTR_3
#define H2S_CSR_G0_HSSXA_CTR_3 (0xA14+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssxa_ctr_3 {
	unsigned _txcad_3_0: 16;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSXA_CTR_4
#define H2S_CSR_G0_HSSXA_CTR_4 (0xA18+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssxa_ctr_4 {
	unsigned _txcbd_3_0: 16;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSXA_CTR_5
#define H2S_CSR_G0_HSSXA_CTR_5 (0xA1C+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssxa_ctr_5 {
	unsigned _txdrvobsd: 4;
	unsigned _txdrvpwrd_2_0: 12;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSXA_CTR_6
#define H2S_CSR_G0_HSSXA_CTR_6 (0xA20+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssxa_ctr_6 {
	unsigned _txdrvsignd: 4;
	unsigned _txdrvslewd_1_0: 8;
	unsigned _txpwrdwnd: 4;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSXA_CTR_7
#define H2S_CSR_G0_HSSXA_CTR_7 (0xA24+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssxa_ctr_7 {
	unsigned _receiver: 4;
	unsigned : 4;
	unsigned : 4;
	unsigned : 20;
};
#endif
#undef H2S_CSR_G0_HSSXA_CTR_8
#define H2S_CSR_G0_HSSXA_CTR_8 (0xA28+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssxa_ctr_8 {
	unsigned _transmitter: 4;
	unsigned : 4;
	unsigned : 24;
};
#endif
#undef H2S_CSR_G0_HSSXA_CTR_9
#define H2S_CSR_G0_HSSXA_CTR_9 (0xA2C+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssxa_ctr_9 {
	unsigned _pro: 4;
	unsigned : 8;
	unsigned : 4;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSXA_STAT_1
#define H2S_CSR_G0_HSSXA_STAT_1 (0xA30+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssxa_stat_1 {
	unsigned _receive: 4;
	unsigned _transmitter: 4;
	unsigned _hss: 1;
	unsigned : 23;
};
#endif
#undef H2S_CSR_G0_PHYXB_LINK_STAT
#define H2S_CSR_G0_PHYXB_LINK_STAT (0xA40+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_phyxb_link_stat {
	unsigned _linkup: 1;
	unsigned _ts1ok: 4;
	unsigned _symbalign: 4;
	unsigned _rxsigdet: 4;
	unsigned _remctr: 3;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_PHYXB_LINK_CTR
#define H2S_CSR_G0_PHYXB_LINK_CTR (0xA44+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_phyxb_link_ctr {
	unsigned _genremctr: 3;
	unsigned _sci_loopback: 1;
	unsigned _wb_source_sel: 2;
	unsigned _reset_lc3core: 1;
	unsigned _reset_hss: 1;
	unsigned _hss_rxsigdet_bypass: 1;
	unsigned _wb_rcv_trx_sel: 1;
	unsigned : 22;
};
#endif
#undef H2S_CSR_G0_PHYXB_ELOG
#define H2S_CSR_G0_PHYXB_ELOG (0xA48+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_phyxb_elog {
	unsigned _10to8_dec_err: 4;
	unsigned _clk_cmp_buf_err: 4;
	unsigned _flg_code_err: 1;
	unsigned _sync_pt_timout: 1;
	unsigned : 22;
};
#endif
#undef H2S_CSR_G0_HSSXB_CTR_1
#define H2S_CSR_G0_HSSXB_CTR_1 (0xA4C+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssxb_ctr_1 {
	unsigned _fddatawrapd: 4;
	unsigned _rxeqd_1_0: 8;
	unsigned _txdrvmoded: 4;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSXB_CTR_2
#define H2S_CSR_G0_HSSXB_CTR_2 (0xA50+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssxb_ctr_2 {
	unsigned _rxphsdnd: 4;
	unsigned _rxphslockd: 4;
	unsigned _rxphsupd: 4;
	unsigned : 20;
};
#endif
#undef H2S_CSR_G0_HSSXB_CTR_3
#define H2S_CSR_G0_HSSXB_CTR_3 (0xA54+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssxb_ctr_3 {
	unsigned _txcad_3_0: 16;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSXB_CTR_4
#define H2S_CSR_G0_HSSXB_CTR_4 (0xA58+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssxb_ctr_4 {
	unsigned _txcbd_3_0: 16;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSXB_CTR_5
#define H2S_CSR_G0_HSSXB_CTR_5 (0xA5C+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssxb_ctr_5 {
	unsigned _txdrvobsd: 4;
	unsigned _txdrvpwrd_2_0: 12;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSXB_CTR_6
#define H2S_CSR_G0_HSSXB_CTR_6 (0xA60+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssxb_ctr_6 {
	unsigned _txdrvsignd: 4;
	unsigned _txdrvslewd_1_0: 8;
	unsigned _txpwrdwnd: 4;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSXB_CTR_7
#define H2S_CSR_G0_HSSXB_CTR_7 (0xA64+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssxb_ctr_7 {
	unsigned _receiver: 4;
	unsigned : 4;
	unsigned : 4;
	unsigned : 20;
};
#endif
#undef H2S_CSR_G0_HSSXB_CTR_8
#define H2S_CSR_G0_HSSXB_CTR_8 (0xA68+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssxb_ctr_8 {
	unsigned _transmitter: 4;
	unsigned : 4;
	unsigned : 24;
};
#endif
#undef H2S_CSR_G0_HSSXB_CTR_9
#define H2S_CSR_G0_HSSXB_CTR_9 (0xA6C+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssxb_ctr_9 {
	unsigned _pro: 4;
	unsigned : 8;
	unsigned : 4;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSXB_STAT_1
#define H2S_CSR_G0_HSSXB_STAT_1 (0xA70+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssxb_stat_1 {
	unsigned _receive: 4;
	unsigned _transmitter: 4;
	unsigned _hss: 1;
	unsigned : 23;
};
#endif
#undef H2S_CSR_G0_PHYYA_LINK_STAT
#define H2S_CSR_G0_PHYYA_LINK_STAT (0xA80+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_phyya_link_stat {
	unsigned _linkup: 1;
	unsigned _ts1ok: 4;
	unsigned _symbalign: 4;
	unsigned _rxsigdet: 4;
	unsigned _remctr: 3;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_PHYYA_LINK_CTR
#define H2S_CSR_G0_PHYYA_LINK_CTR (0xA84+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_phyya_link_ctr {
	unsigned _genremctr: 3;
	unsigned _sci_loopback: 1;
	unsigned _wb_source_sel: 2;
	unsigned _reset_lc3core: 1;
	unsigned _reset_hss: 1;
	unsigned _hss_rxsigdet_bypass: 1;
	unsigned _wb_rcv_trx_sel: 1;
	unsigned : 22;
};
#endif
#undef H2S_CSR_G0_PHYYA_ELOG
#define H2S_CSR_G0_PHYYA_ELOG (0xA88+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_phyya_elog {
	unsigned _10to8_dec_err: 4;
	unsigned _clk_cmp_buf_err: 4;
	unsigned _flg_code_err: 1;
	unsigned _sync_pt_timout: 1;
	unsigned : 22;
};
#endif
#undef H2S_CSR_G0_HSSYA_CTR_1
#define H2S_CSR_G0_HSSYA_CTR_1 (0xA8C+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssya_ctr_1 {
	unsigned _fddatawrapd: 4;
	unsigned _rxeqd_1_0: 8;
	unsigned _txdrvmoded: 4;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSYA_CTR_2
#define H2S_CSR_G0_HSSYA_CTR_2 (0xA90+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssya_ctr_2 {
	unsigned _rxphsdnd: 4;
	unsigned _rxphslockd: 4;
	unsigned _rxphsupd: 4;
	unsigned : 20;
};
#endif
#undef H2S_CSR_G0_HSSYA_CTR_3
#define H2S_CSR_G0_HSSYA_CTR_3 (0xA94+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssya_ctr_3 {
	unsigned _txcad_3_0: 16;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSYA_CTR_4
#define H2S_CSR_G0_HSSYA_CTR_4 (0xA98+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssya_ctr_4 {
	unsigned _txcbd_3_0: 16;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSYA_CTR_5
#define H2S_CSR_G0_HSSYA_CTR_5 (0xA9C+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssya_ctr_5 {
	unsigned _txdrvobsd: 4;
	unsigned _txdrvpwrd_2_0: 12;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSYA_CTR_6
#define H2S_CSR_G0_HSSYA_CTR_6 (0xAA0+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssya_ctr_6 {
	unsigned _txdrvsignd: 4;
	unsigned _txdrvslewd_1_0: 8;
	unsigned _txpwrdwnd: 4;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSYA_CTR_7
#define H2S_CSR_G0_HSSYA_CTR_7 (0xAA4+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssya_ctr_7 {
	unsigned _receiver: 4;
	unsigned : 4;
	unsigned : 4;
	unsigned : 20;
};
#endif
#undef H2S_CSR_G0_HSSYA_CTR_8
#define H2S_CSR_G0_HSSYA_CTR_8 (0xAA8+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssya_ctr_8 {
	unsigned _transmitter: 4;
	unsigned : 4;
	unsigned : 24;
};
#endif
#undef H2S_CSR_G0_HSSYA_CTR_9
#define H2S_CSR_G0_HSSYA_CTR_9 (0xAAC+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssya_ctr_9 {
	unsigned _pro: 4;
	unsigned : 8;
	unsigned : 4;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSYA_STAT_1
#define H2S_CSR_G0_HSSYA_STAT_1 (0xAB0+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssya_stat_1 {
	unsigned _receive: 4;
	unsigned _transmitter: 4;
	unsigned _hss: 1;
	unsigned : 23;
};
#endif
#undef H2S_CSR_G0_PHYYB_LINK_STAT
#define H2S_CSR_G0_PHYYB_LINK_STAT (0xAC0+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_phyyb_link_stat {
	unsigned _linkup: 1;
	unsigned _ts1ok: 4;
	unsigned _symbalign: 4;
	unsigned _rxsigdet: 4;
	unsigned _remctr: 3;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_PHYYB_LINK_CTR
#define H2S_CSR_G0_PHYYB_LINK_CTR (0xAC4+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_phyyb_link_ctr {
	unsigned _genremctr: 3;
	unsigned _sci_loopback: 1;
	unsigned _wb_source_sel: 2;
	unsigned _reset_lc3core: 1;
	unsigned _reset_hss: 1;
	unsigned _hss_rxsigdet_bypass: 1;
	unsigned _wb_rcv_trx_sel: 1;
	unsigned : 22;
};
#endif
#undef H2S_CSR_G0_PHYYB_ELOG
#define H2S_CSR_G0_PHYYB_ELOG (0xAC8+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_phyyb_elog {
	unsigned _10to8_dec_err: 4;
	unsigned _clk_cmp_buf_err: 4;
	unsigned _flg_code_err: 1;
	unsigned _sync_pt_timout: 1;
	unsigned : 22;
};
#endif
#undef H2S_CSR_G0_HSSYB_CTR_1
#define H2S_CSR_G0_HSSYB_CTR_1 (0xACC+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssyb_ctr_1 {
	unsigned _fddatawrapd: 4;
	unsigned _rxeqd_1_0: 8;
	unsigned _txdrvmoded: 4;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSYB_CTR_2
#define H2S_CSR_G0_HSSYB_CTR_2 (0xAD0+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssyb_ctr_2 {
	unsigned _rxphsdnd: 4;
	unsigned _rxphslockd: 4;
	unsigned _rxphsupd: 4;
	unsigned : 20;
};
#endif
#undef H2S_CSR_G0_HSSYB_CTR_3
#define H2S_CSR_G0_HSSYB_CTR_3 (0xAD4+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssyb_ctr_3 {
	unsigned _txcad_3_0: 16;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSYB_CTR_4
#define H2S_CSR_G0_HSSYB_CTR_4 (0xAD8+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssyb_ctr_4 {
	unsigned _txcbd_3_0: 16;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSYB_CTR_5
#define H2S_CSR_G0_HSSYB_CTR_5 (0xADC+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssyb_ctr_5 {
	unsigned _txdrvobsd: 4;
	unsigned _txdrvpwrd_2_0: 12;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSYB_CTR_6
#define H2S_CSR_G0_HSSYB_CTR_6 (0xAE0+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssyb_ctr_6 {
	unsigned _txdrvsignd: 4;
	unsigned _txdrvslewd_1_0: 8;
	unsigned _txpwrdwnd: 4;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSYB_CTR_7
#define H2S_CSR_G0_HSSYB_CTR_7 (0xAE4+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssyb_ctr_7 {
	unsigned _receiver: 4;
	unsigned : 4;
	unsigned : 4;
	unsigned : 20;
};
#endif
#undef H2S_CSR_G0_HSSYB_CTR_8
#define H2S_CSR_G0_HSSYB_CTR_8 (0xAE8+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssyb_ctr_8 {
	unsigned _transmitter: 4;
	unsigned : 4;
	unsigned : 24;
};
#endif
#undef H2S_CSR_G0_HSSYB_CTR_9
#define H2S_CSR_G0_HSSYB_CTR_9 (0xAEC+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssyb_ctr_9 {
	unsigned _pro: 4;
	unsigned : 8;
	unsigned : 4;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSYB_STAT_1
#define H2S_CSR_G0_HSSYB_STAT_1 (0xAF0+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssyb_stat_1 {
	unsigned _receive: 4;
	unsigned _transmitter: 4;
	unsigned _hss: 1;
	unsigned : 23;
};
#endif
#undef H2S_CSR_G0_PHYZA_LINK_STAT
#define H2S_CSR_G0_PHYZA_LINK_STAT (0xB00+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_phyza_link_stat {
	unsigned _linkup: 1;
	unsigned _ts1ok: 4;
	unsigned _symbalign: 4;
	unsigned _rxsigdet: 4;
	unsigned _remctr: 3;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_PHYZA_LINK_CTR
#define H2S_CSR_G0_PHYZA_LINK_CTR (0xB04+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_phyza_link_ctr {
	unsigned _genremctr: 3;
	unsigned _sci_loopback: 1;
	unsigned _wb_source_sel: 2;
	unsigned _reset_lc3core: 1;
	unsigned _reset_hss: 1;
	unsigned _hss_rxsigdet_bypass: 1;
	unsigned _wb_rcv_trx_sel: 1;
	unsigned : 22;
};
#endif
#undef H2S_CSR_G0_PHYZA_ELOG
#define H2S_CSR_G0_PHYZA_ELOG (0xB08+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_phyza_elog {
	unsigned _10to8_dec_err: 4;
	unsigned _clk_cmp_buf_err: 4;
	unsigned _flg_code_err: 1;
	unsigned _sync_pt_timout: 1;
	unsigned : 22;
};
#endif
#undef H2S_CSR_G0_HSSZA_CTR_1
#define H2S_CSR_G0_HSSZA_CTR_1 (0xB0C+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssza_ctr_1 {
	unsigned _fddatawrapd: 4;
	unsigned _rxeqd_1_0: 8;
	unsigned _txdrvmoded: 4;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSZA_CTR_2
#define H2S_CSR_G0_HSSZA_CTR_2 (0xB10+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssza_ctr_2 {
	unsigned _rxphsdnd: 4;
	unsigned _rxphslockd: 4;
	unsigned _rxphsupd: 4;
	unsigned : 20;
};
#endif
#undef H2S_CSR_G0_HSSZA_CTR_3
#define H2S_CSR_G0_HSSZA_CTR_3 (0xB14+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssza_ctr_3 {
	unsigned _txcad_3_0: 16;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSZA_CTR_4
#define H2S_CSR_G0_HSSZA_CTR_4 (0xB18+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssza_ctr_4 {
	unsigned _txcbd_3_0: 16;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSZA_CTR_5
#define H2S_CSR_G0_HSSZA_CTR_5 (0xB1C+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssza_ctr_5 {
	unsigned _txdrvobsd: 4;
	unsigned _txdrvpwrd_2_0: 12;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSZA_CTR_6
#define H2S_CSR_G0_HSSZA_CTR_6 (0xB20+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssza_ctr_6 {
	unsigned _txdrvsignd: 4;
	unsigned _txdrvslewd_1_0: 8;
	unsigned _txpwrdwnd: 4;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSZA_CTR_7
#define H2S_CSR_G0_HSSZA_CTR_7 (0xB24+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssza_ctr_7 {
	unsigned _receiver: 4;
	unsigned : 4;
	unsigned : 4;
	unsigned : 20;
};
#endif
#undef H2S_CSR_G0_HSSZA_CTR_8
#define H2S_CSR_G0_HSSZA_CTR_8 (0xB28+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssza_ctr_8 {
	unsigned _transmitter: 4;
	unsigned : 4;
	unsigned : 24;
};
#endif
#undef H2S_CSR_G0_HSSZA_CTR_9
#define H2S_CSR_G0_HSSZA_CTR_9 (0xB2C+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssza_ctr_9 {
	unsigned _pro: 4;
	unsigned : 8;
	unsigned : 4;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSZA_STAT_1
#define H2S_CSR_G0_HSSZA_STAT_1 (0xB30+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hssza_stat_1 {
	unsigned _receive: 4;
	unsigned _transmitter: 4;
	unsigned _hss: 1;
	unsigned : 23;
};
#endif
#undef H2S_CSR_G0_PHYZB_LINK_STAT
#define H2S_CSR_G0_PHYZB_LINK_STAT (0xB40+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_phyzb_link_stat {
	unsigned _linkup: 1;
	unsigned _ts1ok: 4;
	unsigned _symbalign: 4;
	unsigned _rxsigdet: 4;
	unsigned _remctr: 3;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_PHYZB_LINK_CTR
#define H2S_CSR_G0_PHYZB_LINK_CTR (0xB44+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_phyzb_link_ctr {
	unsigned _genremctr: 3;
	unsigned _sci_loopback: 1;
	unsigned _wb_source_sel: 2;
	unsigned _reset_lc3core: 1;
	unsigned _reset_hss: 1;
	unsigned _hss_rxsigdet_bypass: 1;
	unsigned _wb_rcv_trx_sel: 1;
	unsigned : 22;
};
#endif
#undef H2S_CSR_G0_PHYZB_ELOG
#define H2S_CSR_G0_PHYZB_ELOG (0xB48+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_phyzb_elog {
	unsigned _10to8_dec_err: 4;
	unsigned _clk_cmp_buf_err: 4;
	unsigned _flg_code_err: 1;
	unsigned _sync_pt_timout: 1;
	unsigned : 22;
};
#endif
#undef H2S_CSR_G0_HSSZB_CTR_1
#define H2S_CSR_G0_HSSZB_CTR_1 (0xB4C+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hsszb_ctr_1 {
	unsigned _fddatawrapd: 4;
	unsigned _rxeqd_1_0: 8;
	unsigned _txdrvmoded: 4;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSZB_CTR_2
#define H2S_CSR_G0_HSSZB_CTR_2 (0xB50+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hsszb_ctr_2 {
	unsigned _rxphsdnd: 4;
	unsigned _rxphslockd: 4;
	unsigned _rxphsupd: 4;
	unsigned : 20;
};
#endif
#undef H2S_CSR_G0_HSSZB_CTR_3
#define H2S_CSR_G0_HSSZB_CTR_3 (0xB54+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hsszb_ctr_3 {
	unsigned _txcad_3_0: 16;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSZB_CTR_4
#define H2S_CSR_G0_HSSZB_CTR_4 (0xB58+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hsszb_ctr_4 {
	unsigned _txcbd_3_0: 16;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSZB_CTR_5
#define H2S_CSR_G0_HSSZB_CTR_5 (0xB5C+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hsszb_ctr_5 {
	unsigned _txdrvobsd: 4;
	unsigned _txdrvpwrd_2_0: 12;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSZB_CTR_6
#define H2S_CSR_G0_HSSZB_CTR_6 (0xB60+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hsszb_ctr_6 {
	unsigned _txdrvsignd: 4;
	unsigned _txdrvslewd_1_0: 8;
	unsigned _txpwrdwnd: 4;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSZB_CTR_7
#define H2S_CSR_G0_HSSZB_CTR_7 (0xB64+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hsszb_ctr_7 {
	unsigned _receiver: 4;
	unsigned : 4;
	unsigned : 4;
	unsigned : 20;
};
#endif
#undef H2S_CSR_G0_HSSZB_CTR_8
#define H2S_CSR_G0_HSSZB_CTR_8 (0xB68+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hsszb_ctr_8 {
	unsigned _transmitter: 4;
	unsigned : 4;
	unsigned : 24;
};
#endif
#undef H2S_CSR_G0_HSSZB_CTR_9
#define H2S_CSR_G0_HSSZB_CTR_9 (0xB6C+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hsszb_ctr_9 {
	unsigned _pro: 4;
	unsigned : 8;
	unsigned : 4;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G0_HSSZB_STAT_1
#define H2S_CSR_G0_HSSZB_STAT_1 (0xB70+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hsszb_stat_1 {
	unsigned _receive: 4;
	unsigned _transmitter: 4;
	unsigned _hss: 1;
	unsigned : 23;
};
#endif
#undef H2S_CSR_G0_HSSPLL_CTR_1
#define H2S_CSR_G0_HSSPLL_CTR_1 (0xB80+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hsspll_ctr_1 {
	unsigned : 14;
	unsigned : 18;
};
#endif
#undef H2S_CSR_G0_HSSPLL_CTR_2
#define H2S_CSR_G0_HSSPLL_CTR_2 (0xB84+(0<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g0_hsspll_ctr_2 {
	unsigned : 2;
	unsigned _the: 6;
	unsigned _address: 3;
	unsigned : 1;
	unsigned : 21;
};
#endif
#undef H2S_CSR_G2_F_PREF_MODE
#define H2S_CSR_G2_F_PREF_MODE (0x800+(2<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g2_f_pref_mode {
	unsigned _disable_textbf_0_prefetch_buffer_enabled_textbf_1: 1;
	unsigned _clear_textbf_0_normal_mode_textbf_1: 1;
	unsigned : 6;
	unsigned : 24;
};
#endif
#undef H2S_CSR_G2_M_PREF_MODE
#define H2S_CSR_G2_M_PREF_MODE (0x804+(2<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g2_m_pref_mode {
	unsigned _disable_textbf_0_prefetch_buffer_enabled_textbf_1: 1;
	unsigned _clear_textbf_0_normal_mode_textbf_1: 1;
	unsigned : 6;
	unsigned : 24;
};
#endif
#undef H2S_CSR_G2_SRAM_MODE
#define H2S_CSR_G2_SRAM_MODE (0x808+(2<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g2_sram_mode {
	unsigned _disable_textbf_0_sram_enabled_textbf_1: 1;
	unsigned _clear: 1;
	unsigned _break_textbf_0_normal_mode_textbf_1: 1;
	unsigned _init_check_mode_textbf_0_init_mode_textbf_1: 1;
	unsigned : 4;
	unsigned : 24;
};
#endif
#undef H2S_CSR_G2_DIAG_SRAM_ADDR
#define H2S_CSR_G2_DIAG_SRAM_ADDR (0x80C+(2<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g2_diag_sram_addr {
	unsigned _addressed: 22;
	unsigned : 10;
};
#endif
#undef H2S_CSR_G2_WR_DATA
#define H2S_CSR_G2_WR_DATA (0x810+(2<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g2_wr_data {
	unsigned _with: 32;
};
#endif
#undef H2S_CSR_G2_SRAM_DATA
#define H2S_CSR_G2_SRAM_DATA (0x814+(2<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g2_sram_data {
	unsigned _content: 32;
};
#endif
#undef H2S_CSR_G2_SRAM_LIMIT
#define H2S_CSR_G2_SRAM_LIMIT (0x818+(2<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g2_sram_limit {
	unsigned _sram: 22;
	unsigned : 10;
};
#endif
#undef H2S_CSR_G2_FTAG_STATUS
#define H2S_CSR_G2_FTAG_STATUS (0x81C+(2<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g2_ftag_status {
	unsigned _status_of_sram_init_sequence_textbf_0_init_sequence_inactive_textbf_1: 1;
	unsigned _result_of_sram_check_sequence_textbf_0_sram_check_passed_without_errors_textbf_1: 1;
	unsigned : 30;
};
#endif
#undef H2S_CSR_G2_DDL_CTRL
#define H2S_CSR_G2_DDL_CTRL (0x820+(2<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g2_ddl_ctrl {
	unsigned _2_s: 8;
	unsigned : 8;
	unsigned : 8;
	unsigned _stop: 1;
	unsigned _bypass: 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned _clear: 1;
	unsigned : 1;
};
#endif
#undef H2S_CSR_G2_DDL_STATUS
#define H2S_CSR_G2_DDL_STATUS (0x824+(2<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g2_ddl_status {
	unsigned _calculated: 8;
	unsigned : 8;
	unsigned : 8;
	unsigned _delayout: 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned _overflow: 1;
	unsigned : 1;
};
#endif
#undef H2S_CSR_G2_SRAM_PAD_CTRL
#define H2S_CSR_G2_SRAM_PAD_CTRL (0x828+(2<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g2_sram_pad_ctrl {
	unsigned _pe_sram_k: 1;
	unsigned _pe_sram_ctrl: 1;
	unsigned _pe_sram_assr: 1;
	unsigned _pe_sram_d: 1;
	unsigned _sram_mcdhalf_k: 1;
	unsigned _sram_mcdhalf_ctrl: 1;
	unsigned _sram_mcdhalf_addr: 1;
	unsigned _sram_mcdhalf_d: 1;
	unsigned _sram_mctt1_cq: 1;
	unsigned _sram_mctt0_cq: 1;
	unsigned _sram_mctt1_q: 1;
	unsigned _sram_mctt0_q: 1;
	unsigned : 4;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G3_APIC_MAP_SHIFT
#define H2S_CSR_G3_APIC_MAP_SHIFT (0x008+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_apic_map_shift {
	unsigned _shift_bits_shift_bits: 2;
	unsigned : 1;
	unsigned : 29;
};
#endif
#undef H2S_CSR_G3_CSR_BASE_ADDRESS
#define H2S_CSR_G3_CSR_BASE_ADDRESS (0x00C+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_csr_base_address {
	unsigned _bits: 16;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G3_MMCFG_BASE
#define H2S_CSR_G3_MMCFG_BASE (0x010+(3<<12))
#undef H2S_CSR_G3_MMCFG_CONTROL
#define H2S_CSR_G3_MMCFG_CONTROL (0x014+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_mmcfg_control {
	unsigned _number_limit_number_of_bits: 4;
	unsigned _dnc_node_limit_number: 4;
	unsigned : 24;
};
#endif
#undef H2S_CSR_G3_PCI_SEG0
#define H2S_CSR_G3_PCI_SEG0 (0x018+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_pci_seg0 {
	unsigned _pci_seg0_is_local: 1;
	unsigned : 15;
	unsigned _pci_seg0_node_sci_id: 16;
};
#endif
#undef H2S_CSR_G3_DRAM_SHARED_BASE
#define H2S_CSR_G3_DRAM_SHARED_BASE (0x01C+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_dram_shared_base {
	unsigned _address_base_47_24: 24;
	unsigned : 8;
};
#endif
#undef H2S_CSR_G3_DRAM_SHARED_LIMIT
#define H2S_CSR_G3_DRAM_SHARED_LIMIT (0x020+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_dram_shared_limit {
	unsigned _address_base_47_24_an_access_terminated_at_a_given_system_is_considered_to_be_coherently_distributed_over_the_dnc_fabric_if_the_address_accessed_satisfies_textbf_address_47_24_dram_shared_base_23_0_address_47_24_dram_shared_limit_23_0: 24;
	unsigned : 8;
};
#endif
#undef H2S_CSR_G3_HT_NODEID
#define H2S_CSR_G3_HT_NODEID (0x024+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_ht_nodeid {
	unsigned _hypertransport: 3;
	unsigned : 29;
};
#endif
#undef H2S_CSR_G3_CSR_HTX_RESET
#define H2S_CSR_G3_CSR_HTX_RESET (0x028+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_csr_htx_reset {
	unsigned : 1;
	unsigned : 31;
};
#endif
#undef H2S_CSR_G3_FAB_CONTROL
#define H2S_CSR_G3_FAB_CONTROL (0x02C+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_fab_control {
	unsigned : 30;
	unsigned _is_ready: 1;
	unsigned _go_ahead: 1;
};
#endif
#undef H2S_CSR_G3_EXT_INTERRUPT_GEN
#define H2S_CSR_G3_EXT_INTERRUPT_GEN (0x030+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_ext_interrupt_gen {
	unsigned _vector: 8;
	unsigned _msgtype: 3;
	unsigned _index: 5;
	unsigned _destination_apic_id: 16;
};
#endif
#undef H2S_CSR_G3_EXT_INTERRUPT_STATUS
#define H2S_CSR_G3_EXT_INTERRUPT_STATUS (0x034+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_ext_interrupt_status {
	unsigned _result: 32;
};
#endif
#undef H2S_CSR_G3_EXT_INTERRUPT_DEST
#define H2S_CSR_G3_EXT_INTERRUPT_DEST (0x038+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_ext_interrupt_dest {
	unsigned _interrupt: 8;
	unsigned : 24;
};
#endif
#undef H2S_CSR_G3_HREQ_CTRL
#define H2S_CSR_G3_HREQ_CTRL (0x400+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_hreq_ctrl {
	unsigned _disable: 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned _strategy: 2;
	unsigned _update: 1;
	unsigned : 1;
	unsigned : 1;
	unsigned _send: 1;
	unsigned _h2s: 1;
	unsigned _delay: 3;
	unsigned _enable: 1;
	unsigned _value: 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 6;
	unsigned _cache: 2;
	unsigned _tracer: 2;
	unsigned : 2;
};
#endif
#undef H2S_CSR_G3_HPRB_CTRL
#define H2S_CSR_G3_HPRB_CTRL (0x404+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_hprb_ctrl {
	unsigned : 1;
	unsigned _disable: 1;
	unsigned : 2;
	unsigned _gsm: 12;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
};
#endif
#undef H2S_CSR_G3_SREQ_CTRL
#define H2S_CSR_G3_SREQ_CTRL (0x408+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_sreq_ctrl {
	unsigned : 4;
	unsigned _gsm: 12;
	unsigned _disable: 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned _watchbus_select: 4;
	unsigned : 4;
};
#endif
#undef H2S_CSR_G3_SPRB_CTRL
#define H2S_CSR_G3_SPRB_CTRL (0x40C+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_sprb_ctrl {
	unsigned : 32;
};
#endif
#undef H2S_CSR_G3_H2S_CTRL
#define H2S_CSR_G3_H2S_CTRL (0x410+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_h2s_ctrl {
	unsigned _0: 1;
	unsigned : 31;
};
#endif
#undef H2S_CSR_G3_TRANS_REC_CORE
#define H2S_CSR_G3_TRANS_REC_CORE (0x418+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_trans_rec_core {
	unsigned _the: 8;
	unsigned : 8;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G3_SPI_INSTRUCTION_AND_STATUS
#define H2S_CSR_G3_SPI_INSTRUCTION_AND_STATUS (0x41C+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_spi_instruction_and_status {
	unsigned _instruction: 8;
	unsigned _the: 1;
	unsigned : 9;
	unsigned _write: 14;
};
#endif
#undef H2S_CSR_G3_EEPROM_ADDR_IIC_SMBUS_AND_AND_UID
#define H2S_CSR_G3_EEPROM_ADDR_IIC_SMBUS_AND_AND_UID (0x420+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_eeprom_addr_iic_smbus_and_and_uid {
	unsigned _seprom: 16;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G3_ERROR_STATUS
#define H2S_CSR_G3_ERROR_STATUS (0x424+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_error_status {
	unsigned _cdata: 1;
	unsigned : 1;
	unsigned _mc: 1;
	unsigned : 1;
	unsigned : 28;
};
#endif
#undef H2S_CSR_G3_CONTROLLER_IIC
#define H2S_CSR_G3_CONTROLLER_IIC (0x428+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_controller_iic {
	unsigned _iic: 1;
	unsigned : 31;
};
#endif
#undef H2S_CSR_G3_CONTROLLER_IIC
#define H2S_CSR_G3_CONTROLLER_IIC (0x428+(3<<12))
#undef H2S_CSR_G3_SPI_READ_WRITE_DATA
#define H2S_CSR_G3_SPI_READ_WRITE_DATA (0x42C+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_spi_read_write_data {
	unsigned _31_0: 32;
};
#endif
#undef H2S_CSR_G3_EEPROM_ADDR_HSS_PLL_AND_HSS_DATA
#define H2S_CSR_G3_EEPROM_ADDR_HSS_PLL_AND_HSS_DATA (0x430+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_eeprom_addr_hss_pll_and_hss_data {
	unsigned _seprom: 16;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G3_THE_INITIALIZATION_OF_THE_IBM_RAMS
#define H2S_CSR_G3_THE_INITIALIZATION_OF_THE_IBM_RAMS (0x7F8+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_the_initialization_of_the_ibm_rams {
	unsigned _0_the_initialisation_of_the_ibm_rams_of_the_whole_dnc_about_10_ms_textbf_1: 1;
	unsigned : 31;
};
#endif
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT
#define H2S_CSR_G3_NC_ATT_MAP_SELECT (0x7FC+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_nc_att_map_select {
	unsigned _upper_address_bits: 4;
	unsigned _select_ram: 4;
	unsigned : 24;
};
#endif
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_0
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_0 (0x800+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_1
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_1 (0x804+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_2
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_2 (0x808+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_3
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_3 (0x80c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_4
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_4 (0x810+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_5
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_5 (0x814+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_6
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_6 (0x818+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_7
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_7 (0x81c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_8
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_8 (0x820+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_9
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_9 (0x824+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_10
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_10 (0x828+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_11
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_11 (0x82c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_12
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_12 (0x830+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_13
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_13 (0x834+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_14
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_14 (0x838+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_15
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_15 (0x83c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_16
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_16 (0x840+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_17
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_17 (0x844+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_18
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_18 (0x848+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_19
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_19 (0x84c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_20
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_20 (0x850+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_21
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_21 (0x854+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_22
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_22 (0x858+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_23
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_23 (0x85c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_24
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_24 (0x860+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_25
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_25 (0x864+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_26
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_26 (0x868+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_27
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_27 (0x86c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_28
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_28 (0x870+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_29
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_29 (0x874+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_30
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_30 (0x878+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_31
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_31 (0x87c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_32
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_32 (0x880+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_33
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_33 (0x884+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_34
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_34 (0x888+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_35
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_35 (0x88c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_36
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_36 (0x890+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_37
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_37 (0x894+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_38
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_38 (0x898+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_39
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_39 (0x89c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_40
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_40 (0x8a0+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_41
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_41 (0x8a4+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_42
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_42 (0x8a8+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_43
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_43 (0x8ac+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_44
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_44 (0x8b0+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_45
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_45 (0x8b4+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_46
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_46 (0x8b8+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_47
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_47 (0x8bc+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_48
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_48 (0x8c0+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_49
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_49 (0x8c4+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_50
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_50 (0x8c8+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_51
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_51 (0x8cc+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_52
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_52 (0x8d0+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_53
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_53 (0x8d4+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_54
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_54 (0x8d8+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_55
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_55 (0x8dc+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_56
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_56 (0x8e0+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_57
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_57 (0x8e4+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_58
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_58 (0x8e8+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_59
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_59 (0x8ec+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_60
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_60 (0x8f0+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_61
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_61 (0x8f4+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_62
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_62 (0x8f8+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_63
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_63 (0x8fc+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_64
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_64 (0x900+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_65
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_65 (0x904+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_66
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_66 (0x908+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_67
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_67 (0x90c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_68
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_68 (0x910+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_69
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_69 (0x914+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_70
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_70 (0x918+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_71
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_71 (0x91c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_72
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_72 (0x920+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_73
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_73 (0x924+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_74
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_74 (0x928+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_75
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_75 (0x92c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_76
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_76 (0x930+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_77
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_77 (0x934+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_78
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_78 (0x938+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_79
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_79 (0x93c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_80
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_80 (0x940+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_81
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_81 (0x944+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_82
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_82 (0x948+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_83
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_83 (0x94c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_84
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_84 (0x950+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_85
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_85 (0x954+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_86
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_86 (0x958+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_87
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_87 (0x95c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_88
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_88 (0x960+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_89
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_89 (0x964+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_90
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_90 (0x968+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_91
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_91 (0x96c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_92
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_92 (0x970+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_93
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_93 (0x974+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_94
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_94 (0x978+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_95
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_95 (0x97c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_96
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_96 (0x980+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_97
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_97 (0x984+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_98
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_98 (0x988+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_99
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_99 (0x98c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_100
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_100 (0x990+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_101
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_101 (0x994+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_102
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_102 (0x998+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_103
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_103 (0x99c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_104
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_104 (0x9a0+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_105
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_105 (0x9a4+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_106
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_106 (0x9a8+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_107
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_107 (0x9ac+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_108
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_108 (0x9b0+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_109
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_109 (0x9b4+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_110
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_110 (0x9b8+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_111
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_111 (0x9bc+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_112
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_112 (0x9c0+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_113
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_113 (0x9c4+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_114
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_114 (0x9c8+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_115
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_115 (0x9cc+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_116
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_116 (0x9d0+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_117
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_117 (0x9d4+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_118
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_118 (0x9d8+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_119
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_119 (0x9dc+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_120
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_120 (0x9e0+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_121
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_121 (0x9e4+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_122
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_122 (0x9e8+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_123
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_123 (0x9ec+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_124
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_124 (0x9f0+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_125
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_125 (0x9f4+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_126
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_126 (0x9f8+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_127
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_127 (0x9fc+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_128
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_128 (0xa00+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_129
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_129 (0xa04+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_130
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_130 (0xa08+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_131
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_131 (0xa0c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_132
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_132 (0xa10+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_133
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_133 (0xa14+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_134
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_134 (0xa18+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_135
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_135 (0xa1c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_136
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_136 (0xa20+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_137
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_137 (0xa24+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_138
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_138 (0xa28+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_139
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_139 (0xa2c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_140
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_140 (0xa30+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_141
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_141 (0xa34+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_142
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_142 (0xa38+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_143
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_143 (0xa3c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_144
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_144 (0xa40+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_145
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_145 (0xa44+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_146
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_146 (0xa48+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_147
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_147 (0xa4c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_148
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_148 (0xa50+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_149
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_149 (0xa54+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_150
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_150 (0xa58+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_151
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_151 (0xa5c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_152
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_152 (0xa60+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_153
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_153 (0xa64+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_154
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_154 (0xa68+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_155
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_155 (0xa6c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_156
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_156 (0xa70+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_157
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_157 (0xa74+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_158
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_158 (0xa78+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_159
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_159 (0xa7c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_160
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_160 (0xa80+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_161
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_161 (0xa84+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_162
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_162 (0xa88+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_163
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_163 (0xa8c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_164
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_164 (0xa90+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_165
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_165 (0xa94+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_166
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_166 (0xa98+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_167
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_167 (0xa9c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_168
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_168 (0xaa0+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_169
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_169 (0xaa4+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_170
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_170 (0xaa8+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_171
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_171 (0xaac+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_172
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_172 (0xab0+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_173
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_173 (0xab4+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_174
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_174 (0xab8+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_175
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_175 (0xabc+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_176
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_176 (0xac0+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_177
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_177 (0xac4+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_178
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_178 (0xac8+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_179
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_179 (0xacc+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_180
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_180 (0xad0+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_181
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_181 (0xad4+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_182
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_182 (0xad8+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_183
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_183 (0xadc+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_184
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_184 (0xae0+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_185
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_185 (0xae4+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_186
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_186 (0xae8+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_187
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_187 (0xaec+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_188
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_188 (0xaf0+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_189
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_189 (0xaf4+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_190
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_190 (0xaf8+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_191
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_191 (0xafc+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_192
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_192 (0xb00+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_193
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_193 (0xb04+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_194
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_194 (0xb08+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_195
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_195 (0xb0c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_196
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_196 (0xb10+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_197
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_197 (0xb14+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_198
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_198 (0xb18+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_199
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_199 (0xb1c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_200
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_200 (0xb20+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_201
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_201 (0xb24+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_202
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_202 (0xb28+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_203
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_203 (0xb2c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_204
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_204 (0xb30+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_205
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_205 (0xb34+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_206
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_206 (0xb38+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_207
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_207 (0xb3c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_208
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_208 (0xb40+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_209
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_209 (0xb44+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_210
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_210 (0xb48+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_211
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_211 (0xb4c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_212
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_212 (0xb50+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_213
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_213 (0xb54+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_214
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_214 (0xb58+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_215
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_215 (0xb5c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_216
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_216 (0xb60+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_217
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_217 (0xb64+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_218
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_218 (0xb68+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_219
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_219 (0xb6c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_220
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_220 (0xb70+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_221
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_221 (0xb74+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_222
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_222 (0xb78+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_223
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_223 (0xb7c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_224
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_224 (0xb80+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_225
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_225 (0xb84+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_226
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_226 (0xb88+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_227
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_227 (0xb8c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_228
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_228 (0xb90+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_229
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_229 (0xb94+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_230
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_230 (0xb98+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_231
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_231 (0xb9c+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_232
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_232 (0xba0+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_233
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_233 (0xba4+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_234
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_234 (0xba8+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_235
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_235 (0xbac+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_236
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_236 (0xbb0+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_237
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_237 (0xbb4+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_238
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_238 (0xbb8+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_239
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_239 (0xbbc+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_240
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_240 (0xbc0+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_241
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_241 (0xbc4+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_242
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_242 (0xbc8+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_243
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_243 (0xbcc+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_244
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_244 (0xbd0+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_245
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_245 (0xbd4+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_246
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_246 (0xbd8+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_247
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_247 (0xbdc+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_248
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_248 (0xbe0+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_249
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_249 (0xbe4+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_250
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_250 (0xbe8+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_251
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_251 (0xbec+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_252
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_252 (0xbf0+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_253
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_253 (0xbf4+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_254
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_254 (0xbf8+(3<<12))
#undef H2S_CSR_G3_NC_ATT_MAP_SELECT_255
#define H2S_CSR_G3_NC_ATT_MAP_SELECT_255 (0xbfc+(3<<12))
#undef H2S_CSR_G3_WATCH_BUS_SELECT
#define H2S_CSR_G3_WATCH_BUS_SELECT (0xC00+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_watch_bus_select {
	unsigned _watch_bus_select_dnc: 5;
	unsigned _watch_bus_select_h2s: 1;
	unsigned : 3;
	unsigned : 23;
};
#endif
#undef H2S_CSR_G3_TRACERCTRL
#define H2S_CSR_G3_TRACERCTRL (0xC04+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_tracerctrl {
	unsigned : 2;
	unsigned : 10;
	unsigned : 20;
};
#endif
#undef H2S_CSR_G3_TRACERSTAT
#define H2S_CSR_G3_TRACERSTAT (0xC08+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_tracerstat {
	unsigned _0: 1;
	unsigned _current: 10;
	unsigned : 21;
};
#endif
#undef H2S_CSR_G3_TRACER_EVENT_ADDRESS_UPPER_BITS
#define H2S_CSR_G3_TRACER_EVENT_ADDRESS_UPPER_BITS (0xC0C+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_tracer_event_address_upper_bits {
	unsigned _tracer: 10;
	unsigned : 22;
};
#endif
#undef H2S_CSR_G3_TRACER_EVENT_ADDRESS_LOWER_BITS
#define H2S_CSR_G3_TRACER_EVENT_ADDRESS_LOWER_BITS (0xC10+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_tracer_event_address_lower_bits {
	unsigned _tracer: 32;
};
#endif
#undef H2S_CSR_G3_TRACER_SELECT_COMPARE_AND_MASK
#define H2S_CSR_G3_TRACER_SELECT_COMPARE_AND_MASK (0xC14+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_tracer_select_compare_and_mask {
	unsigned _tracer: 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned _select: 1;
	unsigned : 15;
};
#endif
#undef H2S_CSR_G3_SELECT_COUNTER
#define H2S_CSR_G3_SELECT_COUNTER (0xF78+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_select_counter {
	unsigned _select_counter_0: 3;
	unsigned : 1;
	unsigned _select_counter_1: 3;
	unsigned : 1;
	unsigned _select_counter_2: 3;
	unsigned : 1;
	unsigned _select_counter_3: 3;
	unsigned : 1;
	unsigned _select_counter_4: 3;
	unsigned : 1;
	unsigned _select_counter_5: 3;
	unsigned : 1;
	unsigned _select_counter_6: 3;
	unsigned : 1;
	unsigned _select_counter_7: 3;
	unsigned : 1;
};
#endif
#undef H2S_CSR_G3_OVERFLOW_COUNTER_0_7
#define H2S_CSR_G3_OVERFLOW_COUNTER_0_7 (0xF98+(3<<12))
#undef H2S_CSR_G3_TIMER_FOR_ECC_COUNTER_7
#define H2S_CSR_G3_TIMER_FOR_ECC_COUNTER_7 (0xF9C+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_timer_for_ecc_counter_7 {
	unsigned _enable: 1;
	unsigned _clock: 31;
};
#endif
#undef H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_0
#define H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_0 (0xFA0+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_compare_and_mask_of_counter_0 {
	unsigned _mask: 8;
	unsigned _compare: 8;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_1
#define H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_1 (0xFA4+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_compare_and_mask_of_counter_1 {
	unsigned _mask: 8;
	unsigned _compare: 8;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_2
#define H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_2 (0xFA8+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_compare_and_mask_of_counter_2 {
	unsigned _mask: 8;
	unsigned _compare: 8;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_3
#define H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_3 (0xFAC+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_compare_and_mask_of_counter_3 {
	unsigned _mask: 8;
	unsigned _compare: 8;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_4
#define H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_4 (0xFB0+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_compare_and_mask_of_counter_4 {
	unsigned _mask: 8;
	unsigned _compare: 8;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_5
#define H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_5 (0xFB4+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_compare_and_mask_of_counter_5 {
	unsigned _mask: 8;
	unsigned _compare: 8;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_6
#define H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_6 (0xFB8+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_compare_and_mask_of_counter_6 {
	unsigned _mask: 8;
	unsigned _compare: 8;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_7
#define H2S_CSR_G3_COMPARE_AND_MASK_OF_COUNTER_7 (0xFBC+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_compare_and_mask_of_counter_7 {
	unsigned _mask: 8;
	unsigned _compare: 8;
	unsigned : 16;
};
#endif
#undef H2S_CSR_G3_PERFORMANCE_COUNTER_0_40_BIT_UPPER_BITS
#define H2S_CSR_G3_PERFORMANCE_COUNTER_0_40_BIT_UPPER_BITS (0xFC0+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_performance_counter_0_40_bit_upper_bits {
	unsigned _upper: 8;
	unsigned : 24;
};
#endif
#undef H2S_CSR_G3_PERFORMANCE_COUNTER_0_40_BIT_LOWER_BITS
#define H2S_CSR_G3_PERFORMANCE_COUNTER_0_40_BIT_LOWER_BITS (0xFC4+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_performance_counter_0_40_bit_lower_bits {
	unsigned _lower: 32;
};
#endif
#undef H2S_CSR_G3_PERFORMANCE_COUNTER_1_40_BIT_UPPER_BITS
#define H2S_CSR_G3_PERFORMANCE_COUNTER_1_40_BIT_UPPER_BITS (0xFC8+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_performance_counter_1_40_bit_upper_bits {
	unsigned _upper: 8;
	unsigned : 24;
};
#endif
#undef H2S_CSR_G3_PERFORMANCE_COUNTER_1_40_BIT_LOWER_BITS
#define H2S_CSR_G3_PERFORMANCE_COUNTER_1_40_BIT_LOWER_BITS (0xFCC+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_performance_counter_1_40_bit_lower_bits {
	unsigned _lower: 32;
};
#endif
#undef H2S_CSR_G3_PERFORMANCE_COUNTER_2_40_BIT_UPPER_BITS
#define H2S_CSR_G3_PERFORMANCE_COUNTER_2_40_BIT_UPPER_BITS (0xFD0+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_performance_counter_2_40_bit_upper_bits {
	unsigned _upper: 8;
	unsigned : 24;
};
#endif
#undef H2S_CSR_G3_PERFORMANCE_COUNTER_2_40_BIT_LOWER_BITS
#define H2S_CSR_G3_PERFORMANCE_COUNTER_2_40_BIT_LOWER_BITS (0xFD4+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_performance_counter_2_40_bit_lower_bits {
	unsigned _lower: 32;
};
#endif
#undef H2S_CSR_G3_PERFORMANCE_COUNTER_3_40_BIT_UPPER_BITS
#define H2S_CSR_G3_PERFORMANCE_COUNTER_3_40_BIT_UPPER_BITS (0xFD8+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_performance_counter_3_40_bit_upper_bits {
	unsigned _upper: 8;
	unsigned : 24;
};
#endif
#undef H2S_CSR_G3_PERFORMANCE_COUNTER_3_40_BIT_LOWER_BITS
#define H2S_CSR_G3_PERFORMANCE_COUNTER_3_40_BIT_LOWER_BITS (0xFDC+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_performance_counter_3_40_bit_lower_bits {
	unsigned _lower: 32;
};
#endif
#undef H2S_CSR_G3_PERFORMANCE_COUNTER_4_40_BIT_UPPER_BITS
#define H2S_CSR_G3_PERFORMANCE_COUNTER_4_40_BIT_UPPER_BITS (0xFE0+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_performance_counter_4_40_bit_upper_bits {
	unsigned _upper: 8;
	unsigned : 24;
};
#endif
#undef H2S_CSR_G3_PERFORMANCE_COUNTER_4_40_BIT_LOWER_BITS
#define H2S_CSR_G3_PERFORMANCE_COUNTER_4_40_BIT_LOWER_BITS (0xFE4+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_performance_counter_4_40_bit_lower_bits {
	unsigned _lower: 32;
};
#endif
#undef H2S_CSR_G3_PERFORMANCE_COUNTER_5_40_BIT_UPPER_BITS
#define H2S_CSR_G3_PERFORMANCE_COUNTER_5_40_BIT_UPPER_BITS (0xFE8+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_performance_counter_5_40_bit_upper_bits {
	unsigned _upper: 8;
	unsigned : 24;
};
#endif
#undef H2S_CSR_G3_PERFORMANCE_COUNTER_5_40_BIT_LOWER_BITS
#define H2S_CSR_G3_PERFORMANCE_COUNTER_5_40_BIT_LOWER_BITS (0xFEC+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_performance_counter_5_40_bit_lower_bits {
	unsigned _lower: 32;
};
#endif
#undef H2S_CSR_G3_PERFORMANCE_COUNTER_6_40_BIT_UPPER_BITS
#define H2S_CSR_G3_PERFORMANCE_COUNTER_6_40_BIT_UPPER_BITS (0xFF0+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_performance_counter_6_40_bit_upper_bits {
	unsigned _upper: 8;
	unsigned : 24;
};
#endif
#undef H2S_CSR_G3_PERFORMANCE_COUNTER_6_40_BIT_LOWER_BITS
#define H2S_CSR_G3_PERFORMANCE_COUNTER_6_40_BIT_LOWER_BITS (0xFF4+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_performance_counter_6_40_bit_lower_bits {
	unsigned _lower: 32;
};
#endif
#undef H2S_CSR_G3_PERFORMANCE_COUNTER_7_40_BIT_UPPER_BITS
#define H2S_CSR_G3_PERFORMANCE_COUNTER_7_40_BIT_UPPER_BITS (0xFF8+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_performance_counter_7_40_bit_upper_bits {
	unsigned _upper: 8;
	unsigned : 24;
};
#endif
#undef H2S_CSR_G3_PERFORMANCE_COUNTER_7_40_BIT_LOWER_BITS
#define H2S_CSR_G3_PERFORMANCE_COUNTER_7_40_BIT_LOWER_BITS (0xFFC+(3<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g3_performance_counter_7_40_bit_lower_bits {
	unsigned _lower: 32;
};
#endif
#undef H2S_CSR_G4_MCTAG_MAINTR
#define H2S_CSR_G4_MCTAG_MAINTR (0x700+(4<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g4_mctag_maintr {
	unsigned _interrupt_en: 1;
	unsigned _prefbuf_en: 1;
	unsigned _ctagc_en: 1;
	unsigned _sw_interrupt: 1;
	unsigned _mem_ini_enable: 1;
	unsigned _auto_scrub_enable: 1;
	unsigned _auto_cor_err_wb: 1;
	unsigned _diagnose: 1;
	unsigned : 1;
	unsigned : 23;
};
#endif
#undef H2S_CSR_G4_MCTAG_COM_CTRLR
#define H2S_CSR_G4_MCTAG_COM_CTRLR (0x704+(4<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g4_mctag_com_ctrlr {
	unsigned _csr_mem_acc: 1;
	unsigned _csr_mem_rd_acc: 1;
	unsigned _csr_mem_wrmask_acc: 1;
	unsigned _srefresh_enter: 1;
	unsigned _mem_size: 3;
	unsigned _ftagbig: 1;
	unsigned _ranks: 2;
	unsigned _udimm: 1;
	unsigned _bsstl18_driver_strength: 1;
	unsigned _dram_available_textbf_rw_0_dram_requests_from_ftag_hreq_and_sprb_are_not_executed_xxx_rdy_0_textbf_rw_1: 1;
	unsigned _prio_port0_mctag: 1;
	unsigned _prio_port1_mctag: 1;
	unsigned : 17;
};
#endif
#undef H2S_CSR_G4_MCTAG_COM_STATR
#define H2S_CSR_G4_MCTAG_COM_STATR (0x708+(4<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g4_mctag_com_statr {
	unsigned : 3;
	unsigned _auto_cor_active: 1;
	unsigned _mctr_q_almost_full: 1;
	unsigned _mctr_refresh_in_process: 1;
	unsigned _mctr_controller_busy: 1;
	unsigned _mctr_sum_interrupt: 1;
	unsigned _mctr_ports_busy: 2;
	unsigned _mctr_data_out_rd_id: 2;
	unsigned _denali: 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 15;
};
#endif
#undef H2S_CSR_G4_MCTAG_ERROR_STATR
#define H2S_CSR_G4_MCTAG_ERROR_STATR (0x70C+(4<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g4_mctag_error_statr {
	unsigned : 6;
	unsigned : 26;
};
#endif
#undef H2S_CSR_G4_MCTAG_ERROR_MASK
#define H2S_CSR_G4_MCTAG_ERROR_MASK (0x710+(4<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g4_mctag_error_mask {
	unsigned _mask_ecc_dout_uncorr: 1;
	unsigned _mask_d0_ecc_dout_corr: 1;
	unsigned _mask_d1_ecc_dout_corr: 1;
	unsigned _mask_rhc_read_err: 1;
	unsigned _mask_rsc_read_err: 1;
	unsigned _mask_den_interrupt: 1;
	unsigned : 26;
};
#endif
#undef H2S_CSR_G4_MCTAG_SCRUBBER_ADDR
#define H2S_CSR_G4_MCTAG_SCRUBBER_ADDR (0x714+(4<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g4_mctag_scrubber_addr {
	unsigned : 2;
	unsigned _16byte: 30;
};
#endif
#undef H2S_CSR_G4_MCTAG_MEMORY_ADDR
#define H2S_CSR_G4_MCTAG_MEMORY_ADDR (0x718+(4<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g4_mctag_memory_addr {
	unsigned _4byte: 32;
};
#endif
#undef H2S_CSR_G4_MCTAG_MEMORY_WDATA
#define H2S_CSR_G4_MCTAG_MEMORY_WDATA (0x71C+(4<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g4_mctag_memory_wdata {
	unsigned : 32;
};
#endif
#undef H2S_CSR_G4_MCTAG_MEMORY_RDATA
#define H2S_CSR_G4_MCTAG_MEMORY_RDATA (0x720+(4<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g4_mctag_memory_rdata {
	unsigned : 32;
};
#endif
#undef H2S_CSR_G4_MCTAG_C1B_MEM_ADDR
#define H2S_CSR_G4_MCTAG_C1B_MEM_ADDR (0x724+(4<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g4_mctag_c1b_mem_addr {
	unsigned _4byte: 32;
};
#endif
#undef H2S_CSR_G4_G2XAA8
#define H2S_CSR_G4_G2XAA8 (0x800+(4<<12))
#undef H2S_CSR_G4_CDATA_MAINTR
#define H2S_CSR_G4_CDATA_MAINTR (0xF00+(4<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g4_cdata_maintr {
	unsigned _interrupt_en: 1;
	unsigned _prefbuf_en: 1;
	unsigned _ctagc_en: 1;
	unsigned _sw_interrupt: 1;
	unsigned _mem_ini_enable: 1;
	unsigned _auto_scrub_enable: 1;
	unsigned _auto_cor_err_wb: 1;
	unsigned _diagnose: 1;
	unsigned : 1;
	unsigned : 23;
};
#endif
#undef H2S_CSR_G4_CDATA_COM_CTRLR
#define H2S_CSR_G4_CDATA_COM_CTRLR (0xF04+(4<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g4_cdata_com_ctrlr {
	unsigned _csr_mem_acc: 1;
	unsigned _csr_mem_rd_acc: 1;
	unsigned _csr_mem_wrmask_acc: 1;
	unsigned _srefresh_enter: 1;
	unsigned _mem_size: 3;
	unsigned _ftagbig: 1;
	unsigned _ranks: 2;
	unsigned _udimm: 1;
	unsigned _bsstl18_driver_strength: 1;
	unsigned _dram_available_textbf_rw_0_dram_requests_from_ftag_hreq_and_sprb_are_not_executed_xxx_rdy_0_textbf_rw_1: 1;
	unsigned _prio_port0_cdata: 1;
	unsigned _prio_port1_cdata: 1;
	unsigned : 17;
};
#endif
#undef H2S_CSR_G4_CDATA_COM_STATR
#define H2S_CSR_G4_CDATA_COM_STATR (0xF08+(4<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g4_cdata_com_statr {
	unsigned : 3;
	unsigned _auto_cor_active: 1;
	unsigned _mctr_q_almost_full: 1;
	unsigned _mctr_refresh_in_process: 1;
	unsigned _mctr_controller_busy: 1;
	unsigned _mctr_sum_interrupt: 1;
	unsigned _mctr_ports_busy: 2;
	unsigned _mctr_data_out_rd_id: 2;
	unsigned _denali: 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 1;
	unsigned : 15;
};
#endif
#undef H2S_CSR_G4_CDATA_ERROR_STATR
#define H2S_CSR_G4_CDATA_ERROR_STATR (0xF0C+(4<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g4_cdata_error_statr {
	unsigned : 6;
	unsigned : 26;
};
#endif
#undef H2S_CSR_G4_CDATA_ERROR_MASK
#define H2S_CSR_G4_CDATA_ERROR_MASK (0xF10+(4<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g4_cdata_error_mask {
	unsigned _mask_ecc_dout_uncorr: 1;
	unsigned _mask_d0_ecc_dout_corr: 1;
	unsigned _mask_d1_ecc_dout_corr: 1;
	unsigned _mask_rhc_read_err: 1;
	unsigned _mask_rsc_read_err: 1;
	unsigned _mask_den_interrupt: 1;
	unsigned : 26;
};
#endif
#undef H2S_CSR_G4_CDATA_SCRUBBER_ADDR
#define H2S_CSR_G4_CDATA_SCRUBBER_ADDR (0xF14+(4<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g4_cdata_scrubber_addr {
	unsigned : 2;
	unsigned _16byte: 30;
};
#endif
#undef H2S_CSR_G4_CDATA_MEMORY_ADDR
#define H2S_CSR_G4_CDATA_MEMORY_ADDR (0xF18+(4<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g4_cdata_memory_addr {
	unsigned _4byte: 32;
};
#endif
#undef H2S_CSR_G4_CDATA_MEMORY_WDATA
#define H2S_CSR_G4_CDATA_MEMORY_WDATA (0xF1C+(4<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g4_cdata_memory_wdata {
	unsigned : 32;
};
#endif
#undef H2S_CSR_G4_CDATA_MEMORY_RDATA
#define H2S_CSR_G4_CDATA_MEMORY_RDATA (0xF20+(4<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g4_cdata_memory_rdata {
	unsigned : 32;
};
#endif
#undef H2S_CSR_G4_CDATA_C1B_MEM_ADDR
#define H2S_CSR_G4_CDATA_C1B_MEM_ADDR (0xF24+(4<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g4_cdata_c1b_mem_addr {
	unsigned _4byte: 32;
};
#endif
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_0
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_0 (0x0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1 (0x4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_2
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_2 (0x8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_3
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_3 (0xc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_4
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_4 (0x10+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_5
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_5 (0x14+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_6
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_6 (0x18+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_7
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_7 (0x1c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_8
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_8 (0x20+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_9
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_9 (0x24+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_10
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_10 (0x28+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_11
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_11 (0x2c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_12
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_12 (0x30+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_13
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_13 (0x34+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_14
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_14 (0x38+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_15
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_15 (0x3c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_16
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_16 (0x40+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_17
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_17 (0x44+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_18
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_18 (0x48+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_19
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_19 (0x4c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_20
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_20 (0x50+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_21
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_21 (0x54+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_22
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_22 (0x58+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_23
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_23 (0x5c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_24
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_24 (0x60+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_25
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_25 (0x64+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_26
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_26 (0x68+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_27
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_27 (0x6c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_28
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_28 (0x70+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_29
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_29 (0x74+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_30
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_30 (0x78+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_31
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_31 (0x7c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_32
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_32 (0x80+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_33
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_33 (0x84+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_34
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_34 (0x88+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_35
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_35 (0x8c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_36
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_36 (0x90+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_37
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_37 (0x94+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_38
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_38 (0x98+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_39
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_39 (0x9c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_40
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_40 (0xa0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_41
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_41 (0xa4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_42
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_42 (0xa8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_43
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_43 (0xac+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_44
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_44 (0xb0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_45
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_45 (0xb4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_46
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_46 (0xb8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_47
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_47 (0xbc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_48
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_48 (0xc0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_49
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_49 (0xc4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_50
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_50 (0xc8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_51
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_51 (0xcc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_52
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_52 (0xd0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_53
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_53 (0xd4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_54
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_54 (0xd8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_55
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_55 (0xdc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_56
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_56 (0xe0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_57
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_57 (0xe4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_58
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_58 (0xe8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_59
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_59 (0xec+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_60
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_60 (0xf0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_61
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_61 (0xf4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_62
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_62 (0xf8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_63
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_63 (0xfc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_64
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_64 (0x100+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_65
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_65 (0x104+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_66
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_66 (0x108+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_67
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_67 (0x10c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_68
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_68 (0x110+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_69
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_69 (0x114+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_70
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_70 (0x118+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_71
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_71 (0x11c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_72
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_72 (0x120+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_73
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_73 (0x124+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_74
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_74 (0x128+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_75
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_75 (0x12c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_76
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_76 (0x130+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_77
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_77 (0x134+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_78
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_78 (0x138+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_79
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_79 (0x13c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_80
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_80 (0x140+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_81
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_81 (0x144+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_82
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_82 (0x148+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_83
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_83 (0x14c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_84
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_84 (0x150+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_85
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_85 (0x154+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_86
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_86 (0x158+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_87
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_87 (0x15c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_88
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_88 (0x160+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_89
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_89 (0x164+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_90
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_90 (0x168+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_91
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_91 (0x16c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_92
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_92 (0x170+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_93
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_93 (0x174+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_94
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_94 (0x178+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_95
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_95 (0x17c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_96
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_96 (0x180+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_97
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_97 (0x184+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_98
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_98 (0x188+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_99
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_99 (0x18c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_100
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_100 (0x190+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_101
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_101 (0x194+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_102
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_102 (0x198+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_103
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_103 (0x19c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_104
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_104 (0x1a0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_105
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_105 (0x1a4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_106
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_106 (0x1a8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_107
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_107 (0x1ac+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_108
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_108 (0x1b0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_109
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_109 (0x1b4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_110
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_110 (0x1b8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_111
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_111 (0x1bc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_112
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_112 (0x1c0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_113
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_113 (0x1c4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_114
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_114 (0x1c8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_115
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_115 (0x1cc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_116
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_116 (0x1d0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_117
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_117 (0x1d4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_118
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_118 (0x1d8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_119
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_119 (0x1dc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_120
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_120 (0x1e0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_121
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_121 (0x1e4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_122
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_122 (0x1e8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_123
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_123 (0x1ec+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_124
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_124 (0x1f0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_125
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_125 (0x1f4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_126
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_126 (0x1f8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_127
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_127 (0x1fc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_128
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_128 (0x200+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_129
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_129 (0x204+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_130
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_130 (0x208+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_131
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_131 (0x20c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_132
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_132 (0x210+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_133
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_133 (0x214+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_134
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_134 (0x218+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_135
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_135 (0x21c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_136
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_136 (0x220+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_137
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_137 (0x224+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_138
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_138 (0x228+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_139
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_139 (0x22c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_140
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_140 (0x230+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_141
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_141 (0x234+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_142
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_142 (0x238+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_143
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_143 (0x23c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_144
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_144 (0x240+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_145
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_145 (0x244+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_146
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_146 (0x248+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_147
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_147 (0x24c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_148
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_148 (0x250+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_149
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_149 (0x254+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_150
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_150 (0x258+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_151
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_151 (0x25c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_152
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_152 (0x260+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_153
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_153 (0x264+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_154
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_154 (0x268+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_155
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_155 (0x26c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_156
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_156 (0x270+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_157
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_157 (0x274+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_158
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_158 (0x278+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_159
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_159 (0x27c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_160
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_160 (0x280+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_161
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_161 (0x284+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_162
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_162 (0x288+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_163
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_163 (0x28c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_164
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_164 (0x290+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_165
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_165 (0x294+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_166
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_166 (0x298+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_167
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_167 (0x29c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_168
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_168 (0x2a0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_169
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_169 (0x2a4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_170
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_170 (0x2a8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_171
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_171 (0x2ac+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_172
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_172 (0x2b0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_173
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_173 (0x2b4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_174
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_174 (0x2b8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_175
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_175 (0x2bc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_176
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_176 (0x2c0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_177
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_177 (0x2c4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_178
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_178 (0x2c8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_179
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_179 (0x2cc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_180
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_180 (0x2d0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_181
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_181 (0x2d4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_182
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_182 (0x2d8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_183
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_183 (0x2dc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_184
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_184 (0x2e0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_185
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_185 (0x2e4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_186
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_186 (0x2e8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_187
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_187 (0x2ec+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_188
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_188 (0x2f0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_189
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_189 (0x2f4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_190
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_190 (0x2f8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_191
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_191 (0x2fc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_192
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_192 (0x300+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_193
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_193 (0x304+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_194
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_194 (0x308+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_195
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_195 (0x30c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_196
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_196 (0x310+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_197
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_197 (0x314+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_198
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_198 (0x318+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_199
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_199 (0x31c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_200
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_200 (0x320+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_201
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_201 (0x324+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_202
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_202 (0x328+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_203
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_203 (0x32c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_204
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_204 (0x330+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_205
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_205 (0x334+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_206
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_206 (0x338+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_207
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_207 (0x33c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_208
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_208 (0x340+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_209
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_209 (0x344+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_210
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_210 (0x348+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_211
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_211 (0x34c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_212
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_212 (0x350+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_213
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_213 (0x354+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_214
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_214 (0x358+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_215
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_215 (0x35c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_216
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_216 (0x360+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_217
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_217 (0x364+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_218
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_218 (0x368+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_219
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_219 (0x36c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_220
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_220 (0x370+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_221
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_221 (0x374+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_222
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_222 (0x378+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_223
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_223 (0x37c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_224
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_224 (0x380+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_225
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_225 (0x384+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_226
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_226 (0x388+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_227
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_227 (0x38c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_228
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_228 (0x390+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_229
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_229 (0x394+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_230
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_230 (0x398+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_231
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_231 (0x39c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_232
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_232 (0x3a0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_233
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_233 (0x3a4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_234
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_234 (0x3a8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_235
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_235 (0x3ac+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_236
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_236 (0x3b0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_237
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_237 (0x3b4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_238
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_238 (0x3b8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_239
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_239 (0x3bc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_240
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_240 (0x3c0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_241
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_241 (0x3c4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_242
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_242 (0x3c8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_243
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_243 (0x3cc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_244
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_244 (0x3d0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_245
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_245 (0x3d4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_246
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_246 (0x3d8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_247
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_247 (0x3dc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_248
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_248 (0x3e0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_249
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_249 (0x3e4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_250
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_250 (0x3e8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_251
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_251 (0x3ec+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_252
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_252 (0x3f0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_253
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_253 (0x3f4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_254
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_254 (0x3f8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_255
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_255 (0x3fc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_256
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_256 (0x400+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_257
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_257 (0x404+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_258
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_258 (0x408+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_259
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_259 (0x40c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_260
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_260 (0x410+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_261
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_261 (0x414+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_262
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_262 (0x418+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_263
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_263 (0x41c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_264
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_264 (0x420+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_265
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_265 (0x424+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_266
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_266 (0x428+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_267
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_267 (0x42c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_268
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_268 (0x430+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_269
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_269 (0x434+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_270
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_270 (0x438+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_271
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_271 (0x43c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_272
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_272 (0x440+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_273
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_273 (0x444+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_274
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_274 (0x448+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_275
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_275 (0x44c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_276
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_276 (0x450+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_277
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_277 (0x454+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_278
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_278 (0x458+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_279
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_279 (0x45c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_280
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_280 (0x460+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_281
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_281 (0x464+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_282
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_282 (0x468+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_283
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_283 (0x46c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_284
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_284 (0x470+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_285
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_285 (0x474+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_286
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_286 (0x478+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_287
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_287 (0x47c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_288
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_288 (0x480+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_289
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_289 (0x484+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_290
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_290 (0x488+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_291
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_291 (0x48c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_292
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_292 (0x490+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_293
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_293 (0x494+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_294
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_294 (0x498+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_295
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_295 (0x49c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_296
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_296 (0x4a0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_297
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_297 (0x4a4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_298
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_298 (0x4a8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_299
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_299 (0x4ac+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_300
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_300 (0x4b0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_301
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_301 (0x4b4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_302
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_302 (0x4b8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_303
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_303 (0x4bc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_304
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_304 (0x4c0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_305
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_305 (0x4c4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_306
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_306 (0x4c8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_307
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_307 (0x4cc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_308
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_308 (0x4d0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_309
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_309 (0x4d4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_310
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_310 (0x4d8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_311
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_311 (0x4dc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_312
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_312 (0x4e0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_313
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_313 (0x4e4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_314
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_314 (0x4e8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_315
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_315 (0x4ec+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_316
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_316 (0x4f0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_317
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_317 (0x4f4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_318
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_318 (0x4f8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_319
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_319 (0x4fc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_320
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_320 (0x500+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_321
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_321 (0x504+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_322
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_322 (0x508+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_323
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_323 (0x50c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_324
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_324 (0x510+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_325
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_325 (0x514+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_326
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_326 (0x518+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_327
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_327 (0x51c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_328
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_328 (0x520+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_329
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_329 (0x524+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_330
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_330 (0x528+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_331
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_331 (0x52c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_332
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_332 (0x530+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_333
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_333 (0x534+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_334
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_334 (0x538+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_335
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_335 (0x53c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_336
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_336 (0x540+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_337
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_337 (0x544+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_338
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_338 (0x548+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_339
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_339 (0x54c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_340
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_340 (0x550+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_341
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_341 (0x554+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_342
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_342 (0x558+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_343
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_343 (0x55c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_344
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_344 (0x560+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_345
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_345 (0x564+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_346
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_346 (0x568+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_347
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_347 (0x56c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_348
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_348 (0x570+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_349
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_349 (0x574+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_350
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_350 (0x578+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_351
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_351 (0x57c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_352
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_352 (0x580+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_353
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_353 (0x584+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_354
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_354 (0x588+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_355
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_355 (0x58c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_356
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_356 (0x590+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_357
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_357 (0x594+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_358
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_358 (0x598+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_359
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_359 (0x59c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_360
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_360 (0x5a0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_361
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_361 (0x5a4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_362
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_362 (0x5a8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_363
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_363 (0x5ac+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_364
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_364 (0x5b0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_365
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_365 (0x5b4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_366
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_366 (0x5b8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_367
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_367 (0x5bc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_368
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_368 (0x5c0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_369
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_369 (0x5c4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_370
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_370 (0x5c8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_371
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_371 (0x5cc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_372
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_372 (0x5d0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_373
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_373 (0x5d4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_374
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_374 (0x5d8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_375
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_375 (0x5dc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_376
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_376 (0x5e0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_377
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_377 (0x5e4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_378
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_378 (0x5e8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_379
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_379 (0x5ec+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_380
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_380 (0x5f0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_381
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_381 (0x5f4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_382
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_382 (0x5f8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_383
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_383 (0x5fc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_384
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_384 (0x600+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_385
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_385 (0x604+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_386
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_386 (0x608+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_387
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_387 (0x60c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_388
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_388 (0x610+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_389
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_389 (0x614+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_390
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_390 (0x618+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_391
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_391 (0x61c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_392
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_392 (0x620+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_393
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_393 (0x624+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_394
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_394 (0x628+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_395
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_395 (0x62c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_396
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_396 (0x630+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_397
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_397 (0x634+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_398
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_398 (0x638+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_399
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_399 (0x63c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_400
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_400 (0x640+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_401
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_401 (0x644+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_402
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_402 (0x648+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_403
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_403 (0x64c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_404
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_404 (0x650+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_405
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_405 (0x654+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_406
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_406 (0x658+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_407
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_407 (0x65c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_408
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_408 (0x660+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_409
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_409 (0x664+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_410
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_410 (0x668+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_411
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_411 (0x66c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_412
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_412 (0x670+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_413
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_413 (0x674+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_414
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_414 (0x678+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_415
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_415 (0x67c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_416
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_416 (0x680+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_417
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_417 (0x684+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_418
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_418 (0x688+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_419
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_419 (0x68c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_420
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_420 (0x690+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_421
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_421 (0x694+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_422
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_422 (0x698+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_423
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_423 (0x69c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_424
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_424 (0x6a0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_425
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_425 (0x6a4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_426
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_426 (0x6a8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_427
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_427 (0x6ac+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_428
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_428 (0x6b0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_429
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_429 (0x6b4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_430
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_430 (0x6b8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_431
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_431 (0x6bc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_432
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_432 (0x6c0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_433
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_433 (0x6c4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_434
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_434 (0x6c8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_435
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_435 (0x6cc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_436
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_436 (0x6d0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_437
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_437 (0x6d4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_438
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_438 (0x6d8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_439
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_439 (0x6dc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_440
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_440 (0x6e0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_441
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_441 (0x6e4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_442
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_442 (0x6e8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_443
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_443 (0x6ec+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_444
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_444 (0x6f0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_445
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_445 (0x6f4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_446
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_446 (0x6f8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_447
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_447 (0x6fc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_448
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_448 (0x700+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_449
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_449 (0x704+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_450
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_450 (0x708+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_451
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_451 (0x70c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_452
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_452 (0x710+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_453
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_453 (0x714+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_454
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_454 (0x718+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_455
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_455 (0x71c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_456
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_456 (0x720+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_457
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_457 (0x724+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_458
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_458 (0x728+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_459
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_459 (0x72c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_460
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_460 (0x730+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_461
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_461 (0x734+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_462
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_462 (0x738+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_463
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_463 (0x73c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_464
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_464 (0x740+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_465
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_465 (0x744+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_466
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_466 (0x748+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_467
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_467 (0x74c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_468
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_468 (0x750+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_469
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_469 (0x754+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_470
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_470 (0x758+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_471
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_471 (0x75c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_472
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_472 (0x760+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_473
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_473 (0x764+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_474
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_474 (0x768+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_475
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_475 (0x76c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_476
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_476 (0x770+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_477
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_477 (0x774+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_478
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_478 (0x778+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_479
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_479 (0x77c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_480
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_480 (0x780+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_481
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_481 (0x784+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_482
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_482 (0x788+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_483
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_483 (0x78c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_484
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_484 (0x790+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_485
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_485 (0x794+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_486
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_486 (0x798+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_487
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_487 (0x79c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_488
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_488 (0x7a0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_489
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_489 (0x7a4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_490
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_490 (0x7a8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_491
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_491 (0x7ac+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_492
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_492 (0x7b0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_493
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_493 (0x7b4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_494
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_494 (0x7b8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_495
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_495 (0x7bc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_496
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_496 (0x7c0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_497
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_497 (0x7c4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_498
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_498 (0x7c8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_499
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_499 (0x7cc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_500
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_500 (0x7d0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_501
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_501 (0x7d4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_502
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_502 (0x7d8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_503
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_503 (0x7dc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_504
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_504 (0x7e0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_505
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_505 (0x7e4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_506
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_506 (0x7e8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_507
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_507 (0x7ec+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_508
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_508 (0x7f0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_509
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_509 (0x7f4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_510
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_510 (0x7f8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_511
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_511 (0x7fc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_512
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_512 (0x800+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_513
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_513 (0x804+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_514
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_514 (0x808+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_515
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_515 (0x80c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_516
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_516 (0x810+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_517
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_517 (0x814+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_518
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_518 (0x818+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_519
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_519 (0x81c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_520
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_520 (0x820+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_521
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_521 (0x824+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_522
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_522 (0x828+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_523
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_523 (0x82c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_524
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_524 (0x830+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_525
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_525 (0x834+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_526
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_526 (0x838+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_527
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_527 (0x83c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_528
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_528 (0x840+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_529
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_529 (0x844+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_530
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_530 (0x848+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_531
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_531 (0x84c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_532
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_532 (0x850+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_533
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_533 (0x854+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_534
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_534 (0x858+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_535
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_535 (0x85c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_536
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_536 (0x860+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_537
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_537 (0x864+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_538
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_538 (0x868+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_539
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_539 (0x86c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_540
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_540 (0x870+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_541
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_541 (0x874+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_542
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_542 (0x878+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_543
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_543 (0x87c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_544
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_544 (0x880+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_545
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_545 (0x884+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_546
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_546 (0x888+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_547
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_547 (0x88c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_548
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_548 (0x890+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_549
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_549 (0x894+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_550
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_550 (0x898+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_551
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_551 (0x89c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_552
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_552 (0x8a0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_553
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_553 (0x8a4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_554
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_554 (0x8a8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_555
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_555 (0x8ac+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_556
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_556 (0x8b0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_557
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_557 (0x8b4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_558
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_558 (0x8b8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_559
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_559 (0x8bc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_560
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_560 (0x8c0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_561
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_561 (0x8c4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_562
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_562 (0x8c8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_563
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_563 (0x8cc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_564
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_564 (0x8d0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_565
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_565 (0x8d4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_566
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_566 (0x8d8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_567
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_567 (0x8dc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_568
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_568 (0x8e0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_569
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_569 (0x8e4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_570
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_570 (0x8e8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_571
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_571 (0x8ec+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_572
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_572 (0x8f0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_573
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_573 (0x8f4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_574
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_574 (0x8f8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_575
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_575 (0x8fc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_576
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_576 (0x900+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_577
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_577 (0x904+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_578
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_578 (0x908+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_579
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_579 (0x90c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_580
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_580 (0x910+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_581
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_581 (0x914+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_582
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_582 (0x918+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_583
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_583 (0x91c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_584
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_584 (0x920+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_585
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_585 (0x924+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_586
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_586 (0x928+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_587
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_587 (0x92c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_588
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_588 (0x930+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_589
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_589 (0x934+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_590
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_590 (0x938+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_591
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_591 (0x93c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_592
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_592 (0x940+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_593
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_593 (0x944+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_594
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_594 (0x948+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_595
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_595 (0x94c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_596
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_596 (0x950+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_597
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_597 (0x954+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_598
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_598 (0x958+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_599
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_599 (0x95c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_600
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_600 (0x960+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_601
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_601 (0x964+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_602
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_602 (0x968+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_603
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_603 (0x96c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_604
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_604 (0x970+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_605
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_605 (0x974+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_606
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_606 (0x978+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_607
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_607 (0x97c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_608
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_608 (0x980+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_609
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_609 (0x984+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_610
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_610 (0x988+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_611
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_611 (0x98c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_612
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_612 (0x990+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_613
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_613 (0x994+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_614
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_614 (0x998+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_615
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_615 (0x99c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_616
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_616 (0x9a0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_617
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_617 (0x9a4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_618
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_618 (0x9a8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_619
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_619 (0x9ac+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_620
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_620 (0x9b0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_621
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_621 (0x9b4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_622
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_622 (0x9b8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_623
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_623 (0x9bc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_624
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_624 (0x9c0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_625
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_625 (0x9c4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_626
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_626 (0x9c8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_627
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_627 (0x9cc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_628
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_628 (0x9d0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_629
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_629 (0x9d4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_630
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_630 (0x9d8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_631
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_631 (0x9dc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_632
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_632 (0x9e0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_633
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_633 (0x9e4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_634
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_634 (0x9e8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_635
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_635 (0x9ec+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_636
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_636 (0x9f0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_637
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_637 (0x9f4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_638
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_638 (0x9f8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_639
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_639 (0x9fc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_640
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_640 (0xa00+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_641
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_641 (0xa04+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_642
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_642 (0xa08+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_643
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_643 (0xa0c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_644
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_644 (0xa10+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_645
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_645 (0xa14+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_646
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_646 (0xa18+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_647
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_647 (0xa1c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_648
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_648 (0xa20+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_649
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_649 (0xa24+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_650
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_650 (0xa28+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_651
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_651 (0xa2c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_652
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_652 (0xa30+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_653
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_653 (0xa34+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_654
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_654 (0xa38+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_655
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_655 (0xa3c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_656
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_656 (0xa40+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_657
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_657 (0xa44+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_658
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_658 (0xa48+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_659
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_659 (0xa4c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_660
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_660 (0xa50+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_661
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_661 (0xa54+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_662
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_662 (0xa58+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_663
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_663 (0xa5c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_664
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_664 (0xa60+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_665
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_665 (0xa64+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_666
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_666 (0xa68+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_667
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_667 (0xa6c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_668
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_668 (0xa70+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_669
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_669 (0xa74+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_670
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_670 (0xa78+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_671
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_671 (0xa7c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_672
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_672 (0xa80+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_673
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_673 (0xa84+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_674
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_674 (0xa88+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_675
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_675 (0xa8c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_676
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_676 (0xa90+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_677
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_677 (0xa94+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_678
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_678 (0xa98+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_679
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_679 (0xa9c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_680
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_680 (0xaa0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_681
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_681 (0xaa4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_682
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_682 (0xaa8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_683
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_683 (0xaac+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_684
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_684 (0xab0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_685
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_685 (0xab4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_686
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_686 (0xab8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_687
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_687 (0xabc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_688
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_688 (0xac0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_689
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_689 (0xac4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_690
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_690 (0xac8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_691
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_691 (0xacc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_692
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_692 (0xad0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_693
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_693 (0xad4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_694
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_694 (0xad8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_695
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_695 (0xadc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_696
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_696 (0xae0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_697
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_697 (0xae4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_698
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_698 (0xae8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_699
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_699 (0xaec+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_700
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_700 (0xaf0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_701
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_701 (0xaf4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_702
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_702 (0xaf8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_703
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_703 (0xafc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_704
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_704 (0xb00+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_705
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_705 (0xb04+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_706
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_706 (0xb08+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_707
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_707 (0xb0c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_708
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_708 (0xb10+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_709
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_709 (0xb14+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_710
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_710 (0xb18+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_711
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_711 (0xb1c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_712
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_712 (0xb20+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_713
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_713 (0xb24+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_714
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_714 (0xb28+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_715
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_715 (0xb2c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_716
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_716 (0xb30+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_717
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_717 (0xb34+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_718
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_718 (0xb38+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_719
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_719 (0xb3c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_720
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_720 (0xb40+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_721
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_721 (0xb44+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_722
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_722 (0xb48+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_723
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_723 (0xb4c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_724
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_724 (0xb50+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_725
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_725 (0xb54+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_726
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_726 (0xb58+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_727
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_727 (0xb5c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_728
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_728 (0xb60+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_729
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_729 (0xb64+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_730
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_730 (0xb68+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_731
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_731 (0xb6c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_732
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_732 (0xb70+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_733
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_733 (0xb74+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_734
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_734 (0xb78+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_735
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_735 (0xb7c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_736
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_736 (0xb80+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_737
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_737 (0xb84+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_738
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_738 (0xb88+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_739
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_739 (0xb8c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_740
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_740 (0xb90+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_741
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_741 (0xb94+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_742
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_742 (0xb98+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_743
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_743 (0xb9c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_744
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_744 (0xba0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_745
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_745 (0xba4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_746
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_746 (0xba8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_747
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_747 (0xbac+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_748
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_748 (0xbb0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_749
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_749 (0xbb4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_750
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_750 (0xbb8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_751
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_751 (0xbbc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_752
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_752 (0xbc0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_753
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_753 (0xbc4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_754
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_754 (0xbc8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_755
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_755 (0xbcc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_756
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_756 (0xbd0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_757
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_757 (0xbd4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_758
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_758 (0xbd8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_759
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_759 (0xbdc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_760
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_760 (0xbe0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_761
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_761 (0xbe4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_762
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_762 (0xbe8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_763
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_763 (0xbec+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_764
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_764 (0xbf0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_765
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_765 (0xbf4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_766
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_766 (0xbf8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_767
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_767 (0xbfc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_768
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_768 (0xc00+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_769
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_769 (0xc04+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_770
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_770 (0xc08+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_771
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_771 (0xc0c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_772
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_772 (0xc10+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_773
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_773 (0xc14+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_774
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_774 (0xc18+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_775
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_775 (0xc1c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_776
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_776 (0xc20+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_777
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_777 (0xc24+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_778
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_778 (0xc28+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_779
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_779 (0xc2c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_780
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_780 (0xc30+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_781
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_781 (0xc34+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_782
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_782 (0xc38+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_783
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_783 (0xc3c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_784
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_784 (0xc40+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_785
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_785 (0xc44+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_786
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_786 (0xc48+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_787
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_787 (0xc4c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_788
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_788 (0xc50+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_789
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_789 (0xc54+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_790
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_790 (0xc58+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_791
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_791 (0xc5c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_792
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_792 (0xc60+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_793
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_793 (0xc64+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_794
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_794 (0xc68+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_795
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_795 (0xc6c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_796
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_796 (0xc70+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_797
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_797 (0xc74+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_798
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_798 (0xc78+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_799
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_799 (0xc7c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_800
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_800 (0xc80+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_801
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_801 (0xc84+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_802
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_802 (0xc88+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_803
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_803 (0xc8c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_804
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_804 (0xc90+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_805
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_805 (0xc94+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_806
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_806 (0xc98+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_807
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_807 (0xc9c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_808
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_808 (0xca0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_809
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_809 (0xca4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_810
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_810 (0xca8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_811
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_811 (0xcac+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_812
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_812 (0xcb0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_813
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_813 (0xcb4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_814
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_814 (0xcb8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_815
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_815 (0xcbc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_816
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_816 (0xcc0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_817
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_817 (0xcc4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_818
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_818 (0xcc8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_819
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_819 (0xccc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_820
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_820 (0xcd0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_821
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_821 (0xcd4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_822
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_822 (0xcd8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_823
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_823 (0xcdc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_824
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_824 (0xce0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_825
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_825 (0xce4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_826
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_826 (0xce8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_827
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_827 (0xcec+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_828
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_828 (0xcf0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_829
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_829 (0xcf4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_830
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_830 (0xcf8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_831
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_831 (0xcfc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_832
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_832 (0xd00+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_833
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_833 (0xd04+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_834
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_834 (0xd08+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_835
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_835 (0xd0c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_836
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_836 (0xd10+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_837
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_837 (0xd14+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_838
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_838 (0xd18+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_839
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_839 (0xd1c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_840
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_840 (0xd20+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_841
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_841 (0xd24+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_842
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_842 (0xd28+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_843
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_843 (0xd2c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_844
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_844 (0xd30+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_845
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_845 (0xd34+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_846
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_846 (0xd38+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_847
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_847 (0xd3c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_848
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_848 (0xd40+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_849
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_849 (0xd44+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_850
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_850 (0xd48+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_851
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_851 (0xd4c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_852
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_852 (0xd50+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_853
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_853 (0xd54+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_854
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_854 (0xd58+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_855
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_855 (0xd5c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_856
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_856 (0xd60+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_857
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_857 (0xd64+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_858
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_858 (0xd68+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_859
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_859 (0xd6c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_860
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_860 (0xd70+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_861
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_861 (0xd74+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_862
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_862 (0xd78+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_863
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_863 (0xd7c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_864
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_864 (0xd80+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_865
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_865 (0xd84+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_866
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_866 (0xd88+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_867
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_867 (0xd8c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_868
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_868 (0xd90+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_869
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_869 (0xd94+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_870
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_870 (0xd98+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_871
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_871 (0xd9c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_872
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_872 (0xda0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_873
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_873 (0xda4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_874
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_874 (0xda8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_875
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_875 (0xdac+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_876
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_876 (0xdb0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_877
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_877 (0xdb4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_878
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_878 (0xdb8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_879
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_879 (0xdbc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_880
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_880 (0xdc0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_881
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_881 (0xdc4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_882
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_882 (0xdc8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_883
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_883 (0xdcc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_884
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_884 (0xdd0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_885
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_885 (0xdd4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_886
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_886 (0xdd8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_887
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_887 (0xddc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_888
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_888 (0xde0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_889
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_889 (0xde4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_890
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_890 (0xde8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_891
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_891 (0xdec+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_892
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_892 (0xdf0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_893
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_893 (0xdf4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_894
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_894 (0xdf8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_895
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_895 (0xdfc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_896
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_896 (0xe00+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_897
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_897 (0xe04+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_898
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_898 (0xe08+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_899
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_899 (0xe0c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_900
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_900 (0xe10+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_901
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_901 (0xe14+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_902
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_902 (0xe18+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_903
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_903 (0xe1c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_904
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_904 (0xe20+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_905
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_905 (0xe24+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_906
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_906 (0xe28+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_907
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_907 (0xe2c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_908
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_908 (0xe30+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_909
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_909 (0xe34+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_910
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_910 (0xe38+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_911
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_911 (0xe3c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_912
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_912 (0xe40+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_913
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_913 (0xe44+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_914
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_914 (0xe48+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_915
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_915 (0xe4c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_916
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_916 (0xe50+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_917
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_917 (0xe54+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_918
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_918 (0xe58+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_919
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_919 (0xe5c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_920
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_920 (0xe60+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_921
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_921 (0xe64+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_922
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_922 (0xe68+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_923
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_923 (0xe6c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_924
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_924 (0xe70+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_925
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_925 (0xe74+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_926
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_926 (0xe78+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_927
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_927 (0xe7c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_928
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_928 (0xe80+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_929
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_929 (0xe84+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_930
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_930 (0xe88+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_931
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_931 (0xe8c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_932
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_932 (0xe90+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_933
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_933 (0xe94+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_934
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_934 (0xe98+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_935
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_935 (0xe9c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_936
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_936 (0xea0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_937
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_937 (0xea4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_938
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_938 (0xea8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_939
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_939 (0xeac+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_940
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_940 (0xeb0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_941
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_941 (0xeb4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_942
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_942 (0xeb8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_943
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_943 (0xebc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_944
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_944 (0xec0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_945
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_945 (0xec4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_946
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_946 (0xec8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_947
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_947 (0xecc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_948
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_948 (0xed0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_949
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_949 (0xed4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_950
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_950 (0xed8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_951
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_951 (0xedc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_952
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_952 (0xee0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_953
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_953 (0xee4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_954
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_954 (0xee8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_955
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_955 (0xeec+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_956
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_956 (0xef0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_957
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_957 (0xef4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_958
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_958 (0xef8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_959
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_959 (0xefc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_960
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_960 (0xf00+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_961
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_961 (0xf04+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_962
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_962 (0xf08+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_963
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_963 (0xf0c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_964
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_964 (0xf10+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_965
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_965 (0xf14+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_966
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_966 (0xf18+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_967
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_967 (0xf1c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_968
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_968 (0xf20+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_969
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_969 (0xf24+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_970
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_970 (0xf28+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_971
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_971 (0xf2c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_972
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_972 (0xf30+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_973
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_973 (0xf34+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_974
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_974 (0xf38+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_975
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_975 (0xf3c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_976
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_976 (0xf40+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_977
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_977 (0xf44+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_978
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_978 (0xf48+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_979
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_979 (0xf4c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_980
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_980 (0xf50+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_981
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_981 (0xf54+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_982
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_982 (0xf58+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_983
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_983 (0xf5c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_984
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_984 (0xf60+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_985
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_985 (0xf64+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_986
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_986 (0xf68+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_987
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_987 (0xf6c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_988
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_988 (0xf70+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_989
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_989 (0xf74+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_990
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_990 (0xf78+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_991
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_991 (0xf7c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_992
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_992 (0xf80+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_993
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_993 (0xf84+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_994
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_994 (0xf88+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_995
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_995 (0xf8c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_996
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_996 (0xf90+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_997
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_997 (0xf94+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_998
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_998 (0xf98+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_999
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_999 (0xf9c+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1000
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1000 (0xfa0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1001
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1001 (0xfa4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1002
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1002 (0xfa8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1003
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1003 (0xfac+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1004
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1004 (0xfb0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1005
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1005 (0xfb4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1006
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1006 (0xfb8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1007
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1007 (0xfbc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1008
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1008 (0xfc0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1009
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1009 (0xfc4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1010
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1010 (0xfc8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1011
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1011 (0xfcc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1012
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1012 (0xfd0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1013
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1013 (0xfd4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1014
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1014 (0xfd8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1015
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1015 (0xfdc+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1016
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1016 (0xfe0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1017
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1017 (0xfe4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1018
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1018 (0xfe8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1019
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1019 (0xfec+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1020
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1020 (0xff0+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1021
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1021 (0xff4+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1022
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1022 (0xff8+(5<<12))
#undef H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1023
#define H2S_CSR_G5_TRACER_RAM_1024X_32BIT_1023 (0xffc+(5<<12))
#ifndef __ASSEMBLER__
struct dnc_csr_g5_tracer_ram_1024x_32bit {
	unsigned _iic: 1;
	unsigned : 1;
	unsigned : 3;
	unsigned _smbus: 7;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 4;
	unsigned : 1;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 8;
	unsigned : 1;
	unsigned : 3;
	unsigned : 4;
	unsigned : 32;
};
#endif
#define H2S_RESET_CSR_G2(a) (a)[0x800/4] = 0x00000000;\
(a)[0x804/4] = 0x00000000;\
(a)[0x808/4] = 0x00000000;\
(a)[0x80C/4] = 0x00000000;\
(a)[0x810/4] = 0x00000000;\
(a)[0x818/4] = 0x001FFFFF;\
(a)[0x81C/4] = 0x00000000;\
(a)[0x820/4] = 0x00F7F7F7;\
(a)[0x828/4] = 0x00000A0F
#define H2S_RESET_CSR_F0(a) (a)[0x00/4] = 0x06011B47;\
(a)[0x04/4] = 0x00100000;\
(a)[0x08/4] = 0x06000000;\
(a)[0x0C/4] = 0x00800000;\
(a)[0x10/4] = 0x00000000;\
(a)[0x14/4] = 0x00000000;\
(a)[0x18/4] = 0x00000000;\
(a)[0x1C/4] = 0x00000000;\
(a)[0x20/4] = 0x00000000;\
(a)[0x24/4] = 0x00000000;\
(a)[0x28/4] = 0x00000000;\
(a)[0x2C/4] = 0x00000000;\
(a)[0x30/4] = 0x00000000;\
(a)[0x34/4] = 0x00000080;\
(a)[0x38/4] = 0x00000000;\
(a)[0x3C/4] = 0x00000000;\
(a)[0x80/4] = 0x2101A408;\
(a)[0x84/4] = 0x77110000;\
(a)[0x88/4] = 0x00210022;\
(a)[0x8C/4] = 0x00000013;\
(a)[0xA4/4] = 0x4B00C408;\
(a)[0xA8/4] = 0x048A9944;\
(a)[0xAC/4] = 0x00000000;\
(a)[0xB0/4] = 0x00000010;\
(a)[0xC4/4] = 0x4A00E808;\
(a)[0xC8/4] = 0x00000007;\
(a)[0xCC/4] = 0x00000000;\
(a)[0xD0/4] = 0x000000E0;\
(a)[0xD4/4] = 0x00000000;\
(a)[0xD8/4] = 0x00000001;\
(a)[0xDC/4] = 0x00000004;\
(a)[0xE8/4] = 0x49040008
#define H2S_RESET_CSR_G3(a) (a)[0x008/4] = 0x00000000;\
(a)[0x00C/4] = 0xFFFFFFFF;\
(a)[0x010/4] = 0x00FF0000;\
(a)[0x014/4] = 0x000000C8;\
(a)[0x018/4] = 0x00000001;\
(a)[0x01C/4] = 0x00000000;\
(a)[0x020/4] = 0x00000000;\
(a)[0x024/4] = 0x00000007;\
(a)[0x028/4] = 0x00000000;\
(a)[0x02C/4] = 0x00000000;\
(a)[0x030/4] = 0x00000000;\
(a)[0x034/4] = 0x00000000;\
(a)[0x038/4] = 0x00000000;\
(a)[0x400/4] = 0x00020000;\
(a)[0x404/4] = 0x00000000;\
(a)[0x408/4] = 0x00000000;\
(a)[0x40C/4] = 0x00000000;\
(a)[0x410/4] = 0x00000000;\
(a)[0x418/4] = 0x00006666;\
(a)[0x41C/4] = 0x00000003;\
(a)[0x420/4] = 0xFFFCFFF8;\
(a)[0x424/4] = 0x00000000;\
(a)[0x428/4] = 0x00000000;\
(a)[0x428/4] = 0x00000000;\
(a)[0x42C/4] = 0x00000000;\
(a)[0x430/4] = 0xFFF4FFD0;\
(a)[0x7F8/4] = 0x00000000;\
(a)[0x7FC/4] = 0x00000000;\
(a)[0x800/4] = 0x00000000;\
(a)[0x804/4] = 0x00000000;\
(a)[0x808/4] = 0x00000000;\
(a)[0x80c/4] = 0x00000000;\
(a)[0x810/4] = 0x00000000;\
(a)[0x814/4] = 0x00000000;\
(a)[0x818/4] = 0x00000000;\
(a)[0x81c/4] = 0x00000000;\
(a)[0x820/4] = 0x00000000;\
(a)[0x824/4] = 0x00000000;\
(a)[0x828/4] = 0x00000000;\
(a)[0x82c/4] = 0x00000000;\
(a)[0x830/4] = 0x00000000;\
(a)[0x834/4] = 0x00000000;\
(a)[0x838/4] = 0x00000000;\
(a)[0x83c/4] = 0x00000000;\
(a)[0x840/4] = 0x00000000;\
(a)[0x844/4] = 0x00000000;\
(a)[0x848/4] = 0x00000000;\
(a)[0x84c/4] = 0x00000000;\
(a)[0x850/4] = 0x00000000;\
(a)[0x854/4] = 0x00000000;\
(a)[0x858/4] = 0x00000000;\
(a)[0x85c/4] = 0x00000000;\
(a)[0x860/4] = 0x00000000;\
(a)[0x864/4] = 0x00000000;\
(a)[0x868/4] = 0x00000000;\
(a)[0x86c/4] = 0x00000000;\
(a)[0x870/4] = 0x00000000;\
(a)[0x874/4] = 0x00000000;\
(a)[0x878/4] = 0x00000000;\
(a)[0x87c/4] = 0x00000000;\
(a)[0x880/4] = 0x00000000;\
(a)[0x884/4] = 0x00000000;\
(a)[0x888/4] = 0x00000000;\
(a)[0x88c/4] = 0x00000000;\
(a)[0x890/4] = 0x00000000;\
(a)[0x894/4] = 0x00000000;\
(a)[0x898/4] = 0x00000000;\
(a)[0x89c/4] = 0x00000000;\
(a)[0x8a0/4] = 0x00000000;\
(a)[0x8a4/4] = 0x00000000;\
(a)[0x8a8/4] = 0x00000000;\
(a)[0x8ac/4] = 0x00000000;\
(a)[0x8b0/4] = 0x00000000;\
(a)[0x8b4/4] = 0x00000000;\
(a)[0x8b8/4] = 0x00000000;\
(a)[0x8bc/4] = 0x00000000;\
(a)[0x8c0/4] = 0x00000000;\
(a)[0x8c4/4] = 0x00000000;\
(a)[0x8c8/4] = 0x00000000;\
(a)[0x8cc/4] = 0x00000000;\
(a)[0x8d0/4] = 0x00000000;\
(a)[0x8d4/4] = 0x00000000;\
(a)[0x8d8/4] = 0x00000000;\
(a)[0x8dc/4] = 0x00000000;\
(a)[0x8e0/4] = 0x00000000;\
(a)[0x8e4/4] = 0x00000000;\
(a)[0x8e8/4] = 0x00000000;\
(a)[0x8ec/4] = 0x00000000;\
(a)[0x8f0/4] = 0x00000000;\
(a)[0x8f4/4] = 0x00000000;\
(a)[0x8f8/4] = 0x00000000;\
(a)[0x8fc/4] = 0x00000000;\
(a)[0x900/4] = 0x00000000;\
(a)[0x904/4] = 0x00000000;\
(a)[0x908/4] = 0x00000000;\
(a)[0x90c/4] = 0x00000000;\
(a)[0x910/4] = 0x00000000;\
(a)[0x914/4] = 0x00000000;\
(a)[0x918/4] = 0x00000000;\
(a)[0x91c/4] = 0x00000000;\
(a)[0x920/4] = 0x00000000;\
(a)[0x924/4] = 0x00000000;\
(a)[0x928/4] = 0x00000000;\
(a)[0x92c/4] = 0x00000000;\
(a)[0x930/4] = 0x00000000;\
(a)[0x934/4] = 0x00000000;\
(a)[0x938/4] = 0x00000000;\
(a)[0x93c/4] = 0x00000000;\
(a)[0x940/4] = 0x00000000;\
(a)[0x944/4] = 0x00000000;\
(a)[0x948/4] = 0x00000000;\
(a)[0x94c/4] = 0x00000000;\
(a)[0x950/4] = 0x00000000;\
(a)[0x954/4] = 0x00000000;\
(a)[0x958/4] = 0x00000000;\
(a)[0x95c/4] = 0x00000000;\
(a)[0x960/4] = 0x00000000;\
(a)[0x964/4] = 0x00000000;\
(a)[0x968/4] = 0x00000000;\
(a)[0x96c/4] = 0x00000000;\
(a)[0x970/4] = 0x00000000;\
(a)[0x974/4] = 0x00000000;\
(a)[0x978/4] = 0x00000000;\
(a)[0x97c/4] = 0x00000000;\
(a)[0x980/4] = 0x00000000;\
(a)[0x984/4] = 0x00000000;\
(a)[0x988/4] = 0x00000000;\
(a)[0x98c/4] = 0x00000000;\
(a)[0x990/4] = 0x00000000;\
(a)[0x994/4] = 0x00000000;\
(a)[0x998/4] = 0x00000000;\
(a)[0x99c/4] = 0x00000000;\
(a)[0x9a0/4] = 0x00000000;\
(a)[0x9a4/4] = 0x00000000;\
(a)[0x9a8/4] = 0x00000000;\
(a)[0x9ac/4] = 0x00000000;\
(a)[0x9b0/4] = 0x00000000;\
(a)[0x9b4/4] = 0x00000000;\
(a)[0x9b8/4] = 0x00000000;\
(a)[0x9bc/4] = 0x00000000;\
(a)[0x9c0/4] = 0x00000000;\
(a)[0x9c4/4] = 0x00000000;\
(a)[0x9c8/4] = 0x00000000;\
(a)[0x9cc/4] = 0x00000000;\
(a)[0x9d0/4] = 0x00000000;\
(a)[0x9d4/4] = 0x00000000;\
(a)[0x9d8/4] = 0x00000000;\
(a)[0x9dc/4] = 0x00000000;\
(a)[0x9e0/4] = 0x00000000;\
(a)[0x9e4/4] = 0x00000000;\
(a)[0x9e8/4] = 0x00000000;\
(a)[0x9ec/4] = 0x00000000;\
(a)[0x9f0/4] = 0x00000000;\
(a)[0x9f4/4] = 0x00000000;\
(a)[0x9f8/4] = 0x00000000;\
(a)[0x9fc/4] = 0x00000000;\
(a)[0xa00/4] = 0x00000000;\
(a)[0xa04/4] = 0x00000000;\
(a)[0xa08/4] = 0x00000000;\
(a)[0xa0c/4] = 0x00000000;\
(a)[0xa10/4] = 0x00000000;\
(a)[0xa14/4] = 0x00000000;\
(a)[0xa18/4] = 0x00000000;\
(a)[0xa1c/4] = 0x00000000;\
(a)[0xa20/4] = 0x00000000;\
(a)[0xa24/4] = 0x00000000;\
(a)[0xa28/4] = 0x00000000;\
(a)[0xa2c/4] = 0x00000000;\
(a)[0xa30/4] = 0x00000000;\
(a)[0xa34/4] = 0x00000000;\
(a)[0xa38/4] = 0x00000000;\
(a)[0xa3c/4] = 0x00000000;\
(a)[0xa40/4] = 0x00000000;\
(a)[0xa44/4] = 0x00000000;\
(a)[0xa48/4] = 0x00000000;\
(a)[0xa4c/4] = 0x00000000;\
(a)[0xa50/4] = 0x00000000;\
(a)[0xa54/4] = 0x00000000;\
(a)[0xa58/4] = 0x00000000;\
(a)[0xa5c/4] = 0x00000000;\
(a)[0xa60/4] = 0x00000000;\
(a)[0xa64/4] = 0x00000000;\
(a)[0xa68/4] = 0x00000000;\
(a)[0xa6c/4] = 0x00000000;\
(a)[0xa70/4] = 0x00000000;\
(a)[0xa74/4] = 0x00000000;\
(a)[0xa78/4] = 0x00000000;\
(a)[0xa7c/4] = 0x00000000;\
(a)[0xa80/4] = 0x00000000;\
(a)[0xa84/4] = 0x00000000;\
(a)[0xa88/4] = 0x00000000;\
(a)[0xa8c/4] = 0x00000000;\
(a)[0xa90/4] = 0x00000000;\
(a)[0xa94/4] = 0x00000000;\
(a)[0xa98/4] = 0x00000000;\
(a)[0xa9c/4] = 0x00000000;\
(a)[0xaa0/4] = 0x00000000;\
(a)[0xaa4/4] = 0x00000000;\
(a)[0xaa8/4] = 0x00000000;\
(a)[0xaac/4] = 0x00000000;\
(a)[0xab0/4] = 0x00000000;\
(a)[0xab4/4] = 0x00000000;\
(a)[0xab8/4] = 0x00000000;\
(a)[0xabc/4] = 0x00000000;\
(a)[0xac0/4] = 0x00000000;\
(a)[0xac4/4] = 0x00000000;\
(a)[0xac8/4] = 0x00000000;\
(a)[0xacc/4] = 0x00000000;\
(a)[0xad0/4] = 0x00000000;\
(a)[0xad4/4] = 0x00000000;\
(a)[0xad8/4] = 0x00000000;\
(a)[0xadc/4] = 0x00000000;\
(a)[0xae0/4] = 0x00000000;\
(a)[0xae4/4] = 0x00000000;\
(a)[0xae8/4] = 0x00000000;\
(a)[0xaec/4] = 0x00000000;\
(a)[0xaf0/4] = 0x00000000;\
(a)[0xaf4/4] = 0x00000000;\
(a)[0xaf8/4] = 0x00000000;\
(a)[0xafc/4] = 0x00000000;\
(a)[0xb00/4] = 0x00000000;\
(a)[0xb04/4] = 0x00000000;\
(a)[0xb08/4] = 0x00000000;\
(a)[0xb0c/4] = 0x00000000;\
(a)[0xb10/4] = 0x00000000;\
(a)[0xb14/4] = 0x00000000;\
(a)[0xb18/4] = 0x00000000;\
(a)[0xb1c/4] = 0x00000000;\
(a)[0xb20/4] = 0x00000000;\
(a)[0xb24/4] = 0x00000000;\
(a)[0xb28/4] = 0x00000000;\
(a)[0xb2c/4] = 0x00000000;\
(a)[0xb30/4] = 0x00000000;\
(a)[0xb34/4] = 0x00000000;\
(a)[0xb38/4] = 0x00000000;\
(a)[0xb3c/4] = 0x00000000;\
(a)[0xb40/4] = 0x00000000;\
(a)[0xb44/4] = 0x00000000;\
(a)[0xb48/4] = 0x00000000;\
(a)[0xb4c/4] = 0x00000000;\
(a)[0xb50/4] = 0x00000000;\
(a)[0xb54/4] = 0x00000000;\
(a)[0xb58/4] = 0x00000000;\
(a)[0xb5c/4] = 0x00000000;\
(a)[0xb60/4] = 0x00000000;\
(a)[0xb64/4] = 0x00000000;\
(a)[0xb68/4] = 0x00000000;\
(a)[0xb6c/4] = 0x00000000;\
(a)[0xb70/4] = 0x00000000;\
(a)[0xb74/4] = 0x00000000;\
(a)[0xb78/4] = 0x00000000;\
(a)[0xb7c/4] = 0x00000000;\
(a)[0xb80/4] = 0x00000000;\
(a)[0xb84/4] = 0x00000000;\
(a)[0xb88/4] = 0x00000000;\
(a)[0xb8c/4] = 0x00000000;\
(a)[0xb90/4] = 0x00000000;\
(a)[0xb94/4] = 0x00000000;\
(a)[0xb98/4] = 0x00000000;\
(a)[0xb9c/4] = 0x00000000;\
(a)[0xba0/4] = 0x00000000;\
(a)[0xba4/4] = 0x00000000;\
(a)[0xba8/4] = 0x00000000;\
(a)[0xbac/4] = 0x00000000;\
(a)[0xbb0/4] = 0x00000000;\
(a)[0xbb4/4] = 0x00000000;\
(a)[0xbb8/4] = 0x00000000;\
(a)[0xbbc/4] = 0x00000000;\
(a)[0xbc0/4] = 0x00000000;\
(a)[0xbc4/4] = 0x00000000;\
(a)[0xbc8/4] = 0x00000000;\
(a)[0xbcc/4] = 0x00000000;\
(a)[0xbd0/4] = 0x00000000;\
(a)[0xbd4/4] = 0x00000000;\
(a)[0xbd8/4] = 0x00000000;\
(a)[0xbdc/4] = 0x00000000;\
(a)[0xbe0/4] = 0x00000000;\
(a)[0xbe4/4] = 0x00000000;\
(a)[0xbe8/4] = 0x00000000;\
(a)[0xbec/4] = 0x00000000;\
(a)[0xbf0/4] = 0x00000000;\
(a)[0xbf4/4] = 0x00000000;\
(a)[0xbf8/4] = 0x00000000;\
(a)[0xbfc/4] = 0x00000000;\
(a)[0xC00/4] = 0x00000000;\
(a)[0xC04/4] = 0x00000000;\
(a)[0xC08/4] = 0x00000001;\
(a)[0xC0C/4] = 0x00000000;\
(a)[0xC10/4] = 0x00000000;\
(a)[0xC14/4] = 0x00000000;\
(a)[0xF78/4] = 0x00000000;\
(a)[0xF98/4] = 0x00000000;\
(a)[0xF9C/4] = 0x00000001;\
(a)[0xFA0/4] = 0x00000000;\
(a)[0xFA4/4] = 0x00000000;\
(a)[0xFA8/4] = 0x00000000;\
(a)[0xFAC/4] = 0x00000000;\
(a)[0xFB0/4] = 0x00000000;\
(a)[0xFB4/4] = 0x00000000;\
(a)[0xFB8/4] = 0x00000000;\
(a)[0xFBC/4] = 0x00000000;\
(a)[0xFC0/4] = 0x00000000;\
(a)[0xFC4/4] = 0x00000000;\
(a)[0xFC8/4] = 0x00000000;\
(a)[0xFCC/4] = 0x00000000;\
(a)[0xFD0/4] = 0x00000000;\
(a)[0xFD4/4] = 0x00000000;\
(a)[0xFD8/4] = 0x00000000;\
(a)[0xFDC/4] = 0x00000000;\
(a)[0xFE0/4] = 0x00000000;\
(a)[0xFE4/4] = 0x00000000;\
(a)[0xFE8/4] = 0x00000000;\
(a)[0xFEC/4] = 0x00000000;\
(a)[0xFF0/4] = 0x00000000;\
(a)[0xFF4/4] = 0x00000000;\
(a)[0xFF8/4] = 0x00000000;\
(a)[0xFFC/4] = 0x00000000
#define H2S_RESET_CSR_F1(a) (a)[0x00/4] = 0x06021B47;\
(a)[0x04/4] = 0x00100000;\
(a)[0x08/4] = 0x06000000;\
(a)[0x0C/4] = 0x00800000;\
(a)[0x10/4] = 0x00000000;\
(a)[0x14/4] = 0x00000000;\
(a)[0x18/4] = 0x00000000;\
(a)[0x1C/4] = 0x00000000;\
(a)[0x20/4] = 0x00000000;\
(a)[0x24/4] = 0x00000000;\
(a)[0x28/4] = 0x00000000;\
(a)[0x2C/4] = 0x00000000;\
(a)[0x30/4] = 0x00000000;\
(a)[0x34/4] = 0x00000040;\
(a)[0x38/4] = 0x00000000;\
(a)[0x3C/4] = 0x00000000;\
(a)[0x40/4] = 0x4C000008;\
(a)[0x44/4] = 0x00000000;\
(a)[0x48/4] = 0x00000000;\
(a)[0x4C/4] = 0x00000000;\
(a)[0x50/4] = 0x00000000;\
(a)[0x54/4] = 0x00000000;\
(a)[0x58/4] = 0x00000000;\
(a)[0x5C/4] = 0x00000000;\
(a)[0x60/4] = 0x00000000;\
(a)[0x64/4] = 0x00000000;\
(a)[0x68/4] = 0x00000000
#define H2S_RESET_CSR_G0(a) (a)[0xA00/4] = 0x00000000;\
(a)[0xA04/4] = 0x00000000;\
(a)[0xA08/4] = 0x00000000;\
(a)[0xA0C/4] = 0x00000FF0;\
(a)[0xA10/4] = 0x00000000;\
(a)[0xA14/4] = 0x00000000;\
(a)[0xA18/4] = 0x00000000;\
(a)[0xA1C/4] = 0x00000000;\
(a)[0xA20/4] = 0x00000FF0;\
(a)[0xA24/4] = 0x00000000;\
(a)[0xA28/4] = 0x00000000;\
(a)[0xA2C/4] = 0x00000000;\
(a)[0xA30/4] = 0x00000000;\
(a)[0xA40/4] = 0x00000000;\
(a)[0xA44/4] = 0x00000000;\
(a)[0xA48/4] = 0x00000000;\
(a)[0xA4C/4] = 0x00000FF0;\
(a)[0xA50/4] = 0x00000000;\
(a)[0xA54/4] = 0x00000000;\
(a)[0xA58/4] = 0x00000000;\
(a)[0xA5C/4] = 0x00000000;\
(a)[0xA60/4] = 0x00000FF0;\
(a)[0xA64/4] = 0x00000000;\
(a)[0xA68/4] = 0x00000000;\
(a)[0xA6C/4] = 0x00000000;\
(a)[0xA70/4] = 0x00000000;\
(a)[0xA80/4] = 0x00000000;\
(a)[0xA84/4] = 0x00000000;\
(a)[0xA88/4] = 0x00000000;\
(a)[0xA8C/4] = 0x00000FF0;\
(a)[0xA90/4] = 0x00000000;\
(a)[0xA94/4] = 0x00000000;\
(a)[0xA98/4] = 0x00000000;\
(a)[0xA9C/4] = 0x00000000;\
(a)[0xAA0/4] = 0x00000FF0;\
(a)[0xAA4/4] = 0x00000000;\
(a)[0xAA8/4] = 0x00000000;\
(a)[0xAAC/4] = 0x00000000;\
(a)[0xAB0/4] = 0x00000000;\
(a)[0xAC0/4] = 0x00000000;\
(a)[0xAC4/4] = 0x00000000;\
(a)[0xAC8/4] = 0x00000000;\
(a)[0xACC/4] = 0x00000FF0;\
(a)[0xAD0/4] = 0x00000000;\
(a)[0xAD4/4] = 0x00000000;\
(a)[0xAD8/4] = 0x00000000;\
(a)[0xADC/4] = 0x00000000;\
(a)[0xAE0/4] = 0x00000FF0;\
(a)[0xAE4/4] = 0x00000000;\
(a)[0xAE8/4] = 0x00000000;\
(a)[0xAEC/4] = 0x00000000;\
(a)[0xAF0/4] = 0x00000000;\
(a)[0xB00/4] = 0x00000000;\
(a)[0xB04/4] = 0x00000000;\
(a)[0xB08/4] = 0x00000000;\
(a)[0xB0C/4] = 0x00000FF0;\
(a)[0xB10/4] = 0x00000000;\
(a)[0xB14/4] = 0x00000000;\
(a)[0xB18/4] = 0x00000000;\
(a)[0xB1C/4] = 0x00000000;\
(a)[0xB20/4] = 0x00000FF0;\
(a)[0xB24/4] = 0x00000000;\
(a)[0xB28/4] = 0x00000000;\
(a)[0xB2C/4] = 0x00000000;\
(a)[0xB30/4] = 0x00000000;\
(a)[0xB40/4] = 0x00000000;\
(a)[0xB44/4] = 0x00000000;\
(a)[0xB48/4] = 0x00000000;\
(a)[0xB4C/4] = 0x00000FF0;\
(a)[0xB50/4] = 0x00000000;\
(a)[0xB54/4] = 0x00000000;\
(a)[0xB58/4] = 0x00000000;\
(a)[0xB5C/4] = 0x00000000;\
(a)[0xB60/4] = 0x00000FF0;\
(a)[0xB64/4] = 0x00000000;\
(a)[0xB68/4] = 0x00000000;\
(a)[0xB6C/4] = 0x00000000;\
(a)[0xB70/4] = 0x00000000;\
(a)[0xB80/4] = 0x00003454;\
(a)[0xB84/4] = 0x0000027E
#define H2S_RESET_CSR_G4(a) (a)[0x700/4] = 0x00000007;\
(a)[0x704/4] = 0x00000000;\
(a)[0x708/4] = 0x00000000;\
(a)[0x70C/4] = 0x00000000;\
(a)[0x710/4] = 0x0000003F;\
(a)[0x714/4] = 0x00000000;\
(a)[0x718/4] = 0x00000000;\
(a)[0x71C/4] = 0x00000000;\
(a)[0x720/4] = 0x00000000;\
(a)[0x724/4] = 0x00000000;\
(a)[0xF00/4] = 0x00000007;\
(a)[0xF04/4] = 0x00000000;\
(a)[0xF08/4] = 0x00000000;\
(a)[0xF0C/4] = 0x00000000;\
(a)[0xF10/4] = 0x0000003F;\
(a)[0xF14/4] = 0x00000000;\
(a)[0xF18/4] = 0x00000000;\
(a)[0xF1C/4] = 0x00000000;\
(a)[0xF20/4] = 0x00000000;\
(a)[0xF24/4] = 0x00000000
#define H2S_MASK_CSR_G2(a) (a)[0x800/4] = 0xFFFFFF00;\
(a)[0x804/4] = 0xFFFFFF00;\
(a)[0x808/4] = 0xFFFFFF00;\
(a)[0x80C/4] = 0xFFC00000;\
(a)[0x818/4] = 0xFFC00000;\
(a)[0x81C/4] = 0xFFFFFFFF;\
(a)[0x824/4] = 0xFFFFFFFF;\
(a)[0x828/4] = 0xFFFF0000
#define H2S_MASK_CSR_F0(a) (a)[0x00/4] = 0xFFFFFFFF;\
(a)[0x04/4] = 0xFFFFFFFD;\
(a)[0x08/4] = 0xFFFFFFFF;\
(a)[0x0C/4] = 0xFFFFFFFF;\
(a)[0x10/4] = 0x000FFFFF;\
(a)[0x14/4] = 0xFFFFFFFF;\
(a)[0x18/4] = 0xFFFFFFFF;\
(a)[0x1C/4] = 0xFFFFFFFF;\
(a)[0x20/4] = 0xFFFFFFFF;\
(a)[0x24/4] = 0xFFFFFFFF;\
(a)[0x28/4] = 0xFFFFFFFF;\
(a)[0x2C/4] = 0xFFFFFFFF;\
(a)[0x30/4] = 0x0000FFFE;\
(a)[0x34/4] = 0xFFFFFFFF;\
(a)[0x38/4] = 0xFFFFFFFF;\
(a)[0x3C/4] = 0xFFFFFFFF;\
(a)[0x80/4] = 0xFFFFFFFF;\
(a)[0x84/4] = 0x88FF0FF5;\
(a)[0x88/4] = 0xFFFFF0FF;\
(a)[0x8C/4] = 0xFFFFFFFF;\
(a)[0xA4/4] = 0xFFFFFFFF;\
(a)[0xA8/4] = 0x70000000;\
(a)[0xAC/4] = 0xE000FFFF;\
(a)[0xB0/4] = 0xFFFFFFFF;\
(a)[0xC4/4] = 0xFFFFFFFF;\
(a)[0xC8/4] = 0xF8F8F8F8;\
(a)[0xCC/4] = 0xFFFF0000;\
(a)[0xD0/4] = 0xFFFFFFFF;\
(a)[0xD4/4] = 0xFE61F4EF;\
(a)[0xD8/4] = 0xFFFFFFFE;\
(a)[0xDC/4] = 0xFFFFFFFF;\
(a)[0xE8/4] = 0xFFFFFFFF
#define H2S_MASK_CSR_G5(a) (a)[0x0/4] = 0x0000FE01;\
(a)[0x4/4] = 0x0000FE01;\
(a)[0x8/4] = 0x0000FE01;\
(a)[0xc/4] = 0x0000FE01;\
(a)[0x10/4] = 0x0000FE01;\
(a)[0x14/4] = 0x0000FE01;\
(a)[0x18/4] = 0x0000FE01;\
(a)[0x1c/4] = 0x0000FE01;\
(a)[0x20/4] = 0x0000FE01;\
(a)[0x24/4] = 0x0000FE01;\
(a)[0x28/4] = 0x0000FE01;\
(a)[0x2c/4] = 0x0000FE01;\
(a)[0x30/4] = 0x0000FE01;\
(a)[0x34/4] = 0x0000FE01;\
(a)[0x38/4] = 0x0000FE01;\
(a)[0x3c/4] = 0x0000FE01;\
(a)[0x40/4] = 0x0000FE01;\
(a)[0x44/4] = 0x0000FE01;\
(a)[0x48/4] = 0x0000FE01;\
(a)[0x4c/4] = 0x0000FE01;\
(a)[0x50/4] = 0x0000FE01;\
(a)[0x54/4] = 0x0000FE01;\
(a)[0x58/4] = 0x0000FE01;\
(a)[0x5c/4] = 0x0000FE01;\
(a)[0x60/4] = 0x0000FE01;\
(a)[0x64/4] = 0x0000FE01;\
(a)[0x68/4] = 0x0000FE01;\
(a)[0x6c/4] = 0x0000FE01;\
(a)[0x70/4] = 0x0000FE01;\
(a)[0x74/4] = 0x0000FE01;\
(a)[0x78/4] = 0x0000FE01;\
(a)[0x7c/4] = 0x0000FE01;\
(a)[0x80/4] = 0x0000FE01;\
(a)[0x84/4] = 0x0000FE01;\
(a)[0x88/4] = 0x0000FE01;\
(a)[0x8c/4] = 0x0000FE01;\
(a)[0x90/4] = 0x0000FE01;\
(a)[0x94/4] = 0x0000FE01;\
(a)[0x98/4] = 0x0000FE01;\
(a)[0x9c/4] = 0x0000FE01;\
(a)[0xa0/4] = 0x0000FE01;\
(a)[0xa4/4] = 0x0000FE01;\
(a)[0xa8/4] = 0x0000FE01;\
(a)[0xac/4] = 0x0000FE01;\
(a)[0xb0/4] = 0x0000FE01;\
(a)[0xb4/4] = 0x0000FE01;\
(a)[0xb8/4] = 0x0000FE01;\
(a)[0xbc/4] = 0x0000FE01;\
(a)[0xc0/4] = 0x0000FE01;\
(a)[0xc4/4] = 0x0000FE01;\
(a)[0xc8/4] = 0x0000FE01;\
(a)[0xcc/4] = 0x0000FE01;\
(a)[0xd0/4] = 0x0000FE01;\
(a)[0xd4/4] = 0x0000FE01;\
(a)[0xd8/4] = 0x0000FE01;\
(a)[0xdc/4] = 0x0000FE01;\
(a)[0xe0/4] = 0x0000FE01;\
(a)[0xe4/4] = 0x0000FE01;\
(a)[0xe8/4] = 0x0000FE01;\
(a)[0xec/4] = 0x0000FE01;\
(a)[0xf0/4] = 0x0000FE01;\
(a)[0xf4/4] = 0x0000FE01;\
(a)[0xf8/4] = 0x0000FE01;\
(a)[0xfc/4] = 0x0000FE01;\
(a)[0x100/4] = 0x0000FE01;\
(a)[0x104/4] = 0x0000FE01;\
(a)[0x108/4] = 0x0000FE01;\
(a)[0x10c/4] = 0x0000FE01;\
(a)[0x110/4] = 0x0000FE01;\
(a)[0x114/4] = 0x0000FE01;\
(a)[0x118/4] = 0x0000FE01;\
(a)[0x11c/4] = 0x0000FE01;\
(a)[0x120/4] = 0x0000FE01;\
(a)[0x124/4] = 0x0000FE01;\
(a)[0x128/4] = 0x0000FE01;\
(a)[0x12c/4] = 0x0000FE01;\
(a)[0x130/4] = 0x0000FE01;\
(a)[0x134/4] = 0x0000FE01;\
(a)[0x138/4] = 0x0000FE01;\
(a)[0x13c/4] = 0x0000FE01;\
(a)[0x140/4] = 0x0000FE01;\
(a)[0x144/4] = 0x0000FE01;\
(a)[0x148/4] = 0x0000FE01;\
(a)[0x14c/4] = 0x0000FE01;\
(a)[0x150/4] = 0x0000FE01;\
(a)[0x154/4] = 0x0000FE01;\
(a)[0x158/4] = 0x0000FE01;\
(a)[0x15c/4] = 0x0000FE01;\
(a)[0x160/4] = 0x0000FE01;\
(a)[0x164/4] = 0x0000FE01;\
(a)[0x168/4] = 0x0000FE01;\
(a)[0x16c/4] = 0x0000FE01;\
(a)[0x170/4] = 0x0000FE01;\
(a)[0x174/4] = 0x0000FE01;\
(a)[0x178/4] = 0x0000FE01;\
(a)[0x17c/4] = 0x0000FE01;\
(a)[0x180/4] = 0x0000FE01;\
(a)[0x184/4] = 0x0000FE01;\
(a)[0x188/4] = 0x0000FE01;\
(a)[0x18c/4] = 0x0000FE01;\
(a)[0x190/4] = 0x0000FE01;\
(a)[0x194/4] = 0x0000FE01;\
(a)[0x198/4] = 0x0000FE01;\
(a)[0x19c/4] = 0x0000FE01;\
(a)[0x1a0/4] = 0x0000FE01;\
(a)[0x1a4/4] = 0x0000FE01;\
(a)[0x1a8/4] = 0x0000FE01;\
(a)[0x1ac/4] = 0x0000FE01;\
(a)[0x1b0/4] = 0x0000FE01;\
(a)[0x1b4/4] = 0x0000FE01;\
(a)[0x1b8/4] = 0x0000FE01;\
(a)[0x1bc/4] = 0x0000FE01;\
(a)[0x1c0/4] = 0x0000FE01;\
(a)[0x1c4/4] = 0x0000FE01;\
(a)[0x1c8/4] = 0x0000FE01;\
(a)[0x1cc/4] = 0x0000FE01;\
(a)[0x1d0/4] = 0x0000FE01;\
(a)[0x1d4/4] = 0x0000FE01;\
(a)[0x1d8/4] = 0x0000FE01;\
(a)[0x1dc/4] = 0x0000FE01;\
(a)[0x1e0/4] = 0x0000FE01;\
(a)[0x1e4/4] = 0x0000FE01;\
(a)[0x1e8/4] = 0x0000FE01;\
(a)[0x1ec/4] = 0x0000FE01;\
(a)[0x1f0/4] = 0x0000FE01;\
(a)[0x1f4/4] = 0x0000FE01;\
(a)[0x1f8/4] = 0x0000FE01;\
(a)[0x1fc/4] = 0x0000FE01;\
(a)[0x200/4] = 0x0000FE01;\
(a)[0x204/4] = 0x0000FE01;\
(a)[0x208/4] = 0x0000FE01;\
(a)[0x20c/4] = 0x0000FE01;\
(a)[0x210/4] = 0x0000FE01;\
(a)[0x214/4] = 0x0000FE01;\
(a)[0x218/4] = 0x0000FE01;\
(a)[0x21c/4] = 0x0000FE01;\
(a)[0x220/4] = 0x0000FE01;\
(a)[0x224/4] = 0x0000FE01;\
(a)[0x228/4] = 0x0000FE01;\
(a)[0x22c/4] = 0x0000FE01;\
(a)[0x230/4] = 0x0000FE01;\
(a)[0x234/4] = 0x0000FE01;\
(a)[0x238/4] = 0x0000FE01;\
(a)[0x23c/4] = 0x0000FE01;\
(a)[0x240/4] = 0x0000FE01;\
(a)[0x244/4] = 0x0000FE01;\
(a)[0x248/4] = 0x0000FE01;\
(a)[0x24c/4] = 0x0000FE01;\
(a)[0x250/4] = 0x0000FE01;\
(a)[0x254/4] = 0x0000FE01;\
(a)[0x258/4] = 0x0000FE01;\
(a)[0x25c/4] = 0x0000FE01;\
(a)[0x260/4] = 0x0000FE01;\
(a)[0x264/4] = 0x0000FE01;\
(a)[0x268/4] = 0x0000FE01;\
(a)[0x26c/4] = 0x0000FE01;\
(a)[0x270/4] = 0x0000FE01;\
(a)[0x274/4] = 0x0000FE01;\
(a)[0x278/4] = 0x0000FE01;\
(a)[0x27c/4] = 0x0000FE01;\
(a)[0x280/4] = 0x0000FE01;\
(a)[0x284/4] = 0x0000FE01;\
(a)[0x288/4] = 0x0000FE01;\
(a)[0x28c/4] = 0x0000FE01;\
(a)[0x290/4] = 0x0000FE01;\
(a)[0x294/4] = 0x0000FE01;\
(a)[0x298/4] = 0x0000FE01;\
(a)[0x29c/4] = 0x0000FE01;\
(a)[0x2a0/4] = 0x0000FE01;\
(a)[0x2a4/4] = 0x0000FE01;\
(a)[0x2a8/4] = 0x0000FE01;\
(a)[0x2ac/4] = 0x0000FE01;\
(a)[0x2b0/4] = 0x0000FE01;\
(a)[0x2b4/4] = 0x0000FE01;\
(a)[0x2b8/4] = 0x0000FE01;\
(a)[0x2bc/4] = 0x0000FE01;\
(a)[0x2c0/4] = 0x0000FE01;\
(a)[0x2c4/4] = 0x0000FE01;\
(a)[0x2c8/4] = 0x0000FE01;\
(a)[0x2cc/4] = 0x0000FE01;\
(a)[0x2d0/4] = 0x0000FE01;\
(a)[0x2d4/4] = 0x0000FE01;\
(a)[0x2d8/4] = 0x0000FE01;\
(a)[0x2dc/4] = 0x0000FE01;\
(a)[0x2e0/4] = 0x0000FE01;\
(a)[0x2e4/4] = 0x0000FE01;\
(a)[0x2e8/4] = 0x0000FE01;\
(a)[0x2ec/4] = 0x0000FE01;\
(a)[0x2f0/4] = 0x0000FE01;\
(a)[0x2f4/4] = 0x0000FE01;\
(a)[0x2f8/4] = 0x0000FE01;\
(a)[0x2fc/4] = 0x0000FE01;\
(a)[0x300/4] = 0x0000FE01;\
(a)[0x304/4] = 0x0000FE01;\
(a)[0x308/4] = 0x0000FE01;\
(a)[0x30c/4] = 0x0000FE01;\
(a)[0x310/4] = 0x0000FE01;\
(a)[0x314/4] = 0x0000FE01;\
(a)[0x318/4] = 0x0000FE01;\
(a)[0x31c/4] = 0x0000FE01;\
(a)[0x320/4] = 0x0000FE01;\
(a)[0x324/4] = 0x0000FE01;\
(a)[0x328/4] = 0x0000FE01;\
(a)[0x32c/4] = 0x0000FE01;\
(a)[0x330/4] = 0x0000FE01;\
(a)[0x334/4] = 0x0000FE01;\
(a)[0x338/4] = 0x0000FE01;\
(a)[0x33c/4] = 0x0000FE01;\
(a)[0x340/4] = 0x0000FE01;\
(a)[0x344/4] = 0x0000FE01;\
(a)[0x348/4] = 0x0000FE01;\
(a)[0x34c/4] = 0x0000FE01;\
(a)[0x350/4] = 0x0000FE01;\
(a)[0x354/4] = 0x0000FE01;\
(a)[0x358/4] = 0x0000FE01;\
(a)[0x35c/4] = 0x0000FE01;\
(a)[0x360/4] = 0x0000FE01;\
(a)[0x364/4] = 0x0000FE01;\
(a)[0x368/4] = 0x0000FE01;\
(a)[0x36c/4] = 0x0000FE01;\
(a)[0x370/4] = 0x0000FE01;\
(a)[0x374/4] = 0x0000FE01;\
(a)[0x378/4] = 0x0000FE01;\
(a)[0x37c/4] = 0x0000FE01;\
(a)[0x380/4] = 0x0000FE01;\
(a)[0x384/4] = 0x0000FE01;\
(a)[0x388/4] = 0x0000FE01;\
(a)[0x38c/4] = 0x0000FE01;\
(a)[0x390/4] = 0x0000FE01;\
(a)[0x394/4] = 0x0000FE01;\
(a)[0x398/4] = 0x0000FE01;\
(a)[0x39c/4] = 0x0000FE01;\
(a)[0x3a0/4] = 0x0000FE01;\
(a)[0x3a4/4] = 0x0000FE01;\
(a)[0x3a8/4] = 0x0000FE01;\
(a)[0x3ac/4] = 0x0000FE01;\
(a)[0x3b0/4] = 0x0000FE01;\
(a)[0x3b4/4] = 0x0000FE01;\
(a)[0x3b8/4] = 0x0000FE01;\
(a)[0x3bc/4] = 0x0000FE01;\
(a)[0x3c0/4] = 0x0000FE01;\
(a)[0x3c4/4] = 0x0000FE01;\
(a)[0x3c8/4] = 0x0000FE01;\
(a)[0x3cc/4] = 0x0000FE01;\
(a)[0x3d0/4] = 0x0000FE01;\
(a)[0x3d4/4] = 0x0000FE01;\
(a)[0x3d8/4] = 0x0000FE01;\
(a)[0x3dc/4] = 0x0000FE01;\
(a)[0x3e0/4] = 0x0000FE01;\
(a)[0x3e4/4] = 0x0000FE01;\
(a)[0x3e8/4] = 0x0000FE01;\
(a)[0x3ec/4] = 0x0000FE01;\
(a)[0x3f0/4] = 0x0000FE01;\
(a)[0x3f4/4] = 0x0000FE01;\
(a)[0x3f8/4] = 0x0000FE01;\
(a)[0x3fc/4] = 0x0000FE01;\
(a)[0x400/4] = 0x0000FE01;\
(a)[0x404/4] = 0x0000FE01;\
(a)[0x408/4] = 0x0000FE01;\
(a)[0x40c/4] = 0x0000FE01;\
(a)[0x410/4] = 0x0000FE01;\
(a)[0x414/4] = 0x0000FE01;\
(a)[0x418/4] = 0x0000FE01;\
(a)[0x41c/4] = 0x0000FE01;\
(a)[0x420/4] = 0x0000FE01;\
(a)[0x424/4] = 0x0000FE01;\
(a)[0x428/4] = 0x0000FE01;\
(a)[0x42c/4] = 0x0000FE01;\
(a)[0x430/4] = 0x0000FE01;\
(a)[0x434/4] = 0x0000FE01;\
(a)[0x438/4] = 0x0000FE01;\
(a)[0x43c/4] = 0x0000FE01;\
(a)[0x440/4] = 0x0000FE01;\
(a)[0x444/4] = 0x0000FE01;\
(a)[0x448/4] = 0x0000FE01;\
(a)[0x44c/4] = 0x0000FE01;\
(a)[0x450/4] = 0x0000FE01;\
(a)[0x454/4] = 0x0000FE01;\
(a)[0x458/4] = 0x0000FE01;\
(a)[0x45c/4] = 0x0000FE01;\
(a)[0x460/4] = 0x0000FE01;\
(a)[0x464/4] = 0x0000FE01;\
(a)[0x468/4] = 0x0000FE01;\
(a)[0x46c/4] = 0x0000FE01;\
(a)[0x470/4] = 0x0000FE01;\
(a)[0x474/4] = 0x0000FE01;\
(a)[0x478/4] = 0x0000FE01;\
(a)[0x47c/4] = 0x0000FE01;\
(a)[0x480/4] = 0x0000FE01;\
(a)[0x484/4] = 0x0000FE01;\
(a)[0x488/4] = 0x0000FE01;\
(a)[0x48c/4] = 0x0000FE01;\
(a)[0x490/4] = 0x0000FE01;\
(a)[0x494/4] = 0x0000FE01;\
(a)[0x498/4] = 0x0000FE01;\
(a)[0x49c/4] = 0x0000FE01;\
(a)[0x4a0/4] = 0x0000FE01;\
(a)[0x4a4/4] = 0x0000FE01;\
(a)[0x4a8/4] = 0x0000FE01;\
(a)[0x4ac/4] = 0x0000FE01;\
(a)[0x4b0/4] = 0x0000FE01;\
(a)[0x4b4/4] = 0x0000FE01;\
(a)[0x4b8/4] = 0x0000FE01;\
(a)[0x4bc/4] = 0x0000FE01;\
(a)[0x4c0/4] = 0x0000FE01;\
(a)[0x4c4/4] = 0x0000FE01;\
(a)[0x4c8/4] = 0x0000FE01;\
(a)[0x4cc/4] = 0x0000FE01;\
(a)[0x4d0/4] = 0x0000FE01;\
(a)[0x4d4/4] = 0x0000FE01;\
(a)[0x4d8/4] = 0x0000FE01;\
(a)[0x4dc/4] = 0x0000FE01;\
(a)[0x4e0/4] = 0x0000FE01;\
(a)[0x4e4/4] = 0x0000FE01;\
(a)[0x4e8/4] = 0x0000FE01;\
(a)[0x4ec/4] = 0x0000FE01;\
(a)[0x4f0/4] = 0x0000FE01;\
(a)[0x4f4/4] = 0x0000FE01;\
(a)[0x4f8/4] = 0x0000FE01;\
(a)[0x4fc/4] = 0x0000FE01;\
(a)[0x500/4] = 0x0000FE01;\
(a)[0x504/4] = 0x0000FE01;\
(a)[0x508/4] = 0x0000FE01;\
(a)[0x50c/4] = 0x0000FE01;\
(a)[0x510/4] = 0x0000FE01;\
(a)[0x514/4] = 0x0000FE01;\
(a)[0x518/4] = 0x0000FE01;\
(a)[0x51c/4] = 0x0000FE01;\
(a)[0x520/4] = 0x0000FE01;\
(a)[0x524/4] = 0x0000FE01;\
(a)[0x528/4] = 0x0000FE01;\
(a)[0x52c/4] = 0x0000FE01;\
(a)[0x530/4] = 0x0000FE01;\
(a)[0x534/4] = 0x0000FE01;\
(a)[0x538/4] = 0x0000FE01;\
(a)[0x53c/4] = 0x0000FE01;\
(a)[0x540/4] = 0x0000FE01;\
(a)[0x544/4] = 0x0000FE01;\
(a)[0x548/4] = 0x0000FE01;\
(a)[0x54c/4] = 0x0000FE01;\
(a)[0x550/4] = 0x0000FE01;\
(a)[0x554/4] = 0x0000FE01;\
(a)[0x558/4] = 0x0000FE01;\
(a)[0x55c/4] = 0x0000FE01;\
(a)[0x560/4] = 0x0000FE01;\
(a)[0x564/4] = 0x0000FE01;\
(a)[0x568/4] = 0x0000FE01;\
(a)[0x56c/4] = 0x0000FE01;\
(a)[0x570/4] = 0x0000FE01;\
(a)[0x574/4] = 0x0000FE01;\
(a)[0x578/4] = 0x0000FE01;\
(a)[0x57c/4] = 0x0000FE01;\
(a)[0x580/4] = 0x0000FE01;\
(a)[0x584/4] = 0x0000FE01;\
(a)[0x588/4] = 0x0000FE01;\
(a)[0x58c/4] = 0x0000FE01;\
(a)[0x590/4] = 0x0000FE01;\
(a)[0x594/4] = 0x0000FE01;\
(a)[0x598/4] = 0x0000FE01;\
(a)[0x59c/4] = 0x0000FE01;\
(a)[0x5a0/4] = 0x0000FE01;\
(a)[0x5a4/4] = 0x0000FE01;\
(a)[0x5a8/4] = 0x0000FE01;\
(a)[0x5ac/4] = 0x0000FE01;\
(a)[0x5b0/4] = 0x0000FE01;\
(a)[0x5b4/4] = 0x0000FE01;\
(a)[0x5b8/4] = 0x0000FE01;\
(a)[0x5bc/4] = 0x0000FE01;\
(a)[0x5c0/4] = 0x0000FE01;\
(a)[0x5c4/4] = 0x0000FE01;\
(a)[0x5c8/4] = 0x0000FE01;\
(a)[0x5cc/4] = 0x0000FE01;\
(a)[0x5d0/4] = 0x0000FE01;\
(a)[0x5d4/4] = 0x0000FE01;\
(a)[0x5d8/4] = 0x0000FE01;\
(a)[0x5dc/4] = 0x0000FE01;\
(a)[0x5e0/4] = 0x0000FE01;\
(a)[0x5e4/4] = 0x0000FE01;\
(a)[0x5e8/4] = 0x0000FE01;\
(a)[0x5ec/4] = 0x0000FE01;\
(a)[0x5f0/4] = 0x0000FE01;\
(a)[0x5f4/4] = 0x0000FE01;\
(a)[0x5f8/4] = 0x0000FE01;\
(a)[0x5fc/4] = 0x0000FE01;\
(a)[0x600/4] = 0x0000FE01;\
(a)[0x604/4] = 0x0000FE01;\
(a)[0x608/4] = 0x0000FE01;\
(a)[0x60c/4] = 0x0000FE01;\
(a)[0x610/4] = 0x0000FE01;\
(a)[0x614/4] = 0x0000FE01;\
(a)[0x618/4] = 0x0000FE01;\
(a)[0x61c/4] = 0x0000FE01;\
(a)[0x620/4] = 0x0000FE01;\
(a)[0x624/4] = 0x0000FE01;\
(a)[0x628/4] = 0x0000FE01;\
(a)[0x62c/4] = 0x0000FE01;\
(a)[0x630/4] = 0x0000FE01;\
(a)[0x634/4] = 0x0000FE01;\
(a)[0x638/4] = 0x0000FE01;\
(a)[0x63c/4] = 0x0000FE01;\
(a)[0x640/4] = 0x0000FE01;\
(a)[0x644/4] = 0x0000FE01;\
(a)[0x648/4] = 0x0000FE01;\
(a)[0x64c/4] = 0x0000FE01;\
(a)[0x650/4] = 0x0000FE01;\
(a)[0x654/4] = 0x0000FE01;\
(a)[0x658/4] = 0x0000FE01;\
(a)[0x65c/4] = 0x0000FE01;\
(a)[0x660/4] = 0x0000FE01;\
(a)[0x664/4] = 0x0000FE01;\
(a)[0x668/4] = 0x0000FE01;\
(a)[0x66c/4] = 0x0000FE01;\
(a)[0x670/4] = 0x0000FE01;\
(a)[0x674/4] = 0x0000FE01;\
(a)[0x678/4] = 0x0000FE01;\
(a)[0x67c/4] = 0x0000FE01;\
(a)[0x680/4] = 0x0000FE01;\
(a)[0x684/4] = 0x0000FE01;\
(a)[0x688/4] = 0x0000FE01;\
(a)[0x68c/4] = 0x0000FE01;\
(a)[0x690/4] = 0x0000FE01;\
(a)[0x694/4] = 0x0000FE01;\
(a)[0x698/4] = 0x0000FE01;\
(a)[0x69c/4] = 0x0000FE01;\
(a)[0x6a0/4] = 0x0000FE01;\
(a)[0x6a4/4] = 0x0000FE01;\
(a)[0x6a8/4] = 0x0000FE01;\
(a)[0x6ac/4] = 0x0000FE01;\
(a)[0x6b0/4] = 0x0000FE01;\
(a)[0x6b4/4] = 0x0000FE01;\
(a)[0x6b8/4] = 0x0000FE01;\
(a)[0x6bc/4] = 0x0000FE01;\
(a)[0x6c0/4] = 0x0000FE01;\
(a)[0x6c4/4] = 0x0000FE01;\
(a)[0x6c8/4] = 0x0000FE01;\
(a)[0x6cc/4] = 0x0000FE01;\
(a)[0x6d0/4] = 0x0000FE01;\
(a)[0x6d4/4] = 0x0000FE01;\
(a)[0x6d8/4] = 0x0000FE01;\
(a)[0x6dc/4] = 0x0000FE01;\
(a)[0x6e0/4] = 0x0000FE01;\
(a)[0x6e4/4] = 0x0000FE01;\
(a)[0x6e8/4] = 0x0000FE01;\
(a)[0x6ec/4] = 0x0000FE01;\
(a)[0x6f0/4] = 0x0000FE01;\
(a)[0x6f4/4] = 0x0000FE01;\
(a)[0x6f8/4] = 0x0000FE01;\
(a)[0x6fc/4] = 0x0000FE01;\
(a)[0x700/4] = 0x0000FE01;\
(a)[0x704/4] = 0x0000FE01;\
(a)[0x708/4] = 0x0000FE01;\
(a)[0x70c/4] = 0x0000FE01;\
(a)[0x710/4] = 0x0000FE01;\
(a)[0x714/4] = 0x0000FE01;\
(a)[0x718/4] = 0x0000FE01;\
(a)[0x71c/4] = 0x0000FE01;\
(a)[0x720/4] = 0x0000FE01;\
(a)[0x724/4] = 0x0000FE01;\
(a)[0x728/4] = 0x0000FE01;\
(a)[0x72c/4] = 0x0000FE01;\
(a)[0x730/4] = 0x0000FE01;\
(a)[0x734/4] = 0x0000FE01;\
(a)[0x738/4] = 0x0000FE01;\
(a)[0x73c/4] = 0x0000FE01;\
(a)[0x740/4] = 0x0000FE01;\
(a)[0x744/4] = 0x0000FE01;\
(a)[0x748/4] = 0x0000FE01;\
(a)[0x74c/4] = 0x0000FE01;\
(a)[0x750/4] = 0x0000FE01;\
(a)[0x754/4] = 0x0000FE01;\
(a)[0x758/4] = 0x0000FE01;\
(a)[0x75c/4] = 0x0000FE01;\
(a)[0x760/4] = 0x0000FE01;\
(a)[0x764/4] = 0x0000FE01;\
(a)[0x768/4] = 0x0000FE01;\
(a)[0x76c/4] = 0x0000FE01;\
(a)[0x770/4] = 0x0000FE01;\
(a)[0x774/4] = 0x0000FE01;\
(a)[0x778/4] = 0x0000FE01;\
(a)[0x77c/4] = 0x0000FE01;\
(a)[0x780/4] = 0x0000FE01;\
(a)[0x784/4] = 0x0000FE01;\
(a)[0x788/4] = 0x0000FE01;\
(a)[0x78c/4] = 0x0000FE01;\
(a)[0x790/4] = 0x0000FE01;\
(a)[0x794/4] = 0x0000FE01;\
(a)[0x798/4] = 0x0000FE01;\
(a)[0x79c/4] = 0x0000FE01;\
(a)[0x7a0/4] = 0x0000FE01;\
(a)[0x7a4/4] = 0x0000FE01;\
(a)[0x7a8/4] = 0x0000FE01;\
(a)[0x7ac/4] = 0x0000FE01;\
(a)[0x7b0/4] = 0x0000FE01;\
(a)[0x7b4/4] = 0x0000FE01;\
(a)[0x7b8/4] = 0x0000FE01;\
(a)[0x7bc/4] = 0x0000FE01;\
(a)[0x7c0/4] = 0x0000FE01;\
(a)[0x7c4/4] = 0x0000FE01;\
(a)[0x7c8/4] = 0x0000FE01;\
(a)[0x7cc/4] = 0x0000FE01;\
(a)[0x7d0/4] = 0x0000FE01;\
(a)[0x7d4/4] = 0x0000FE01;\
(a)[0x7d8/4] = 0x0000FE01;\
(a)[0x7dc/4] = 0x0000FE01;\
(a)[0x7e0/4] = 0x0000FE01;\
(a)[0x7e4/4] = 0x0000FE01;\
(a)[0x7e8/4] = 0x0000FE01;\
(a)[0x7ec/4] = 0x0000FE01;\
(a)[0x7f0/4] = 0x0000FE01;\
(a)[0x7f4/4] = 0x0000FE01;\
(a)[0x7f8/4] = 0x0000FE01;\
(a)[0x7fc/4] = 0x0000FE01;\
(a)[0x800/4] = 0x0000FE01;\
(a)[0x804/4] = 0x0000FE01;\
(a)[0x808/4] = 0x0000FE01;\
(a)[0x80c/4] = 0x0000FE01;\
(a)[0x810/4] = 0x0000FE01;\
(a)[0x814/4] = 0x0000FE01;\
(a)[0x818/4] = 0x0000FE01;\
(a)[0x81c/4] = 0x0000FE01;\
(a)[0x820/4] = 0x0000FE01;\
(a)[0x824/4] = 0x0000FE01;\
(a)[0x828/4] = 0x0000FE01;\
(a)[0x82c/4] = 0x0000FE01;\
(a)[0x830/4] = 0x0000FE01;\
(a)[0x834/4] = 0x0000FE01;\
(a)[0x838/4] = 0x0000FE01;\
(a)[0x83c/4] = 0x0000FE01;\
(a)[0x840/4] = 0x0000FE01;\
(a)[0x844/4] = 0x0000FE01;\
(a)[0x848/4] = 0x0000FE01;\
(a)[0x84c/4] = 0x0000FE01;\
(a)[0x850/4] = 0x0000FE01;\
(a)[0x854/4] = 0x0000FE01;\
(a)[0x858/4] = 0x0000FE01;\
(a)[0x85c/4] = 0x0000FE01;\
(a)[0x860/4] = 0x0000FE01;\
(a)[0x864/4] = 0x0000FE01;\
(a)[0x868/4] = 0x0000FE01;\
(a)[0x86c/4] = 0x0000FE01;\
(a)[0x870/4] = 0x0000FE01;\
(a)[0x874/4] = 0x0000FE01;\
(a)[0x878/4] = 0x0000FE01;\
(a)[0x87c/4] = 0x0000FE01;\
(a)[0x880/4] = 0x0000FE01;\
(a)[0x884/4] = 0x0000FE01;\
(a)[0x888/4] = 0x0000FE01;\
(a)[0x88c/4] = 0x0000FE01;\
(a)[0x890/4] = 0x0000FE01;\
(a)[0x894/4] = 0x0000FE01;\
(a)[0x898/4] = 0x0000FE01;\
(a)[0x89c/4] = 0x0000FE01;\
(a)[0x8a0/4] = 0x0000FE01;\
(a)[0x8a4/4] = 0x0000FE01;\
(a)[0x8a8/4] = 0x0000FE01;\
(a)[0x8ac/4] = 0x0000FE01;\
(a)[0x8b0/4] = 0x0000FE01;\
(a)[0x8b4/4] = 0x0000FE01;\
(a)[0x8b8/4] = 0x0000FE01;\
(a)[0x8bc/4] = 0x0000FE01;\
(a)[0x8c0/4] = 0x0000FE01;\
(a)[0x8c4/4] = 0x0000FE01;\
(a)[0x8c8/4] = 0x0000FE01;\
(a)[0x8cc/4] = 0x0000FE01;\
(a)[0x8d0/4] = 0x0000FE01;\
(a)[0x8d4/4] = 0x0000FE01;\
(a)[0x8d8/4] = 0x0000FE01;\
(a)[0x8dc/4] = 0x0000FE01;\
(a)[0x8e0/4] = 0x0000FE01;\
(a)[0x8e4/4] = 0x0000FE01;\
(a)[0x8e8/4] = 0x0000FE01;\
(a)[0x8ec/4] = 0x0000FE01;\
(a)[0x8f0/4] = 0x0000FE01;\
(a)[0x8f4/4] = 0x0000FE01;\
(a)[0x8f8/4] = 0x0000FE01;\
(a)[0x8fc/4] = 0x0000FE01;\
(a)[0x900/4] = 0x0000FE01;\
(a)[0x904/4] = 0x0000FE01;\
(a)[0x908/4] = 0x0000FE01;\
(a)[0x90c/4] = 0x0000FE01;\
(a)[0x910/4] = 0x0000FE01;\
(a)[0x914/4] = 0x0000FE01;\
(a)[0x918/4] = 0x0000FE01;\
(a)[0x91c/4] = 0x0000FE01;\
(a)[0x920/4] = 0x0000FE01;\
(a)[0x924/4] = 0x0000FE01;\
(a)[0x928/4] = 0x0000FE01;\
(a)[0x92c/4] = 0x0000FE01;\
(a)[0x930/4] = 0x0000FE01;\
(a)[0x934/4] = 0x0000FE01;\
(a)[0x938/4] = 0x0000FE01;\
(a)[0x93c/4] = 0x0000FE01;\
(a)[0x940/4] = 0x0000FE01;\
(a)[0x944/4] = 0x0000FE01;\
(a)[0x948/4] = 0x0000FE01;\
(a)[0x94c/4] = 0x0000FE01;\
(a)[0x950/4] = 0x0000FE01;\
(a)[0x954/4] = 0x0000FE01;\
(a)[0x958/4] = 0x0000FE01;\
(a)[0x95c/4] = 0x0000FE01;\
(a)[0x960/4] = 0x0000FE01;\
(a)[0x964/4] = 0x0000FE01;\
(a)[0x968/4] = 0x0000FE01;\
(a)[0x96c/4] = 0x0000FE01;\
(a)[0x970/4] = 0x0000FE01;\
(a)[0x974/4] = 0x0000FE01;\
(a)[0x978/4] = 0x0000FE01;\
(a)[0x97c/4] = 0x0000FE01;\
(a)[0x980/4] = 0x0000FE01;\
(a)[0x984/4] = 0x0000FE01;\
(a)[0x988/4] = 0x0000FE01;\
(a)[0x98c/4] = 0x0000FE01;\
(a)[0x990/4] = 0x0000FE01;\
(a)[0x994/4] = 0x0000FE01;\
(a)[0x998/4] = 0x0000FE01;\
(a)[0x99c/4] = 0x0000FE01;\
(a)[0x9a0/4] = 0x0000FE01;\
(a)[0x9a4/4] = 0x0000FE01;\
(a)[0x9a8/4] = 0x0000FE01;\
(a)[0x9ac/4] = 0x0000FE01;\
(a)[0x9b0/4] = 0x0000FE01;\
(a)[0x9b4/4] = 0x0000FE01;\
(a)[0x9b8/4] = 0x0000FE01;\
(a)[0x9bc/4] = 0x0000FE01;\
(a)[0x9c0/4] = 0x0000FE01;\
(a)[0x9c4/4] = 0x0000FE01;\
(a)[0x9c8/4] = 0x0000FE01;\
(a)[0x9cc/4] = 0x0000FE01;\
(a)[0x9d0/4] = 0x0000FE01;\
(a)[0x9d4/4] = 0x0000FE01;\
(a)[0x9d8/4] = 0x0000FE01;\
(a)[0x9dc/4] = 0x0000FE01;\
(a)[0x9e0/4] = 0x0000FE01;\
(a)[0x9e4/4] = 0x0000FE01;\
(a)[0x9e8/4] = 0x0000FE01;\
(a)[0x9ec/4] = 0x0000FE01;\
(a)[0x9f0/4] = 0x0000FE01;\
(a)[0x9f4/4] = 0x0000FE01;\
(a)[0x9f8/4] = 0x0000FE01;\
(a)[0x9fc/4] = 0x0000FE01;\
(a)[0xa00/4] = 0x0000FE01;\
(a)[0xa04/4] = 0x0000FE01;\
(a)[0xa08/4] = 0x0000FE01;\
(a)[0xa0c/4] = 0x0000FE01;\
(a)[0xa10/4] = 0x0000FE01;\
(a)[0xa14/4] = 0x0000FE01;\
(a)[0xa18/4] = 0x0000FE01;\
(a)[0xa1c/4] = 0x0000FE01;\
(a)[0xa20/4] = 0x0000FE01;\
(a)[0xa24/4] = 0x0000FE01;\
(a)[0xa28/4] = 0x0000FE01;\
(a)[0xa2c/4] = 0x0000FE01;\
(a)[0xa30/4] = 0x0000FE01;\
(a)[0xa34/4] = 0x0000FE01;\
(a)[0xa38/4] = 0x0000FE01;\
(a)[0xa3c/4] = 0x0000FE01;\
(a)[0xa40/4] = 0x0000FE01;\
(a)[0xa44/4] = 0x0000FE01;\
(a)[0xa48/4] = 0x0000FE01;\
(a)[0xa4c/4] = 0x0000FE01;\
(a)[0xa50/4] = 0x0000FE01;\
(a)[0xa54/4] = 0x0000FE01;\
(a)[0xa58/4] = 0x0000FE01;\
(a)[0xa5c/4] = 0x0000FE01;\
(a)[0xa60/4] = 0x0000FE01;\
(a)[0xa64/4] = 0x0000FE01;\
(a)[0xa68/4] = 0x0000FE01;\
(a)[0xa6c/4] = 0x0000FE01;\
(a)[0xa70/4] = 0x0000FE01;\
(a)[0xa74/4] = 0x0000FE01;\
(a)[0xa78/4] = 0x0000FE01;\
(a)[0xa7c/4] = 0x0000FE01;\
(a)[0xa80/4] = 0x0000FE01;\
(a)[0xa84/4] = 0x0000FE01;\
(a)[0xa88/4] = 0x0000FE01;\
(a)[0xa8c/4] = 0x0000FE01;\
(a)[0xa90/4] = 0x0000FE01;\
(a)[0xa94/4] = 0x0000FE01;\
(a)[0xa98/4] = 0x0000FE01;\
(a)[0xa9c/4] = 0x0000FE01;\
(a)[0xaa0/4] = 0x0000FE01;\
(a)[0xaa4/4] = 0x0000FE01;\
(a)[0xaa8/4] = 0x0000FE01;\
(a)[0xaac/4] = 0x0000FE01;\
(a)[0xab0/4] = 0x0000FE01;\
(a)[0xab4/4] = 0x0000FE01;\
(a)[0xab8/4] = 0x0000FE01;\
(a)[0xabc/4] = 0x0000FE01;\
(a)[0xac0/4] = 0x0000FE01;\
(a)[0xac4/4] = 0x0000FE01;\
(a)[0xac8/4] = 0x0000FE01;\
(a)[0xacc/4] = 0x0000FE01;\
(a)[0xad0/4] = 0x0000FE01;\
(a)[0xad4/4] = 0x0000FE01;\
(a)[0xad8/4] = 0x0000FE01;\
(a)[0xadc/4] = 0x0000FE01;\
(a)[0xae0/4] = 0x0000FE01;\
(a)[0xae4/4] = 0x0000FE01;\
(a)[0xae8/4] = 0x0000FE01;\
(a)[0xaec/4] = 0x0000FE01;\
(a)[0xaf0/4] = 0x0000FE01;\
(a)[0xaf4/4] = 0x0000FE01;\
(a)[0xaf8/4] = 0x0000FE01;\
(a)[0xafc/4] = 0x0000FE01;\
(a)[0xb00/4] = 0x0000FE01;\
(a)[0xb04/4] = 0x0000FE01;\
(a)[0xb08/4] = 0x0000FE01;\
(a)[0xb0c/4] = 0x0000FE01;\
(a)[0xb10/4] = 0x0000FE01;\
(a)[0xb14/4] = 0x0000FE01;\
(a)[0xb18/4] = 0x0000FE01;\
(a)[0xb1c/4] = 0x0000FE01;\
(a)[0xb20/4] = 0x0000FE01;\
(a)[0xb24/4] = 0x0000FE01;\
(a)[0xb28/4] = 0x0000FE01;\
(a)[0xb2c/4] = 0x0000FE01;\
(a)[0xb30/4] = 0x0000FE01;\
(a)[0xb34/4] = 0x0000FE01;\
(a)[0xb38/4] = 0x0000FE01;\
(a)[0xb3c/4] = 0x0000FE01;\
(a)[0xb40/4] = 0x0000FE01;\
(a)[0xb44/4] = 0x0000FE01;\
(a)[0xb48/4] = 0x0000FE01;\
(a)[0xb4c/4] = 0x0000FE01;\
(a)[0xb50/4] = 0x0000FE01;\
(a)[0xb54/4] = 0x0000FE01;\
(a)[0xb58/4] = 0x0000FE01;\
(a)[0xb5c/4] = 0x0000FE01;\
(a)[0xb60/4] = 0x0000FE01;\
(a)[0xb64/4] = 0x0000FE01;\
(a)[0xb68/4] = 0x0000FE01;\
(a)[0xb6c/4] = 0x0000FE01;\
(a)[0xb70/4] = 0x0000FE01;\
(a)[0xb74/4] = 0x0000FE01;\
(a)[0xb78/4] = 0x0000FE01;\
(a)[0xb7c/4] = 0x0000FE01;\
(a)[0xb80/4] = 0x0000FE01;\
(a)[0xb84/4] = 0x0000FE01;\
(a)[0xb88/4] = 0x0000FE01;\
(a)[0xb8c/4] = 0x0000FE01;\
(a)[0xb90/4] = 0x0000FE01;\
(a)[0xb94/4] = 0x0000FE01;\
(a)[0xb98/4] = 0x0000FE01;\
(a)[0xb9c/4] = 0x0000FE01;\
(a)[0xba0/4] = 0x0000FE01;\
(a)[0xba4/4] = 0x0000FE01;\
(a)[0xba8/4] = 0x0000FE01;\
(a)[0xbac/4] = 0x0000FE01;\
(a)[0xbb0/4] = 0x0000FE01;\
(a)[0xbb4/4] = 0x0000FE01;\
(a)[0xbb8/4] = 0x0000FE01;\
(a)[0xbbc/4] = 0x0000FE01;\
(a)[0xbc0/4] = 0x0000FE01;\
(a)[0xbc4/4] = 0x0000FE01;\
(a)[0xbc8/4] = 0x0000FE01;\
(a)[0xbcc/4] = 0x0000FE01;\
(a)[0xbd0/4] = 0x0000FE01;\
(a)[0xbd4/4] = 0x0000FE01;\
(a)[0xbd8/4] = 0x0000FE01;\
(a)[0xbdc/4] = 0x0000FE01;\
(a)[0xbe0/4] = 0x0000FE01;\
(a)[0xbe4/4] = 0x0000FE01;\
(a)[0xbe8/4] = 0x0000FE01;\
(a)[0xbec/4] = 0x0000FE01;\
(a)[0xbf0/4] = 0x0000FE01;\
(a)[0xbf4/4] = 0x0000FE01;\
(a)[0xbf8/4] = 0x0000FE01;\
(a)[0xbfc/4] = 0x0000FE01;\
(a)[0xc00/4] = 0x0000FE01;\
(a)[0xc04/4] = 0x0000FE01;\
(a)[0xc08/4] = 0x0000FE01;\
(a)[0xc0c/4] = 0x0000FE01;\
(a)[0xc10/4] = 0x0000FE01;\
(a)[0xc14/4] = 0x0000FE01;\
(a)[0xc18/4] = 0x0000FE01;\
(a)[0xc1c/4] = 0x0000FE01;\
(a)[0xc20/4] = 0x0000FE01;\
(a)[0xc24/4] = 0x0000FE01;\
(a)[0xc28/4] = 0x0000FE01;\
(a)[0xc2c/4] = 0x0000FE01;\
(a)[0xc30/4] = 0x0000FE01;\
(a)[0xc34/4] = 0x0000FE01;\
(a)[0xc38/4] = 0x0000FE01;\
(a)[0xc3c/4] = 0x0000FE01;\
(a)[0xc40/4] = 0x0000FE01;\
(a)[0xc44/4] = 0x0000FE01;\
(a)[0xc48/4] = 0x0000FE01;\
(a)[0xc4c/4] = 0x0000FE01;\
(a)[0xc50/4] = 0x0000FE01;\
(a)[0xc54/4] = 0x0000FE01;\
(a)[0xc58/4] = 0x0000FE01;\
(a)[0xc5c/4] = 0x0000FE01;\
(a)[0xc60/4] = 0x0000FE01;\
(a)[0xc64/4] = 0x0000FE01;\
(a)[0xc68/4] = 0x0000FE01;\
(a)[0xc6c/4] = 0x0000FE01;\
(a)[0xc70/4] = 0x0000FE01;\
(a)[0xc74/4] = 0x0000FE01;\
(a)[0xc78/4] = 0x0000FE01;\
(a)[0xc7c/4] = 0x0000FE01;\
(a)[0xc80/4] = 0x0000FE01;\
(a)[0xc84/4] = 0x0000FE01;\
(a)[0xc88/4] = 0x0000FE01;\
(a)[0xc8c/4] = 0x0000FE01;\
(a)[0xc90/4] = 0x0000FE01;\
(a)[0xc94/4] = 0x0000FE01;\
(a)[0xc98/4] = 0x0000FE01;\
(a)[0xc9c/4] = 0x0000FE01;\
(a)[0xca0/4] = 0x0000FE01;\
(a)[0xca4/4] = 0x0000FE01;\
(a)[0xca8/4] = 0x0000FE01;\
(a)[0xcac/4] = 0x0000FE01;\
(a)[0xcb0/4] = 0x0000FE01;\
(a)[0xcb4/4] = 0x0000FE01;\
(a)[0xcb8/4] = 0x0000FE01;\
(a)[0xcbc/4] = 0x0000FE01;\
(a)[0xcc0/4] = 0x0000FE01;\
(a)[0xcc4/4] = 0x0000FE01;\
(a)[0xcc8/4] = 0x0000FE01;\
(a)[0xccc/4] = 0x0000FE01;\
(a)[0xcd0/4] = 0x0000FE01;\
(a)[0xcd4/4] = 0x0000FE01;\
(a)[0xcd8/4] = 0x0000FE01;\
(a)[0xcdc/4] = 0x0000FE01;\
(a)[0xce0/4] = 0x0000FE01;\
(a)[0xce4/4] = 0x0000FE01;\
(a)[0xce8/4] = 0x0000FE01;\
(a)[0xcec/4] = 0x0000FE01;\
(a)[0xcf0/4] = 0x0000FE01;\
(a)[0xcf4/4] = 0x0000FE01;\
(a)[0xcf8/4] = 0x0000FE01;\
(a)[0xcfc/4] = 0x0000FE01;\
(a)[0xd00/4] = 0x0000FE01;\
(a)[0xd04/4] = 0x0000FE01;\
(a)[0xd08/4] = 0x0000FE01;\
(a)[0xd0c/4] = 0x0000FE01;\
(a)[0xd10/4] = 0x0000FE01;\
(a)[0xd14/4] = 0x0000FE01;\
(a)[0xd18/4] = 0x0000FE01;\
(a)[0xd1c/4] = 0x0000FE01;\
(a)[0xd20/4] = 0x0000FE01;\
(a)[0xd24/4] = 0x0000FE01;\
(a)[0xd28/4] = 0x0000FE01;\
(a)[0xd2c/4] = 0x0000FE01;\
(a)[0xd30/4] = 0x0000FE01;\
(a)[0xd34/4] = 0x0000FE01;\
(a)[0xd38/4] = 0x0000FE01;\
(a)[0xd3c/4] = 0x0000FE01;\
(a)[0xd40/4] = 0x0000FE01;\
(a)[0xd44/4] = 0x0000FE01;\
(a)[0xd48/4] = 0x0000FE01;\
(a)[0xd4c/4] = 0x0000FE01;\
(a)[0xd50/4] = 0x0000FE01;\
(a)[0xd54/4] = 0x0000FE01;\
(a)[0xd58/4] = 0x0000FE01;\
(a)[0xd5c/4] = 0x0000FE01;\
(a)[0xd60/4] = 0x0000FE01;\
(a)[0xd64/4] = 0x0000FE01;\
(a)[0xd68/4] = 0x0000FE01;\
(a)[0xd6c/4] = 0x0000FE01;\
(a)[0xd70/4] = 0x0000FE01;\
(a)[0xd74/4] = 0x0000FE01;\
(a)[0xd78/4] = 0x0000FE01;\
(a)[0xd7c/4] = 0x0000FE01;\
(a)[0xd80/4] = 0x0000FE01;\
(a)[0xd84/4] = 0x0000FE01;\
(a)[0xd88/4] = 0x0000FE01;\
(a)[0xd8c/4] = 0x0000FE01;\
(a)[0xd90/4] = 0x0000FE01;\
(a)[0xd94/4] = 0x0000FE01;\
(a)[0xd98/4] = 0x0000FE01;\
(a)[0xd9c/4] = 0x0000FE01;\
(a)[0xda0/4] = 0x0000FE01;\
(a)[0xda4/4] = 0x0000FE01;\
(a)[0xda8/4] = 0x0000FE01;\
(a)[0xdac/4] = 0x0000FE01;\
(a)[0xdb0/4] = 0x0000FE01;\
(a)[0xdb4/4] = 0x0000FE01;\
(a)[0xdb8/4] = 0x0000FE01;\
(a)[0xdbc/4] = 0x0000FE01;\
(a)[0xdc0/4] = 0x0000FE01;\
(a)[0xdc4/4] = 0x0000FE01;\
(a)[0xdc8/4] = 0x0000FE01;\
(a)[0xdcc/4] = 0x0000FE01;\
(a)[0xdd0/4] = 0x0000FE01;\
(a)[0xdd4/4] = 0x0000FE01;\
(a)[0xdd8/4] = 0x0000FE01;\
(a)[0xddc/4] = 0x0000FE01;\
(a)[0xde0/4] = 0x0000FE01;\
(a)[0xde4/4] = 0x0000FE01;\
(a)[0xde8/4] = 0x0000FE01;\
(a)[0xdec/4] = 0x0000FE01;\
(a)[0xdf0/4] = 0x0000FE01;\
(a)[0xdf4/4] = 0x0000FE01;\
(a)[0xdf8/4] = 0x0000FE01;\
(a)[0xdfc/4] = 0x0000FE01;\
(a)[0xe00/4] = 0x0000FE01;\
(a)[0xe04/4] = 0x0000FE01;\
(a)[0xe08/4] = 0x0000FE01;\
(a)[0xe0c/4] = 0x0000FE01;\
(a)[0xe10/4] = 0x0000FE01;\
(a)[0xe14/4] = 0x0000FE01;\
(a)[0xe18/4] = 0x0000FE01;\
(a)[0xe1c/4] = 0x0000FE01;\
(a)[0xe20/4] = 0x0000FE01;\
(a)[0xe24/4] = 0x0000FE01;\
(a)[0xe28/4] = 0x0000FE01;\
(a)[0xe2c/4] = 0x0000FE01;\
(a)[0xe30/4] = 0x0000FE01;\
(a)[0xe34/4] = 0x0000FE01;\
(a)[0xe38/4] = 0x0000FE01;\
(a)[0xe3c/4] = 0x0000FE01;\
(a)[0xe40/4] = 0x0000FE01;\
(a)[0xe44/4] = 0x0000FE01;\
(a)[0xe48/4] = 0x0000FE01;\
(a)[0xe4c/4] = 0x0000FE01;\
(a)[0xe50/4] = 0x0000FE01;\
(a)[0xe54/4] = 0x0000FE01;\
(a)[0xe58/4] = 0x0000FE01;\
(a)[0xe5c/4] = 0x0000FE01;\
(a)[0xe60/4] = 0x0000FE01;\
(a)[0xe64/4] = 0x0000FE01;\
(a)[0xe68/4] = 0x0000FE01;\
(a)[0xe6c/4] = 0x0000FE01;\
(a)[0xe70/4] = 0x0000FE01;\
(a)[0xe74/4] = 0x0000FE01;\
(a)[0xe78/4] = 0x0000FE01;\
(a)[0xe7c/4] = 0x0000FE01;\
(a)[0xe80/4] = 0x0000FE01;\
(a)[0xe84/4] = 0x0000FE01;\
(a)[0xe88/4] = 0x0000FE01;\
(a)[0xe8c/4] = 0x0000FE01;\
(a)[0xe90/4] = 0x0000FE01;\
(a)[0xe94/4] = 0x0000FE01;\
(a)[0xe98/4] = 0x0000FE01;\
(a)[0xe9c/4] = 0x0000FE01;\
(a)[0xea0/4] = 0x0000FE01;\
(a)[0xea4/4] = 0x0000FE01;\
(a)[0xea8/4] = 0x0000FE01;\
(a)[0xeac/4] = 0x0000FE01;\
(a)[0xeb0/4] = 0x0000FE01;\
(a)[0xeb4/4] = 0x0000FE01;\
(a)[0xeb8/4] = 0x0000FE01;\
(a)[0xebc/4] = 0x0000FE01;\
(a)[0xec0/4] = 0x0000FE01;\
(a)[0xec4/4] = 0x0000FE01;\
(a)[0xec8/4] = 0x0000FE01;\
(a)[0xecc/4] = 0x0000FE01;\
(a)[0xed0/4] = 0x0000FE01;\
(a)[0xed4/4] = 0x0000FE01;\
(a)[0xed8/4] = 0x0000FE01;\
(a)[0xedc/4] = 0x0000FE01;\
(a)[0xee0/4] = 0x0000FE01;\
(a)[0xee4/4] = 0x0000FE01;\
(a)[0xee8/4] = 0x0000FE01;\
(a)[0xeec/4] = 0x0000FE01;\
(a)[0xef0/4] = 0x0000FE01;\
(a)[0xef4/4] = 0x0000FE01;\
(a)[0xef8/4] = 0x0000FE01;\
(a)[0xefc/4] = 0x0000FE01;\
(a)[0xf00/4] = 0x0000FE01;\
(a)[0xf04/4] = 0x0000FE01;\
(a)[0xf08/4] = 0x0000FE01;\
(a)[0xf0c/4] = 0x0000FE01;\
(a)[0xf10/4] = 0x0000FE01;\
(a)[0xf14/4] = 0x0000FE01;\
(a)[0xf18/4] = 0x0000FE01;\
(a)[0xf1c/4] = 0x0000FE01;\
(a)[0xf20/4] = 0x0000FE01;\
(a)[0xf24/4] = 0x0000FE01;\
(a)[0xf28/4] = 0x0000FE01;\
(a)[0xf2c/4] = 0x0000FE01;\
(a)[0xf30/4] = 0x0000FE01;\
(a)[0xf34/4] = 0x0000FE01;\
(a)[0xf38/4] = 0x0000FE01;\
(a)[0xf3c/4] = 0x0000FE01;\
(a)[0xf40/4] = 0x0000FE01;\
(a)[0xf44/4] = 0x0000FE01;\
(a)[0xf48/4] = 0x0000FE01;\
(a)[0xf4c/4] = 0x0000FE01;\
(a)[0xf50/4] = 0x0000FE01;\
(a)[0xf54/4] = 0x0000FE01;\
(a)[0xf58/4] = 0x0000FE01;\
(a)[0xf5c/4] = 0x0000FE01;\
(a)[0xf60/4] = 0x0000FE01;\
(a)[0xf64/4] = 0x0000FE01;\
(a)[0xf68/4] = 0x0000FE01;\
(a)[0xf6c/4] = 0x0000FE01;\
(a)[0xf70/4] = 0x0000FE01;\
(a)[0xf74/4] = 0x0000FE01;\
(a)[0xf78/4] = 0x0000FE01;\
(a)[0xf7c/4] = 0x0000FE01;\
(a)[0xf80/4] = 0x0000FE01;\
(a)[0xf84/4] = 0x0000FE01;\
(a)[0xf88/4] = 0x0000FE01;\
(a)[0xf8c/4] = 0x0000FE01;\
(a)[0xf90/4] = 0x0000FE01;\
(a)[0xf94/4] = 0x0000FE01;\
(a)[0xf98/4] = 0x0000FE01;\
(a)[0xf9c/4] = 0x0000FE01;\
(a)[0xfa0/4] = 0x0000FE01;\
(a)[0xfa4/4] = 0x0000FE01;\
(a)[0xfa8/4] = 0x0000FE01;\
(a)[0xfac/4] = 0x0000FE01;\
(a)[0xfb0/4] = 0x0000FE01;\
(a)[0xfb4/4] = 0x0000FE01;\
(a)[0xfb8/4] = 0x0000FE01;\
(a)[0xfbc/4] = 0x0000FE01;\
(a)[0xfc0/4] = 0x0000FE01;\
(a)[0xfc4/4] = 0x0000FE01;\
(a)[0xfc8/4] = 0x0000FE01;\
(a)[0xfcc/4] = 0x0000FE01;\
(a)[0xfd0/4] = 0x0000FE01;\
(a)[0xfd4/4] = 0x0000FE01;\
(a)[0xfd8/4] = 0x0000FE01;\
(a)[0xfdc/4] = 0x0000FE01;\
(a)[0xfe0/4] = 0x0000FE01;\
(a)[0xfe4/4] = 0x0000FE01;\
(a)[0xfe8/4] = 0x0000FE01;\
(a)[0xfec/4] = 0x0000FE01;\
(a)[0xff0/4] = 0x0000FE01;\
(a)[0xff4/4] = 0x0000FE01;\
(a)[0xff8/4] = 0x0000FE01;\
(a)[0xffc/4] = 0x0000FE01
#define H2S_MASK_CSR_G3(a) (a)[0x008/4] = 0xFFFFFFFC;\
(a)[0x00C/4] = 0xFFFF0000;\
(a)[0x010/4] = 0xFFFFFFFF;\
(a)[0x014/4] = 0xFFFFFF00;\
(a)[0x018/4] = 0x0000FFFE;\
(a)[0x01C/4] = 0xFF000000;\
(a)[0x020/4] = 0xFF000000;\
(a)[0x024/4] = 0xFFFFFFFF;\
(a)[0x028/4] = 0xFFFFFFFF;\
(a)[0x02C/4] = 0x3FFFFFFF;\
(a)[0x030/4] = 0xFFFFFFFF;\
(a)[0x034/4] = 0xFFFFFFFF;\
(a)[0x038/4] = 0xFFFFFF00;\
(a)[0x404/4] = 0x00000001;\
(a)[0x410/4] = 0xFFFFFFFF;\
(a)[0x418/4] = 0xFFFF0000;\
(a)[0x41C/4] = 0x0003FF00;\
(a)[0x424/4] = 0xFFFFFFFF;\
(a)[0x428/4] = 0xFFFFFFFE;\
(a)[0x428/4] = 0xFFFFFFFC;\
(a)[0x7F8/4] = 0xFFFFFFFF;\
(a)[0x7FC/4] = 0xFFFFFF00;\
(a)[0x800/4] = 0xFFFF0000;\
(a)[0x804/4] = 0xFFFF0000;\
(a)[0x808/4] = 0xFFFF0000;\
(a)[0x80c/4] = 0xFFFF0000;\
(a)[0x810/4] = 0xFFFF0000;\
(a)[0x814/4] = 0xFFFF0000;\
(a)[0x818/4] = 0xFFFF0000;\
(a)[0x81c/4] = 0xFFFF0000;\
(a)[0x820/4] = 0xFFFF0000;\
(a)[0x824/4] = 0xFFFF0000;\
(a)[0x828/4] = 0xFFFF0000;\
(a)[0x82c/4] = 0xFFFF0000;\
(a)[0x830/4] = 0xFFFF0000;\
(a)[0x834/4] = 0xFFFF0000;\
(a)[0x838/4] = 0xFFFF0000;\
(a)[0x83c/4] = 0xFFFF0000;\
(a)[0x840/4] = 0xFFFF0000;\
(a)[0x844/4] = 0xFFFF0000;\
(a)[0x848/4] = 0xFFFF0000;\
(a)[0x84c/4] = 0xFFFF0000;\
(a)[0x850/4] = 0xFFFF0000;\
(a)[0x854/4] = 0xFFFF0000;\
(a)[0x858/4] = 0xFFFF0000;\
(a)[0x85c/4] = 0xFFFF0000;\
(a)[0x860/4] = 0xFFFF0000;\
(a)[0x864/4] = 0xFFFF0000;\
(a)[0x868/4] = 0xFFFF0000;\
(a)[0x86c/4] = 0xFFFF0000;\
(a)[0x870/4] = 0xFFFF0000;\
(a)[0x874/4] = 0xFFFF0000;\
(a)[0x878/4] = 0xFFFF0000;\
(a)[0x87c/4] = 0xFFFF0000;\
(a)[0x880/4] = 0xFFFF0000;\
(a)[0x884/4] = 0xFFFF0000;\
(a)[0x888/4] = 0xFFFF0000;\
(a)[0x88c/4] = 0xFFFF0000;\
(a)[0x890/4] = 0xFFFF0000;\
(a)[0x894/4] = 0xFFFF0000;\
(a)[0x898/4] = 0xFFFF0000;\
(a)[0x89c/4] = 0xFFFF0000;\
(a)[0x8a0/4] = 0xFFFF0000;\
(a)[0x8a4/4] = 0xFFFF0000;\
(a)[0x8a8/4] = 0xFFFF0000;\
(a)[0x8ac/4] = 0xFFFF0000;\
(a)[0x8b0/4] = 0xFFFF0000;\
(a)[0x8b4/4] = 0xFFFF0000;\
(a)[0x8b8/4] = 0xFFFF0000;\
(a)[0x8bc/4] = 0xFFFF0000;\
(a)[0x8c0/4] = 0xFFFF0000;\
(a)[0x8c4/4] = 0xFFFF0000;\
(a)[0x8c8/4] = 0xFFFF0000;\
(a)[0x8cc/4] = 0xFFFF0000;\
(a)[0x8d0/4] = 0xFFFF0000;\
(a)[0x8d4/4] = 0xFFFF0000;\
(a)[0x8d8/4] = 0xFFFF0000;\
(a)[0x8dc/4] = 0xFFFF0000;\
(a)[0x8e0/4] = 0xFFFF0000;\
(a)[0x8e4/4] = 0xFFFF0000;\
(a)[0x8e8/4] = 0xFFFF0000;\
(a)[0x8ec/4] = 0xFFFF0000;\
(a)[0x8f0/4] = 0xFFFF0000;\
(a)[0x8f4/4] = 0xFFFF0000;\
(a)[0x8f8/4] = 0xFFFF0000;\
(a)[0x8fc/4] = 0xFFFF0000;\
(a)[0x900/4] = 0xFFFF0000;\
(a)[0x904/4] = 0xFFFF0000;\
(a)[0x908/4] = 0xFFFF0000;\
(a)[0x90c/4] = 0xFFFF0000;\
(a)[0x910/4] = 0xFFFF0000;\
(a)[0x914/4] = 0xFFFF0000;\
(a)[0x918/4] = 0xFFFF0000;\
(a)[0x91c/4] = 0xFFFF0000;\
(a)[0x920/4] = 0xFFFF0000;\
(a)[0x924/4] = 0xFFFF0000;\
(a)[0x928/4] = 0xFFFF0000;\
(a)[0x92c/4] = 0xFFFF0000;\
(a)[0x930/4] = 0xFFFF0000;\
(a)[0x934/4] = 0xFFFF0000;\
(a)[0x938/4] = 0xFFFF0000;\
(a)[0x93c/4] = 0xFFFF0000;\
(a)[0x940/4] = 0xFFFF0000;\
(a)[0x944/4] = 0xFFFF0000;\
(a)[0x948/4] = 0xFFFF0000;\
(a)[0x94c/4] = 0xFFFF0000;\
(a)[0x950/4] = 0xFFFF0000;\
(a)[0x954/4] = 0xFFFF0000;\
(a)[0x958/4] = 0xFFFF0000;\
(a)[0x95c/4] = 0xFFFF0000;\
(a)[0x960/4] = 0xFFFF0000;\
(a)[0x964/4] = 0xFFFF0000;\
(a)[0x968/4] = 0xFFFF0000;\
(a)[0x96c/4] = 0xFFFF0000;\
(a)[0x970/4] = 0xFFFF0000;\
(a)[0x974/4] = 0xFFFF0000;\
(a)[0x978/4] = 0xFFFF0000;\
(a)[0x97c/4] = 0xFFFF0000;\
(a)[0x980/4] = 0xFFFF0000;\
(a)[0x984/4] = 0xFFFF0000;\
(a)[0x988/4] = 0xFFFF0000;\
(a)[0x98c/4] = 0xFFFF0000;\
(a)[0x990/4] = 0xFFFF0000;\
(a)[0x994/4] = 0xFFFF0000;\
(a)[0x998/4] = 0xFFFF0000;\
(a)[0x99c/4] = 0xFFFF0000;\
(a)[0x9a0/4] = 0xFFFF0000;\
(a)[0x9a4/4] = 0xFFFF0000;\
(a)[0x9a8/4] = 0xFFFF0000;\
(a)[0x9ac/4] = 0xFFFF0000;\
(a)[0x9b0/4] = 0xFFFF0000;\
(a)[0x9b4/4] = 0xFFFF0000;\
(a)[0x9b8/4] = 0xFFFF0000;\
(a)[0x9bc/4] = 0xFFFF0000;\
(a)[0x9c0/4] = 0xFFFF0000;\
(a)[0x9c4/4] = 0xFFFF0000;\
(a)[0x9c8/4] = 0xFFFF0000;\
(a)[0x9cc/4] = 0xFFFF0000;\
(a)[0x9d0/4] = 0xFFFF0000;\
(a)[0x9d4/4] = 0xFFFF0000;\
(a)[0x9d8/4] = 0xFFFF0000;\
(a)[0x9dc/4] = 0xFFFF0000;\
(a)[0x9e0/4] = 0xFFFF0000;\
(a)[0x9e4/4] = 0xFFFF0000;\
(a)[0x9e8/4] = 0xFFFF0000;\
(a)[0x9ec/4] = 0xFFFF0000;\
(a)[0x9f0/4] = 0xFFFF0000;\
(a)[0x9f4/4] = 0xFFFF0000;\
(a)[0x9f8/4] = 0xFFFF0000;\
(a)[0x9fc/4] = 0xFFFF0000;\
(a)[0xa00/4] = 0xFFFF0000;\
(a)[0xa04/4] = 0xFFFF0000;\
(a)[0xa08/4] = 0xFFFF0000;\
(a)[0xa0c/4] = 0xFFFF0000;\
(a)[0xa10/4] = 0xFFFF0000;\
(a)[0xa14/4] = 0xFFFF0000;\
(a)[0xa18/4] = 0xFFFF0000;\
(a)[0xa1c/4] = 0xFFFF0000;\
(a)[0xa20/4] = 0xFFFF0000;\
(a)[0xa24/4] = 0xFFFF0000;\
(a)[0xa28/4] = 0xFFFF0000;\
(a)[0xa2c/4] = 0xFFFF0000;\
(a)[0xa30/4] = 0xFFFF0000;\
(a)[0xa34/4] = 0xFFFF0000;\
(a)[0xa38/4] = 0xFFFF0000;\
(a)[0xa3c/4] = 0xFFFF0000;\
(a)[0xa40/4] = 0xFFFF0000;\
(a)[0xa44/4] = 0xFFFF0000;\
(a)[0xa48/4] = 0xFFFF0000;\
(a)[0xa4c/4] = 0xFFFF0000;\
(a)[0xa50/4] = 0xFFFF0000;\
(a)[0xa54/4] = 0xFFFF0000;\
(a)[0xa58/4] = 0xFFFF0000;\
(a)[0xa5c/4] = 0xFFFF0000;\
(a)[0xa60/4] = 0xFFFF0000;\
(a)[0xa64/4] = 0xFFFF0000;\
(a)[0xa68/4] = 0xFFFF0000;\
(a)[0xa6c/4] = 0xFFFF0000;\
(a)[0xa70/4] = 0xFFFF0000;\
(a)[0xa74/4] = 0xFFFF0000;\
(a)[0xa78/4] = 0xFFFF0000;\
(a)[0xa7c/4] = 0xFFFF0000;\
(a)[0xa80/4] = 0xFFFF0000;\
(a)[0xa84/4] = 0xFFFF0000;\
(a)[0xa88/4] = 0xFFFF0000;\
(a)[0xa8c/4] = 0xFFFF0000;\
(a)[0xa90/4] = 0xFFFF0000;\
(a)[0xa94/4] = 0xFFFF0000;\
(a)[0xa98/4] = 0xFFFF0000;\
(a)[0xa9c/4] = 0xFFFF0000;\
(a)[0xaa0/4] = 0xFFFF0000;\
(a)[0xaa4/4] = 0xFFFF0000;\
(a)[0xaa8/4] = 0xFFFF0000;\
(a)[0xaac/4] = 0xFFFF0000;\
(a)[0xab0/4] = 0xFFFF0000;\
(a)[0xab4/4] = 0xFFFF0000;\
(a)[0xab8/4] = 0xFFFF0000;\
(a)[0xabc/4] = 0xFFFF0000;\
(a)[0xac0/4] = 0xFFFF0000;\
(a)[0xac4/4] = 0xFFFF0000;\
(a)[0xac8/4] = 0xFFFF0000;\
(a)[0xacc/4] = 0xFFFF0000;\
(a)[0xad0/4] = 0xFFFF0000;\
(a)[0xad4/4] = 0xFFFF0000;\
(a)[0xad8/4] = 0xFFFF0000;\
(a)[0xadc/4] = 0xFFFF0000;\
(a)[0xae0/4] = 0xFFFF0000;\
(a)[0xae4/4] = 0xFFFF0000;\
(a)[0xae8/4] = 0xFFFF0000;\
(a)[0xaec/4] = 0xFFFF0000;\
(a)[0xaf0/4] = 0xFFFF0000;\
(a)[0xaf4/4] = 0xFFFF0000;\
(a)[0xaf8/4] = 0xFFFF0000;\
(a)[0xafc/4] = 0xFFFF0000;\
(a)[0xb00/4] = 0xFFFF0000;\
(a)[0xb04/4] = 0xFFFF0000;\
(a)[0xb08/4] = 0xFFFF0000;\
(a)[0xb0c/4] = 0xFFFF0000;\
(a)[0xb10/4] = 0xFFFF0000;\
(a)[0xb14/4] = 0xFFFF0000;\
(a)[0xb18/4] = 0xFFFF0000;\
(a)[0xb1c/4] = 0xFFFF0000;\
(a)[0xb20/4] = 0xFFFF0000;\
(a)[0xb24/4] = 0xFFFF0000;\
(a)[0xb28/4] = 0xFFFF0000;\
(a)[0xb2c/4] = 0xFFFF0000;\
(a)[0xb30/4] = 0xFFFF0000;\
(a)[0xb34/4] = 0xFFFF0000;\
(a)[0xb38/4] = 0xFFFF0000;\
(a)[0xb3c/4] = 0xFFFF0000;\
(a)[0xb40/4] = 0xFFFF0000;\
(a)[0xb44/4] = 0xFFFF0000;\
(a)[0xb48/4] = 0xFFFF0000;\
(a)[0xb4c/4] = 0xFFFF0000;\
(a)[0xb50/4] = 0xFFFF0000;\
(a)[0xb54/4] = 0xFFFF0000;\
(a)[0xb58/4] = 0xFFFF0000;\
(a)[0xb5c/4] = 0xFFFF0000;\
(a)[0xb60/4] = 0xFFFF0000;\
(a)[0xb64/4] = 0xFFFF0000;\
(a)[0xb68/4] = 0xFFFF0000;\
(a)[0xb6c/4] = 0xFFFF0000;\
(a)[0xb70/4] = 0xFFFF0000;\
(a)[0xb74/4] = 0xFFFF0000;\
(a)[0xb78/4] = 0xFFFF0000;\
(a)[0xb7c/4] = 0xFFFF0000;\
(a)[0xb80/4] = 0xFFFF0000;\
(a)[0xb84/4] = 0xFFFF0000;\
(a)[0xb88/4] = 0xFFFF0000;\
(a)[0xb8c/4] = 0xFFFF0000;\
(a)[0xb90/4] = 0xFFFF0000;\
(a)[0xb94/4] = 0xFFFF0000;\
(a)[0xb98/4] = 0xFFFF0000;\
(a)[0xb9c/4] = 0xFFFF0000;\
(a)[0xba0/4] = 0xFFFF0000;\
(a)[0xba4/4] = 0xFFFF0000;\
(a)[0xba8/4] = 0xFFFF0000;\
(a)[0xbac/4] = 0xFFFF0000;\
(a)[0xbb0/4] = 0xFFFF0000;\
(a)[0xbb4/4] = 0xFFFF0000;\
(a)[0xbb8/4] = 0xFFFF0000;\
(a)[0xbbc/4] = 0xFFFF0000;\
(a)[0xbc0/4] = 0xFFFF0000;\
(a)[0xbc4/4] = 0xFFFF0000;\
(a)[0xbc8/4] = 0xFFFF0000;\
(a)[0xbcc/4] = 0xFFFF0000;\
(a)[0xbd0/4] = 0xFFFF0000;\
(a)[0xbd4/4] = 0xFFFF0000;\
(a)[0xbd8/4] = 0xFFFF0000;\
(a)[0xbdc/4] = 0xFFFF0000;\
(a)[0xbe0/4] = 0xFFFF0000;\
(a)[0xbe4/4] = 0xFFFF0000;\
(a)[0xbe8/4] = 0xFFFF0000;\
(a)[0xbec/4] = 0xFFFF0000;\
(a)[0xbf0/4] = 0xFFFF0000;\
(a)[0xbf4/4] = 0xFFFF0000;\
(a)[0xbf8/4] = 0xFFFF0000;\
(a)[0xbfc/4] = 0xFFFF0000;\
(a)[0xC00/4] = 0xFFFFFE00;\
(a)[0xC04/4] = 0xFFFFF003;\
(a)[0xC08/4] = 0xFFFFFFFF;\
(a)[0xC0C/4] = 0xFFFFFC00;\
(a)[0xC14/4] = 0xFFFE8080;\
(a)[0xF78/4] = 0x88888888;\
(a)[0xF98/4] = 0xFFFFFFFF;\
(a)[0xFA0/4] = 0xFFFF0000;\
(a)[0xFA4/4] = 0xFFFF0000;\
(a)[0xFA8/4] = 0xFFFF0000;\
(a)[0xFAC/4] = 0xFFFF0000;\
(a)[0xFB0/4] = 0xFFFF0000;\
(a)[0xFB4/4] = 0xFFFF0000;\
(a)[0xFB8/4] = 0xFFFF0000;\
(a)[0xFBC/4] = 0xFFFF0000;\
(a)[0xFC0/4] = 0xFFFFFF00;\
(a)[0xFC8/4] = 0xFFFFFF00;\
(a)[0xFD0/4] = 0xFFFFFF00;\
(a)[0xFD8/4] = 0xFFFFFF00;\
(a)[0xFE0/4] = 0xFFFFFF00;\
(a)[0xFE8/4] = 0xFFFFFF00;\
(a)[0xFF0/4] = 0xFFFFFF00;\
(a)[0xFF8/4] = 0xFFFFFF00
#define H2S_MASK_CSR_F1(a) (a)[0x00/4] = 0xFFFFFFFF;\
(a)[0x04/4] = 0xFFFFFFFD;\
(a)[0x08/4] = 0xFFFFFFFF;\
(a)[0x0C/4] = 0xFFFFFFFF;\
(a)[0x10/4] = 0xFFFFFFFF;\
(a)[0x14/4] = 0xFFFFFFFF;\
(a)[0x18/4] = 0xFFFFFFFF;\
(a)[0x1C/4] = 0xFFFFFFFF;\
(a)[0x20/4] = 0xFFFFFFFF;\
(a)[0x24/4] = 0xFFFFFFFF;\
(a)[0x28/4] = 0xFFFFFFFF;\
(a)[0x2C/4] = 0xFFFFFFFF;\
(a)[0x30/4] = 0xFFFFFFFF;\
(a)[0x34/4] = 0xFFFFFFFF;\
(a)[0x38/4] = 0xFFFFFFFF;\
(a)[0x3C/4] = 0xFFFFFFFF;\
(a)[0x40/4] = 0xFFFFFFFF;\
(a)[0x44/4] = 0xFFFFFFF8;\
(a)[0x48/4] = 0x000000FC;\
(a)[0x4C/4] = 0x000000F8;\
(a)[0x50/4] = 0x000000FC;\
(a)[0x54/4] = 0x00000078;\
(a)[0x58/4] = 0x000000D8;\
(a)[0x5C/4] = 0xE00000FE;\
(a)[0x60/4] = 0xFE000FF0;\
(a)[0x64/4] = 0xFE000FF8;\
(a)[0x68/4] = 0x0000FF88
#define H2S_MASK_CSR_G0(a) (a)[0xA00/4] = 0xFFFFFFFF;\
(a)[0xA04/4] = 0xFFFFFC00;\
(a)[0xA08/4] = 0xFFFFFFFF;\
(a)[0xA0C/4] = 0xFFFF0000;\
(a)[0xA10/4] = 0xFFFFF000;\
(a)[0xA14/4] = 0xFFFF0000;\
(a)[0xA18/4] = 0xFFFF0000;\
(a)[0xA1C/4] = 0xFFFF0000;\
(a)[0xA20/4] = 0xFFFF0000;\
(a)[0xA24/4] = 0xFFFFF000;\
(a)[0xA28/4] = 0xFFFFFF00;\
(a)[0xA2C/4] = 0xFFFF0000;\
(a)[0xA30/4] = 0xFFFFFFFF;\
(a)[0xA40/4] = 0xFFFFFFFF;\
(a)[0xA44/4] = 0xFFFFFC00;\
(a)[0xA48/4] = 0xFFFFFFFF;\
(a)[0xA4C/4] = 0xFFFF0000;\
(a)[0xA50/4] = 0xFFFFF000;\
(a)[0xA54/4] = 0xFFFF0000;\
(a)[0xA58/4] = 0xFFFF0000;\
(a)[0xA5C/4] = 0xFFFF0000;\
(a)[0xA60/4] = 0xFFFF0000;\
(a)[0xA64/4] = 0xFFFFF000;\
(a)[0xA68/4] = 0xFFFFFF00;\
(a)[0xA6C/4] = 0xFFFF0000;\
(a)[0xA70/4] = 0xFFFFFFFF;\
(a)[0xA80/4] = 0xFFFFFFFF;\
(a)[0xA84/4] = 0xFFFFFC00;\
(a)[0xA88/4] = 0xFFFFFFFF;\
(a)[0xA8C/4] = 0xFFFF0000;\
(a)[0xA90/4] = 0xFFFFF000;\
(a)[0xA94/4] = 0xFFFF0000;\
(a)[0xA98/4] = 0xFFFF0000;\
(a)[0xA9C/4] = 0xFFFF0000;\
(a)[0xAA0/4] = 0xFFFF0000;\
(a)[0xAA4/4] = 0xFFFFF000;\
(a)[0xAA8/4] = 0xFFFFFF00;\
(a)[0xAAC/4] = 0xFFFF0000;\
(a)[0xAB0/4] = 0xFFFFFFFF;\
(a)[0xAC0/4] = 0xFFFFFFFF;\
(a)[0xAC4/4] = 0xFFFFFC00;\
(a)[0xAC8/4] = 0xFFFFFFFF;\
(a)[0xACC/4] = 0xFFFF0000;\
(a)[0xAD0/4] = 0xFFFFF000;\
(a)[0xAD4/4] = 0xFFFF0000;\
(a)[0xAD8/4] = 0xFFFF0000;\
(a)[0xADC/4] = 0xFFFF0000;\
(a)[0xAE0/4] = 0xFFFF0000;\
(a)[0xAE4/4] = 0xFFFFF000;\
(a)[0xAE8/4] = 0xFFFFFF00;\
(a)[0xAEC/4] = 0xFFFF0000;\
(a)[0xAF0/4] = 0xFFFFFFFF;\
(a)[0xB00/4] = 0xFFFFFFFF;\
(a)[0xB04/4] = 0xFFFFFC00;\
(a)[0xB08/4] = 0xFFFFFFFF;\
(a)[0xB0C/4] = 0xFFFF0000;\
(a)[0xB10/4] = 0xFFFFF000;\
(a)[0xB14/4] = 0xFFFF0000;\
(a)[0xB18/4] = 0xFFFF0000;\
(a)[0xB1C/4] = 0xFFFF0000;\
(a)[0xB20/4] = 0xFFFF0000;\
(a)[0xB24/4] = 0xFFFFF000;\
(a)[0xB28/4] = 0xFFFFFF00;\
(a)[0xB2C/4] = 0xFFFF0000;\
(a)[0xB30/4] = 0xFFFFFFFF;\
(a)[0xB40/4] = 0xFFFFFFFF;\
(a)[0xB44/4] = 0xFFFFFC00;\
(a)[0xB48/4] = 0xFFFFFFFF;\
(a)[0xB4C/4] = 0xFFFF0000;\
(a)[0xB50/4] = 0xFFFFF000;\
(a)[0xB54/4] = 0xFFFF0000;\
(a)[0xB58/4] = 0xFFFF0000;\
(a)[0xB5C/4] = 0xFFFF0000;\
(a)[0xB60/4] = 0xFFFF0000;\
(a)[0xB64/4] = 0xFFFFF000;\
(a)[0xB68/4] = 0xFFFFFF00;\
(a)[0xB6C/4] = 0xFFFF0000;\
(a)[0xB70/4] = 0xFFFFFFFF;\
(a)[0xB80/4] = 0xFFFFFFFF;\
(a)[0xB84/4] = 0xFFFFF803
#define H2S_MASK_CSR_G4(a) (a)[0x700/4] = 0xFFFFFE00;\
(a)[0x704/4] = 0xFFFF8000;\
(a)[0x708/4] = 0xFFFFFFFF;\
(a)[0x70C/4] = 0xFFFFFFFF;\
(a)[0x710/4] = 0xFFFFFFC0;\
(a)[0x714/4] = 0x00000003;\
(a)[0x724/4] = 0xFFFFFFFF;\
(a)[0x800/4] = 0xFFFFFFFF;\
(a)[0xF00/4] = 0xFFFFFE00;\
(a)[0xF04/4] = 0xFFFF8000;\
(a)[0xF08/4] = 0xFFFFFFFF;\
(a)[0xF0C/4] = 0xFFFFFFFF;\
(a)[0xF10/4] = 0xFFFFFFC0;\
(a)[0xF14/4] = 0x00000003;\
(a)[0xF24/4] = 0xFFFFFFFF
#define NUMACHIP_STR_INIT_AUTO(cfg, csr) cfg[0][0x00/4] = "DEVICE_VENDOR_ID_REGISTER";\
cfg[0][0x04/4] = "STATUS_COMMAND_REGISTER";\
cfg[0][0x08/4] = "CLASS_CODE_REVISION_ID_REGISTER";\
cfg[0][0x0C/4] = "HEADER_TYPE_REGISTER";\
cfg[0][0x10/4] = "BASE_ADDRESS_REGISTER_0";\
cfg[0][0x14/4] = "BASE_ADDRESS_REGISTER_1";\
cfg[0][0x18/4] = "BASE_ADDRESS_REGISTER_2";\
cfg[0][0x1C/4] = "BASE_ADDRESS_REGISTER_3";\
cfg[0][0x20/4] = "BASE_ADDRESS_REGISTER_4";\
cfg[0][0x24/4] = "BASE_ADDRESS_REGISTER_5";\
cfg[0][0x28/4] = "CARDBUS_CIS_POINTER";\
cfg[0][0x2C/4] = "SUB_SYSTEM_ID_VENDOR_ID_REGISTER";\
cfg[0][0x30/4] = "EXPANSION_ROM_BASE_ADDRESS";\
cfg[0][0x34/4] = "CAPABILITIES_POINTER_REGISTER";\
cfg[0][0x38/4] = "RESERVED";\
cfg[0][0x3C/4] = "MAX_LATENCY_MIN_GNT_INT_PIN_INT_LINE_REGISTER";\
cfg[0][0x80/4] = "LINK_CAPABILITY_HEADER";\
cfg[0][0x84/4] = "LINK_CONTROL_REGISTER";\
cfg[0][0x88/4] = "LINK_FREQUENCY_REVISION_REGISTER";\
cfg[0][0x8C/4] = "LINK_FEATURE_CAPABILITY_REGISTER";\
cfg[0][0xA4/4] = "CHTX_COHERENT_LINK_CAPABILITY_HEADER";\
cfg[0][0xA8/4] = "LINK_BASE_CHANNEL_BUFFER_COUNT_REGISTER";\
cfg[0][0xAC/4] = "LINK_ISOCHRONOUS_CHANNEL_BUFFER_COUNT_REGISTERS";\
cfg[0][0xB0/4] = "LINK_TYPE_REGISTERS";\
cfg[0][0xC4/4] = "CHTX_COHERENCY_CAPABILITY_HEADER";\
cfg[0][0xC8/4] = "CHTX_NODE_ID";\
cfg[0][0xCC/4] = "CHTX_CPU_COUNT";\
cfg[0][0xD0/4] = "CHTX_UNIT_ID";\
cfg[0][0xD4/4] = "CHTX_LINK_TRANSACTION_CONTROL";\
cfg[0][0xD8/4] = "CHTX_LINK_INITIALIZATION_CONTROL";\
cfg[0][0xDC/4] = "CHTX_ADDITIONAL_LINK_TRANSACTION_CONTROL";\
cfg[0][0xE8/4] = "ROUTING_TABLE_CAPABILITY_HEADER";\
cfg[1][0x00/4] = "DEVICE_VENDOR_ID_REGISTER";\
cfg[1][0x04/4] = "STATUS_COMMAND_REGISTER";\
cfg[1][0x08/4] = "CLASS_CODE_REVISION_ID_REGISTER";\
cfg[1][0x0C/4] = "HEADER_TYPE_REGISTER";\
cfg[1][0x10/4] = "BASE_ADDRESS_REGISTER_0";\
cfg[1][0x14/4] = "BASE_ADDRESS_REGISTER_1";\
cfg[1][0x18/4] = "BASE_ADDRESS_REGISTER_2";\
cfg[1][0x1C/4] = "BASE_ADDRESS_REGISTER_3";\
cfg[1][0x20/4] = "BASE_ADDRESS_REGISTER_4";\
cfg[1][0x24/4] = "BASE_ADDRESS_REGISTER_5";\
cfg[1][0x28/4] = "CARDBUS_CIS_POINTER";\
cfg[1][0x2C/4] = "SUB_SYSTEM_ID_VENDOR_ID_REGISTER";\
cfg[1][0x30/4] = "EXPANSION_ROM_BASE_ADDRESS";\
cfg[1][0x34/4] = "CAPABILITIES_POINTER_REGISTER";\
cfg[1][0x38/4] = "RESERVED";\
cfg[1][0x3C/4] = "MAX_LATENCY_MIN_GNT_INT_PIN_INT_LINE_REGISTER";\
cfg[1][0x40/4] = "RESOURCE_MAPPING_CAPABILITY_HEADER";\
cfg[1][0x44/4] = "RESOURCE_MAPPING_ENTRY_INDEX";\
cfg[1][0x48/4] = "DRAM_BASE_ADDRESS_REGISTERS";\
cfg[1][0x4C/4] = "DRAM_LIMIT_ADDRESS_REGISTERS";\
cfg[1][0x50/4] = "MMIO_BASE_ADDRESS_REGISTERS";\
cfg[1][0x54/4] = "MMIO_LIMIT_ADDRESS_REGISTERS";\
cfg[1][0x58/4] = "EXT_D_MMIO_ADDRESS_BASE_REGISTERS";\
cfg[1][0x5C/4] = "EXT_D_MMIO_ADDRESS_MASK_REGISTERS";\
cfg[1][0x60/4] = "IO_SPACE_BASE_ADDRESS_REGISTERS";\
cfg[1][0x64/4] = "IO_SPACE_LIMIT_ADDRESS_REGISTERS";\
cfg[1][0x68/4] = "CONFIGURATION_MAP_REGISTERS";\
csr[0][0xA00/4] = "PHYXA_LINK_STAT";\
csr[0][0xA04/4] = "PHYXA_LINK_CTR";\
csr[0][0xA08/4] = "PHYXA_ELOG";\
csr[0][0xA0C/4] = "HSSXA_CTR_1";\
csr[0][0xA10/4] = "HSSXA_CTR_2";\
csr[0][0xA14/4] = "HSSXA_CTR_3";\
csr[0][0xA18/4] = "HSSXA_CTR_4";\
csr[0][0xA1C/4] = "HSSXA_CTR_5";\
csr[0][0xA20/4] = "HSSXA_CTR_6";\
csr[0][0xA24/4] = "HSSXA_CTR_7";\
csr[0][0xA28/4] = "HSSXA_CTR_8";\
csr[0][0xA2C/4] = "HSSXA_CTR_9";\
csr[0][0xA30/4] = "HSSXA_STAT_1";\
csr[0][0xA40/4] = "PHYXB_LINK_STAT";\
csr[0][0xA44/4] = "PHYXB_LINK_CTR";\
csr[0][0xA48/4] = "PHYXB_ELOG";\
csr[0][0xA4C/4] = "HSSXB_CTR_1";\
csr[0][0xA50/4] = "HSSXB_CTR_2";\
csr[0][0xA54/4] = "HSSXB_CTR_3";\
csr[0][0xA58/4] = "HSSXB_CTR_4";\
csr[0][0xA5C/4] = "HSSXB_CTR_5";\
csr[0][0xA60/4] = "HSSXB_CTR_6";\
csr[0][0xA64/4] = "HSSXB_CTR_7";\
csr[0][0xA68/4] = "HSSXB_CTR_8";\
csr[0][0xA6C/4] = "HSSXB_CTR_9";\
csr[0][0xA70/4] = "HSSXB_STAT_1";\
csr[0][0xA80/4] = "PHYYA_LINK_STAT";\
csr[0][0xA84/4] = "PHYYA_LINK_CTR";\
csr[0][0xA88/4] = "PHYYA_ELOG";\
csr[0][0xA8C/4] = "HSSYA_CTR_1";\
csr[0][0xA90/4] = "HSSYA_CTR_2";\
csr[0][0xA94/4] = "HSSYA_CTR_3";\
csr[0][0xA98/4] = "HSSYA_CTR_4";\
csr[0][0xA9C/4] = "HSSYA_CTR_5";\
csr[0][0xAA0/4] = "HSSYA_CTR_6";\
csr[0][0xAA4/4] = "HSSYA_CTR_7";\
csr[0][0xAA8/4] = "HSSYA_CTR_8";\
csr[0][0xAAC/4] = "HSSYA_CTR_9";\
csr[0][0xAB0/4] = "HSSYA_STAT_1";\
csr[0][0xAC0/4] = "PHYYB_LINK_STAT";\
csr[0][0xAC4/4] = "PHYYB_LINK_CTR";\
csr[0][0xAC8/4] = "PHYYB_ELOG";\
csr[0][0xACC/4] = "HSSYB_CTR_1";\
csr[0][0xAD0/4] = "HSSYB_CTR_2";\
csr[0][0xAD4/4] = "HSSYB_CTR_3";\
csr[0][0xAD8/4] = "HSSYB_CTR_4";\
csr[0][0xADC/4] = "HSSYB_CTR_5";\
csr[0][0xAE0/4] = "HSSYB_CTR_6";\
csr[0][0xAE4/4] = "HSSYB_CTR_7";\
csr[0][0xAE8/4] = "HSSYB_CTR_8";\
csr[0][0xAEC/4] = "HSSYB_CTR_9";\
csr[0][0xAF0/4] = "HSSYB_STAT_1";\
csr[0][0xB00/4] = "PHYZA_LINK_STAT";\
csr[0][0xB04/4] = "PHYZA_LINK_CTR";\
csr[0][0xB08/4] = "PHYZA_ELOG";\
csr[0][0xB0C/4] = "HSSZA_CTR_1";\
csr[0][0xB10/4] = "HSSZA_CTR_2";\
csr[0][0xB14/4] = "HSSZA_CTR_3";\
csr[0][0xB18/4] = "HSSZA_CTR_4";\
csr[0][0xB1C/4] = "HSSZA_CTR_5";\
csr[0][0xB20/4] = "HSSZA_CTR_6";\
csr[0][0xB24/4] = "HSSZA_CTR_7";\
csr[0][0xB28/4] = "HSSZA_CTR_8";\
csr[0][0xB2C/4] = "HSSZA_CTR_9";\
csr[0][0xB30/4] = "HSSZA_STAT_1";\
csr[0][0xB40/4] = "PHYZB_LINK_STAT";\
csr[0][0xB44/4] = "PHYZB_LINK_CTR";\
csr[0][0xB48/4] = "PHYZB_ELOG";\
csr[0][0xB4C/4] = "HSSZB_CTR_1";\
csr[0][0xB50/4] = "HSSZB_CTR_2";\
csr[0][0xB54/4] = "HSSZB_CTR_3";\
csr[0][0xB58/4] = "HSSZB_CTR_4";\
csr[0][0xB5C/4] = "HSSZB_CTR_5";\
csr[0][0xB60/4] = "HSSZB_CTR_6";\
csr[0][0xB64/4] = "HSSZB_CTR_7";\
csr[0][0xB68/4] = "HSSZB_CTR_8";\
csr[0][0xB6C/4] = "HSSZB_CTR_9";\
csr[0][0xB70/4] = "HSSZB_STAT_1";\
csr[0][0xB80/4] = "HSSPLL_CTR_1";\
csr[0][0xB84/4] = "HSSPLL_CTR_2";\
csr[2][0x800/4] = "F_PREF_MODE";\
csr[2][0x804/4] = "M_PREF_MODE";\
csr[2][0x808/4] = "SRAM_MODE";\
csr[2][0x80C/4] = "DIAG_SRAM_ADDR";\
csr[2][0x810/4] = "WR_DATA";\
csr[2][0x814/4] = "SRAM_DATA";\
csr[2][0x818/4] = "SRAM_LIMIT";\
csr[2][0x81C/4] = "FTAG_STATUS";\
csr[2][0x820/4] = "DDL_CTRL";\
csr[2][0x824/4] = "DDL_STATUS";\
csr[2][0x828/4] = "SRAM_PAD_CTRL";\
csr[3][0x008/4] = "APIC_MAP_SHIFT";\
csr[3][0x00C/4] = "CSR_BASE_ADDRESS";\
csr[3][0x010/4] = "MMCFG_BASE";\
csr[3][0x014/4] = "MMCFG_CONTROL";\
csr[3][0x018/4] = "PCI_SEG0";\
csr[3][0x01C/4] = "DRAM_SHARED_BASE";\
csr[3][0x020/4] = "DRAM_SHARED_LIMIT";\
csr[3][0x024/4] = "HT_NODEID";\
csr[3][0x028/4] = "CSR_HTX_RESET";\
csr[3][0x02C/4] = "FAB_CONTROL";\
csr[3][0x030/4] = "EXT_INTERRUPT_GEN";\
csr[3][0x034/4] = "EXT_INTERRUPT_STATUS";\
csr[3][0x038/4] = "EXT_INTERRUPT_DEST";\
csr[3][0x400/4] = "HREQ_CTRL";\
csr[3][0x404/4] = "HPRB_CTRL";\
csr[3][0x408/4] = "SREQ_CTRL";\
csr[3][0x40C/4] = "SPRB_CTRL";\
csr[3][0x410/4] = "H2S_CTRL";\
csr[3][0x418/4] = "TRANS_REC_CORE";\
csr[3][0x41C/4] = "SPI_INSTRUCTION_AND_STATUS";\
csr[3][0x420/4] = "EEPROM_ADDR_IIC_SMBUS_AND_AND_UID";\
csr[3][0x424/4] = "ERROR_STATUS";\
csr[3][0x428/4] = "CONTROLLER_IIC";\
csr[3][0x428/4] = "CONTROLLER_IIC";\
csr[3][0x42C/4] = "SPI_READ_WRITE_DATA";\
csr[3][0x430/4] = "EEPROM_ADDR_HSS_PLL_AND_HSS_DATA";\
csr[3][0x7F8/4] = "THE_INITIALIZATION_OF_THE_IBM_RAMS";\
csr[3][0x7FC/4] = "NC_ATT_MAP_SELECT";\
csr[3][0x800/4] = "NC_ATT_MAP_SELECT_0";\
csr[3][0x804/4] = "NC_ATT_MAP_SELECT_1";\
csr[3][0x808/4] = "NC_ATT_MAP_SELECT_2";\
csr[3][0x80c/4] = "NC_ATT_MAP_SELECT_3";\
csr[3][0x810/4] = "NC_ATT_MAP_SELECT_4";\
csr[3][0x814/4] = "NC_ATT_MAP_SELECT_5";\
csr[3][0x818/4] = "NC_ATT_MAP_SELECT_6";\
csr[3][0x81c/4] = "NC_ATT_MAP_SELECT_7";\
csr[3][0x820/4] = "NC_ATT_MAP_SELECT_8";\
csr[3][0x824/4] = "NC_ATT_MAP_SELECT_9";\
csr[3][0x828/4] = "NC_ATT_MAP_SELECT_10";\
csr[3][0x82c/4] = "NC_ATT_MAP_SELECT_11";\
csr[3][0x830/4] = "NC_ATT_MAP_SELECT_12";\
csr[3][0x834/4] = "NC_ATT_MAP_SELECT_13";\
csr[3][0x838/4] = "NC_ATT_MAP_SELECT_14";\
csr[3][0x83c/4] = "NC_ATT_MAP_SELECT_15";\
csr[3][0x840/4] = "NC_ATT_MAP_SELECT_16";\
csr[3][0x844/4] = "NC_ATT_MAP_SELECT_17";\
csr[3][0x848/4] = "NC_ATT_MAP_SELECT_18";\
csr[3][0x84c/4] = "NC_ATT_MAP_SELECT_19";\
csr[3][0x850/4] = "NC_ATT_MAP_SELECT_20";\
csr[3][0x854/4] = "NC_ATT_MAP_SELECT_21";\
csr[3][0x858/4] = "NC_ATT_MAP_SELECT_22";\
csr[3][0x85c/4] = "NC_ATT_MAP_SELECT_23";\
csr[3][0x860/4] = "NC_ATT_MAP_SELECT_24";\
csr[3][0x864/4] = "NC_ATT_MAP_SELECT_25";\
csr[3][0x868/4] = "NC_ATT_MAP_SELECT_26";\
csr[3][0x86c/4] = "NC_ATT_MAP_SELECT_27";\
csr[3][0x870/4] = "NC_ATT_MAP_SELECT_28";\
csr[3][0x874/4] = "NC_ATT_MAP_SELECT_29";\
csr[3][0x878/4] = "NC_ATT_MAP_SELECT_30";\
csr[3][0x87c/4] = "NC_ATT_MAP_SELECT_31";\
csr[3][0x880/4] = "NC_ATT_MAP_SELECT_32";\
csr[3][0x884/4] = "NC_ATT_MAP_SELECT_33";\
csr[3][0x888/4] = "NC_ATT_MAP_SELECT_34";\
csr[3][0x88c/4] = "NC_ATT_MAP_SELECT_35";\
csr[3][0x890/4] = "NC_ATT_MAP_SELECT_36";\
csr[3][0x894/4] = "NC_ATT_MAP_SELECT_37";\
csr[3][0x898/4] = "NC_ATT_MAP_SELECT_38";\
csr[3][0x89c/4] = "NC_ATT_MAP_SELECT_39";\
csr[3][0x8a0/4] = "NC_ATT_MAP_SELECT_40";\
csr[3][0x8a4/4] = "NC_ATT_MAP_SELECT_41";\
csr[3][0x8a8/4] = "NC_ATT_MAP_SELECT_42";\
csr[3][0x8ac/4] = "NC_ATT_MAP_SELECT_43";\
csr[3][0x8b0/4] = "NC_ATT_MAP_SELECT_44";\
csr[3][0x8b4/4] = "NC_ATT_MAP_SELECT_45";\
csr[3][0x8b8/4] = "NC_ATT_MAP_SELECT_46";\
csr[3][0x8bc/4] = "NC_ATT_MAP_SELECT_47";\
csr[3][0x8c0/4] = "NC_ATT_MAP_SELECT_48";\
csr[3][0x8c4/4] = "NC_ATT_MAP_SELECT_49";\
csr[3][0x8c8/4] = "NC_ATT_MAP_SELECT_50";\
csr[3][0x8cc/4] = "NC_ATT_MAP_SELECT_51";\
csr[3][0x8d0/4] = "NC_ATT_MAP_SELECT_52";\
csr[3][0x8d4/4] = "NC_ATT_MAP_SELECT_53";\
csr[3][0x8d8/4] = "NC_ATT_MAP_SELECT_54";\
csr[3][0x8dc/4] = "NC_ATT_MAP_SELECT_55";\
csr[3][0x8e0/4] = "NC_ATT_MAP_SELECT_56";\
csr[3][0x8e4/4] = "NC_ATT_MAP_SELECT_57";\
csr[3][0x8e8/4] = "NC_ATT_MAP_SELECT_58";\
csr[3][0x8ec/4] = "NC_ATT_MAP_SELECT_59";\
csr[3][0x8f0/4] = "NC_ATT_MAP_SELECT_60";\
csr[3][0x8f4/4] = "NC_ATT_MAP_SELECT_61";\
csr[3][0x8f8/4] = "NC_ATT_MAP_SELECT_62";\
csr[3][0x8fc/4] = "NC_ATT_MAP_SELECT_63";\
csr[3][0x900/4] = "NC_ATT_MAP_SELECT_64";\
csr[3][0x904/4] = "NC_ATT_MAP_SELECT_65";\
csr[3][0x908/4] = "NC_ATT_MAP_SELECT_66";\
csr[3][0x90c/4] = "NC_ATT_MAP_SELECT_67";\
csr[3][0x910/4] = "NC_ATT_MAP_SELECT_68";\
csr[3][0x914/4] = "NC_ATT_MAP_SELECT_69";\
csr[3][0x918/4] = "NC_ATT_MAP_SELECT_70";\
csr[3][0x91c/4] = "NC_ATT_MAP_SELECT_71";\
csr[3][0x920/4] = "NC_ATT_MAP_SELECT_72";\
csr[3][0x924/4] = "NC_ATT_MAP_SELECT_73";\
csr[3][0x928/4] = "NC_ATT_MAP_SELECT_74";\
csr[3][0x92c/4] = "NC_ATT_MAP_SELECT_75";\
csr[3][0x930/4] = "NC_ATT_MAP_SELECT_76";\
csr[3][0x934/4] = "NC_ATT_MAP_SELECT_77";\
csr[3][0x938/4] = "NC_ATT_MAP_SELECT_78";\
csr[3][0x93c/4] = "NC_ATT_MAP_SELECT_79";\
csr[3][0x940/4] = "NC_ATT_MAP_SELECT_80";\
csr[3][0x944/4] = "NC_ATT_MAP_SELECT_81";\
csr[3][0x948/4] = "NC_ATT_MAP_SELECT_82";\
csr[3][0x94c/4] = "NC_ATT_MAP_SELECT_83";\
csr[3][0x950/4] = "NC_ATT_MAP_SELECT_84";\
csr[3][0x954/4] = "NC_ATT_MAP_SELECT_85";\
csr[3][0x958/4] = "NC_ATT_MAP_SELECT_86";\
csr[3][0x95c/4] = "NC_ATT_MAP_SELECT_87";\
csr[3][0x960/4] = "NC_ATT_MAP_SELECT_88";\
csr[3][0x964/4] = "NC_ATT_MAP_SELECT_89";\
csr[3][0x968/4] = "NC_ATT_MAP_SELECT_90";\
csr[3][0x96c/4] = "NC_ATT_MAP_SELECT_91";\
csr[3][0x970/4] = "NC_ATT_MAP_SELECT_92";\
csr[3][0x974/4] = "NC_ATT_MAP_SELECT_93";\
csr[3][0x978/4] = "NC_ATT_MAP_SELECT_94";\
csr[3][0x97c/4] = "NC_ATT_MAP_SELECT_95";\
csr[3][0x980/4] = "NC_ATT_MAP_SELECT_96";\
csr[3][0x984/4] = "NC_ATT_MAP_SELECT_97";\
csr[3][0x988/4] = "NC_ATT_MAP_SELECT_98";\
csr[3][0x98c/4] = "NC_ATT_MAP_SELECT_99";\
csr[3][0x990/4] = "NC_ATT_MAP_SELECT_100";\
csr[3][0x994/4] = "NC_ATT_MAP_SELECT_101";\
csr[3][0x998/4] = "NC_ATT_MAP_SELECT_102";\
csr[3][0x99c/4] = "NC_ATT_MAP_SELECT_103";\
csr[3][0x9a0/4] = "NC_ATT_MAP_SELECT_104";\
csr[3][0x9a4/4] = "NC_ATT_MAP_SELECT_105";\
csr[3][0x9a8/4] = "NC_ATT_MAP_SELECT_106";\
csr[3][0x9ac/4] = "NC_ATT_MAP_SELECT_107";\
csr[3][0x9b0/4] = "NC_ATT_MAP_SELECT_108";\
csr[3][0x9b4/4] = "NC_ATT_MAP_SELECT_109";\
csr[3][0x9b8/4] = "NC_ATT_MAP_SELECT_110";\
csr[3][0x9bc/4] = "NC_ATT_MAP_SELECT_111";\
csr[3][0x9c0/4] = "NC_ATT_MAP_SELECT_112";\
csr[3][0x9c4/4] = "NC_ATT_MAP_SELECT_113";\
csr[3][0x9c8/4] = "NC_ATT_MAP_SELECT_114";\
csr[3][0x9cc/4] = "NC_ATT_MAP_SELECT_115";\
csr[3][0x9d0/4] = "NC_ATT_MAP_SELECT_116";\
csr[3][0x9d4/4] = "NC_ATT_MAP_SELECT_117";\
csr[3][0x9d8/4] = "NC_ATT_MAP_SELECT_118";\
csr[3][0x9dc/4] = "NC_ATT_MAP_SELECT_119";\
csr[3][0x9e0/4] = "NC_ATT_MAP_SELECT_120";\
csr[3][0x9e4/4] = "NC_ATT_MAP_SELECT_121";\
csr[3][0x9e8/4] = "NC_ATT_MAP_SELECT_122";\
csr[3][0x9ec/4] = "NC_ATT_MAP_SELECT_123";\
csr[3][0x9f0/4] = "NC_ATT_MAP_SELECT_124";\
csr[3][0x9f4/4] = "NC_ATT_MAP_SELECT_125";\
csr[3][0x9f8/4] = "NC_ATT_MAP_SELECT_126";\
csr[3][0x9fc/4] = "NC_ATT_MAP_SELECT_127";\
csr[3][0xa00/4] = "NC_ATT_MAP_SELECT_128";\
csr[3][0xa04/4] = "NC_ATT_MAP_SELECT_129";\
csr[3][0xa08/4] = "NC_ATT_MAP_SELECT_130";\
csr[3][0xa0c/4] = "NC_ATT_MAP_SELECT_131";\
csr[3][0xa10/4] = "NC_ATT_MAP_SELECT_132";\
csr[3][0xa14/4] = "NC_ATT_MAP_SELECT_133";\
csr[3][0xa18/4] = "NC_ATT_MAP_SELECT_134";\
csr[3][0xa1c/4] = "NC_ATT_MAP_SELECT_135";\
csr[3][0xa20/4] = "NC_ATT_MAP_SELECT_136";\
csr[3][0xa24/4] = "NC_ATT_MAP_SELECT_137";\
csr[3][0xa28/4] = "NC_ATT_MAP_SELECT_138";\
csr[3][0xa2c/4] = "NC_ATT_MAP_SELECT_139";\
csr[3][0xa30/4] = "NC_ATT_MAP_SELECT_140";\
csr[3][0xa34/4] = "NC_ATT_MAP_SELECT_141";\
csr[3][0xa38/4] = "NC_ATT_MAP_SELECT_142";\
csr[3][0xa3c/4] = "NC_ATT_MAP_SELECT_143";\
csr[3][0xa40/4] = "NC_ATT_MAP_SELECT_144";\
csr[3][0xa44/4] = "NC_ATT_MAP_SELECT_145";\
csr[3][0xa48/4] = "NC_ATT_MAP_SELECT_146";\
csr[3][0xa4c/4] = "NC_ATT_MAP_SELECT_147";\
csr[3][0xa50/4] = "NC_ATT_MAP_SELECT_148";\
csr[3][0xa54/4] = "NC_ATT_MAP_SELECT_149";\
csr[3][0xa58/4] = "NC_ATT_MAP_SELECT_150";\
csr[3][0xa5c/4] = "NC_ATT_MAP_SELECT_151";\
csr[3][0xa60/4] = "NC_ATT_MAP_SELECT_152";\
csr[3][0xa64/4] = "NC_ATT_MAP_SELECT_153";\
csr[3][0xa68/4] = "NC_ATT_MAP_SELECT_154";\
csr[3][0xa6c/4] = "NC_ATT_MAP_SELECT_155";\
csr[3][0xa70/4] = "NC_ATT_MAP_SELECT_156";\
csr[3][0xa74/4] = "NC_ATT_MAP_SELECT_157";\
csr[3][0xa78/4] = "NC_ATT_MAP_SELECT_158";\
csr[3][0xa7c/4] = "NC_ATT_MAP_SELECT_159";\
csr[3][0xa80/4] = "NC_ATT_MAP_SELECT_160";\
csr[3][0xa84/4] = "NC_ATT_MAP_SELECT_161";\
csr[3][0xa88/4] = "NC_ATT_MAP_SELECT_162";\
csr[3][0xa8c/4] = "NC_ATT_MAP_SELECT_163";\
csr[3][0xa90/4] = "NC_ATT_MAP_SELECT_164";\
csr[3][0xa94/4] = "NC_ATT_MAP_SELECT_165";\
csr[3][0xa98/4] = "NC_ATT_MAP_SELECT_166";\
csr[3][0xa9c/4] = "NC_ATT_MAP_SELECT_167";\
csr[3][0xaa0/4] = "NC_ATT_MAP_SELECT_168";\
csr[3][0xaa4/4] = "NC_ATT_MAP_SELECT_169";\
csr[3][0xaa8/4] = "NC_ATT_MAP_SELECT_170";\
csr[3][0xaac/4] = "NC_ATT_MAP_SELECT_171";\
csr[3][0xab0/4] = "NC_ATT_MAP_SELECT_172";\
csr[3][0xab4/4] = "NC_ATT_MAP_SELECT_173";\
csr[3][0xab8/4] = "NC_ATT_MAP_SELECT_174";\
csr[3][0xabc/4] = "NC_ATT_MAP_SELECT_175";\
csr[3][0xac0/4] = "NC_ATT_MAP_SELECT_176";\
csr[3][0xac4/4] = "NC_ATT_MAP_SELECT_177";\
csr[3][0xac8/4] = "NC_ATT_MAP_SELECT_178";\
csr[3][0xacc/4] = "NC_ATT_MAP_SELECT_179";\
csr[3][0xad0/4] = "NC_ATT_MAP_SELECT_180";\
csr[3][0xad4/4] = "NC_ATT_MAP_SELECT_181";\
csr[3][0xad8/4] = "NC_ATT_MAP_SELECT_182";\
csr[3][0xadc/4] = "NC_ATT_MAP_SELECT_183";\
csr[3][0xae0/4] = "NC_ATT_MAP_SELECT_184";\
csr[3][0xae4/4] = "NC_ATT_MAP_SELECT_185";\
csr[3][0xae8/4] = "NC_ATT_MAP_SELECT_186";\
csr[3][0xaec/4] = "NC_ATT_MAP_SELECT_187";\
csr[3][0xaf0/4] = "NC_ATT_MAP_SELECT_188";\
csr[3][0xaf4/4] = "NC_ATT_MAP_SELECT_189";\
csr[3][0xaf8/4] = "NC_ATT_MAP_SELECT_190";\
csr[3][0xafc/4] = "NC_ATT_MAP_SELECT_191";\
csr[3][0xb00/4] = "NC_ATT_MAP_SELECT_192";\
csr[3][0xb04/4] = "NC_ATT_MAP_SELECT_193";\
csr[3][0xb08/4] = "NC_ATT_MAP_SELECT_194";\
csr[3][0xb0c/4] = "NC_ATT_MAP_SELECT_195";\
csr[3][0xb10/4] = "NC_ATT_MAP_SELECT_196";\
csr[3][0xb14/4] = "NC_ATT_MAP_SELECT_197";\
csr[3][0xb18/4] = "NC_ATT_MAP_SELECT_198";\
csr[3][0xb1c/4] = "NC_ATT_MAP_SELECT_199";\
csr[3][0xb20/4] = "NC_ATT_MAP_SELECT_200";\
csr[3][0xb24/4] = "NC_ATT_MAP_SELECT_201";\
csr[3][0xb28/4] = "NC_ATT_MAP_SELECT_202";\
csr[3][0xb2c/4] = "NC_ATT_MAP_SELECT_203";\
csr[3][0xb30/4] = "NC_ATT_MAP_SELECT_204";\
csr[3][0xb34/4] = "NC_ATT_MAP_SELECT_205";\
csr[3][0xb38/4] = "NC_ATT_MAP_SELECT_206";\
csr[3][0xb3c/4] = "NC_ATT_MAP_SELECT_207";\
csr[3][0xb40/4] = "NC_ATT_MAP_SELECT_208";\
csr[3][0xb44/4] = "NC_ATT_MAP_SELECT_209";\
csr[3][0xb48/4] = "NC_ATT_MAP_SELECT_210";\
csr[3][0xb4c/4] = "NC_ATT_MAP_SELECT_211";\
csr[3][0xb50/4] = "NC_ATT_MAP_SELECT_212";\
csr[3][0xb54/4] = "NC_ATT_MAP_SELECT_213";\
csr[3][0xb58/4] = "NC_ATT_MAP_SELECT_214";\
csr[3][0xb5c/4] = "NC_ATT_MAP_SELECT_215";\
csr[3][0xb60/4] = "NC_ATT_MAP_SELECT_216";\
csr[3][0xb64/4] = "NC_ATT_MAP_SELECT_217";\
csr[3][0xb68/4] = "NC_ATT_MAP_SELECT_218";\
csr[3][0xb6c/4] = "NC_ATT_MAP_SELECT_219";\
csr[3][0xb70/4] = "NC_ATT_MAP_SELECT_220";\
csr[3][0xb74/4] = "NC_ATT_MAP_SELECT_221";\
csr[3][0xb78/4] = "NC_ATT_MAP_SELECT_222";\
csr[3][0xb7c/4] = "NC_ATT_MAP_SELECT_223";\
csr[3][0xb80/4] = "NC_ATT_MAP_SELECT_224";\
csr[3][0xb84/4] = "NC_ATT_MAP_SELECT_225";\
csr[3][0xb88/4] = "NC_ATT_MAP_SELECT_226";\
csr[3][0xb8c/4] = "NC_ATT_MAP_SELECT_227";\
csr[3][0xb90/4] = "NC_ATT_MAP_SELECT_228";\
csr[3][0xb94/4] = "NC_ATT_MAP_SELECT_229";\
csr[3][0xb98/4] = "NC_ATT_MAP_SELECT_230";\
csr[3][0xb9c/4] = "NC_ATT_MAP_SELECT_231";\
csr[3][0xba0/4] = "NC_ATT_MAP_SELECT_232";\
csr[3][0xba4/4] = "NC_ATT_MAP_SELECT_233";\
csr[3][0xba8/4] = "NC_ATT_MAP_SELECT_234";\
csr[3][0xbac/4] = "NC_ATT_MAP_SELECT_235";\
csr[3][0xbb0/4] = "NC_ATT_MAP_SELECT_236";\
csr[3][0xbb4/4] = "NC_ATT_MAP_SELECT_237";\
csr[3][0xbb8/4] = "NC_ATT_MAP_SELECT_238";\
csr[3][0xbbc/4] = "NC_ATT_MAP_SELECT_239";\
csr[3][0xbc0/4] = "NC_ATT_MAP_SELECT_240";\
csr[3][0xbc4/4] = "NC_ATT_MAP_SELECT_241";\
csr[3][0xbc8/4] = "NC_ATT_MAP_SELECT_242";\
csr[3][0xbcc/4] = "NC_ATT_MAP_SELECT_243";\
csr[3][0xbd0/4] = "NC_ATT_MAP_SELECT_244";\
csr[3][0xbd4/4] = "NC_ATT_MAP_SELECT_245";\
csr[3][0xbd8/4] = "NC_ATT_MAP_SELECT_246";\
csr[3][0xbdc/4] = "NC_ATT_MAP_SELECT_247";\
csr[3][0xbe0/4] = "NC_ATT_MAP_SELECT_248";\
csr[3][0xbe4/4] = "NC_ATT_MAP_SELECT_249";\
csr[3][0xbe8/4] = "NC_ATT_MAP_SELECT_250";\
csr[3][0xbec/4] = "NC_ATT_MAP_SELECT_251";\
csr[3][0xbf0/4] = "NC_ATT_MAP_SELECT_252";\
csr[3][0xbf4/4] = "NC_ATT_MAP_SELECT_253";\
csr[3][0xbf8/4] = "NC_ATT_MAP_SELECT_254";\
csr[3][0xbfc/4] = "NC_ATT_MAP_SELECT_255";\
csr[3][0xC00/4] = "WATCH_BUS_SELECT";\
csr[3][0xC04/4] = "TRACERCTRL";\
csr[3][0xC08/4] = "TRACERSTAT";\
csr[3][0xC0C/4] = "TRACER_EVENT_ADDRESS_UPPER_BITS";\
csr[3][0xC10/4] = "TRACER_EVENT_ADDRESS_LOWER_BITS";\
csr[3][0xC14/4] = "TRACER_SELECT_COMPARE_AND_MASK";\
csr[3][0xF78/4] = "SELECT_COUNTER";\
csr[3][0xF98/4] = "OVERFLOW_COUNTER_0_7";\
csr[3][0xF9C/4] = "TIMER_FOR_ECC_COUNTER_7";\
csr[3][0xFA0/4] = "COMPARE_AND_MASK_OF_COUNTER_0";\
csr[3][0xFA4/4] = "COMPARE_AND_MASK_OF_COUNTER_1";\
csr[3][0xFA8/4] = "COMPARE_AND_MASK_OF_COUNTER_2";\
csr[3][0xFAC/4] = "COMPARE_AND_MASK_OF_COUNTER_3";\
csr[3][0xFB0/4] = "COMPARE_AND_MASK_OF_COUNTER_4";\
csr[3][0xFB4/4] = "COMPARE_AND_MASK_OF_COUNTER_5";\
csr[3][0xFB8/4] = "COMPARE_AND_MASK_OF_COUNTER_6";\
csr[3][0xFBC/4] = "COMPARE_AND_MASK_OF_COUNTER_7";\
csr[3][0xFC0/4] = "PERFORMANCE_COUNTER_0_40_BIT_UPPER_BITS";\
csr[3][0xFC4/4] = "PERFORMANCE_COUNTER_0_40_BIT_LOWER_BITS";\
csr[3][0xFC8/4] = "PERFORMANCE_COUNTER_1_40_BIT_UPPER_BITS";\
csr[3][0xFCC/4] = "PERFORMANCE_COUNTER_1_40_BIT_LOWER_BITS";\
csr[3][0xFD0/4] = "PERFORMANCE_COUNTER_2_40_BIT_UPPER_BITS";\
csr[3][0xFD4/4] = "PERFORMANCE_COUNTER_2_40_BIT_LOWER_BITS";\
csr[3][0xFD8/4] = "PERFORMANCE_COUNTER_3_40_BIT_UPPER_BITS";\
csr[3][0xFDC/4] = "PERFORMANCE_COUNTER_3_40_BIT_LOWER_BITS";\
csr[3][0xFE0/4] = "PERFORMANCE_COUNTER_4_40_BIT_UPPER_BITS";\
csr[3][0xFE4/4] = "PERFORMANCE_COUNTER_4_40_BIT_LOWER_BITS";\
csr[3][0xFE8/4] = "PERFORMANCE_COUNTER_5_40_BIT_UPPER_BITS";\
csr[3][0xFEC/4] = "PERFORMANCE_COUNTER_5_40_BIT_LOWER_BITS";\
csr[3][0xFF0/4] = "PERFORMANCE_COUNTER_6_40_BIT_UPPER_BITS";\
csr[3][0xFF4/4] = "PERFORMANCE_COUNTER_6_40_BIT_LOWER_BITS";\
csr[3][0xFF8/4] = "PERFORMANCE_COUNTER_7_40_BIT_UPPER_BITS";\
csr[3][0xFFC/4] = "PERFORMANCE_COUNTER_7_40_BIT_LOWER_BITS";\
csr[4][0x700/4] = "MCTAG_MAINTR";\
csr[4][0x704/4] = "MCTAG_COM_CTRLR";\
csr[4][0x708/4] = "MCTAG_COM_STATR";\
csr[4][0x70C/4] = "MCTAG_ERROR_STATR";\
csr[4][0x710/4] = "MCTAG_ERROR_MASK";\
csr[4][0x714/4] = "MCTAG_SCRUBBER_ADDR";\
csr[4][0x718/4] = "MCTAG_MEMORY_ADDR";\
csr[4][0x71C/4] = "MCTAG_MEMORY_WDATA";\
csr[4][0x720/4] = "MCTAG_MEMORY_RDATA";\
csr[4][0x724/4] = "MCTAG_C1B_MEM_ADDR";\
csr[4][0x800/4] = "G2XAA8";\
csr[4][0xF00/4] = "CDATA_MAINTR";\
csr[4][0xF04/4] = "CDATA_COM_CTRLR";\
csr[4][0xF08/4] = "CDATA_COM_STATR";\
csr[4][0xF0C/4] = "CDATA_ERROR_STATR";\
csr[4][0xF10/4] = "CDATA_ERROR_MASK";\
csr[4][0xF14/4] = "CDATA_SCRUBBER_ADDR";\
csr[4][0xF18/4] = "CDATA_MEMORY_ADDR";\
csr[4][0xF1C/4] = "CDATA_MEMORY_WDATA";\
csr[4][0xF20/4] = "CDATA_MEMORY_RDATA";\
csr[4][0xF24/4] = "CDATA_C1B_MEM_ADDR";\
csr[5][0x0/4] = "TRACER_RAM_1024X_32BIT_0";\
csr[5][0x4/4] = "TRACER_RAM_1024X_32BIT_1";\
csr[5][0x8/4] = "TRACER_RAM_1024X_32BIT_2";\
csr[5][0xc/4] = "TRACER_RAM_1024X_32BIT_3";\
csr[5][0x10/4] = "TRACER_RAM_1024X_32BIT_4";\
csr[5][0x14/4] = "TRACER_RAM_1024X_32BIT_5";\
csr[5][0x18/4] = "TRACER_RAM_1024X_32BIT_6";\
csr[5][0x1c/4] = "TRACER_RAM_1024X_32BIT_7";\
csr[5][0x20/4] = "TRACER_RAM_1024X_32BIT_8";\
csr[5][0x24/4] = "TRACER_RAM_1024X_32BIT_9";\
csr[5][0x28/4] = "TRACER_RAM_1024X_32BIT_10";\
csr[5][0x2c/4] = "TRACER_RAM_1024X_32BIT_11";\
csr[5][0x30/4] = "TRACER_RAM_1024X_32BIT_12";\
csr[5][0x34/4] = "TRACER_RAM_1024X_32BIT_13";\
csr[5][0x38/4] = "TRACER_RAM_1024X_32BIT_14";\
csr[5][0x3c/4] = "TRACER_RAM_1024X_32BIT_15";\
csr[5][0x40/4] = "TRACER_RAM_1024X_32BIT_16";\
csr[5][0x44/4] = "TRACER_RAM_1024X_32BIT_17";\
csr[5][0x48/4] = "TRACER_RAM_1024X_32BIT_18";\
csr[5][0x4c/4] = "TRACER_RAM_1024X_32BIT_19";\
csr[5][0x50/4] = "TRACER_RAM_1024X_32BIT_20";\
csr[5][0x54/4] = "TRACER_RAM_1024X_32BIT_21";\
csr[5][0x58/4] = "TRACER_RAM_1024X_32BIT_22";\
csr[5][0x5c/4] = "TRACER_RAM_1024X_32BIT_23";\
csr[5][0x60/4] = "TRACER_RAM_1024X_32BIT_24";\
csr[5][0x64/4] = "TRACER_RAM_1024X_32BIT_25";\
csr[5][0x68/4] = "TRACER_RAM_1024X_32BIT_26";\
csr[5][0x6c/4] = "TRACER_RAM_1024X_32BIT_27";\
csr[5][0x70/4] = "TRACER_RAM_1024X_32BIT_28";\
csr[5][0x74/4] = "TRACER_RAM_1024X_32BIT_29";\
csr[5][0x78/4] = "TRACER_RAM_1024X_32BIT_30";\
csr[5][0x7c/4] = "TRACER_RAM_1024X_32BIT_31";\
csr[5][0x80/4] = "TRACER_RAM_1024X_32BIT_32";\
csr[5][0x84/4] = "TRACER_RAM_1024X_32BIT_33";\
csr[5][0x88/4] = "TRACER_RAM_1024X_32BIT_34";\
csr[5][0x8c/4] = "TRACER_RAM_1024X_32BIT_35";\
csr[5][0x90/4] = "TRACER_RAM_1024X_32BIT_36";\
csr[5][0x94/4] = "TRACER_RAM_1024X_32BIT_37";\
csr[5][0x98/4] = "TRACER_RAM_1024X_32BIT_38";\
csr[5][0x9c/4] = "TRACER_RAM_1024X_32BIT_39";\
csr[5][0xa0/4] = "TRACER_RAM_1024X_32BIT_40";\
csr[5][0xa4/4] = "TRACER_RAM_1024X_32BIT_41";\
csr[5][0xa8/4] = "TRACER_RAM_1024X_32BIT_42";\
csr[5][0xac/4] = "TRACER_RAM_1024X_32BIT_43";\
csr[5][0xb0/4] = "TRACER_RAM_1024X_32BIT_44";\
csr[5][0xb4/4] = "TRACER_RAM_1024X_32BIT_45";\
csr[5][0xb8/4] = "TRACER_RAM_1024X_32BIT_46";\
csr[5][0xbc/4] = "TRACER_RAM_1024X_32BIT_47";\
csr[5][0xc0/4] = "TRACER_RAM_1024X_32BIT_48";\
csr[5][0xc4/4] = "TRACER_RAM_1024X_32BIT_49";\
csr[5][0xc8/4] = "TRACER_RAM_1024X_32BIT_50";\
csr[5][0xcc/4] = "TRACER_RAM_1024X_32BIT_51";\
csr[5][0xd0/4] = "TRACER_RAM_1024X_32BIT_52";\
csr[5][0xd4/4] = "TRACER_RAM_1024X_32BIT_53";\
csr[5][0xd8/4] = "TRACER_RAM_1024X_32BIT_54";\
csr[5][0xdc/4] = "TRACER_RAM_1024X_32BIT_55";\
csr[5][0xe0/4] = "TRACER_RAM_1024X_32BIT_56";\
csr[5][0xe4/4] = "TRACER_RAM_1024X_32BIT_57";\
csr[5][0xe8/4] = "TRACER_RAM_1024X_32BIT_58";\
csr[5][0xec/4] = "TRACER_RAM_1024X_32BIT_59";\
csr[5][0xf0/4] = "TRACER_RAM_1024X_32BIT_60";\
csr[5][0xf4/4] = "TRACER_RAM_1024X_32BIT_61";\
csr[5][0xf8/4] = "TRACER_RAM_1024X_32BIT_62";\
csr[5][0xfc/4] = "TRACER_RAM_1024X_32BIT_63";\
csr[5][0x100/4] = "TRACER_RAM_1024X_32BIT_64";\
csr[5][0x104/4] = "TRACER_RAM_1024X_32BIT_65";\
csr[5][0x108/4] = "TRACER_RAM_1024X_32BIT_66";\
csr[5][0x10c/4] = "TRACER_RAM_1024X_32BIT_67";\
csr[5][0x110/4] = "TRACER_RAM_1024X_32BIT_68";\
csr[5][0x114/4] = "TRACER_RAM_1024X_32BIT_69";\
csr[5][0x118/4] = "TRACER_RAM_1024X_32BIT_70";\
csr[5][0x11c/4] = "TRACER_RAM_1024X_32BIT_71";\
csr[5][0x120/4] = "TRACER_RAM_1024X_32BIT_72";\
csr[5][0x124/4] = "TRACER_RAM_1024X_32BIT_73";\
csr[5][0x128/4] = "TRACER_RAM_1024X_32BIT_74";\
csr[5][0x12c/4] = "TRACER_RAM_1024X_32BIT_75";\
csr[5][0x130/4] = "TRACER_RAM_1024X_32BIT_76";\
csr[5][0x134/4] = "TRACER_RAM_1024X_32BIT_77";\
csr[5][0x138/4] = "TRACER_RAM_1024X_32BIT_78";\
csr[5][0x13c/4] = "TRACER_RAM_1024X_32BIT_79";\
csr[5][0x140/4] = "TRACER_RAM_1024X_32BIT_80";\
csr[5][0x144/4] = "TRACER_RAM_1024X_32BIT_81";\
csr[5][0x148/4] = "TRACER_RAM_1024X_32BIT_82";\
csr[5][0x14c/4] = "TRACER_RAM_1024X_32BIT_83";\
csr[5][0x150/4] = "TRACER_RAM_1024X_32BIT_84";\
csr[5][0x154/4] = "TRACER_RAM_1024X_32BIT_85";\
csr[5][0x158/4] = "TRACER_RAM_1024X_32BIT_86";\
csr[5][0x15c/4] = "TRACER_RAM_1024X_32BIT_87";\
csr[5][0x160/4] = "TRACER_RAM_1024X_32BIT_88";\
csr[5][0x164/4] = "TRACER_RAM_1024X_32BIT_89";\
csr[5][0x168/4] = "TRACER_RAM_1024X_32BIT_90";\
csr[5][0x16c/4] = "TRACER_RAM_1024X_32BIT_91";\
csr[5][0x170/4] = "TRACER_RAM_1024X_32BIT_92";\
csr[5][0x174/4] = "TRACER_RAM_1024X_32BIT_93";\
csr[5][0x178/4] = "TRACER_RAM_1024X_32BIT_94";\
csr[5][0x17c/4] = "TRACER_RAM_1024X_32BIT_95";\
csr[5][0x180/4] = "TRACER_RAM_1024X_32BIT_96";\
csr[5][0x184/4] = "TRACER_RAM_1024X_32BIT_97";\
csr[5][0x188/4] = "TRACER_RAM_1024X_32BIT_98";\
csr[5][0x18c/4] = "TRACER_RAM_1024X_32BIT_99";\
csr[5][0x190/4] = "TRACER_RAM_1024X_32BIT_100";\
csr[5][0x194/4] = "TRACER_RAM_1024X_32BIT_101";\
csr[5][0x198/4] = "TRACER_RAM_1024X_32BIT_102";\
csr[5][0x19c/4] = "TRACER_RAM_1024X_32BIT_103";\
csr[5][0x1a0/4] = "TRACER_RAM_1024X_32BIT_104";\
csr[5][0x1a4/4] = "TRACER_RAM_1024X_32BIT_105";\
csr[5][0x1a8/4] = "TRACER_RAM_1024X_32BIT_106";\
csr[5][0x1ac/4] = "TRACER_RAM_1024X_32BIT_107";\
csr[5][0x1b0/4] = "TRACER_RAM_1024X_32BIT_108";\
csr[5][0x1b4/4] = "TRACER_RAM_1024X_32BIT_109";\
csr[5][0x1b8/4] = "TRACER_RAM_1024X_32BIT_110";\
csr[5][0x1bc/4] = "TRACER_RAM_1024X_32BIT_111";\
csr[5][0x1c0/4] = "TRACER_RAM_1024X_32BIT_112";\
csr[5][0x1c4/4] = "TRACER_RAM_1024X_32BIT_113";\
csr[5][0x1c8/4] = "TRACER_RAM_1024X_32BIT_114";\
csr[5][0x1cc/4] = "TRACER_RAM_1024X_32BIT_115";\
csr[5][0x1d0/4] = "TRACER_RAM_1024X_32BIT_116";\
csr[5][0x1d4/4] = "TRACER_RAM_1024X_32BIT_117";\
csr[5][0x1d8/4] = "TRACER_RAM_1024X_32BIT_118";\
csr[5][0x1dc/4] = "TRACER_RAM_1024X_32BIT_119";\
csr[5][0x1e0/4] = "TRACER_RAM_1024X_32BIT_120";\
csr[5][0x1e4/4] = "TRACER_RAM_1024X_32BIT_121";\
csr[5][0x1e8/4] = "TRACER_RAM_1024X_32BIT_122";\
csr[5][0x1ec/4] = "TRACER_RAM_1024X_32BIT_123";\
csr[5][0x1f0/4] = "TRACER_RAM_1024X_32BIT_124";\
csr[5][0x1f4/4] = "TRACER_RAM_1024X_32BIT_125";\
csr[5][0x1f8/4] = "TRACER_RAM_1024X_32BIT_126";\
csr[5][0x1fc/4] = "TRACER_RAM_1024X_32BIT_127";\
csr[5][0x200/4] = "TRACER_RAM_1024X_32BIT_128";\
csr[5][0x204/4] = "TRACER_RAM_1024X_32BIT_129";\
csr[5][0x208/4] = "TRACER_RAM_1024X_32BIT_130";\
csr[5][0x20c/4] = "TRACER_RAM_1024X_32BIT_131";\
csr[5][0x210/4] = "TRACER_RAM_1024X_32BIT_132";\
csr[5][0x214/4] = "TRACER_RAM_1024X_32BIT_133";\
csr[5][0x218/4] = "TRACER_RAM_1024X_32BIT_134";\
csr[5][0x21c/4] = "TRACER_RAM_1024X_32BIT_135";\
csr[5][0x220/4] = "TRACER_RAM_1024X_32BIT_136";\
csr[5][0x224/4] = "TRACER_RAM_1024X_32BIT_137";\
csr[5][0x228/4] = "TRACER_RAM_1024X_32BIT_138";\
csr[5][0x22c/4] = "TRACER_RAM_1024X_32BIT_139";\
csr[5][0x230/4] = "TRACER_RAM_1024X_32BIT_140";\
csr[5][0x234/4] = "TRACER_RAM_1024X_32BIT_141";\
csr[5][0x238/4] = "TRACER_RAM_1024X_32BIT_142";\
csr[5][0x23c/4] = "TRACER_RAM_1024X_32BIT_143";\
csr[5][0x240/4] = "TRACER_RAM_1024X_32BIT_144";\
csr[5][0x244/4] = "TRACER_RAM_1024X_32BIT_145";\
csr[5][0x248/4] = "TRACER_RAM_1024X_32BIT_146";\
csr[5][0x24c/4] = "TRACER_RAM_1024X_32BIT_147";\
csr[5][0x250/4] = "TRACER_RAM_1024X_32BIT_148";\
csr[5][0x254/4] = "TRACER_RAM_1024X_32BIT_149";\
csr[5][0x258/4] = "TRACER_RAM_1024X_32BIT_150";\
csr[5][0x25c/4] = "TRACER_RAM_1024X_32BIT_151";\
csr[5][0x260/4] = "TRACER_RAM_1024X_32BIT_152";\
csr[5][0x264/4] = "TRACER_RAM_1024X_32BIT_153";\
csr[5][0x268/4] = "TRACER_RAM_1024X_32BIT_154";\
csr[5][0x26c/4] = "TRACER_RAM_1024X_32BIT_155";\
csr[5][0x270/4] = "TRACER_RAM_1024X_32BIT_156";\
csr[5][0x274/4] = "TRACER_RAM_1024X_32BIT_157";\
csr[5][0x278/4] = "TRACER_RAM_1024X_32BIT_158";\
csr[5][0x27c/4] = "TRACER_RAM_1024X_32BIT_159";\
csr[5][0x280/4] = "TRACER_RAM_1024X_32BIT_160";\
csr[5][0x284/4] = "TRACER_RAM_1024X_32BIT_161";\
csr[5][0x288/4] = "TRACER_RAM_1024X_32BIT_162";\
csr[5][0x28c/4] = "TRACER_RAM_1024X_32BIT_163";\
csr[5][0x290/4] = "TRACER_RAM_1024X_32BIT_164";\
csr[5][0x294/4] = "TRACER_RAM_1024X_32BIT_165";\
csr[5][0x298/4] = "TRACER_RAM_1024X_32BIT_166";\
csr[5][0x29c/4] = "TRACER_RAM_1024X_32BIT_167";\
csr[5][0x2a0/4] = "TRACER_RAM_1024X_32BIT_168";\
csr[5][0x2a4/4] = "TRACER_RAM_1024X_32BIT_169";\
csr[5][0x2a8/4] = "TRACER_RAM_1024X_32BIT_170";\
csr[5][0x2ac/4] = "TRACER_RAM_1024X_32BIT_171";\
csr[5][0x2b0/4] = "TRACER_RAM_1024X_32BIT_172";\
csr[5][0x2b4/4] = "TRACER_RAM_1024X_32BIT_173";\
csr[5][0x2b8/4] = "TRACER_RAM_1024X_32BIT_174";\
csr[5][0x2bc/4] = "TRACER_RAM_1024X_32BIT_175";\
csr[5][0x2c0/4] = "TRACER_RAM_1024X_32BIT_176";\
csr[5][0x2c4/4] = "TRACER_RAM_1024X_32BIT_177";\
csr[5][0x2c8/4] = "TRACER_RAM_1024X_32BIT_178";\
csr[5][0x2cc/4] = "TRACER_RAM_1024X_32BIT_179";\
csr[5][0x2d0/4] = "TRACER_RAM_1024X_32BIT_180";\
csr[5][0x2d4/4] = "TRACER_RAM_1024X_32BIT_181";\
csr[5][0x2d8/4] = "TRACER_RAM_1024X_32BIT_182";\
csr[5][0x2dc/4] = "TRACER_RAM_1024X_32BIT_183";\
csr[5][0x2e0/4] = "TRACER_RAM_1024X_32BIT_184";\
csr[5][0x2e4/4] = "TRACER_RAM_1024X_32BIT_185";\
csr[5][0x2e8/4] = "TRACER_RAM_1024X_32BIT_186";\
csr[5][0x2ec/4] = "TRACER_RAM_1024X_32BIT_187";\
csr[5][0x2f0/4] = "TRACER_RAM_1024X_32BIT_188";\
csr[5][0x2f4/4] = "TRACER_RAM_1024X_32BIT_189";\
csr[5][0x2f8/4] = "TRACER_RAM_1024X_32BIT_190";\
csr[5][0x2fc/4] = "TRACER_RAM_1024X_32BIT_191";\
csr[5][0x300/4] = "TRACER_RAM_1024X_32BIT_192";\
csr[5][0x304/4] = "TRACER_RAM_1024X_32BIT_193";\
csr[5][0x308/4] = "TRACER_RAM_1024X_32BIT_194";\
csr[5][0x30c/4] = "TRACER_RAM_1024X_32BIT_195";\
csr[5][0x310/4] = "TRACER_RAM_1024X_32BIT_196";\
csr[5][0x314/4] = "TRACER_RAM_1024X_32BIT_197";\
csr[5][0x318/4] = "TRACER_RAM_1024X_32BIT_198";\
csr[5][0x31c/4] = "TRACER_RAM_1024X_32BIT_199";\
csr[5][0x320/4] = "TRACER_RAM_1024X_32BIT_200";\
csr[5][0x324/4] = "TRACER_RAM_1024X_32BIT_201";\
csr[5][0x328/4] = "TRACER_RAM_1024X_32BIT_202";\
csr[5][0x32c/4] = "TRACER_RAM_1024X_32BIT_203";\
csr[5][0x330/4] = "TRACER_RAM_1024X_32BIT_204";\
csr[5][0x334/4] = "TRACER_RAM_1024X_32BIT_205";\
csr[5][0x338/4] = "TRACER_RAM_1024X_32BIT_206";\
csr[5][0x33c/4] = "TRACER_RAM_1024X_32BIT_207";\
csr[5][0x340/4] = "TRACER_RAM_1024X_32BIT_208";\
csr[5][0x344/4] = "TRACER_RAM_1024X_32BIT_209";\
csr[5][0x348/4] = "TRACER_RAM_1024X_32BIT_210";\
csr[5][0x34c/4] = "TRACER_RAM_1024X_32BIT_211";\
csr[5][0x350/4] = "TRACER_RAM_1024X_32BIT_212";\
csr[5][0x354/4] = "TRACER_RAM_1024X_32BIT_213";\
csr[5][0x358/4] = "TRACER_RAM_1024X_32BIT_214";\
csr[5][0x35c/4] = "TRACER_RAM_1024X_32BIT_215";\
csr[5][0x360/4] = "TRACER_RAM_1024X_32BIT_216";\
csr[5][0x364/4] = "TRACER_RAM_1024X_32BIT_217";\
csr[5][0x368/4] = "TRACER_RAM_1024X_32BIT_218";\
csr[5][0x36c/4] = "TRACER_RAM_1024X_32BIT_219";\
csr[5][0x370/4] = "TRACER_RAM_1024X_32BIT_220";\
csr[5][0x374/4] = "TRACER_RAM_1024X_32BIT_221";\
csr[5][0x378/4] = "TRACER_RAM_1024X_32BIT_222";\
csr[5][0x37c/4] = "TRACER_RAM_1024X_32BIT_223";\
csr[5][0x380/4] = "TRACER_RAM_1024X_32BIT_224";\
csr[5][0x384/4] = "TRACER_RAM_1024X_32BIT_225";\
csr[5][0x388/4] = "TRACER_RAM_1024X_32BIT_226";\
csr[5][0x38c/4] = "TRACER_RAM_1024X_32BIT_227";\
csr[5][0x390/4] = "TRACER_RAM_1024X_32BIT_228";\
csr[5][0x394/4] = "TRACER_RAM_1024X_32BIT_229";\
csr[5][0x398/4] = "TRACER_RAM_1024X_32BIT_230";\
csr[5][0x39c/4] = "TRACER_RAM_1024X_32BIT_231";\
csr[5][0x3a0/4] = "TRACER_RAM_1024X_32BIT_232";\
csr[5][0x3a4/4] = "TRACER_RAM_1024X_32BIT_233";\
csr[5][0x3a8/4] = "TRACER_RAM_1024X_32BIT_234";\
csr[5][0x3ac/4] = "TRACER_RAM_1024X_32BIT_235";\
csr[5][0x3b0/4] = "TRACER_RAM_1024X_32BIT_236";\
csr[5][0x3b4/4] = "TRACER_RAM_1024X_32BIT_237";\
csr[5][0x3b8/4] = "TRACER_RAM_1024X_32BIT_238";\
csr[5][0x3bc/4] = "TRACER_RAM_1024X_32BIT_239";\
csr[5][0x3c0/4] = "TRACER_RAM_1024X_32BIT_240";\
csr[5][0x3c4/4] = "TRACER_RAM_1024X_32BIT_241";\
csr[5][0x3c8/4] = "TRACER_RAM_1024X_32BIT_242";\
csr[5][0x3cc/4] = "TRACER_RAM_1024X_32BIT_243";\
csr[5][0x3d0/4] = "TRACER_RAM_1024X_32BIT_244";\
csr[5][0x3d4/4] = "TRACER_RAM_1024X_32BIT_245";\
csr[5][0x3d8/4] = "TRACER_RAM_1024X_32BIT_246";\
csr[5][0x3dc/4] = "TRACER_RAM_1024X_32BIT_247";\
csr[5][0x3e0/4] = "TRACER_RAM_1024X_32BIT_248";\
csr[5][0x3e4/4] = "TRACER_RAM_1024X_32BIT_249";\
csr[5][0x3e8/4] = "TRACER_RAM_1024X_32BIT_250";\
csr[5][0x3ec/4] = "TRACER_RAM_1024X_32BIT_251";\
csr[5][0x3f0/4] = "TRACER_RAM_1024X_32BIT_252";\
csr[5][0x3f4/4] = "TRACER_RAM_1024X_32BIT_253";\
csr[5][0x3f8/4] = "TRACER_RAM_1024X_32BIT_254";\
csr[5][0x3fc/4] = "TRACER_RAM_1024X_32BIT_255";\
csr[5][0x400/4] = "TRACER_RAM_1024X_32BIT_256";\
csr[5][0x404/4] = "TRACER_RAM_1024X_32BIT_257";\
csr[5][0x408/4] = "TRACER_RAM_1024X_32BIT_258";\
csr[5][0x40c/4] = "TRACER_RAM_1024X_32BIT_259";\
csr[5][0x410/4] = "TRACER_RAM_1024X_32BIT_260";\
csr[5][0x414/4] = "TRACER_RAM_1024X_32BIT_261";\
csr[5][0x418/4] = "TRACER_RAM_1024X_32BIT_262";\
csr[5][0x41c/4] = "TRACER_RAM_1024X_32BIT_263";\
csr[5][0x420/4] = "TRACER_RAM_1024X_32BIT_264";\
csr[5][0x424/4] = "TRACER_RAM_1024X_32BIT_265";\
csr[5][0x428/4] = "TRACER_RAM_1024X_32BIT_266";\
csr[5][0x42c/4] = "TRACER_RAM_1024X_32BIT_267";\
csr[5][0x430/4] = "TRACER_RAM_1024X_32BIT_268";\
csr[5][0x434/4] = "TRACER_RAM_1024X_32BIT_269";\
csr[5][0x438/4] = "TRACER_RAM_1024X_32BIT_270";\
csr[5][0x43c/4] = "TRACER_RAM_1024X_32BIT_271";\
csr[5][0x440/4] = "TRACER_RAM_1024X_32BIT_272";\
csr[5][0x444/4] = "TRACER_RAM_1024X_32BIT_273";\
csr[5][0x448/4] = "TRACER_RAM_1024X_32BIT_274";\
csr[5][0x44c/4] = "TRACER_RAM_1024X_32BIT_275";\
csr[5][0x450/4] = "TRACER_RAM_1024X_32BIT_276";\
csr[5][0x454/4] = "TRACER_RAM_1024X_32BIT_277";\
csr[5][0x458/4] = "TRACER_RAM_1024X_32BIT_278";\
csr[5][0x45c/4] = "TRACER_RAM_1024X_32BIT_279";\
csr[5][0x460/4] = "TRACER_RAM_1024X_32BIT_280";\
csr[5][0x464/4] = "TRACER_RAM_1024X_32BIT_281";\
csr[5][0x468/4] = "TRACER_RAM_1024X_32BIT_282";\
csr[5][0x46c/4] = "TRACER_RAM_1024X_32BIT_283";\
csr[5][0x470/4] = "TRACER_RAM_1024X_32BIT_284";\
csr[5][0x474/4] = "TRACER_RAM_1024X_32BIT_285";\
csr[5][0x478/4] = "TRACER_RAM_1024X_32BIT_286";\
csr[5][0x47c/4] = "TRACER_RAM_1024X_32BIT_287";\
csr[5][0x480/4] = "TRACER_RAM_1024X_32BIT_288";\
csr[5][0x484/4] = "TRACER_RAM_1024X_32BIT_289";\
csr[5][0x488/4] = "TRACER_RAM_1024X_32BIT_290";\
csr[5][0x48c/4] = "TRACER_RAM_1024X_32BIT_291";\
csr[5][0x490/4] = "TRACER_RAM_1024X_32BIT_292";\
csr[5][0x494/4] = "TRACER_RAM_1024X_32BIT_293";\
csr[5][0x498/4] = "TRACER_RAM_1024X_32BIT_294";\
csr[5][0x49c/4] = "TRACER_RAM_1024X_32BIT_295";\
csr[5][0x4a0/4] = "TRACER_RAM_1024X_32BIT_296";\
csr[5][0x4a4/4] = "TRACER_RAM_1024X_32BIT_297";\
csr[5][0x4a8/4] = "TRACER_RAM_1024X_32BIT_298";\
csr[5][0x4ac/4] = "TRACER_RAM_1024X_32BIT_299";\
csr[5][0x4b0/4] = "TRACER_RAM_1024X_32BIT_300";\
csr[5][0x4b4/4] = "TRACER_RAM_1024X_32BIT_301";\
csr[5][0x4b8/4] = "TRACER_RAM_1024X_32BIT_302";\
csr[5][0x4bc/4] = "TRACER_RAM_1024X_32BIT_303";\
csr[5][0x4c0/4] = "TRACER_RAM_1024X_32BIT_304";\
csr[5][0x4c4/4] = "TRACER_RAM_1024X_32BIT_305";\
csr[5][0x4c8/4] = "TRACER_RAM_1024X_32BIT_306";\
csr[5][0x4cc/4] = "TRACER_RAM_1024X_32BIT_307";\
csr[5][0x4d0/4] = "TRACER_RAM_1024X_32BIT_308";\
csr[5][0x4d4/4] = "TRACER_RAM_1024X_32BIT_309";\
csr[5][0x4d8/4] = "TRACER_RAM_1024X_32BIT_310";\
csr[5][0x4dc/4] = "TRACER_RAM_1024X_32BIT_311";\
csr[5][0x4e0/4] = "TRACER_RAM_1024X_32BIT_312";\
csr[5][0x4e4/4] = "TRACER_RAM_1024X_32BIT_313";\
csr[5][0x4e8/4] = "TRACER_RAM_1024X_32BIT_314";\
csr[5][0x4ec/4] = "TRACER_RAM_1024X_32BIT_315";\
csr[5][0x4f0/4] = "TRACER_RAM_1024X_32BIT_316";\
csr[5][0x4f4/4] = "TRACER_RAM_1024X_32BIT_317";\
csr[5][0x4f8/4] = "TRACER_RAM_1024X_32BIT_318";\
csr[5][0x4fc/4] = "TRACER_RAM_1024X_32BIT_319";\
csr[5][0x500/4] = "TRACER_RAM_1024X_32BIT_320";\
csr[5][0x504/4] = "TRACER_RAM_1024X_32BIT_321";\
csr[5][0x508/4] = "TRACER_RAM_1024X_32BIT_322";\
csr[5][0x50c/4] = "TRACER_RAM_1024X_32BIT_323";\
csr[5][0x510/4] = "TRACER_RAM_1024X_32BIT_324";\
csr[5][0x514/4] = "TRACER_RAM_1024X_32BIT_325";\
csr[5][0x518/4] = "TRACER_RAM_1024X_32BIT_326";\
csr[5][0x51c/4] = "TRACER_RAM_1024X_32BIT_327";\
csr[5][0x520/4] = "TRACER_RAM_1024X_32BIT_328";\
csr[5][0x524/4] = "TRACER_RAM_1024X_32BIT_329";\
csr[5][0x528/4] = "TRACER_RAM_1024X_32BIT_330";\
csr[5][0x52c/4] = "TRACER_RAM_1024X_32BIT_331";\
csr[5][0x530/4] = "TRACER_RAM_1024X_32BIT_332";\
csr[5][0x534/4] = "TRACER_RAM_1024X_32BIT_333";\
csr[5][0x538/4] = "TRACER_RAM_1024X_32BIT_334";\
csr[5][0x53c/4] = "TRACER_RAM_1024X_32BIT_335";\
csr[5][0x540/4] = "TRACER_RAM_1024X_32BIT_336";\
csr[5][0x544/4] = "TRACER_RAM_1024X_32BIT_337";\
csr[5][0x548/4] = "TRACER_RAM_1024X_32BIT_338";\
csr[5][0x54c/4] = "TRACER_RAM_1024X_32BIT_339";\
csr[5][0x550/4] = "TRACER_RAM_1024X_32BIT_340";\
csr[5][0x554/4] = "TRACER_RAM_1024X_32BIT_341";\
csr[5][0x558/4] = "TRACER_RAM_1024X_32BIT_342";\
csr[5][0x55c/4] = "TRACER_RAM_1024X_32BIT_343";\
csr[5][0x560/4] = "TRACER_RAM_1024X_32BIT_344";\
csr[5][0x564/4] = "TRACER_RAM_1024X_32BIT_345";\
csr[5][0x568/4] = "TRACER_RAM_1024X_32BIT_346";\
csr[5][0x56c/4] = "TRACER_RAM_1024X_32BIT_347";\
csr[5][0x570/4] = "TRACER_RAM_1024X_32BIT_348";\
csr[5][0x574/4] = "TRACER_RAM_1024X_32BIT_349";\
csr[5][0x578/4] = "TRACER_RAM_1024X_32BIT_350";\
csr[5][0x57c/4] = "TRACER_RAM_1024X_32BIT_351";\
csr[5][0x580/4] = "TRACER_RAM_1024X_32BIT_352";\
csr[5][0x584/4] = "TRACER_RAM_1024X_32BIT_353";\
csr[5][0x588/4] = "TRACER_RAM_1024X_32BIT_354";\
csr[5][0x58c/4] = "TRACER_RAM_1024X_32BIT_355";\
csr[5][0x590/4] = "TRACER_RAM_1024X_32BIT_356";\
csr[5][0x594/4] = "TRACER_RAM_1024X_32BIT_357";\
csr[5][0x598/4] = "TRACER_RAM_1024X_32BIT_358";\
csr[5][0x59c/4] = "TRACER_RAM_1024X_32BIT_359";\
csr[5][0x5a0/4] = "TRACER_RAM_1024X_32BIT_360";\
csr[5][0x5a4/4] = "TRACER_RAM_1024X_32BIT_361";\
csr[5][0x5a8/4] = "TRACER_RAM_1024X_32BIT_362";\
csr[5][0x5ac/4] = "TRACER_RAM_1024X_32BIT_363";\
csr[5][0x5b0/4] = "TRACER_RAM_1024X_32BIT_364";\
csr[5][0x5b4/4] = "TRACER_RAM_1024X_32BIT_365";\
csr[5][0x5b8/4] = "TRACER_RAM_1024X_32BIT_366";\
csr[5][0x5bc/4] = "TRACER_RAM_1024X_32BIT_367";\
csr[5][0x5c0/4] = "TRACER_RAM_1024X_32BIT_368";\
csr[5][0x5c4/4] = "TRACER_RAM_1024X_32BIT_369";\
csr[5][0x5c8/4] = "TRACER_RAM_1024X_32BIT_370";\
csr[5][0x5cc/4] = "TRACER_RAM_1024X_32BIT_371";\
csr[5][0x5d0/4] = "TRACER_RAM_1024X_32BIT_372";\
csr[5][0x5d4/4] = "TRACER_RAM_1024X_32BIT_373";\
csr[5][0x5d8/4] = "TRACER_RAM_1024X_32BIT_374";\
csr[5][0x5dc/4] = "TRACER_RAM_1024X_32BIT_375";\
csr[5][0x5e0/4] = "TRACER_RAM_1024X_32BIT_376";\
csr[5][0x5e4/4] = "TRACER_RAM_1024X_32BIT_377";\
csr[5][0x5e8/4] = "TRACER_RAM_1024X_32BIT_378";\
csr[5][0x5ec/4] = "TRACER_RAM_1024X_32BIT_379";\
csr[5][0x5f0/4] = "TRACER_RAM_1024X_32BIT_380";\
csr[5][0x5f4/4] = "TRACER_RAM_1024X_32BIT_381";\
csr[5][0x5f8/4] = "TRACER_RAM_1024X_32BIT_382";\
csr[5][0x5fc/4] = "TRACER_RAM_1024X_32BIT_383";\
csr[5][0x600/4] = "TRACER_RAM_1024X_32BIT_384";\
csr[5][0x604/4] = "TRACER_RAM_1024X_32BIT_385";\
csr[5][0x608/4] = "TRACER_RAM_1024X_32BIT_386";\
csr[5][0x60c/4] = "TRACER_RAM_1024X_32BIT_387";\
csr[5][0x610/4] = "TRACER_RAM_1024X_32BIT_388";\
csr[5][0x614/4] = "TRACER_RAM_1024X_32BIT_389";\
csr[5][0x618/4] = "TRACER_RAM_1024X_32BIT_390";\
csr[5][0x61c/4] = "TRACER_RAM_1024X_32BIT_391";\
csr[5][0x620/4] = "TRACER_RAM_1024X_32BIT_392";\
csr[5][0x624/4] = "TRACER_RAM_1024X_32BIT_393";\
csr[5][0x628/4] = "TRACER_RAM_1024X_32BIT_394";\
csr[5][0x62c/4] = "TRACER_RAM_1024X_32BIT_395";\
csr[5][0x630/4] = "TRACER_RAM_1024X_32BIT_396";\
csr[5][0x634/4] = "TRACER_RAM_1024X_32BIT_397";\
csr[5][0x638/4] = "TRACER_RAM_1024X_32BIT_398";\
csr[5][0x63c/4] = "TRACER_RAM_1024X_32BIT_399";\
csr[5][0x640/4] = "TRACER_RAM_1024X_32BIT_400";\
csr[5][0x644/4] = "TRACER_RAM_1024X_32BIT_401";\
csr[5][0x648/4] = "TRACER_RAM_1024X_32BIT_402";\
csr[5][0x64c/4] = "TRACER_RAM_1024X_32BIT_403";\
csr[5][0x650/4] = "TRACER_RAM_1024X_32BIT_404";\
csr[5][0x654/4] = "TRACER_RAM_1024X_32BIT_405";\
csr[5][0x658/4] = "TRACER_RAM_1024X_32BIT_406";\
csr[5][0x65c/4] = "TRACER_RAM_1024X_32BIT_407";\
csr[5][0x660/4] = "TRACER_RAM_1024X_32BIT_408";\
csr[5][0x664/4] = "TRACER_RAM_1024X_32BIT_409";\
csr[5][0x668/4] = "TRACER_RAM_1024X_32BIT_410";\
csr[5][0x66c/4] = "TRACER_RAM_1024X_32BIT_411";\
csr[5][0x670/4] = "TRACER_RAM_1024X_32BIT_412";\
csr[5][0x674/4] = "TRACER_RAM_1024X_32BIT_413";\
csr[5][0x678/4] = "TRACER_RAM_1024X_32BIT_414";\
csr[5][0x67c/4] = "TRACER_RAM_1024X_32BIT_415";\
csr[5][0x680/4] = "TRACER_RAM_1024X_32BIT_416";\
csr[5][0x684/4] = "TRACER_RAM_1024X_32BIT_417";\
csr[5][0x688/4] = "TRACER_RAM_1024X_32BIT_418";\
csr[5][0x68c/4] = "TRACER_RAM_1024X_32BIT_419";\
csr[5][0x690/4] = "TRACER_RAM_1024X_32BIT_420";\
csr[5][0x694/4] = "TRACER_RAM_1024X_32BIT_421";\
csr[5][0x698/4] = "TRACER_RAM_1024X_32BIT_422";\
csr[5][0x69c/4] = "TRACER_RAM_1024X_32BIT_423";\
csr[5][0x6a0/4] = "TRACER_RAM_1024X_32BIT_424";\
csr[5][0x6a4/4] = "TRACER_RAM_1024X_32BIT_425";\
csr[5][0x6a8/4] = "TRACER_RAM_1024X_32BIT_426";\
csr[5][0x6ac/4] = "TRACER_RAM_1024X_32BIT_427";\
csr[5][0x6b0/4] = "TRACER_RAM_1024X_32BIT_428";\
csr[5][0x6b4/4] = "TRACER_RAM_1024X_32BIT_429";\
csr[5][0x6b8/4] = "TRACER_RAM_1024X_32BIT_430";\
csr[5][0x6bc/4] = "TRACER_RAM_1024X_32BIT_431";\
csr[5][0x6c0/4] = "TRACER_RAM_1024X_32BIT_432";\
csr[5][0x6c4/4] = "TRACER_RAM_1024X_32BIT_433";\
csr[5][0x6c8/4] = "TRACER_RAM_1024X_32BIT_434";\
csr[5][0x6cc/4] = "TRACER_RAM_1024X_32BIT_435";\
csr[5][0x6d0/4] = "TRACER_RAM_1024X_32BIT_436";\
csr[5][0x6d4/4] = "TRACER_RAM_1024X_32BIT_437";\
csr[5][0x6d8/4] = "TRACER_RAM_1024X_32BIT_438";\
csr[5][0x6dc/4] = "TRACER_RAM_1024X_32BIT_439";\
csr[5][0x6e0/4] = "TRACER_RAM_1024X_32BIT_440";\
csr[5][0x6e4/4] = "TRACER_RAM_1024X_32BIT_441";\
csr[5][0x6e8/4] = "TRACER_RAM_1024X_32BIT_442";\
csr[5][0x6ec/4] = "TRACER_RAM_1024X_32BIT_443";\
csr[5][0x6f0/4] = "TRACER_RAM_1024X_32BIT_444";\
csr[5][0x6f4/4] = "TRACER_RAM_1024X_32BIT_445";\
csr[5][0x6f8/4] = "TRACER_RAM_1024X_32BIT_446";\
csr[5][0x6fc/4] = "TRACER_RAM_1024X_32BIT_447";\
csr[5][0x700/4] = "TRACER_RAM_1024X_32BIT_448";\
csr[5][0x704/4] = "TRACER_RAM_1024X_32BIT_449";\
csr[5][0x708/4] = "TRACER_RAM_1024X_32BIT_450";\
csr[5][0x70c/4] = "TRACER_RAM_1024X_32BIT_451";\
csr[5][0x710/4] = "TRACER_RAM_1024X_32BIT_452";\
csr[5][0x714/4] = "TRACER_RAM_1024X_32BIT_453";\
csr[5][0x718/4] = "TRACER_RAM_1024X_32BIT_454";\
csr[5][0x71c/4] = "TRACER_RAM_1024X_32BIT_455";\
csr[5][0x720/4] = "TRACER_RAM_1024X_32BIT_456";\
csr[5][0x724/4] = "TRACER_RAM_1024X_32BIT_457";\
csr[5][0x728/4] = "TRACER_RAM_1024X_32BIT_458";\
csr[5][0x72c/4] = "TRACER_RAM_1024X_32BIT_459";\
csr[5][0x730/4] = "TRACER_RAM_1024X_32BIT_460";\
csr[5][0x734/4] = "TRACER_RAM_1024X_32BIT_461";\
csr[5][0x738/4] = "TRACER_RAM_1024X_32BIT_462";\
csr[5][0x73c/4] = "TRACER_RAM_1024X_32BIT_463";\
csr[5][0x740/4] = "TRACER_RAM_1024X_32BIT_464";\
csr[5][0x744/4] = "TRACER_RAM_1024X_32BIT_465";\
csr[5][0x748/4] = "TRACER_RAM_1024X_32BIT_466";\
csr[5][0x74c/4] = "TRACER_RAM_1024X_32BIT_467";\
csr[5][0x750/4] = "TRACER_RAM_1024X_32BIT_468";\
csr[5][0x754/4] = "TRACER_RAM_1024X_32BIT_469";\
csr[5][0x758/4] = "TRACER_RAM_1024X_32BIT_470";\
csr[5][0x75c/4] = "TRACER_RAM_1024X_32BIT_471";\
csr[5][0x760/4] = "TRACER_RAM_1024X_32BIT_472";\
csr[5][0x764/4] = "TRACER_RAM_1024X_32BIT_473";\
csr[5][0x768/4] = "TRACER_RAM_1024X_32BIT_474";\
csr[5][0x76c/4] = "TRACER_RAM_1024X_32BIT_475";\
csr[5][0x770/4] = "TRACER_RAM_1024X_32BIT_476";\
csr[5][0x774/4] = "TRACER_RAM_1024X_32BIT_477";\
csr[5][0x778/4] = "TRACER_RAM_1024X_32BIT_478";\
csr[5][0x77c/4] = "TRACER_RAM_1024X_32BIT_479";\
csr[5][0x780/4] = "TRACER_RAM_1024X_32BIT_480";\
csr[5][0x784/4] = "TRACER_RAM_1024X_32BIT_481";\
csr[5][0x788/4] = "TRACER_RAM_1024X_32BIT_482";\
csr[5][0x78c/4] = "TRACER_RAM_1024X_32BIT_483";\
csr[5][0x790/4] = "TRACER_RAM_1024X_32BIT_484";\
csr[5][0x794/4] = "TRACER_RAM_1024X_32BIT_485";\
csr[5][0x798/4] = "TRACER_RAM_1024X_32BIT_486";\
csr[5][0x79c/4] = "TRACER_RAM_1024X_32BIT_487";\
csr[5][0x7a0/4] = "TRACER_RAM_1024X_32BIT_488";\
csr[5][0x7a4/4] = "TRACER_RAM_1024X_32BIT_489";\
csr[5][0x7a8/4] = "TRACER_RAM_1024X_32BIT_490";\
csr[5][0x7ac/4] = "TRACER_RAM_1024X_32BIT_491";\
csr[5][0x7b0/4] = "TRACER_RAM_1024X_32BIT_492";\
csr[5][0x7b4/4] = "TRACER_RAM_1024X_32BIT_493";\
csr[5][0x7b8/4] = "TRACER_RAM_1024X_32BIT_494";\
csr[5][0x7bc/4] = "TRACER_RAM_1024X_32BIT_495";\
csr[5][0x7c0/4] = "TRACER_RAM_1024X_32BIT_496";\
csr[5][0x7c4/4] = "TRACER_RAM_1024X_32BIT_497";\
csr[5][0x7c8/4] = "TRACER_RAM_1024X_32BIT_498";\
csr[5][0x7cc/4] = "TRACER_RAM_1024X_32BIT_499";\
csr[5][0x7d0/4] = "TRACER_RAM_1024X_32BIT_500";\
csr[5][0x7d4/4] = "TRACER_RAM_1024X_32BIT_501";\
csr[5][0x7d8/4] = "TRACER_RAM_1024X_32BIT_502";\
csr[5][0x7dc/4] = "TRACER_RAM_1024X_32BIT_503";\
csr[5][0x7e0/4] = "TRACER_RAM_1024X_32BIT_504";\
csr[5][0x7e4/4] = "TRACER_RAM_1024X_32BIT_505";\
csr[5][0x7e8/4] = "TRACER_RAM_1024X_32BIT_506";\
csr[5][0x7ec/4] = "TRACER_RAM_1024X_32BIT_507";\
csr[5][0x7f0/4] = "TRACER_RAM_1024X_32BIT_508";\
csr[5][0x7f4/4] = "TRACER_RAM_1024X_32BIT_509";\
csr[5][0x7f8/4] = "TRACER_RAM_1024X_32BIT_510";\
csr[5][0x7fc/4] = "TRACER_RAM_1024X_32BIT_511";\
csr[5][0x800/4] = "TRACER_RAM_1024X_32BIT_512";\
csr[5][0x804/4] = "TRACER_RAM_1024X_32BIT_513";\
csr[5][0x808/4] = "TRACER_RAM_1024X_32BIT_514";\
csr[5][0x80c/4] = "TRACER_RAM_1024X_32BIT_515";\
csr[5][0x810/4] = "TRACER_RAM_1024X_32BIT_516";\
csr[5][0x814/4] = "TRACER_RAM_1024X_32BIT_517";\
csr[5][0x818/4] = "TRACER_RAM_1024X_32BIT_518";\
csr[5][0x81c/4] = "TRACER_RAM_1024X_32BIT_519";\
csr[5][0x820/4] = "TRACER_RAM_1024X_32BIT_520";\
csr[5][0x824/4] = "TRACER_RAM_1024X_32BIT_521";\
csr[5][0x828/4] = "TRACER_RAM_1024X_32BIT_522";\
csr[5][0x82c/4] = "TRACER_RAM_1024X_32BIT_523";\
csr[5][0x830/4] = "TRACER_RAM_1024X_32BIT_524";\
csr[5][0x834/4] = "TRACER_RAM_1024X_32BIT_525";\
csr[5][0x838/4] = "TRACER_RAM_1024X_32BIT_526";\
csr[5][0x83c/4] = "TRACER_RAM_1024X_32BIT_527";\
csr[5][0x840/4] = "TRACER_RAM_1024X_32BIT_528";\
csr[5][0x844/4] = "TRACER_RAM_1024X_32BIT_529";\
csr[5][0x848/4] = "TRACER_RAM_1024X_32BIT_530";\
csr[5][0x84c/4] = "TRACER_RAM_1024X_32BIT_531";\
csr[5][0x850/4] = "TRACER_RAM_1024X_32BIT_532";\
csr[5][0x854/4] = "TRACER_RAM_1024X_32BIT_533";\
csr[5][0x858/4] = "TRACER_RAM_1024X_32BIT_534";\
csr[5][0x85c/4] = "TRACER_RAM_1024X_32BIT_535";\
csr[5][0x860/4] = "TRACER_RAM_1024X_32BIT_536";\
csr[5][0x864/4] = "TRACER_RAM_1024X_32BIT_537";\
csr[5][0x868/4] = "TRACER_RAM_1024X_32BIT_538";\
csr[5][0x86c/4] = "TRACER_RAM_1024X_32BIT_539";\
csr[5][0x870/4] = "TRACER_RAM_1024X_32BIT_540";\
csr[5][0x874/4] = "TRACER_RAM_1024X_32BIT_541";\
csr[5][0x878/4] = "TRACER_RAM_1024X_32BIT_542";\
csr[5][0x87c/4] = "TRACER_RAM_1024X_32BIT_543";\
csr[5][0x880/4] = "TRACER_RAM_1024X_32BIT_544";\
csr[5][0x884/4] = "TRACER_RAM_1024X_32BIT_545";\
csr[5][0x888/4] = "TRACER_RAM_1024X_32BIT_546";\
csr[5][0x88c/4] = "TRACER_RAM_1024X_32BIT_547";\
csr[5][0x890/4] = "TRACER_RAM_1024X_32BIT_548";\
csr[5][0x894/4] = "TRACER_RAM_1024X_32BIT_549";\
csr[5][0x898/4] = "TRACER_RAM_1024X_32BIT_550";\
csr[5][0x89c/4] = "TRACER_RAM_1024X_32BIT_551";\
csr[5][0x8a0/4] = "TRACER_RAM_1024X_32BIT_552";\
csr[5][0x8a4/4] = "TRACER_RAM_1024X_32BIT_553";\
csr[5][0x8a8/4] = "TRACER_RAM_1024X_32BIT_554";\
csr[5][0x8ac/4] = "TRACER_RAM_1024X_32BIT_555";\
csr[5][0x8b0/4] = "TRACER_RAM_1024X_32BIT_556";\
csr[5][0x8b4/4] = "TRACER_RAM_1024X_32BIT_557";\
csr[5][0x8b8/4] = "TRACER_RAM_1024X_32BIT_558";\
csr[5][0x8bc/4] = "TRACER_RAM_1024X_32BIT_559";\
csr[5][0x8c0/4] = "TRACER_RAM_1024X_32BIT_560";\
csr[5][0x8c4/4] = "TRACER_RAM_1024X_32BIT_561";\
csr[5][0x8c8/4] = "TRACER_RAM_1024X_32BIT_562";\
csr[5][0x8cc/4] = "TRACER_RAM_1024X_32BIT_563";\
csr[5][0x8d0/4] = "TRACER_RAM_1024X_32BIT_564";\
csr[5][0x8d4/4] = "TRACER_RAM_1024X_32BIT_565";\
csr[5][0x8d8/4] = "TRACER_RAM_1024X_32BIT_566";\
csr[5][0x8dc/4] = "TRACER_RAM_1024X_32BIT_567";\
csr[5][0x8e0/4] = "TRACER_RAM_1024X_32BIT_568";\
csr[5][0x8e4/4] = "TRACER_RAM_1024X_32BIT_569";\
csr[5][0x8e8/4] = "TRACER_RAM_1024X_32BIT_570";\
csr[5][0x8ec/4] = "TRACER_RAM_1024X_32BIT_571";\
csr[5][0x8f0/4] = "TRACER_RAM_1024X_32BIT_572";\
csr[5][0x8f4/4] = "TRACER_RAM_1024X_32BIT_573";\
csr[5][0x8f8/4] = "TRACER_RAM_1024X_32BIT_574";\
csr[5][0x8fc/4] = "TRACER_RAM_1024X_32BIT_575";\
csr[5][0x900/4] = "TRACER_RAM_1024X_32BIT_576";\
csr[5][0x904/4] = "TRACER_RAM_1024X_32BIT_577";\
csr[5][0x908/4] = "TRACER_RAM_1024X_32BIT_578";\
csr[5][0x90c/4] = "TRACER_RAM_1024X_32BIT_579";\
csr[5][0x910/4] = "TRACER_RAM_1024X_32BIT_580";\
csr[5][0x914/4] = "TRACER_RAM_1024X_32BIT_581";\
csr[5][0x918/4] = "TRACER_RAM_1024X_32BIT_582";\
csr[5][0x91c/4] = "TRACER_RAM_1024X_32BIT_583";\
csr[5][0x920/4] = "TRACER_RAM_1024X_32BIT_584";\
csr[5][0x924/4] = "TRACER_RAM_1024X_32BIT_585";\
csr[5][0x928/4] = "TRACER_RAM_1024X_32BIT_586";\
csr[5][0x92c/4] = "TRACER_RAM_1024X_32BIT_587";\
csr[5][0x930/4] = "TRACER_RAM_1024X_32BIT_588";\
csr[5][0x934/4] = "TRACER_RAM_1024X_32BIT_589";\
csr[5][0x938/4] = "TRACER_RAM_1024X_32BIT_590";\
csr[5][0x93c/4] = "TRACER_RAM_1024X_32BIT_591";\
csr[5][0x940/4] = "TRACER_RAM_1024X_32BIT_592";\
csr[5][0x944/4] = "TRACER_RAM_1024X_32BIT_593";\
csr[5][0x948/4] = "TRACER_RAM_1024X_32BIT_594";\
csr[5][0x94c/4] = "TRACER_RAM_1024X_32BIT_595";\
csr[5][0x950/4] = "TRACER_RAM_1024X_32BIT_596";\
csr[5][0x954/4] = "TRACER_RAM_1024X_32BIT_597";\
csr[5][0x958/4] = "TRACER_RAM_1024X_32BIT_598";\
csr[5][0x95c/4] = "TRACER_RAM_1024X_32BIT_599";\
csr[5][0x960/4] = "TRACER_RAM_1024X_32BIT_600";\
csr[5][0x964/4] = "TRACER_RAM_1024X_32BIT_601";\
csr[5][0x968/4] = "TRACER_RAM_1024X_32BIT_602";\
csr[5][0x96c/4] = "TRACER_RAM_1024X_32BIT_603";\
csr[5][0x970/4] = "TRACER_RAM_1024X_32BIT_604";\
csr[5][0x974/4] = "TRACER_RAM_1024X_32BIT_605";\
csr[5][0x978/4] = "TRACER_RAM_1024X_32BIT_606";\
csr[5][0x97c/4] = "TRACER_RAM_1024X_32BIT_607";\
csr[5][0x980/4] = "TRACER_RAM_1024X_32BIT_608";\
csr[5][0x984/4] = "TRACER_RAM_1024X_32BIT_609";\
csr[5][0x988/4] = "TRACER_RAM_1024X_32BIT_610";\
csr[5][0x98c/4] = "TRACER_RAM_1024X_32BIT_611";\
csr[5][0x990/4] = "TRACER_RAM_1024X_32BIT_612";\
csr[5][0x994/4] = "TRACER_RAM_1024X_32BIT_613";\
csr[5][0x998/4] = "TRACER_RAM_1024X_32BIT_614";\
csr[5][0x99c/4] = "TRACER_RAM_1024X_32BIT_615";\
csr[5][0x9a0/4] = "TRACER_RAM_1024X_32BIT_616";\
csr[5][0x9a4/4] = "TRACER_RAM_1024X_32BIT_617";\
csr[5][0x9a8/4] = "TRACER_RAM_1024X_32BIT_618";\
csr[5][0x9ac/4] = "TRACER_RAM_1024X_32BIT_619";\
csr[5][0x9b0/4] = "TRACER_RAM_1024X_32BIT_620";\
csr[5][0x9b4/4] = "TRACER_RAM_1024X_32BIT_621";\
csr[5][0x9b8/4] = "TRACER_RAM_1024X_32BIT_622";\
csr[5][0x9bc/4] = "TRACER_RAM_1024X_32BIT_623";\
csr[5][0x9c0/4] = "TRACER_RAM_1024X_32BIT_624";\
csr[5][0x9c4/4] = "TRACER_RAM_1024X_32BIT_625";\
csr[5][0x9c8/4] = "TRACER_RAM_1024X_32BIT_626";\
csr[5][0x9cc/4] = "TRACER_RAM_1024X_32BIT_627";\
csr[5][0x9d0/4] = "TRACER_RAM_1024X_32BIT_628";\
csr[5][0x9d4/4] = "TRACER_RAM_1024X_32BIT_629";\
csr[5][0x9d8/4] = "TRACER_RAM_1024X_32BIT_630";\
csr[5][0x9dc/4] = "TRACER_RAM_1024X_32BIT_631";\
csr[5][0x9e0/4] = "TRACER_RAM_1024X_32BIT_632";\
csr[5][0x9e4/4] = "TRACER_RAM_1024X_32BIT_633";\
csr[5][0x9e8/4] = "TRACER_RAM_1024X_32BIT_634";\
csr[5][0x9ec/4] = "TRACER_RAM_1024X_32BIT_635";\
csr[5][0x9f0/4] = "TRACER_RAM_1024X_32BIT_636";\
csr[5][0x9f4/4] = "TRACER_RAM_1024X_32BIT_637";\
csr[5][0x9f8/4] = "TRACER_RAM_1024X_32BIT_638";\
csr[5][0x9fc/4] = "TRACER_RAM_1024X_32BIT_639";\
csr[5][0xa00/4] = "TRACER_RAM_1024X_32BIT_640";\
csr[5][0xa04/4] = "TRACER_RAM_1024X_32BIT_641";\
csr[5][0xa08/4] = "TRACER_RAM_1024X_32BIT_642";\
csr[5][0xa0c/4] = "TRACER_RAM_1024X_32BIT_643";\
csr[5][0xa10/4] = "TRACER_RAM_1024X_32BIT_644";\
csr[5][0xa14/4] = "TRACER_RAM_1024X_32BIT_645";\
csr[5][0xa18/4] = "TRACER_RAM_1024X_32BIT_646";\
csr[5][0xa1c/4] = "TRACER_RAM_1024X_32BIT_647";\
csr[5][0xa20/4] = "TRACER_RAM_1024X_32BIT_648";\
csr[5][0xa24/4] = "TRACER_RAM_1024X_32BIT_649";\
csr[5][0xa28/4] = "TRACER_RAM_1024X_32BIT_650";\
csr[5][0xa2c/4] = "TRACER_RAM_1024X_32BIT_651";\
csr[5][0xa30/4] = "TRACER_RAM_1024X_32BIT_652";\
csr[5][0xa34/4] = "TRACER_RAM_1024X_32BIT_653";\
csr[5][0xa38/4] = "TRACER_RAM_1024X_32BIT_654";\
csr[5][0xa3c/4] = "TRACER_RAM_1024X_32BIT_655";\
csr[5][0xa40/4] = "TRACER_RAM_1024X_32BIT_656";\
csr[5][0xa44/4] = "TRACER_RAM_1024X_32BIT_657";\
csr[5][0xa48/4] = "TRACER_RAM_1024X_32BIT_658";\
csr[5][0xa4c/4] = "TRACER_RAM_1024X_32BIT_659";\
csr[5][0xa50/4] = "TRACER_RAM_1024X_32BIT_660";\
csr[5][0xa54/4] = "TRACER_RAM_1024X_32BIT_661";\
csr[5][0xa58/4] = "TRACER_RAM_1024X_32BIT_662";\
csr[5][0xa5c/4] = "TRACER_RAM_1024X_32BIT_663";\
csr[5][0xa60/4] = "TRACER_RAM_1024X_32BIT_664";\
csr[5][0xa64/4] = "TRACER_RAM_1024X_32BIT_665";\
csr[5][0xa68/4] = "TRACER_RAM_1024X_32BIT_666";\
csr[5][0xa6c/4] = "TRACER_RAM_1024X_32BIT_667";\
csr[5][0xa70/4] = "TRACER_RAM_1024X_32BIT_668";\
csr[5][0xa74/4] = "TRACER_RAM_1024X_32BIT_669";\
csr[5][0xa78/4] = "TRACER_RAM_1024X_32BIT_670";\
csr[5][0xa7c/4] = "TRACER_RAM_1024X_32BIT_671";\
csr[5][0xa80/4] = "TRACER_RAM_1024X_32BIT_672";\
csr[5][0xa84/4] = "TRACER_RAM_1024X_32BIT_673";\
csr[5][0xa88/4] = "TRACER_RAM_1024X_32BIT_674";\
csr[5][0xa8c/4] = "TRACER_RAM_1024X_32BIT_675";\
csr[5][0xa90/4] = "TRACER_RAM_1024X_32BIT_676";\
csr[5][0xa94/4] = "TRACER_RAM_1024X_32BIT_677";\
csr[5][0xa98/4] = "TRACER_RAM_1024X_32BIT_678";\
csr[5][0xa9c/4] = "TRACER_RAM_1024X_32BIT_679";\
csr[5][0xaa0/4] = "TRACER_RAM_1024X_32BIT_680";\
csr[5][0xaa4/4] = "TRACER_RAM_1024X_32BIT_681";\
csr[5][0xaa8/4] = "TRACER_RAM_1024X_32BIT_682";\
csr[5][0xaac/4] = "TRACER_RAM_1024X_32BIT_683";\
csr[5][0xab0/4] = "TRACER_RAM_1024X_32BIT_684";\
csr[5][0xab4/4] = "TRACER_RAM_1024X_32BIT_685";\
csr[5][0xab8/4] = "TRACER_RAM_1024X_32BIT_686";\
csr[5][0xabc/4] = "TRACER_RAM_1024X_32BIT_687";\
csr[5][0xac0/4] = "TRACER_RAM_1024X_32BIT_688";\
csr[5][0xac4/4] = "TRACER_RAM_1024X_32BIT_689";\
csr[5][0xac8/4] = "TRACER_RAM_1024X_32BIT_690";\
csr[5][0xacc/4] = "TRACER_RAM_1024X_32BIT_691";\
csr[5][0xad0/4] = "TRACER_RAM_1024X_32BIT_692";\
csr[5][0xad4/4] = "TRACER_RAM_1024X_32BIT_693";\
csr[5][0xad8/4] = "TRACER_RAM_1024X_32BIT_694";\
csr[5][0xadc/4] = "TRACER_RAM_1024X_32BIT_695";\
csr[5][0xae0/4] = "TRACER_RAM_1024X_32BIT_696";\
csr[5][0xae4/4] = "TRACER_RAM_1024X_32BIT_697";\
csr[5][0xae8/4] = "TRACER_RAM_1024X_32BIT_698";\
csr[5][0xaec/4] = "TRACER_RAM_1024X_32BIT_699";\
csr[5][0xaf0/4] = "TRACER_RAM_1024X_32BIT_700";\
csr[5][0xaf4/4] = "TRACER_RAM_1024X_32BIT_701";\
csr[5][0xaf8/4] = "TRACER_RAM_1024X_32BIT_702";\
csr[5][0xafc/4] = "TRACER_RAM_1024X_32BIT_703";\
csr[5][0xb00/4] = "TRACER_RAM_1024X_32BIT_704";\
csr[5][0xb04/4] = "TRACER_RAM_1024X_32BIT_705";\
csr[5][0xb08/4] = "TRACER_RAM_1024X_32BIT_706";\
csr[5][0xb0c/4] = "TRACER_RAM_1024X_32BIT_707";\
csr[5][0xb10/4] = "TRACER_RAM_1024X_32BIT_708";\
csr[5][0xb14/4] = "TRACER_RAM_1024X_32BIT_709";\
csr[5][0xb18/4] = "TRACER_RAM_1024X_32BIT_710";\
csr[5][0xb1c/4] = "TRACER_RAM_1024X_32BIT_711";\
csr[5][0xb20/4] = "TRACER_RAM_1024X_32BIT_712";\
csr[5][0xb24/4] = "TRACER_RAM_1024X_32BIT_713";\
csr[5][0xb28/4] = "TRACER_RAM_1024X_32BIT_714";\
csr[5][0xb2c/4] = "TRACER_RAM_1024X_32BIT_715";\
csr[5][0xb30/4] = "TRACER_RAM_1024X_32BIT_716";\
csr[5][0xb34/4] = "TRACER_RAM_1024X_32BIT_717";\
csr[5][0xb38/4] = "TRACER_RAM_1024X_32BIT_718";\
csr[5][0xb3c/4] = "TRACER_RAM_1024X_32BIT_719";\
csr[5][0xb40/4] = "TRACER_RAM_1024X_32BIT_720";\
csr[5][0xb44/4] = "TRACER_RAM_1024X_32BIT_721";\
csr[5][0xb48/4] = "TRACER_RAM_1024X_32BIT_722";\
csr[5][0xb4c/4] = "TRACER_RAM_1024X_32BIT_723";\
csr[5][0xb50/4] = "TRACER_RAM_1024X_32BIT_724";\
csr[5][0xb54/4] = "TRACER_RAM_1024X_32BIT_725";\
csr[5][0xb58/4] = "TRACER_RAM_1024X_32BIT_726";\
csr[5][0xb5c/4] = "TRACER_RAM_1024X_32BIT_727";\
csr[5][0xb60/4] = "TRACER_RAM_1024X_32BIT_728";\
csr[5][0xb64/4] = "TRACER_RAM_1024X_32BIT_729";\
csr[5][0xb68/4] = "TRACER_RAM_1024X_32BIT_730";\
csr[5][0xb6c/4] = "TRACER_RAM_1024X_32BIT_731";\
csr[5][0xb70/4] = "TRACER_RAM_1024X_32BIT_732";\
csr[5][0xb74/4] = "TRACER_RAM_1024X_32BIT_733";\
csr[5][0xb78/4] = "TRACER_RAM_1024X_32BIT_734";\
csr[5][0xb7c/4] = "TRACER_RAM_1024X_32BIT_735";\
csr[5][0xb80/4] = "TRACER_RAM_1024X_32BIT_736";\
csr[5][0xb84/4] = "TRACER_RAM_1024X_32BIT_737";\
csr[5][0xb88/4] = "TRACER_RAM_1024X_32BIT_738";\
csr[5][0xb8c/4] = "TRACER_RAM_1024X_32BIT_739";\
csr[5][0xb90/4] = "TRACER_RAM_1024X_32BIT_740";\
csr[5][0xb94/4] = "TRACER_RAM_1024X_32BIT_741";\
csr[5][0xb98/4] = "TRACER_RAM_1024X_32BIT_742";\
csr[5][0xb9c/4] = "TRACER_RAM_1024X_32BIT_743";\
csr[5][0xba0/4] = "TRACER_RAM_1024X_32BIT_744";\
csr[5][0xba4/4] = "TRACER_RAM_1024X_32BIT_745";\
csr[5][0xba8/4] = "TRACER_RAM_1024X_32BIT_746";\
csr[5][0xbac/4] = "TRACER_RAM_1024X_32BIT_747";\
csr[5][0xbb0/4] = "TRACER_RAM_1024X_32BIT_748";\
csr[5][0xbb4/4] = "TRACER_RAM_1024X_32BIT_749";\
csr[5][0xbb8/4] = "TRACER_RAM_1024X_32BIT_750";\
csr[5][0xbbc/4] = "TRACER_RAM_1024X_32BIT_751";\
csr[5][0xbc0/4] = "TRACER_RAM_1024X_32BIT_752";\
csr[5][0xbc4/4] = "TRACER_RAM_1024X_32BIT_753";\
csr[5][0xbc8/4] = "TRACER_RAM_1024X_32BIT_754";\
csr[5][0xbcc/4] = "TRACER_RAM_1024X_32BIT_755";\
csr[5][0xbd0/4] = "TRACER_RAM_1024X_32BIT_756";\
csr[5][0xbd4/4] = "TRACER_RAM_1024X_32BIT_757";\
csr[5][0xbd8/4] = "TRACER_RAM_1024X_32BIT_758";\
csr[5][0xbdc/4] = "TRACER_RAM_1024X_32BIT_759";\
csr[5][0xbe0/4] = "TRACER_RAM_1024X_32BIT_760";\
csr[5][0xbe4/4] = "TRACER_RAM_1024X_32BIT_761";\
csr[5][0xbe8/4] = "TRACER_RAM_1024X_32BIT_762";\
csr[5][0xbec/4] = "TRACER_RAM_1024X_32BIT_763";\
csr[5][0xbf0/4] = "TRACER_RAM_1024X_32BIT_764";\
csr[5][0xbf4/4] = "TRACER_RAM_1024X_32BIT_765";\
csr[5][0xbf8/4] = "TRACER_RAM_1024X_32BIT_766";\
csr[5][0xbfc/4] = "TRACER_RAM_1024X_32BIT_767";\
csr[5][0xc00/4] = "TRACER_RAM_1024X_32BIT_768";\
csr[5][0xc04/4] = "TRACER_RAM_1024X_32BIT_769";\
csr[5][0xc08/4] = "TRACER_RAM_1024X_32BIT_770";\
csr[5][0xc0c/4] = "TRACER_RAM_1024X_32BIT_771";\
csr[5][0xc10/4] = "TRACER_RAM_1024X_32BIT_772";\
csr[5][0xc14/4] = "TRACER_RAM_1024X_32BIT_773";\
csr[5][0xc18/4] = "TRACER_RAM_1024X_32BIT_774";\
csr[5][0xc1c/4] = "TRACER_RAM_1024X_32BIT_775";\
csr[5][0xc20/4] = "TRACER_RAM_1024X_32BIT_776";\
csr[5][0xc24/4] = "TRACER_RAM_1024X_32BIT_777";\
csr[5][0xc28/4] = "TRACER_RAM_1024X_32BIT_778";\
csr[5][0xc2c/4] = "TRACER_RAM_1024X_32BIT_779";\
csr[5][0xc30/4] = "TRACER_RAM_1024X_32BIT_780";\
csr[5][0xc34/4] = "TRACER_RAM_1024X_32BIT_781";\
csr[5][0xc38/4] = "TRACER_RAM_1024X_32BIT_782";\
csr[5][0xc3c/4] = "TRACER_RAM_1024X_32BIT_783";\
csr[5][0xc40/4] = "TRACER_RAM_1024X_32BIT_784";\
csr[5][0xc44/4] = "TRACER_RAM_1024X_32BIT_785";\
csr[5][0xc48/4] = "TRACER_RAM_1024X_32BIT_786";\
csr[5][0xc4c/4] = "TRACER_RAM_1024X_32BIT_787";\
csr[5][0xc50/4] = "TRACER_RAM_1024X_32BIT_788";\
csr[5][0xc54/4] = "TRACER_RAM_1024X_32BIT_789";\
csr[5][0xc58/4] = "TRACER_RAM_1024X_32BIT_790";\
csr[5][0xc5c/4] = "TRACER_RAM_1024X_32BIT_791";\
csr[5][0xc60/4] = "TRACER_RAM_1024X_32BIT_792";\
csr[5][0xc64/4] = "TRACER_RAM_1024X_32BIT_793";\
csr[5][0xc68/4] = "TRACER_RAM_1024X_32BIT_794";\
csr[5][0xc6c/4] = "TRACER_RAM_1024X_32BIT_795";\
csr[5][0xc70/4] = "TRACER_RAM_1024X_32BIT_796";\
csr[5][0xc74/4] = "TRACER_RAM_1024X_32BIT_797";\
csr[5][0xc78/4] = "TRACER_RAM_1024X_32BIT_798";\
csr[5][0xc7c/4] = "TRACER_RAM_1024X_32BIT_799";\
csr[5][0xc80/4] = "TRACER_RAM_1024X_32BIT_800";\
csr[5][0xc84/4] = "TRACER_RAM_1024X_32BIT_801";\
csr[5][0xc88/4] = "TRACER_RAM_1024X_32BIT_802";\
csr[5][0xc8c/4] = "TRACER_RAM_1024X_32BIT_803";\
csr[5][0xc90/4] = "TRACER_RAM_1024X_32BIT_804";\
csr[5][0xc94/4] = "TRACER_RAM_1024X_32BIT_805";\
csr[5][0xc98/4] = "TRACER_RAM_1024X_32BIT_806";\
csr[5][0xc9c/4] = "TRACER_RAM_1024X_32BIT_807";\
csr[5][0xca0/4] = "TRACER_RAM_1024X_32BIT_808";\
csr[5][0xca4/4] = "TRACER_RAM_1024X_32BIT_809";\
csr[5][0xca8/4] = "TRACER_RAM_1024X_32BIT_810";\
csr[5][0xcac/4] = "TRACER_RAM_1024X_32BIT_811";\
csr[5][0xcb0/4] = "TRACER_RAM_1024X_32BIT_812";\
csr[5][0xcb4/4] = "TRACER_RAM_1024X_32BIT_813";\
csr[5][0xcb8/4] = "TRACER_RAM_1024X_32BIT_814";\
csr[5][0xcbc/4] = "TRACER_RAM_1024X_32BIT_815";\
csr[5][0xcc0/4] = "TRACER_RAM_1024X_32BIT_816";\
csr[5][0xcc4/4] = "TRACER_RAM_1024X_32BIT_817";\
csr[5][0xcc8/4] = "TRACER_RAM_1024X_32BIT_818";\
csr[5][0xccc/4] = "TRACER_RAM_1024X_32BIT_819";\
csr[5][0xcd0/4] = "TRACER_RAM_1024X_32BIT_820";\
csr[5][0xcd4/4] = "TRACER_RAM_1024X_32BIT_821";\
csr[5][0xcd8/4] = "TRACER_RAM_1024X_32BIT_822";\
csr[5][0xcdc/4] = "TRACER_RAM_1024X_32BIT_823";\
csr[5][0xce0/4] = "TRACER_RAM_1024X_32BIT_824";\
csr[5][0xce4/4] = "TRACER_RAM_1024X_32BIT_825";\
csr[5][0xce8/4] = "TRACER_RAM_1024X_32BIT_826";\
csr[5][0xcec/4] = "TRACER_RAM_1024X_32BIT_827";\
csr[5][0xcf0/4] = "TRACER_RAM_1024X_32BIT_828";\
csr[5][0xcf4/4] = "TRACER_RAM_1024X_32BIT_829";\
csr[5][0xcf8/4] = "TRACER_RAM_1024X_32BIT_830";\
csr[5][0xcfc/4] = "TRACER_RAM_1024X_32BIT_831";\
csr[5][0xd00/4] = "TRACER_RAM_1024X_32BIT_832";\
csr[5][0xd04/4] = "TRACER_RAM_1024X_32BIT_833";\
csr[5][0xd08/4] = "TRACER_RAM_1024X_32BIT_834";\
csr[5][0xd0c/4] = "TRACER_RAM_1024X_32BIT_835";\
csr[5][0xd10/4] = "TRACER_RAM_1024X_32BIT_836";\
csr[5][0xd14/4] = "TRACER_RAM_1024X_32BIT_837";\
csr[5][0xd18/4] = "TRACER_RAM_1024X_32BIT_838";\
csr[5][0xd1c/4] = "TRACER_RAM_1024X_32BIT_839";\
csr[5][0xd20/4] = "TRACER_RAM_1024X_32BIT_840";\
csr[5][0xd24/4] = "TRACER_RAM_1024X_32BIT_841";\
csr[5][0xd28/4] = "TRACER_RAM_1024X_32BIT_842";\
csr[5][0xd2c/4] = "TRACER_RAM_1024X_32BIT_843";\
csr[5][0xd30/4] = "TRACER_RAM_1024X_32BIT_844";\
csr[5][0xd34/4] = "TRACER_RAM_1024X_32BIT_845";\
csr[5][0xd38/4] = "TRACER_RAM_1024X_32BIT_846";\
csr[5][0xd3c/4] = "TRACER_RAM_1024X_32BIT_847";\
csr[5][0xd40/4] = "TRACER_RAM_1024X_32BIT_848";\
csr[5][0xd44/4] = "TRACER_RAM_1024X_32BIT_849";\
csr[5][0xd48/4] = "TRACER_RAM_1024X_32BIT_850";\
csr[5][0xd4c/4] = "TRACER_RAM_1024X_32BIT_851";\
csr[5][0xd50/4] = "TRACER_RAM_1024X_32BIT_852";\
csr[5][0xd54/4] = "TRACER_RAM_1024X_32BIT_853";\
csr[5][0xd58/4] = "TRACER_RAM_1024X_32BIT_854";\
csr[5][0xd5c/4] = "TRACER_RAM_1024X_32BIT_855";\
csr[5][0xd60/4] = "TRACER_RAM_1024X_32BIT_856";\
csr[5][0xd64/4] = "TRACER_RAM_1024X_32BIT_857";\
csr[5][0xd68/4] = "TRACER_RAM_1024X_32BIT_858";\
csr[5][0xd6c/4] = "TRACER_RAM_1024X_32BIT_859";\
csr[5][0xd70/4] = "TRACER_RAM_1024X_32BIT_860";\
csr[5][0xd74/4] = "TRACER_RAM_1024X_32BIT_861";\
csr[5][0xd78/4] = "TRACER_RAM_1024X_32BIT_862";\
csr[5][0xd7c/4] = "TRACER_RAM_1024X_32BIT_863";\
csr[5][0xd80/4] = "TRACER_RAM_1024X_32BIT_864";\
csr[5][0xd84/4] = "TRACER_RAM_1024X_32BIT_865";\
csr[5][0xd88/4] = "TRACER_RAM_1024X_32BIT_866";\
csr[5][0xd8c/4] = "TRACER_RAM_1024X_32BIT_867";\
csr[5][0xd90/4] = "TRACER_RAM_1024X_32BIT_868";\
csr[5][0xd94/4] = "TRACER_RAM_1024X_32BIT_869";\
csr[5][0xd98/4] = "TRACER_RAM_1024X_32BIT_870";\
csr[5][0xd9c/4] = "TRACER_RAM_1024X_32BIT_871";\
csr[5][0xda0/4] = "TRACER_RAM_1024X_32BIT_872";\
csr[5][0xda4/4] = "TRACER_RAM_1024X_32BIT_873";\
csr[5][0xda8/4] = "TRACER_RAM_1024X_32BIT_874";\
csr[5][0xdac/4] = "TRACER_RAM_1024X_32BIT_875";\
csr[5][0xdb0/4] = "TRACER_RAM_1024X_32BIT_876";\
csr[5][0xdb4/4] = "TRACER_RAM_1024X_32BIT_877";\
csr[5][0xdb8/4] = "TRACER_RAM_1024X_32BIT_878";\
csr[5][0xdbc/4] = "TRACER_RAM_1024X_32BIT_879";\
csr[5][0xdc0/4] = "TRACER_RAM_1024X_32BIT_880";\
csr[5][0xdc4/4] = "TRACER_RAM_1024X_32BIT_881";\
csr[5][0xdc8/4] = "TRACER_RAM_1024X_32BIT_882";\
csr[5][0xdcc/4] = "TRACER_RAM_1024X_32BIT_883";\
csr[5][0xdd0/4] = "TRACER_RAM_1024X_32BIT_884";\
csr[5][0xdd4/4] = "TRACER_RAM_1024X_32BIT_885";\
csr[5][0xdd8/4] = "TRACER_RAM_1024X_32BIT_886";\
csr[5][0xddc/4] = "TRACER_RAM_1024X_32BIT_887";\
csr[5][0xde0/4] = "TRACER_RAM_1024X_32BIT_888";\
csr[5][0xde4/4] = "TRACER_RAM_1024X_32BIT_889";\
csr[5][0xde8/4] = "TRACER_RAM_1024X_32BIT_890";\
csr[5][0xdec/4] = "TRACER_RAM_1024X_32BIT_891";\
csr[5][0xdf0/4] = "TRACER_RAM_1024X_32BIT_892";\
csr[5][0xdf4/4] = "TRACER_RAM_1024X_32BIT_893";\
csr[5][0xdf8/4] = "TRACER_RAM_1024X_32BIT_894";\
csr[5][0xdfc/4] = "TRACER_RAM_1024X_32BIT_895";\
csr[5][0xe00/4] = "TRACER_RAM_1024X_32BIT_896";\
csr[5][0xe04/4] = "TRACER_RAM_1024X_32BIT_897";\
csr[5][0xe08/4] = "TRACER_RAM_1024X_32BIT_898";\
csr[5][0xe0c/4] = "TRACER_RAM_1024X_32BIT_899";\
csr[5][0xe10/4] = "TRACER_RAM_1024X_32BIT_900";\
csr[5][0xe14/4] = "TRACER_RAM_1024X_32BIT_901";\
csr[5][0xe18/4] = "TRACER_RAM_1024X_32BIT_902";\
csr[5][0xe1c/4] = "TRACER_RAM_1024X_32BIT_903";\
csr[5][0xe20/4] = "TRACER_RAM_1024X_32BIT_904";\
csr[5][0xe24/4] = "TRACER_RAM_1024X_32BIT_905";\
csr[5][0xe28/4] = "TRACER_RAM_1024X_32BIT_906";\
csr[5][0xe2c/4] = "TRACER_RAM_1024X_32BIT_907";\
csr[5][0xe30/4] = "TRACER_RAM_1024X_32BIT_908";\
csr[5][0xe34/4] = "TRACER_RAM_1024X_32BIT_909";\
csr[5][0xe38/4] = "TRACER_RAM_1024X_32BIT_910";\
csr[5][0xe3c/4] = "TRACER_RAM_1024X_32BIT_911";\
csr[5][0xe40/4] = "TRACER_RAM_1024X_32BIT_912";\
csr[5][0xe44/4] = "TRACER_RAM_1024X_32BIT_913";\
csr[5][0xe48/4] = "TRACER_RAM_1024X_32BIT_914";\
csr[5][0xe4c/4] = "TRACER_RAM_1024X_32BIT_915";\
csr[5][0xe50/4] = "TRACER_RAM_1024X_32BIT_916";\
csr[5][0xe54/4] = "TRACER_RAM_1024X_32BIT_917";\
csr[5][0xe58/4] = "TRACER_RAM_1024X_32BIT_918";\
csr[5][0xe5c/4] = "TRACER_RAM_1024X_32BIT_919";\
csr[5][0xe60/4] = "TRACER_RAM_1024X_32BIT_920";\
csr[5][0xe64/4] = "TRACER_RAM_1024X_32BIT_921";\
csr[5][0xe68/4] = "TRACER_RAM_1024X_32BIT_922";\
csr[5][0xe6c/4] = "TRACER_RAM_1024X_32BIT_923";\
csr[5][0xe70/4] = "TRACER_RAM_1024X_32BIT_924";\
csr[5][0xe74/4] = "TRACER_RAM_1024X_32BIT_925";\
csr[5][0xe78/4] = "TRACER_RAM_1024X_32BIT_926";\
csr[5][0xe7c/4] = "TRACER_RAM_1024X_32BIT_927";\
csr[5][0xe80/4] = "TRACER_RAM_1024X_32BIT_928";\
csr[5][0xe84/4] = "TRACER_RAM_1024X_32BIT_929";\
csr[5][0xe88/4] = "TRACER_RAM_1024X_32BIT_930";\
csr[5][0xe8c/4] = "TRACER_RAM_1024X_32BIT_931";\
csr[5][0xe90/4] = "TRACER_RAM_1024X_32BIT_932";\
csr[5][0xe94/4] = "TRACER_RAM_1024X_32BIT_933";\
csr[5][0xe98/4] = "TRACER_RAM_1024X_32BIT_934";\
csr[5][0xe9c/4] = "TRACER_RAM_1024X_32BIT_935";\
csr[5][0xea0/4] = "TRACER_RAM_1024X_32BIT_936";\
csr[5][0xea4/4] = "TRACER_RAM_1024X_32BIT_937";\
csr[5][0xea8/4] = "TRACER_RAM_1024X_32BIT_938";\
csr[5][0xeac/4] = "TRACER_RAM_1024X_32BIT_939";\
csr[5][0xeb0/4] = "TRACER_RAM_1024X_32BIT_940";\
csr[5][0xeb4/4] = "TRACER_RAM_1024X_32BIT_941";\
csr[5][0xeb8/4] = "TRACER_RAM_1024X_32BIT_942";\
csr[5][0xebc/4] = "TRACER_RAM_1024X_32BIT_943";\
csr[5][0xec0/4] = "TRACER_RAM_1024X_32BIT_944";\
csr[5][0xec4/4] = "TRACER_RAM_1024X_32BIT_945";\
csr[5][0xec8/4] = "TRACER_RAM_1024X_32BIT_946";\
csr[5][0xecc/4] = "TRACER_RAM_1024X_32BIT_947";\
csr[5][0xed0/4] = "TRACER_RAM_1024X_32BIT_948";\
csr[5][0xed4/4] = "TRACER_RAM_1024X_32BIT_949";\
csr[5][0xed8/4] = "TRACER_RAM_1024X_32BIT_950";\
csr[5][0xedc/4] = "TRACER_RAM_1024X_32BIT_951";\
csr[5][0xee0/4] = "TRACER_RAM_1024X_32BIT_952";\
csr[5][0xee4/4] = "TRACER_RAM_1024X_32BIT_953";\
csr[5][0xee8/4] = "TRACER_RAM_1024X_32BIT_954";\
csr[5][0xeec/4] = "TRACER_RAM_1024X_32BIT_955";\
csr[5][0xef0/4] = "TRACER_RAM_1024X_32BIT_956";\
csr[5][0xef4/4] = "TRACER_RAM_1024X_32BIT_957";\
csr[5][0xef8/4] = "TRACER_RAM_1024X_32BIT_958";\
csr[5][0xefc/4] = "TRACER_RAM_1024X_32BIT_959";\
csr[5][0xf00/4] = "TRACER_RAM_1024X_32BIT_960";\
csr[5][0xf04/4] = "TRACER_RAM_1024X_32BIT_961";\
csr[5][0xf08/4] = "TRACER_RAM_1024X_32BIT_962";\
csr[5][0xf0c/4] = "TRACER_RAM_1024X_32BIT_963";\
csr[5][0xf10/4] = "TRACER_RAM_1024X_32BIT_964";\
csr[5][0xf14/4] = "TRACER_RAM_1024X_32BIT_965";\
csr[5][0xf18/4] = "TRACER_RAM_1024X_32BIT_966";\
csr[5][0xf1c/4] = "TRACER_RAM_1024X_32BIT_967";\
csr[5][0xf20/4] = "TRACER_RAM_1024X_32BIT_968";\
csr[5][0xf24/4] = "TRACER_RAM_1024X_32BIT_969";\
csr[5][0xf28/4] = "TRACER_RAM_1024X_32BIT_970";\
csr[5][0xf2c/4] = "TRACER_RAM_1024X_32BIT_971";\
csr[5][0xf30/4] = "TRACER_RAM_1024X_32BIT_972";\
csr[5][0xf34/4] = "TRACER_RAM_1024X_32BIT_973";\
csr[5][0xf38/4] = "TRACER_RAM_1024X_32BIT_974";\
csr[5][0xf3c/4] = "TRACER_RAM_1024X_32BIT_975";\
csr[5][0xf40/4] = "TRACER_RAM_1024X_32BIT_976";\
csr[5][0xf44/4] = "TRACER_RAM_1024X_32BIT_977";\
csr[5][0xf48/4] = "TRACER_RAM_1024X_32BIT_978";\
csr[5][0xf4c/4] = "TRACER_RAM_1024X_32BIT_979";\
csr[5][0xf50/4] = "TRACER_RAM_1024X_32BIT_980";\
csr[5][0xf54/4] = "TRACER_RAM_1024X_32BIT_981";\
csr[5][0xf58/4] = "TRACER_RAM_1024X_32BIT_982";\
csr[5][0xf5c/4] = "TRACER_RAM_1024X_32BIT_983";\
csr[5][0xf60/4] = "TRACER_RAM_1024X_32BIT_984";\
csr[5][0xf64/4] = "TRACER_RAM_1024X_32BIT_985";\
csr[5][0xf68/4] = "TRACER_RAM_1024X_32BIT_986";\
csr[5][0xf6c/4] = "TRACER_RAM_1024X_32BIT_987";\
csr[5][0xf70/4] = "TRACER_RAM_1024X_32BIT_988";\
csr[5][0xf74/4] = "TRACER_RAM_1024X_32BIT_989";\
csr[5][0xf78/4] = "TRACER_RAM_1024X_32BIT_990";\
csr[5][0xf7c/4] = "TRACER_RAM_1024X_32BIT_991";\
csr[5][0xf80/4] = "TRACER_RAM_1024X_32BIT_992";\
csr[5][0xf84/4] = "TRACER_RAM_1024X_32BIT_993";\
csr[5][0xf88/4] = "TRACER_RAM_1024X_32BIT_994";\
csr[5][0xf8c/4] = "TRACER_RAM_1024X_32BIT_995";\
csr[5][0xf90/4] = "TRACER_RAM_1024X_32BIT_996";\
csr[5][0xf94/4] = "TRACER_RAM_1024X_32BIT_997";\
csr[5][0xf98/4] = "TRACER_RAM_1024X_32BIT_998";\
csr[5][0xf9c/4] = "TRACER_RAM_1024X_32BIT_999";\
csr[5][0xfa0/4] = "TRACER_RAM_1024X_32BIT_1000";\
csr[5][0xfa4/4] = "TRACER_RAM_1024X_32BIT_1001";\
csr[5][0xfa8/4] = "TRACER_RAM_1024X_32BIT_1002";\
csr[5][0xfac/4] = "TRACER_RAM_1024X_32BIT_1003";\
csr[5][0xfb0/4] = "TRACER_RAM_1024X_32BIT_1004";\
csr[5][0xfb4/4] = "TRACER_RAM_1024X_32BIT_1005";\
csr[5][0xfb8/4] = "TRACER_RAM_1024X_32BIT_1006";\
csr[5][0xfbc/4] = "TRACER_RAM_1024X_32BIT_1007";\
csr[5][0xfc0/4] = "TRACER_RAM_1024X_32BIT_1008";\
csr[5][0xfc4/4] = "TRACER_RAM_1024X_32BIT_1009";\
csr[5][0xfc8/4] = "TRACER_RAM_1024X_32BIT_1010";\
csr[5][0xfcc/4] = "TRACER_RAM_1024X_32BIT_1011";\
csr[5][0xfd0/4] = "TRACER_RAM_1024X_32BIT_1012";\
csr[5][0xfd4/4] = "TRACER_RAM_1024X_32BIT_1013";\
csr[5][0xfd8/4] = "TRACER_RAM_1024X_32BIT_1014";\
csr[5][0xfdc/4] = "TRACER_RAM_1024X_32BIT_1015";\
csr[5][0xfe0/4] = "TRACER_RAM_1024X_32BIT_1016";\
csr[5][0xfe4/4] = "TRACER_RAM_1024X_32BIT_1017";\
csr[5][0xfe8/4] = "TRACER_RAM_1024X_32BIT_1018";\
csr[5][0xfec/4] = "TRACER_RAM_1024X_32BIT_1019";\
csr[5][0xff0/4] = "TRACER_RAM_1024X_32BIT_1020";\
csr[5][0xff4/4] = "TRACER_RAM_1024X_32BIT_1021";\
csr[5][0xff8/4] = "TRACER_RAM_1024X_32BIT_1022";\
csr[5][0xffc/4] = "TRACER_RAM_1024X_32BIT_1023"
