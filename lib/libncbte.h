/* -*- Mode: C; c-basic-offset:8 ; indent-tabs-mode:t ; -*- */
/*
 * Copyright (C) 2008-2015 Numascale AS, support@numascale.com
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

#ifndef __LIBNCBTE_H
#define __LIBNCBTE_H

#ifdef __cplusplus
#  define BEGIN_C_DECLS extern "C" {
#  define END_C_DECLS   }
#else /* !__cplusplus */
#  define BEGIN_C_DECLS
#  define END_C_DECLS
#endif /* __cplusplus */

BEGIN_C_DECLS

#define NCBTE_ALLOCATE_HUGEPAGE 0x1

struct ncbte_context;
struct ncbte_region;
struct ncbte_completion;

/**
 * ncbte_alloc_region() - Allocate and pin a user region of variable size
 * @context:	Current BTE context
 * @ptr:	Pointer to user virtual address if buffer is pre-allocated, else NULL
 * @length:	Size of buffer
 * @flags:	Flags, use NCBTE_ALLOCATE_HUGEPAGE if huge pages are desired
 * @regionp:	Pointer to region structure, allocated and initialized by function
 *
 * This function will allocate and pin a memory buffer of desired size if @vaddrp is not already
 * initialized with a value. If @vaddrp already points to a valid (and committed) buffer
 * address, it will only try to pin the memory.
 *
 * Return: Virtual address of the buffer allocated (or as given by the @ptr argument) if
 * successful, or NULL if error occured.
 */
void *ncbte_alloc_region(struct ncbte_context *context, void *ptr, size_t length, int flags, struct ncbte_region **regionp);

/**
 * ncbte_free_region() - Unpin and free a user region
 * @context:	Current BTE context
 * @region:	Pointer to region structure as returned by ncbte_alloc_region()
 *
 * This function will unpin and free a user buffer if it was allocated
 * by a previous call to ncbte_alloc_region(). If the user buffer was pre-allocated
 * it is the callers responsibility to free that memory, it will still be unpinned
 * by this function however and the resources tied to the region structure freed.
 *
 * Return: 0 if successful, or -1 if error occured.
 */
int ncbte_free_region(struct ncbte_context *context, struct ncbte_region *region);

/**
 * ncbte_write_region() - Read from local region and write to remote region
 * @context:		Current BTE context
 * @local_region:	Pointer to local region structure as returned by ncbte_alloc_region()
 * @local_offset:	Offset into the local region where transfer should start
 * @remote_region:	Pointer to remote region structure as returned by ncbte_alloc_region()
 * @remote_offset:	Offset into the remote region where transfer should start
 * @length:		Length of the transfer
 * @completion:		Optional pointer to completion structure as returned by ncbte_alloc_completion()
 *
 * This function will start a BTE transfer that reads from the local_region and writes to
 * the remote_region. If a @completion pointer is provided, this object will be signalled
 * upon succesful transfer.
 *
 * Return: 0 if successful, or -1 if error occured.
 */
int ncbte_write_region(struct ncbte_context *context,
		       struct ncbte_region *local_region, off_t local_offset,
		       struct ncbte_region *remote_region, off_t remote_offset,
		       size_t length,
		       struct ncbte_completion *completion);

/**
 * ncbte_read_region() - Read from remote region and write to local region
 * @context:		Current BTE context
 * @local_region:	Pointer to local region structure as returned by ncbte_alloc_region()
 * @local_offset:	Offset into the local region where transfer should start
 * @remote_region:	Pointer to remote region structure as returned by ncbte_alloc_region()
 * @remote_offset:	Offset into the remote region where transfer should start
 * @length:		Length of the transfer
 * @completion:		Optional pointer to completion structure as returned by ncbte_alloc_completion()
 *
 * This function will start a BTE transfer that reads from the remote_region and writes to
 * the local_region. If a @completion pointer is provided, this object will be signalled
 * upon succesful transfer.
 *
 * Return: 0 if successful, or -1 if error occured.
 */
int ncbte_read_region(struct ncbte_context *context,
		      struct ncbte_region *local_region, off_t local_offset,
		      struct ncbte_region *remote_region, off_t remote_offset,
		      size_t length,
		      struct ncbte_completion *completion);

/**
 * ncbte_alloc_completion() - Allocate a completion object
 * @context:	Current BTE context
 * @flags:	Length of the transfer
 *
 * This function will allocate a completion object that can be used in read/write
 * transfers and later in ncbte_check_completion() and ncbte_wait_completion().
 *
 * Return: Pointer to the completion object if successful, or NULL if error occured.
 */
struct ncbte_completion *ncbte_alloc_completion(struct ncbte_context *context, int flags);

/**
 * ncbte_free_completion() - Free a completion object
 * @context:	Current BTE context
 * @completion:	Pointer to completion structure as returned by ncbte_alloc_completion()
 *
 * This function will free a completion structure allocated by a previous call to
 * ncbte_alloc_region(). If the completion object has pending transactions, this call
 * will block until they complete or are interrupted.
 *
 * Return: 0 if successful, or -1 if error occured.
 */
int ncbte_free_completion(struct ncbte_context *context, struct ncbte_completion *completion);

/**
 * ncbte_check_completion() - Check a completion object for completion event
 * @context:	Current BTE context
 * @completion:	Pointer to completion structure as returned by ncbte_alloc_completion()
 *
 * This function will check completion structure for pending events.
 *
 * Return: 1 if no pending events, or 0 if there still are pending events.
 */
int ncbte_check_completion(struct ncbte_context *context, struct ncbte_completion *completion);

/**
 * ncbte_wait_completion() - Wait for completion evetns on a completion object
 * @context:	Current BTE context
 * @completion:	Pointer to completion structure as returned by ncbte_alloc_completion()
 *
 * This function will check a completion structure for pending events and wait until
 * all pending events complete.
 */
void ncbte_wait_completion(struct ncbte_context *context, struct ncbte_completion *completion);

/**
 * ncbte_open - Initialize BTE for use
 */
struct ncbte_context *ncbte_open(int flags);

/**
 * ncbte_close - Release BTE
 */
int ncbte_close(struct ncbte_context *context);


END_C_DECLS

#endif
