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

#ifndef _HWCONFIG_H_
#define _HWCONFIG_H_ 1

#define DEBUG_TRAMP 0

/* If defined accesses to the VGA regions are always local */
#define USE_LOCAL_VGA 1

/* If defined, must evaluate to expression that determines whether or
   not a given core is marked as enabled in the ACPI structures, based
   on given core number. */
//#define APIC_ENABLE_MASK(x) ((0xffffffULL & (1ULL<<(x))) ? 1 : 0)

/* If defined, all nodes will have this amount of DRAM reserved in
   e820 maps. */
//#define TRACE_BUF_SIZE (512*1024*1024)

#endif
