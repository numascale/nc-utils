/*
 * Copyright (C) 2008-2012 Numascale AS, support@numascale.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __DNC_DEVICES
#define __DNC_DEVICES 1

#include "dnc-types.h"

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

#define PCI_CAP_NONE					0x00
#define PCI_CAP_MSI						0x05
#define PCI_CAP_PCIE					0x10
#define PCI_ECAP_AER					0x0001
#define PCI_ECAP_SRIOV					0x0010

#define PCI_CLASS_ANY					0xfffffffe
#define PCI_CLASS_FINAL					0xffffffff

#define PCI_CLASS_DISPLAY_VGA           0x030000
#define PCI_CLASS_DISPLAY_CONTROLLER    0x038000

#define PCI_CLASS_SERIAL_USB_UHCI       0x0c0300
#define PCI_CLASS_SERIAL_USB_OHCI       0x0c0310
#define PCI_CLASS_SERIAL_USB_EHCI       0x0c0320
#define PCI_CLASS_SERIAL_USB_XHCI       0x0c0330

#define PCI_CLASS_STORAGE_RAID          0x0104
#define PCI_CLASS_STORAGE_SATA          0x0106

#define PCI_TYPE_ENDPOINT               0x0
#define PCI_TYPE_BRIDGE                 0x1
#define PCI_TYPE_ANY                    0xff

struct devspec {
	const uint32_t classtype;
	const uint8_t classlen;
	const uint8_t type;
	void (*handler)(const uint16_t, const int, const int, const int);
};

void stop_acpi(void);
void disable_kvm_ports(const int port);
void disable_dma_all(void);
void handover_legacy(void);
void pci_setup(void);
void disable_device(const uint16_t sci, const int bus, const int dev, const int fn);
checked uint16_t extcapability(const uint16_t cap, const sci_t sci, const int bus, const int dev, const int fn);
#endif
