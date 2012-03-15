// $Id:$
// This source code including any derived information including but
// not limited by net-lists, fpga bit-streams, and object files is the
// confidential and proprietary property of
// 
// Numascale AS
// Enebakkveien 302A
// NO-1188 Oslo
// Norway
// 
// Any unauthorized use, reproduction or transfer of the information
// provided herein is strictly prohibited.
// 
// Copyright © 2008-2011
// Numascale AS Oslo, Norway. 
// All Rights Reserved.
//

#ifndef __DNC_DEFS
#define __DNC_DEFS 1

#define NB_FUNC_HT    0
#define NB_FUNC_MAPS  1
#define NB_FUNC_DRAM  2
#define NB_FUNC_MISC  3
#define NB_FUNC_LINK  4

#define MSR_FS_BASE   0xc0000100
#define MSR_HWCR      0xc0010015
#define MSR_TOPMEM    0xc001001a
#define MSR_TOPMEM2   0xc001001d
#define MSR_MCFG_BASE 0xc0010058
#define MSR_SYSCFG    0xc0010010
#define MSR_NB_CFG    0xc001001f
#define MSR_APIC_BAR  0x0000001b
#define MSR_LSCFG     0xc0011020
#define MSR_NODE_ID   0xc001100c
#define MSR_SMM_BASE  0xc0010111
#define MSR_CU_CFG2   0xc001102a
#define MSR_OSVW_ID_LEN 0xc0010140
#define MSR_OSVW_STATUS 0xc0010141

#define MSR_MTRR_PHYS_BASE0 0x00000200
#define MSR_MTRR_PHYS_BASE1 0x00000202
#define MSR_MTRR_PHYS_BASE2 0x00000204
#define MSR_MTRR_PHYS_BASE3 0x00000206
#define MSR_MTRR_PHYS_BASE4 0x00000208
#define MSR_MTRR_PHYS_BASE5 0x0000020a
#define MSR_MTRR_PHYS_BASE6 0x0000020c
#define MSR_MTRR_PHYS_BASE7 0x0000020e

#define MSR_MTRR_PHYS_MASK0 0x00000201
#define MSR_MTRR_PHYS_MASK1 0x00000203
#define MSR_MTRR_PHYS_MASK2 0x00000205
#define MSR_MTRR_PHYS_MASK3 0x00000207
#define MSR_MTRR_PHYS_MASK4 0x00000209
#define MSR_MTRR_PHYS_MASK5 0x0000020b
#define MSR_MTRR_PHYS_MASK6 0x0000020d
#define MSR_MTRR_PHYS_MASK7 0x0000020f

#define MSR_IORR_PHYS_BASE0 0xc0010016
#define MSR_IORR_PHYS_BASE1 0xc0010018
#define MSR_IORR_PHYS_MASK0 0xc0010017
#define MSR_IORR_PHYS_MASK1 0xc0010019

#define MSR_MTRR_FIX64K_00000 0x00000250
#define MSR_MTRR_FIX16K_80000 0x00000258
#define MSR_MTRR_FIX16K_A0000 0x00000259
#define MSR_MTRR_FIX4K_C0000 0x00000268
#define MSR_MTRR_FIX4K_C8000 0x00000269
#define MSR_MTRR_FIX4K_D0000 0x0000026a
#define MSR_MTRR_FIX4K_D8000 0x0000026b
#define MSR_MTRR_FIX4K_E0000 0x0000026c
#define MSR_MTRR_FIX4K_E8000 0x0000026d
#define MSR_MTRR_FIX4K_F0000 0x0000026e
#define MSR_MTRR_FIX4K_F8000 0x0000026f

#define MSR_MTRR_DEFAULT 0x000002ff

#endif
