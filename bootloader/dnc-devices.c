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

#include <stdio.h>

#include "dnc-devices.h"
#include "dnc-access.h"
#include "dnc-acpi.h"
#include "dnc-bootloader.h"
#include "dnc-commonlib.h"

static void pci_search(const struct devspec *list, const int bus)
{
	const struct devspec *listp;

	for (int dev = 0; dev < (bus == 0 ? 24 : 32); dev++) {
		for (int fn = 0; fn < 8; fn++) {
			uint32_t val = dnc_read_conf(0xfff0, bus, dev, fn, 0xc);
			/* PCI device functions are not necessarily contiguous */
			if (val == 0xffffffff)
				continue;

			uint8_t type = val >> 16;
			uint32_t ctlcap = dnc_read_conf(0xfff0, bus, dev, fn, 8);

			for (listp = list; listp->classtype != PCI_CLASS_FINAL; listp++)
				if ((listp->classtype == PCI_CLASS_ANY) || ((ctlcap >> ((4 - listp->classlen) * 8)) == listp->classtype))
					if ((listp->type == PCI_TYPE_ANY) || (listp->type == (type & 0x7f)))
						listp->handler(bus, dev, fn);

			/* Recurse down bridges */
			if ((type & 0x7f) == 0x01) {
				int sec = (dnc_read_conf(0xfff0, bus, dev, fn, 0x18) >> 8) & 0xff;
				pci_search(list, sec);
			}

			/* If not multi-function, break out of function loop */
			if (!fn && !(type & 0x80))
				break;
		}
	}
}

static void pci_search_start(const struct devspec *list)
{
	pci_search(list, 0);
}

static void disable_device(const int bus, const int dev, const int fn)
{
	int i;
	/* Disable I/O, memory, DMA and interrupts */
	dnc_write_conf(0xfff0, bus, dev, fn, 0x4, 0);

	/* Clear BARs */
	for (i = 0x10; i <= 0x24; i += 4)
		dnc_write_conf(0xfff0, bus, dev, fn, i, 0);

	/* Clear expansion ROM base address */
	dnc_write_conf(0xfff0, bus, dev, fn, 0x30, 0);
	/* Set Interrupt Line register to 0 (unallocated) */
	dnc_write_conf(0xfff0, bus, dev, fn, 0x3c, 0);

	if (verbose > 1)
		printf("disabled\n");
}

void disable_dma_all(void)
{
	const struct devspec devices[] = {
		{PCI_CLASS_ANY, 0, PCI_TYPE_ENDPOINT, disable_device},
		{PCI_CLASS_FINAL, 0, PCI_TYPE_ANY, NULL}
	};
	pci_search_start(devices);
}

uint16_t capability(const uint8_t cap, const int bus, const int dev, const int fn)
{
	/* Check for capability list */
	if (!(dnc_read_conf(0xfff0, bus, dev, fn, 0x4) & (1 << 20)))
		return PCI_CAP_NONE;

	uint8_t pos = dnc_read_conf(0xfff0, bus, dev, fn, 0x34) & 0xff;

	for (int lim = 0; lim < 48 && pos >= 0x40; lim++) {
		pos &= ~3;

		uint32_t val = dnc_read_conf(0xfff0, bus, dev, fn, pos + 0);
		if (val == 0xffffffff)
			break;

		if ((val & 0xff) == cap)
			return pos;

		pos = (val >> 8) & 0xff;
	}

	return PCI_CAP_NONE;
}

uint16_t extcapability(const uint8_t cap, const int bus, const int dev, const int fn)
{
	uint16_t offset = 0x100;
	uint32_t val;

	do {
		val = dnc_read_conf(0xfff0, bus, dev, fn, offset);
		if (val == 0xffffffff)
			return PCI_CAP_NONE;
		if (cap == (val & 0xffff))
			return offset;

		offset >>= 20;
	} while (offset);

	return PCI_CAP_NONE;
}

static void completion_timeout(const int bus, const int dev, const int fn)
{
	uint32_t val;
	uint16_t cap;
	printf("PCI device @ %02x:%02x.%x: ", bus, dev, fn);

	cap = capability(PCI_CAP_PCIE, bus, dev, fn);
	if (cap != PCI_CAP_NONE) {
		/* Device Control */
		val = dnc_read_conf(0xfff0, bus, dev, fn, cap + 0x8);
		dnc_write_conf(0xfff0, bus, dev, fn, cap + 0x8, val | (1 << 4));
		val = dnc_read_conf(0xfff0, bus, dev, fn, cap + 0x8);
		if (val & (1 << 4))
			printf("Relaxed Ordering enabled");
		else
			printf("failed to enable Relaxed Ordering");

		/* Root Control */
		val = dnc_read_conf(0xfff0, bus, dev, fn, cap + 0x1c);
		if (val & (1 << 1)) {
			dnc_write_conf(0xfff0, bus, dev, fn, cap + 0x1c, val | (1 << 4));
			printf("; disabled SERR on Non-Fatal");
		} else
			printf("; Non-Fatal doesn't trigger SERR");

		/* Device Capabilities/Control 2 */
		val = dnc_read_conf(0xfff0, bus, dev, fn, cap + 0x24);

		/* Select Completion Timeout range D, else disable */
		if (val & (1 << 3)) {
			val = dnc_read_conf(0xfff0, bus, dev, fn, cap + 0x28);
			dnc_write_conf(0xfff0, bus, dev, fn, cap + 0x28, (val & ~0xf) | 0xe);
			printf("; Completion Timeout 17-64s");
		} else {
			if (val & (1 << 4)) {
				/* Disable Completion Timeout instead */
				val = dnc_read_conf(0xfff0, bus, dev, fn, cap + 0x28);
				dnc_write_conf(0xfff0, bus, dev, fn, cap + 0x28, val | (1 << 4));
				printf("; Completion Timeout disabled");
			} else
				printf("; Setting Completion Timeout unsupported");
		}
	} else
		printf("no PCIe");

	cap = extcapability(PCI_CAP_AER, bus, dev, fn);
	if (cap != PCI_CAP_NONE) {
		val = dnc_read_conf(0xfff0, bus, dev, fn, cap + 0x0c);
		if (val & (1 << 14)) {
			dnc_write_conf(0xfff0, bus, dev, fn, cap + 0x0c, val & ~(1 << 14));
			val = dnc_read_conf(0xfff0, bus, dev, fn, cap + 0x0c);
			if (val & (1 << 14))
				printf("; Completion Timeout now non-fatal");
			else
				printf("; failed to set Completion Timeout as non-fatal");
		} else
			printf("; Completion Timeout already non-fatal");
	} else
		printf("; no AER");

	printf("\n");
}

static void stop_ohci(int bus, int dev, int fn)
{
	uint32_t val, bar0;
	printf("OHCI controller @ %02x:%02x.%x: ", bus, dev, fn);
	bar0 = dnc_read_conf(0xfff0, bus, dev, fn, 0x10) & ~0xf;
	if ((bar0 == 0xffffffff) || (bar0 == 0)) {
		printf("BAR not configured\n");
		return;
	}

	val = mem64_read32(bar0 + HcControl);
	if (val & OHCI_CTRL_IR) { /* Interrupt routing enabled, we must request change of ownership */
		uint32_t temp;
		/* This timeout is arbitrary.  we make it long, so systems
		 * depending on usb keyboards may be usable even if the
		 * BIOS/SMM code seems pretty broken
		 */
		temp = 100;	/* Arbitrary: five seconds */
		mem64_write32(bar0 + HcInterruptEnable, OHCI_INTR_OC); /* Enable OwnershipChange interrupt */
		mem64_write32(bar0 + HcCommandStatus, OHCI_OCR); /* Request OwnershipChange */

		while (mem64_read32(bar0 + HcControl) & OHCI_CTRL_IR) {
			udelay(100);

			if (--temp == 0) {
				printf("legacy handover timed out\n");
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
		printf("legacy handover succeeded\n");
	} else {
		printf("legacy support not enabled\n");
	}

	val = mem64_read32(bar0 + HcRevision);

	if (val & (1 << 8)) { /* Legacy emulation is supported */
		val = mem64_read32(bar0 + HceControl);

		if (val & (1 << 0)) {
			printf("legacy support enabled\n");
		}
	}
}

static void stop_ehci(int bus, int dev, int fn)
{
	printf("EHCI controller @ %02x:%02x.%x: ", bus, dev, fn);
	uint32_t bar0 = dnc_read_conf(0xfff0, bus, dev, fn, 0x10) & ~0xf;

	if (bar0 == 0) {
		printf("BAR not configured\n");
		return;
	}

	/* Get EHCI Extended Capabilities Pointer */
	uint32_t ecp = (mem64_read32(bar0 + 0x8) >> 8) & 0xff;

	if (ecp == 0) {
		printf("extended capabilities absent\n");
		return;
	}

	/* Check legacy support register shows BIOS ownership */
	uint32_t legsup = dnc_read_conf(0xfff0, bus, dev, fn, ecp);

	if ((legsup & 0x10100ff) != 0x0010001) {
		printf("legacy support not enabled\n");
		return;
	}

	/* Set OS owned semaphore */
	legsup |= 1 << 24;
	dnc_write_conf(0xfff0, bus, dev, fn, ecp, legsup);
	int limit = 100;

	do {
		udelay(100);
		legsup = dnc_read_conf(0xfff0, bus, dev, fn, ecp);

		if ((legsup & (1 << 16)) == 0) {
			printf("legacy handover succeeded\n");
			return;
		}
	} while (--limit);

	printf("legacy handover timed out\n");
}

static void stop_xhci(int bus, int dev, int fn)
{
	printf("XHCI controller @ %02x:%02x.%x: ", bus, dev, fn);
	uint32_t bar0 = dnc_read_conf(0xfff0, bus, dev, fn, 0x10) & ~0xf;

	if (bar0 == 0) {
		printf("BAR not configured\n");
		return;
	}

	/* Get XHCI Extended Capabilities Pointer */
	uint32_t ecp = (mem64_read32(bar0 + 0x10) & 0xffff0000) >> (16 - 2);

	if (ecp == 0) {
		printf("extended capabilities absent\n");
		return;
	}

	/* Check legacy support register shows BIOS ownership */
	uint32_t legsup = mem64_read32(bar0 + ecp);

	if ((legsup & 0x10100ff) != 0x0010001) {
		printf("legacy support not enabled\n");
		return;
	}

	/* Set OS owned semaphore */
	legsup |= 1 << 24;
	mem64_write32(bar0 + ecp, legsup);
	int limit = 100;

	do {
		udelay(100);
		legsup = mem64_read32(bar0 + ecp);

		if ((legsup & (1 << 16)) == 0) {
			printf("legacy handover succeeded\n");
			return;
		}
	} while (--limit);

	printf("legacy handover timed out\n");
}

static void stop_ahci(int bus, int dev, int fn)
{
	printf("AHCI controller @ %02x:%02x.%x: ", bus, dev, fn);
	/* BAR5 (ABAR) contains legacy control registers */
	uint32_t bar5 = dnc_read_conf(0xfff0, bus, dev, fn, 0x24) & ~0xf;

	if (bar5 == 0) {
		printf("BAR not configured\n");
		return;
	}

	/* Check legacy support register shows BIOS ownership */
	uint32_t legsup = mem64_read32(bar5 + 0x24);

	if ((legsup & 1) == 0) {
		printf("legacy support not implemented\n");
		return;
	}

	legsup = mem64_read32(bar5 + 0x28);

	if ((legsup & 1) != 1) {
		printf("legacy support not enabled\n");
		return;
	}

	/* Set OS owned semaphore */
	legsup |= (1 << 1);
	mem64_write32(bar5 + 0x28, legsup);
	int limit = 100;

	do {
		udelay(100);
		legsup = mem64_read32(bar5 + 0x28);

		if ((legsup & 1) == 0) {
			printf("legacy handover succeeded\n");
			return;
		}
	} while (--limit);

	printf("legacy handover timed out\n");
}

void stop_acpi(void)
{
	printf("ACPI handover: ");
	acpi_sdt_p fadt = find_sdt("FACP");

	if (!fadt) {
		printf("ACPI FACP table not found\n");
		return;
	}

	unsigned char *val = &fadt->data[48 - 36];
	const uint32_t smi_cmd = *(uint32_t *)val;
	const uint8_t acpi_enable = fadt->data[52 - 36];

	if (!smi_cmd || !acpi_enable) {
		printf("legacy support not enabled\n");
		return;
	}

	val = &fadt->data[64 - 36];
	const uint32_t acpipm1cntblk = *(uint32_t *)val;
	uint16_t sci_en = inb(acpipm1cntblk);
	outb(acpi_enable, smi_cmd);
	int limit = 100;

	do {
		udelay(100);
		sci_en = inb(acpipm1cntblk);

		if ((sci_en & 1) == 1) {
			printf("legacy handover succeeded\n");
			return;
		}
	} while (--limit);

	printf("ACPI handover timed out\n");
}

void handover_legacy(void)
{
	/* Stop ACPI first, as Linux requests ownership of this before other subsystems */
	stop_acpi();
	const struct devspec devices[] = {
		{PCI_CLASS_SERIAL_USB_OHCI, 3, PCI_TYPE_ENDPOINT, stop_ohci},
		{PCI_CLASS_SERIAL_USB_EHCI, 3, PCI_TYPE_ENDPOINT, stop_ehci},
		{PCI_CLASS_SERIAL_USB_XHCI, 3, PCI_TYPE_ENDPOINT, stop_xhci},
		{PCI_CLASS_STORAGE_SATA,    2, PCI_TYPE_ENDPOINT, stop_ahci},
		{PCI_CLASS_STORAGE_RAID,    2, PCI_TYPE_ENDPOINT, stop_ahci},
		{PCI_CLASS_FINAL, 0, PCI_TYPE_ANY, NULL}
	};
	pci_search_start(devices);
}

void pci_setup(void)
{
	const struct devspec devices[] = {
		{PCI_CLASS_ANY, 0, PCI_TYPE_ANY, completion_timeout},
		{PCI_CLASS_FINAL, 0, PCI_TYPE_ANY, NULL}
	};
	pci_search_start(devices);
}

