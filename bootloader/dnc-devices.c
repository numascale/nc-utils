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
#include "dnc-devices.h"
#include "dnc-access.h"
#include "dnc-bootloader.h"
#include "dnc-commonlib.h"

static void stop_ohci(u8 bus, u8 dev, u8 fn)
{
    u32 val, bar0;

    bar0 = dnc_read_conf(0xfff0, bus, dev, fn, 0x10);
    if ((bar0 != 0xffffffff) && (bar0 != 0)) {
        val = mem64_read32(bar0 + HcHCCA);
        printf("Found OHCI controller at %02x:%02x:%x, BAR0 @%08x, HCCA @%08x\n",
               bus, dev, fn, bar0, val);
        val = mem64_read32(bar0 + HcControl);
        if (val & OHCI_CTRL_IR) { /* Interrupt routing enabled, we must request change of ownership */
            u32 temp;
            printf("Requesting Change of Ownership on OHCI controller %02x:%02x:%x\n",
                   bus, dev, fn);
            /* This timeout is arbitrary.  we make it long, so systems
             * depending on usb keyboards may be usable even if the
             * BIOS/SMM code seems pretty broken
             */
            temp = 500;	/* Arbitrary: five seconds */
            
            mem64_write32(bar0 + HcInterruptEnable, OHCI_INTR_OC); /* Enable OwnershipChange interrupt */
            mem64_write32(bar0 + HcCommandStatus, OHCI_OCR); /* Request OwnershipChange */
            while (mem64_read32(bar0 + HcControl) & OHCI_CTRL_IR) {
                tsc_wait(1000);
                if (--temp == 0) {
                    printf("OHCI HC takeover failed on %02x:%02x:%x! (BIOS/SMM bug)\n",
                           bus, dev, fn);
                    return;
                }
            }

            /* Shutdown */
            mem64_write32(bar0 + HcInterruptDisable, OHCI_INTR_MIE);
            val = mem64_read32(bar0 + HcControl);
            val &= OHCI_CTRL_RWC;
            mem64_write32(bar0 + HcControl, val);
            /* Flush the writes */
            val = mem64_read32(bar0 + HcControl);
        } else {
            printf("OHCI controller %02x:%02x:%x not in use by BIOS/SMM, skipping\n",
                   bus, dev, fn);
        }
        val = mem64_read32(bar0 + HcRevision);
        if (val & (1 << 8)) { /* Legacy emulation is supported */
            val = mem64_read32(bar0 + HceControl);
            if (val & (1 << 0)) {
                printf("Legacy support enabled!\n");
            }
        }
    }
}

void stop_usb(void)
{
    u32 val;
    u8 bus = 0;
    u8 dev;
    u8 fn;

    /* If SMM is disabled, USB functionality may have been lost */
    if (disable_smm)
	return;

    /* Scan bus 0 for OHCI controllers and disable them if used by BIOS/SMM */
    for (dev = 0; dev < 0x18; dev++) {
        for (fn = 0; fn < 7; fn++) {
            val = dnc_read_conf(0xfff0, bus, dev, fn, 8);
            if ((val >> 8) == PCI_CLASS_SERIAL_USB_OHCI)
                stop_ohci(bus, dev, fn);
        }
    }
}

