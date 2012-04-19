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

#ifndef __DNC_DEVICES
#define __DNC_DEVICES 1

#define HcRevision 0x00

#define HcControl 0x04

/*
 * HcControl (control) register masks
 */
#define OHCI_CTRL_CBSR  (3 << 0)        /* Control/bulk service ratio */
#define OHCI_CTRL_PLE   (1 << 2)        /* Periodic list enable */
#define OHCI_CTRL_IE    (1 << 3)        /* Isochronous enable */
#define OHCI_CTRL_CLE   (1 << 4)        /* Control list enable */
#define OHCI_CTRL_BLE   (1 << 5)        /* Bulk list enable */
#define OHCI_CTRL_HCFS  (3 << 6)        /* Host controller functional state */
#define OHCI_CTRL_IR    (1 << 8)        /* Interrupt routing */
#define OHCI_CTRL_RWC   (1 << 9)        /* Remote wakeup connected */
#define OHCI_CTRL_RWE   (1 << 10)       /* Remote wakeup enable */


#define HcCommandStatus 0x08

/*
 * HcCommandStatus (cmdstatus) register masks
 */
#define OHCI_HCR        (1 << 0)        /* Host controller reset */
#define OHCI_CLF        (1 << 1)        /* Control list filled */
#define OHCI_BLF        (1 << 2)        /* Bulk list filled */
#define OHCI_OCR        (1 << 3)        /* Ownership change request */
#define OHCI_SOC        (3 << 16)       /* Scheduling overrun count */

#define HcInterruptStatus 0x0C
#define HcInterruptEnable 0x10
#define HcInterruptDisable 0x14

/*
 * Masks used with interrupt registers:
 * HcInterruptStatus (intrstatus)
 * HcInterruptEnable (intrenable)
 * HcInterruptDisable (intrdisable)
 */
#define OHCI_INTR_SO	(1 << 0)	/* Scheduling overrun */
#define OHCI_INTR_WDH	(1 << 1)	/* Writeback of done_head */
#define OHCI_INTR_SF	(1 << 2)	/* Start frame */
#define OHCI_INTR_RD	(1 << 3)	/* Resume detect */
#define OHCI_INTR_UE	(1 << 4)	/* Unrecoverable error */
#define OHCI_INTR_FNO	(1 << 5)	/* Frame number overflow */
#define OHCI_INTR_RHSC	(1 << 6)	/* Root hub status change */
#define OHCI_INTR_OC	(1 << 30)	/* Ownership change */
#define OHCI_INTR_MIE	(1 << 31)	/* Master interrupt enable */

#define HcHCCA 0x18

/* Legacy emulation registers (if enabled in the revision register, bit8) */
#define HceControl 0x100

#define PCI_CLASS_SERIAL_USB_OHCI     0x0c0310

void stop_usb(void);

#endif
