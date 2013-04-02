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
#include <string.h>

#include "dnc-commonlib.h"
#include "dnc-acpi.h"

static struct acpi_rsdp *rptr = NULL;

uint8_t checksum(void *addr, int len)
{
	uint8_t sum = 0;
	int i;

	for (i = 0; i < len; i++)
		sum += *(uint8_t *)(addr + i);

	return sum;
}

static int checksum_ok(void *addr, int len)
{
	return checksum(addr, len) == 0;
}

static void *find_rsdp(void *start, int len)
{
	void *ret = NULL;
	int i;

	for (i = 0; i < len; i += 16) {
		if (*(uint32_t *)(start + i) == STR_DW_H("RSD ") &&
		    *(uint32_t *)(start + i + 4) == STR_DW_H("PTR ")) {
			ret = start + i;
			break;
		}
	}

	return ret;
}

static int rdsp_exists(void)
{
	void *ebda = (void *)(*((unsigned short *)0x40e) * 16);
	rptr = find_rsdp(ebda, 1024);

	if (!rptr)
		rptr = find_rsdp((void *)0x0e0000, 128 * 1024);

	if (!rptr)
		return 0;

	return 1;
}

static acpi_sdt_p find_child(const char *sig, acpi_sdt_p parent,
                             int ptrsize)
{
	uint64_t childp;
	acpi_sdt_p table;
	int i;

	for (i = 0; i + sizeof(*parent) < parent->len; i += ptrsize) {
		childp = 0;
		memcpy(&childp, &parent->data[i], ptrsize);

		if (childp > 0xffffffffULL) {
			printf("Error: Child pointer at %d (%p) outside 32-bit range (0x%llx)",
			       i, &parent->data[i], childp);
			continue;
		}

		memcpy(&table, &childp, sizeof(table));

		if (!checksum_ok(table, table->len)) {
			printf("Error: Bad table %p checksum %.4s\n", table, table->sig.s);
			continue;
		}

		if (table->sig.l == STR_DW_H(sig))
			return table;
	}

	return NULL;
}

/* Return the number of bytes after an ACPI table before the next */
static uint32_t slack(acpi_sdt_p parent)
{
	acpi_sdt_p next_table = (void *)0xffffffff;
	acpi_sdt_p rsdt = (acpi_sdt_p)rptr->rsdt_addr;
	uint32_t *rsdt_entries = (uint32_t *) & (rsdt->data);
	uint64_t xsdtp;
	acpi_sdt_p xsdt;
	uint64_t *xsdt_entries;
	int i;

	for (i = 0; i * 4 + sizeof(*rsdt) < rsdt->len; i++) {
		acpi_sdt_p table = (acpi_sdt_p)rsdt_entries[i];

		if (!checksum_ok(table, table->len)) {
			printf(" Bad table checksum in '%.4s'\n", table->sig.s);
			continue;
		}

		/* Find the nearest table after parent */
		if (table > parent && table < next_table)
			next_table = table;

		/* Check the FACP table for the DSDT table entry as well */
		if (table->sig.l == STR_DW_H("FACP")) {
			acpi_sdt_p dsdt;
			memcpy(&dsdt, &table->data[4], sizeof(dsdt));

			if (!checksum_ok(dsdt, dsdt->len)) {
				printf(" Bad table checksum in '%.4s'\n", dsdt->sig.s);
				continue;
			}

			if (dsdt > parent && dsdt < next_table)
				next_table = dsdt;
		}
	}

	/* Check location of RSDT table also */
	if (rsdt > parent && rsdt < next_table)
		next_table = rsdt;

	xsdtp = rptr->xsdt_addr;

	if ((xsdtp == 0) || (xsdtp == ~0ULL))
		goto out;

	if (xsdtp > 0xffffffffULL) {
		printf("Error: XSDT ptr at (%p) outside 32-bit range (0x%llx)",
		       &rptr->xsdt_addr, xsdtp);
		goto out;
	}

	memcpy(&xsdt, &xsdtp, sizeof(xsdt));
	xsdt_entries = (uint64_t *) & (xsdt->data);

	for (i = 0; i * 8 + sizeof(*xsdt) < xsdt->len; i++) {
		uint64_t childp = xsdt_entries[i];
		acpi_sdt_p table;

		if (childp > 0xffffffffULL) {
			printf("Error: XSDT child pointer at %d (%p) outside 32-bit range (0x%llx)",
			       i, &xsdt_entries[i], childp);
			continue;
		}

		memcpy(&table, &childp, sizeof(table));

		if (!checksum_ok(table, table->len)) {
			printf("Error: Bad table checksum in '%.4s'\n", table->sig.s);
			continue;
		}

		/* Find the nearest table after parent */
		if (table > parent && table < next_table)
			next_table = table;

		/* Check the FACP table for the DSDT table entry as well */
		if (table->sig.l == STR_DW_H("FACP")) {
			acpi_sdt_p dsdt;
			memcpy(&dsdt, &table->data[4], sizeof(dsdt));

			if (!checksum_ok(dsdt, dsdt->len)) {
				printf("Error: Bad table checksum in '%.4s'\n", dsdt->sig.s);
				continue;
			}

			if (dsdt > parent && dsdt < next_table)
				next_table = dsdt;
		}
	}

	/* Check location of XSDT table also */
	if (xsdt > parent && xsdt < next_table)
		next_table = xsdt;

out:
	/* Calculate gap between end of parent and next table */
	return (uint32_t)next_table - (uint32_t)parent - parent->len;
}

int replace_child(const char *sig, acpi_sdt_p new, acpi_sdt_p parent, unsigned int ptrsize)
{
	uint64_t newp, childp;
	acpi_sdt_p table;

	if (!checksum_ok(new, new->len)) {
		printf("Error: Bad new %.4s table checksum\n", sig);
		return 0;
	}

	newp = 0;
	memcpy(&newp, &new, sizeof(new));
	int i;

	for (i = 0; i + sizeof(*parent) < parent->len; i += ptrsize) {
		childp = 0;
		memcpy(&childp, &parent->data[i], ptrsize);

		if (childp > 0xffffffffULL) {
			printf("Error: Child pointer at %d (%p) outside 32-bit range (0x%llx)",
			       i, &parent->data[i], childp);
			continue;
		}

		memcpy(&table, &childp, sizeof(table));

		if (!checksum_ok(table, table->len)) {
			printf("Error: Bad table %p checksum %.4s\n", table, table->sig.s);
			continue;
		}

		if (table->sig.l == STR_DW_H(sig)) {
			memcpy(&parent->data[i], &newp, ptrsize);
			parent->checksum -= checksum(parent, parent->len);
			return 1;
		}
	}

	if (slack(parent) < ptrsize) {
		printf("Error: Not enough space to add %.4s table to %.4s\n", sig, parent->sig.s);
		return 0;
	}

	/* Append entry to end of table */
	memcpy(&parent->data[i], &newp, ptrsize);
	parent->len += ptrsize;
	assert(parent->len < RSDT_MAX);
	parent->checksum -= checksum(parent, parent->len);
	return 1;
}

bool add_child(acpi_sdt_p new, acpi_sdt_p parent, unsigned int ptrsize)
{
	if (slack(parent) < ptrsize) {
		printf("Error: Not enough space to add %.4s table to %.4s\n", new->sig.s, parent->sig.s);
		return 1;
	}

	assert(checksum_ok(new, new->len));
	uint64_t newp = 0;
	memcpy(&newp, &new, sizeof(new));
	int i = parent->len - sizeof(*parent);
	memcpy(&parent->data[i], &newp, ptrsize);
	parent->len += ptrsize;
	assert(parent->len < RSDT_MAX);
	parent->checksum -= checksum(parent, parent->len);

	return 0;
}

acpi_sdt_p find_root(const char *sig)
{
	if (!rdsp_exists())
		return NULL;

	if (!checksum_ok(rptr, 20)) {
		printf("Error: Bad RSDP checksum\n");
		return NULL;
	}

	if (STR_DW_H(sig) == STR_DW_H("RSDT"))
		return (acpi_sdt_p)rptr->rsdt_addr;

	if (STR_DW_H(sig) == STR_DW_H("XSDT")) {
		if ((rptr->len >= 33) && checksum_ok(rptr, rptr->len)) {
			uint64_t xsdtp;
			acpi_sdt_p xsdt;
			xsdtp = rptr->xsdt_addr;

			if ((xsdtp == 0) || (xsdtp == ~0ULL))
				return NULL;

			if (xsdtp > 0xffffffffULL) {
				printf("Error: XSDT pointer at (%p) outside 32-bit range (0x%llx)",
				       &rptr->xsdt_addr, xsdtp);
				return NULL;
			}

			memcpy(&xsdt, &xsdtp, sizeof(xsdt));
			return xsdt;
		}
	}

	return NULL;
}

int replace_root(const char *sig, acpi_sdt_p new)
{
	if (!rdsp_exists())
		return 0;

	if (!checksum_ok(rptr, 20)) {
		printf("Error: Bad RSDP checksum\n");
		return 0;
	}

	if (STR_DW_H(sig) == STR_DW_H("RSDT")) {
		rptr->rsdt_addr = (uint32_t)new;
		rptr->checksum -= checksum(rptr, 20);

		if (rptr->len > 20)
			rptr->echecksum -= checksum(rptr, rptr->len);

		return 1;
	}

	if (STR_DW_H(sig) == STR_DW_H("XSDT")) {
		if ((rptr->len >= 33) && checksum_ok(rptr, rptr->len)) {
			rptr->xsdt_addr = (uint32_t)new;
			rptr->echecksum -= checksum(rptr, rptr->len);
			return 1;
		}
	}

	return 0;
}

acpi_sdt_p find_sdt(char *sig)
{
	acpi_sdt_p root;
	acpi_sdt_p res = NULL;
	root = find_root("XSDT");

	if (root)
		res = find_child(sig, root, 8);

	if (!res) {
		root = find_root("RSDT");

		if (root)
			res = find_child(sig, root, 4);
	}

	return res;
}

#ifdef UNUSED
void debug_acpi_srat(acpi_sdt_p srat)
{
	int i = 12;

	while (i + sizeof(*srat) < srat->len) {
		if (srat->data[i] == 0) {
			struct acpi_core_affinity *af =
			    (struct acpi_core_affinity *) & (srat->data[i]);
			printf(" SRAT core aff: pxlo:%d apic:%d enable:%d"
			       " sapic:%d pxhi:%d\n",
			       af->prox_low,
			       af->apic_id,
			       af->enabled,
			       af->sapic_eid,
			       af->prox_hi);
			i += af->len;
		} else if (srat->data[i] == 1) {
			struct acpi_mem_affinity *af =
			    (struct acpi_mem_affinity *) & (srat->data[i]);
			printf(" SRAT mem aff:  prdom:%d base:%llx size:%llx"
			       " enable:%d hotplug:%d nonvol:%d\n",
			       af->prox_dom,
			       af->mem_base,
			       af->mem_size,
			       af->enabled,
			       af->hotplug,
			       af->nonvol);
			i += af->len;
		} else {
			printf("SRAT: Unknown affinity type: %d\n", srat->data[i]);
			return;
		}
	}
}

static void debug_acpi_apic(acpi_sdt_p apic)
{
	unsigned int i;

	for (i = 44; i < apic->len;) {
		struct acpi_local_apic *lapic = (void *)&apic->data[i - sizeof(*apic)];
		printf("APIC object: type:%d, len:%d", lapic->type, lapic->len);

		if (lapic->type == 0)
			printf(", proc_id:%d, apicid:0x%x, flags:0x%x\n",
			       lapic->proc_id, lapic->apic_id, lapic->flags);
		else
			printf("\n");

		if (lapic->len == 0) {
			printf("Error: APIC entry at %p (offset %d) reports len 0\n",
			       lapic, i);
			break;
		}

		i += lapic->len;
	}
}
#endif

static void acpi_dump(acpi_sdt_p table)
{
	int i;
	unsigned char *data = (unsigned char *)table;
	printf("Dumping %.4s:\n", table->sig.s);

	while (data < ((unsigned char *)table + table->len)) {
		for (i = 0; i < 8; i++) {
			uint32_t val = *(uint32_t *)data;
			printf(" 0x%08x,", val);
			data += sizeof(val);
		}

		printf("\n");
	}

	printf("\n");
}

bool acpi_append(acpi_sdt_p parent, int ptrsize, const char *sig, const unsigned char *extra, uint32_t extra_len)
{
	/* Check if enough space to append to SSDT */
	acpi_sdt_p table = find_child(sig, parent, ptrsize);

	if (!table || slack(table) < extra_len)
		return false;

	memcpy((unsigned char *)table + table->len, extra, extra_len);
	table->len += extra_len;
	table->checksum -= checksum(table, table->len);

	if (verbose > 1)
		acpi_dump(table);

	return true;
}

void debug_acpi(void)
{
	if (!rdsp_exists())
		return;

	printf("ACPI settings:\n");

	if (!checksum_ok(rptr, 20)) {
		printf("Error: Bad RSDP checksum\n");
		return;
	}

	printf(" ptr:   %p, RSDP, %.6s, %d, %08x, %d\n",
	       rptr,
	       rptr->oemid,
	       rptr->revision,
	       rptr->rsdt_addr,
	       rptr->len);
	acpi_sdt_p rsdt = (acpi_sdt_p)rptr->rsdt_addr;

	if (!checksum_ok(rsdt, rsdt->len)) {
		printf("Error: Bad RSDT checksum\n");
		return;
	}

	printf(" table: %p, %08x, %.4s, %.6s, %.8s, %d, %d, %d, %d\n",
	       rsdt,
	       rsdt->sig.l,
	       rsdt->sig.s,
	       rsdt->oemid,
	       rsdt->oemtableid,
	       rsdt->checksum,
	       rsdt->revision,
	       rsdt->len,
	       sizeof(*rsdt));
	uint32_t *rsdt_entries = (uint32_t *) & (rsdt->data);
	int i;

	for (i = 0; i * 4 + sizeof(*rsdt) < rsdt->len; i++) {
		acpi_sdt_p table = (acpi_sdt_p)rsdt_entries[i];

		if (!checksum_ok(table, table->len)) {
			printf("Error: Bad table checksum in '%.4s'", table->sig.s);
			continue;
		}

		printf(" table: %p, %08x, %.4s, %.6s, %.8s, %d, %d, %d\n",
		       table,
		       table->sig.l,
		       table->sig.s,
		       table->oemid,
		       table->oemtableid,
		       table->checksum,
		       table->revision,
		       table->len);

		/* Find the DSDT table also */
		if (table->sig.l == STR_DW_H("FACP")) {
			acpi_sdt_p dsdt;
			memcpy(&dsdt, &table->data[4], sizeof(dsdt));

			if (!checksum_ok(dsdt, dsdt->len)) {
				printf("Error: Bad table checksum in '%.4s'\n", dsdt->sig.s);
				continue;
			}

			printf(" table: %p, %08x, %.4s, %.6s, %.8s, %d, %d, %d\n",
			       dsdt,
			       dsdt->sig.l,
			       dsdt->sig.s,
			       dsdt->oemid,
			       dsdt->oemtableid,
			       dsdt->checksum,
			       dsdt->revision,
			       dsdt->len);
		}

#ifdef UNUSED

		if (table->sig.l == STR_DW_H("SRAT")) {
			debug_acpi_srat(table);
		} else if (table->sig.l == STR_DW_H("APIC")) {
			debug_acpi_apic(table);
		}

#endif
	}

	if ((rptr->len >= 33) && checksum_ok(rptr, rptr->len)) {
		if ((rptr->xsdt_addr != 0ULL) && (rptr->xsdt_addr != ~0ULL)) {
			acpi_sdt_p xsdt;
			memcpy(&xsdt, &rptr->xsdt_addr, sizeof(xsdt));

			if (!checksum_ok(xsdt, xsdt->len)) {
				printf("Error: Bad XSDT checksum\n");
				return;
			}

			printf(" table: %p, %08x, %.4s, %.6s, %.8s, %d, %d, %d, %d\n",
			       xsdt,
			       xsdt->sig.l,
			       xsdt->sig.s,
			       xsdt->oemid,
			       xsdt->oemtableid,
			       xsdt->checksum,
			       xsdt->revision,
			       xsdt->len,
			       sizeof(*xsdt));
			uint64_t *xsdt_entries = (void *) & (xsdt->data);
			int i;

			for (i = 0; i * 8 + sizeof(*xsdt) < xsdt->len; i++) {
				acpi_sdt_p table;
				memcpy(&table, &xsdt_entries[i], sizeof(table));

				if (!checksum_ok(table, table->len)) {
					printf("Error: Bad table checksum in '%.4s'\n", table->sig.s);
					continue;
				}

				printf(" table: %p, %08x, %.4s, %.6s, %.8s, %d, %d, %d\n",
				       table,
				       table->sig.l,
				       table->sig.s,
				       table->oemid,
				       table->oemtableid,
				       table->checksum,
				       table->revision,
				       table->len);

				/* Find the DSDT table also */
				if (table->sig.l == STR_DW_H("FACP")) {
					acpi_sdt_p dsdt;
					memcpy(&dsdt, &table->data[4], sizeof(dsdt));

					if (!checksum_ok(dsdt, dsdt->len)) {
						printf(" Bad table checksum in '%.4s'\n", dsdt->sig.s);
						continue;
					}

					printf(" table: %p, %08x, %.4s, %.6s, %.8s, %d, %d, %d\n",
					       dsdt,
					       dsdt->sig.l,
					       dsdt->sig.s,
					       dsdt->oemid,
					       dsdt->oemtableid,
					       dsdt->checksum,
					       dsdt->revision,
					       dsdt->len);
				}

#ifdef UNUSED

				if (table->sig.l == STR_DW_H("SRAT")) {
					debug_acpi_srat(table);
				} else if (table->sig.l == STR_DW_H("APIC")) {
					debug_acpi_apic(table);
				}
#endif
			}
		}
	}
}

