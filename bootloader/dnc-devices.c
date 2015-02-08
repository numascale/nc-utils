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

void pci_search(const struct devspec *list, const sci_t sci, const int bus)
{
	const struct devspec *listp;

	for (int dev = 0; dev < (bus == 0 ? 24 : 32); dev++) {
		for (int fn = 0; fn < 8; fn++) {
			uint32_t val = dnc_read_conf(sci, bus, dev, fn, 0xc);
			/* PCI device functions are not necessarily contiguous */
			if (val == 0xffffffff)
				continue;

			uint8_t type = val >> 16;
			uint32_t ctlcap = dnc_read_conf(sci, bus, dev, fn, 8);

			for (listp = list; listp->classtype != PCI_CLASS_FINAL; listp++)
				if ((listp->classtype == PCI_CLASS_ANY) || ((ctlcap >> ((4 - listp->classlen) * 8)) == listp->classtype))
					if ((listp->type == PCI_TYPE_ANY) || (listp->type == (type & 0x7f)))
						listp->handler(sci, bus, dev, fn);

			/* Recurse down bridges */
			if ((type & 0x7f) == PCI_TYPE_BRIDGE) {
				int sec = (dnc_read_conf(sci, bus, dev, fn, 0x18) >> 8) & 0xff;
				pci_search(list, sci, sec);
			}

			/* If not multi-function, break out of function loop */
			if (!fn && !(type & 0x80))
				break;
		}
	}
}

static void pci_search_start(const struct devspec *list)
{
	pci_search(list, 0xfff0, 0);
}

void disable_kvm_ports(const int port) {
	/* Disable AMI Virtual Keyboard and Mouse ports, since they generate a lot of interrupts */
	uint32_t val = dnc_read_conf(0xfff0, 0, 19, 0, 0x40);
	dnc_write_conf(0xfff0, 0, 19, 0, 0x40, val | (1 << (port + 16)));
}

static uint16_t capability(const uint8_t cap, const sci_t sci, const int bus, const int dev, const int fn)
{
	/* Check for capability list */
	if (!(dnc_read_conf(sci, bus, dev, fn, 0x4) & (1 << 20)))
		return PCI_CAP_NONE;

	uint8_t pos = dnc_read_conf(sci, bus, dev, fn, 0x34) & 0xff;

	for (int lim = 0; lim < 48 && pos >= 0x40; lim++) {
		pos &= ~3;

		uint32_t val = dnc_read_conf(sci, bus, dev, fn, pos + 0);
		if (val == 0xffffffff)
			break;

		if ((val & 0xff) == cap)
			return pos;

		pos = (val >> 8) & 0xfc;
	}

	return PCI_CAP_NONE;
}

uint16_t extcapability(const uint16_t cap, const sci_t sci, const int bus, const int dev, const int fn)
{
	uint16_t cap2 = capability(PCI_CAP_PCIE, sci, bus, dev, fn);

	if (cap2 == PCI_CAP_NONE)
		return PCI_CAP_NONE;

	uint8_t visited[0x1000];
	uint16_t offset = 0x100;

	memset(visited, 0, sizeof(visited));

	do {
		uint32_t val = dnc_read_conf(sci, bus, dev, fn, offset);
		if (val == 0xffffffff || val == 0)
			return PCI_CAP_NONE;

		if (cap == (val & 0xffff))
			return offset;

		offset = (val >> 20) & ~3;
	} while (offset > 0xff && visited[offset]++ == 0);

	return PCI_CAP_NONE;
}

static void disable_common(const sci_t sci, const int bus, const int dev, const int fn)
{
	/* Disable MSI interrupt */
	uint16_t cap = capability(PCI_CAP_MSI, sci, bus, dev, fn);
	if (cap != PCI_CAP_NONE) {
		bool s64 = dnc_read_conf(sci, bus, dev, fn, cap + 0) & (1 << 7);
		for (unsigned offset = 0x0; offset <= (s64 ? 0xc : 0x8); offset += 4)
			dnc_write_conf(sci, bus, dev, fn, cap + offset, 0);
	}
}

void disable_device(const sci_t sci, const int bus, const int dev, const int fn)
{
	/* Disable I/O, DMA and legacy interrupts; enable memory decode */
	dnc_write_conf(sci, bus, dev, fn, 0x4, 0x0402);

	/* Clear BARs */
	for (unsigned i = 0x10; i <= 0x24; i += 4)
		dnc_write_conf(sci, bus, dev, fn, i, 0);

	/* Clear expansion ROM base address */
	dnc_write_conf(sci, bus, dev, fn, 0x30, 0);

	/* The Interrupt Line register cannot be cleared, since the Nvidia driver refuses to initialise */

	/* Disable SR-IOV BARs */
	uint16_t cap = extcapability(PCI_ECAP_SRIOV, sci, bus, dev, fn);
	if (cap != PCI_CAP_NONE)
		for (unsigned offset = 0x24; offset <= 0x38; offset += 4)
			dnc_write_conf(sci, bus, dev, fn, cap + offset, 0);

	disable_common(sci, bus, dev, fn);
}

void disable_bridge(const sci_t sci, const int bus, const int dev, const int fn)
{
	/* Disable I/O, DMA and legacy interrupts; enable memory decode */
	dnc_write_conf(sci, bus, dev, fn, 0x4, 0x0402);

	/* Clear BARs */
	for (unsigned i = 0x10; i <= 0x14; i += 4)
		dnc_write_conf(sci, bus, dev, fn, i, 0);

	/* Disable IO and memory ranges */
	dnc_write_conf(sci, bus, dev, fn, 0x1c, 0x000000f0);
	dnc_write_conf(sci, bus, dev, fn, 0x20, 0x0000fff0);
	dnc_write_conf(sci, bus, dev, fn, 0x20, 0x0000fff0);

	/* Clear upper bits */
	for (unsigned i = 0x28; i <= 0x30; i += 4)
		dnc_write_conf(sci, bus, dev, fn, i, 0);

	/* Clear expansion ROM base address */
	dnc_write_conf(sci, bus, dev, fn, 0x38, 0);

	/* Set Interrupt Line register to 0 (unallocated) */
	dnc_write_conf(sci, bus, dev, fn, 0x3c, 0);

	disable_common(sci, bus, dev, fn);
}

static void completion_timeout(const uint16_t sci, const int bus, const int dev, const int fn)
{
	uint32_t val;
	printf("PCI device @ %02x:%02x.%x: ", bus, dev, fn);

	/* For legacy devices */
	val = dnc_read_conf(sci, bus, dev, fn, 4);
	dnc_write_conf(sci, bus, dev, fn, 4, val & ~(1 << 8));
	printf("disabled SERR");

	uint16_t cap = capability(PCI_CAP_PCIE, sci, bus, dev, fn);
	if (cap != PCI_CAP_NONE) {
		/* Device Control */
		val = dnc_read_conf(sci, bus, dev, fn, cap + 0x8);
		dnc_write_conf(sci, bus, dev, fn, cap + 0x8, val | (1 << 4) | (1 << 8) | (1 << 11));
		val = dnc_read_conf(sci, bus, dev, fn, cap + 0x8);
		if (val & (1 << 4))
			printf("; Relaxed Ordering enabled");
		else
			printf("; failed to enable Relaxed Ordering");

		if (val & (1 << 8))
			printf("; Extended Tag enabled");
		else
			printf("; failed to enable Extended Tag");

		if (val & (1 << 11))
			printf("; No Snoop enabled");
		else
			printf("; failed to enable No Snoop");

		/* Root Control */
		val = dnc_read_conf(sci, bus, dev, fn, cap + 0x1c);
		if (val & (1 << 1)) {
			dnc_write_conf(sci, bus, dev, fn, cap + 0x1c, val | (1 << 4));
			printf("; disabled SERR on Non-Fatal");
		} else
			printf("; Non-Fatal doesn't trigger SERR");

		/* Device Capabilities/Control 2 */
		val = dnc_read_conf(sci, bus, dev, fn, cap + 0x24);

		/* Select Completion Timeout range D if possible */
		if (val & (1 << 3)) {
			uint32_t val2 = dnc_read_conf(sci, bus, dev, fn, cap + 0x28);
			dnc_write_conf(sci, bus, dev, fn, cap + 0x28, (val2 & ~0xf) | 0xe);
			printf("; Completion Timeout 17-64s");
		} else
			printf("; Setting Completion Timeout unsupported");

		/* Disable Completion Timeout if possible */
		if (val & (1 << 4)) {
			val = dnc_read_conf(sci, bus, dev, fn, cap + 0x28);
			dnc_write_conf(sci, bus, dev, fn, cap + 0x28, val | (1 << 4));
			printf("; Completion Timeout disabled");
		} else
			printf("; Disabling Completion Timeout unsupported");
	}

	cap = extcapability(PCI_ECAP_AER, sci, bus, dev, fn);
	if (cap != PCI_CAP_NONE) {
		val = dnc_read_conf(sci, bus, dev, fn, cap + 0x0c);
		if (val & (1 << 14)) {
			dnc_write_conf(sci, bus, dev, fn, cap + 0x0c, val & ~(1 << 14));
			val = dnc_read_conf(sci, bus, dev, fn, cap + 0x0c);
			if (val & (1 << 14))
				printf("; Completion Timeout now non-fatal");
			else
				printf("; failed to set Completion Timeout as non-fatal");
		} else
			printf("; Completion Timeout already non-fatal");

		/* Mask root complex error reporting */
		val = dnc_read_conf(sci, bus, dev, fn, cap + 0x2c);
		dnc_write_conf(sci, bus, dev, fn, cap + 0x2c, val | ~7);
	} else
		printf("; no AER");

	printf("\n");
}

static void adjust_bridge(const uint16_t sci, const int bus, const int dev, const int fn)
{
	uint32_t val = dnc_read_conf(sci, bus, dev, fn, 0x3c);
	val &= ~(1 << 17); /* Disable SERR# Enable */
	val &= ~(1 << 24); /* Set primary Discard Timer to 2^15 cycles */
	val &= ~(1 << 25); /* Set secondary Discard Timer to 2^15 cycles */
	val &= ~(1 << 27); /* Disable Discard Timer SERR# Enable */
	dnc_write_conf(sci, bus, dev, fn, 0x3c, val);
}

static void stop_ohci(const uint16_t sci, const int bus, const int dev, const int fn)
{
	uint32_t val, bar0;
	printf("OHCI controller @ %02x:%02x.%x: ", bus, dev, fn);
	bar0 = dnc_read_conf(sci, bus, dev, fn, 0x10) & ~0xf;
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
		temp = 1000;
		mem64_write32(bar0 + HcInterruptEnable, OHCI_INTR_OC); /* Enable OwnershipChange interrupt */
		mem64_write32(bar0 + HcCommandStatus, OHCI_OCR); /* Request OwnershipChange */

		while (mem64_read32(bar0 + HcControl) & OHCI_CTRL_IR) {
			udelay(1000);

			if (--temp == 0)
				fatal("legacy handover timed out\n");
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

static void stop_ehci(const uint16_t sci, const int bus, const int dev, const int fn)
{
	printf("EHCI controller @ %02x:%02x.%x: ", bus, dev, fn);
	uint32_t bar0 = dnc_read_conf(sci, bus, dev, fn, 0x10) & ~0xf;

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
	uint32_t legsup = dnc_read_conf(sci, bus, dev, fn, ecp);

	if ((legsup & 0x10100ff) != 0x0010001) {
		printf("legacy support not enabled\n");
		return;
	}

	/* Set OS owned semaphore */
	legsup |= 1 << 24;
	dnc_write_conf(sci, bus, dev, fn, ecp, legsup);
	int limit = 1000;

	do {
		udelay(1000);
		legsup = dnc_read_conf(sci, bus, dev, fn, ecp);

		if ((legsup & (1 << 16)) == 0) {
			printf("legacy handover succeeded\n");
			return;
		}
	} while (--limit);

	fatal("legacy handover timed out\n");
}

static void stop_xhci(const uint16_t sci, const int bus, const int dev, const int fn)
{
	printf("XHCI controller @ %02x:%02x.%x: ", bus, dev, fn);
	uint32_t bar0 = dnc_read_conf(sci, bus, dev, fn, 0x10) & ~0xf;

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
	int limit = 1000;

	do {
		udelay(1000);
		legsup = mem64_read32(bar0 + ecp);

		if ((legsup & (1 << 16)) == 0) {
			printf("legacy handover succeeded\n");
			return;
		}
	} while (--limit);

	fatal("legacy handover timed out\n");
}

static void stop_ahci(const uint16_t sci, const int bus, const int dev, const int fn)
{
	printf("AHCI controller @ %02x:%02x.%x: ", bus, dev, fn);
	/* BAR5 (ABAR) contains legacy control registers */
	uint32_t bar5 = dnc_read_conf(sci, bus, dev, fn, 0x24) & ~0xf;

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
	int limit = 1000;

	do {
		udelay(1000);
		legsup = mem64_read32(bar5 + 0x28);

		if ((legsup & 1) == 0) {
			printf("legacy handover succeeded\n");
			return;
		}
	} while (--limit);

	fatal("legacy handover timed out\n");
}

void stop_acpi(void)
{
	printf("ACPI handover: ");
	acpi_sdt_p fadt = find_sdt("FACP");

	if (!fadt) {
		printf("FACP table not found\n");
		return;
	}

	unsigned char *val = &fadt->data[48 - 36];
	const uint32_t smi_cmd = *(uint32_t *)val;
	const uint8_t acpi_enable = fadt->data[52 - 36];

	if (!smi_cmd || !acpi_enable) {
		printf("not needed\n");
		return;
	}

	val = &fadt->data[64 - 36];
	const uint32_t acpipm1cntblk = *(uint32_t *)val;
	uint16_t sci_en = inb(acpipm1cntblk);

	if ((sci_en & 1) == 1) {
		printf("already handed over\n");
		return;
	}

	outb(acpi_enable, smi_cmd);
	int limit = 1000;

	do {
		udelay(1000);
		sci_en = inb(acpipm1cntblk);

		if ((sci_en & 1) == 1) {
			printf("succeeded\n");
			return;
		}
	} while (--limit);

	fatal("ACPI handover timed out\n");
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
		{PCI_CLASS_ANY,             0, PCI_TYPE_ANY, completion_timeout},
		{PCI_CLASS_ANY,             0, PCI_TYPE_BRIDGE, adjust_bridge},
		{PCI_CLASS_FINAL,           0, PCI_TYPE_ANY, NULL}
	};

	printf("Adjusting PCI parameters:\n");
	pci_search_start(devices);
}
