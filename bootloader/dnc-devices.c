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

static void pci_search(const struct devspec *list)
{
    int bus, dev, fn;
    uint32_t val;
    const struct devspec *listp;

    for (bus = 0; bus < 256; bus++)
	for (dev = 0; dev < (bus == 0 ? 24 : 32); dev++)
	    for (fn = 0; fn < 8; fn++) {
		val = dnc_read_conf(0xfff0, bus, dev, fn, 8);
		/* PCI device functions are not necessarily contiguous */
		if (val == 0xffffffff)
		    continue;

		for (listp = list; listp->class; listp++)
		    if ((val >> ((4 - listp->classlen) * 8)) == listp->class)
			listp->handler(bus, dev, fn);
	    }
}

static void disable_device(int bus, int dev, int fn)
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

    printf("disabled\n");
}

void disable_dma_all(void)
{
    int bus, dev, fn;

    for (bus = 0; bus < 256; bus++)
	for (dev = 0; dev < (bus == 0 ? 24 : 32); dev++)
	    for (fn = 0; fn < 8; fn++) {
		uint32_t type = dnc_read_conf(0xfff0, bus, dev, fn, 0xc);
		/* PCI device functions are not necessarily contiguous */
		if (type == 0xffffffff)
		    continue;

		switch ((type >> 16) & 0x7f) {
		case 0:
		    printf("device at %02x:%02x.%x: ", bus, dev, fn);
		    disable_device(bus, dev, fn);
		    break;
		case 1:
		    printf("bridge at %02x:%02x.%x\n", bus, dev, fn);
		    break;
		}

		/* If not multi-function, break out of function loop */
		if (!fn && !(type & 0x800000))
		    break;
	    }
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

    val = mem64_read32(bar0 + HcHCCA);
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
		printf("legacy handoff timed out\n");
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
	printf("legacy handoff succeeded\n");
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
	printf("legacy support not enabled (status 0x%08x)\n", legsup);
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
	    printf("legacy handoff succeeded\n");
	    return;
	}
    } while (--limit);

    printf("legacy handoff timed out\n");
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
	printf("legacy support not enabled (status 0x%08x)\n", legsup);
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
	    printf("legacy handoff succeeded\n");
	    return;
	}
    } while (--limit);

    printf("legacy handoff timed out\n");
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
	printf("legacy support not enabled (status 0x%08x)\n", legsup);
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
	    printf("legacy handoff succeeded\n");
	    return;
	}
    } while (--limit);

    printf("legacy handoff timed out\n");
}

void stop_acpi(void)
{
    printf("ACPI handoff: ");

    acpi_sdt_p fadt = find_sdt("FACP");
    if (!fadt) {
	printf("ACPI FACP table not found\n");
	return;
    }

    uint32_t smi_cmd = *(uint32_t *)&fadt->data[48 - 36];
    uint8_t acpi_enable = fadt->data[52 - 36];
    if (!smi_cmd || !acpi_enable) {
	printf("legacy support not enabled\n");
	return;
    }

    uint32_t acpipm1cntblk = *(uint32_t *)&fadt->data[64 - 36];
    uint16_t sci_en = inb(acpipm1cntblk);

    outb(acpi_enable, smi_cmd);

    int limit = 100;

    do {
	udelay(100);
	sci_en = inb(acpipm1cntblk);
	if ((sci_en & 1) == 1) {
	    printf("legacy handoff succeeded\n");
	    return;
	}
    } while (--limit);

    printf("ACPI handoff timed out\n");
}

void handover_legacy(void)
{
    /* If SMM is disabled, USB handover functionality may have been lost */
    if (disable_smm)
	return;

    /* Stop ACPI first, as Linux requests ownership of this before other subsystems */
    stop_acpi();

    const struct devspec devices[] = {
	{PCI_CLASS_SERIAL_USB_OHCI, 3, stop_ohci},
	{PCI_CLASS_SERIAL_USB_EHCI, 3, stop_ehci},
	{PCI_CLASS_SERIAL_USB_XHCI, 3, stop_xhci},
	{PCI_CLASS_STORAGE_SATA, 2, stop_ahci},
	{PCI_CLASS_STORAGE_RAID, 2, stop_ahci},
	{0, 0, 0}
    };

    pci_search(devices);
}

