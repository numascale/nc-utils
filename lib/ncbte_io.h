/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * User API for Numachip Block Transfer Engine driver
 *
 * Copyright (C) 2011 Numascale AS. All rights reserved.
 *
 * Send feedback to <support@numascale.com>
 *
 */

#ifndef __NCBTE_IO_H__
#define __NCBTE_IO_H__

#include <linux/types.h>
#include <linux/ioctl.h>

#define NCBTE_DEVNAME		"ncbte"
#define NCBTE_IOC_CODE		0xca
#define NCBTE_MAX_MMAP_SIZE	0x100000

/**
 * struct ncbte_region - Memory pinning/unpinning information
 * @addr:          virtual user space address
 * @size:          size of the area pin/dma-map/unmap
 * @page_cnt:      number of page entries
 * @phys_addr:     array holding physical addresses for pages
 *
 */
struct ncbte_mem {
	__u64 addr;
	__u64 size;
	__u64* phys_addr;
	__u32 nr_pages;
};

#define NCBTE_PIN_MEM		_IOWR(NCBTE_IOC_CODE, 100, struct ncbte_mem)
#define NCBTE_UNPIN_MEM		_IOWR(NCBTE_IOC_CODE, 101, struct ncbte_mem)

#endif /* __NCBTE_IO_H__ */
