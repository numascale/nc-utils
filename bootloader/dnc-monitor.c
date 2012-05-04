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
#include "dnc-types.h"
#include "dnc-regs.h"
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

    printf("Measuring quiescent link activity...\n");

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

