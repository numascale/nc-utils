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

#include <stdio.h>
#include "dnc-monitor.h"
#include "dnc-types.h"
#include "dnc-regs.h"
#include "dnc-defs.h"
#include "dnc-access.h"
#include "dnc-bootloader.h"

/* Linear feedback register to count */
static int lfb_to_count(u32 val)
{
    unsigned long cMSB = 15;
    unsigned long lsb;
    unsigned long seq = 0;
    unsigned long start = 1 << cMSB;
    unsigned long lfsr = start;

    while (lfsr != val) {
	lsb = lfsr;
	lfsr >>= 1;
	lfsr |= ((lsb ^ lfsr) & 1) << cMSB;
	seq++;
    }

    /* Subtract fixed offset */
    return seq - 2;
}

void lc3_activity(void)
{
#ifdef __i386
    int lc, lim = 2;

    printf("Profiling quiescent link activity...\n");

    /* Count LC3 B-Link grant, packet reject and accept events */
    for (lc = 1; lc <= lim; lc++) {
	dnc_write_csr(0xfff1 + lc, LC3_CSR_PCSEL, 0x13);

	/* Clear count */
	for (lc = 1; lc <= lim; lc++)
	    dnc_write_csr(0xfff1 + lc, LC3_CSR_PCCNT, 0);

	msleep(500);

	for (lc = 1; lc <= lim; lc++) {
	    u32 val = dnc_read_csr(0xfff1 + lc, LC3_CSR_PCCNT);
	    printf("- %d events on link %d\n", lfb_to_count(val), lc);
	}
    }

    /* Disable counting */
    for (lc = 1; lc <= lim; lc++)
	dnc_write_csr(0xfff1 + lc, LC3_CSR_PCSEL, 0);
#endif
}

void system_activity(void)
{
    static struct perf_ev events[] = {
	{0x040, 0x00, "Data cache access"},
	{0x02b, 0x00, "SMI received"},
	{0x0cf, 0x00, "Interrupt received"},
	{0x0e0, 0x3f, "DRAM access"},
	{0x065, 0x01, "Request to UC memory"},
	{0x065, 0x02, "Request to WC or WC buffer flush to WB memory"},
	{0x065, 0x80, "Streaming store request"},
	{0x06d, 0x01, "Octwords written to system"},
	{0x1f0, 0x03, "Memory controller reads and write"},
	{0x0e9, 0xa8, "Xbar CPU to memory: Local to Local"},
	{0x0e9, 0x98, "Xbar CPU to memory: Local to Remote"},
	{0x0e9, 0xa4, "Xbar CPU to IO: Local to Local"},
	{0x0e9, 0x94, "Xbar CPU to IO: Local to Remote"},
	{0x0e9, 0x64, "Xbar CPU to IO: Remote to Local"},
	{0x0e9, 0xa2, "Xbar IO to memory: Local to Local"},
	{0x0e9, 0x92, "Xbar IO to memory: Local to Remote"},
	{0x0e9, 0xa1, "Xbar IO to IO: Local to Local"},
	{0x0e9, 0x91, "Xbar IO to IO: Local to Remote"},
	{0x0e9, 0x61, "Xbar IO to IO: Remote to Local"},
	{0x0eb, 0x01, "Sized Commands: Non-Posted SzWr Byte"},
	{0x0eb, 0x02, "Sized Commands: Non-Posted SzWr DW"},
	{0x0eb, 0x04, "Sized Commands: Posted SzWr Byte"},
	{0x0eb, 0x08, "Sized Commands: Posted SzWr DW"},
	{0x0eb, 0x10, "Sized Commands: SzRd Byte"},
	{0x0eb, 0x20, "Sized Commands: SzRd DW"},
	{0x000, 0x0, ""}
    };

    struct perf_ev *ev;
    u64 val;

    printf("Profiling quiescent system activity...\n");

    for (ev = events; ev->event; ev++) {
	dnc_wrmsr(MSR_PERF_CTL0, 0); /* Disable counter */
	dnc_wrmsr(MSR_PERF_CTR0, 0); /* Reset count */

	tsc_wait(10); /* Warmup */

	dnc_wrmsr(MSR_PERF_CTL0,
	    (ev->event & 0xff) | (ev->unitmask << 8) | (3 << 16) |
	    (1 << 22) | ((ev->event & 0xf00ULL) << (32 - 8)));

	tsc_wait(2000);

	val = dnc_rdmsr(MSR_PERF_CTR0);

	printf("- %lld %s events\n", val, ev->name);
	dnc_wrmsr(MSR_PERF_CTL0, 0); /* Disable counter */
    }
}

