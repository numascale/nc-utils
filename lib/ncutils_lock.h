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

#ifndef __NCUTILS_LOCK_H
#define __NCUTILS_LOCK_H

#include <stdint.h>
#include "ncutils_atomic.h"

/* Branch prediction macros */
#ifndef likely
# define likely(x)      __builtin_expect(!!(x), 1)
#endif

#ifndef unlikely
# define unlikely(x)    __builtin_expect(!!(x), 0)
#endif

#define ACCESS_ONCE(x) (*(volatile __typeof(x) *)&(x))

#define cpu_relax() __asm __volatile("pause\n": : :"memory");

#define barrier() __asm __volatile("": : :"memory")

#ifdef __cplusplus
#  define BEGIN_C_DECLS extern "C" {
#  define END_C_DECLS   }
#else /* !__cplusplus */
#  define BEGIN_C_DECLS
#  define END_C_DECLS
#endif /* __cplusplus */

BEGIN_C_DECLS

// Simple spin locks
typedef uint32_t SpinLock_t;

static __always_inline void SpinInit(SpinLock_t *lock)
{
	smp_store_release(lock, 0);
}

static __always_inline void SpinAcquire(SpinLock_t *lock)
{
	while (unlikely(atomic_exchange(lock, 1) != 0))
		cpu_relax();
}

static __always_inline void SpinRelease(SpinLock_t *lock)
{
	smp_store_release(lock, 0);
}

// Ticketing spin locks
typedef uint16_t __ticket_t;
typedef uint32_t __ticketpair_t;

typedef struct {
	union {
		__ticketpair_t head_tail;
		struct __raw_tickets {
			__ticket_t head, tail;
		} tickets;
	};
} TicketLock_t;

static __always_inline void TicketInit(TicketLock_t *lock)
{
	smp_store_release(&lock->head_tail, 0);
}

static __always_inline void TicketAcquire(TicketLock_t *lock) {
	register struct __raw_tickets inc = { .tail = 1 };

	inc = atomic_exchange_and_add(&lock->tickets, inc);
	if (likely(inc.head == inc.tail))
		goto out;

	for (;;) {
		inc.head = ACCESS_ONCE(lock->tickets.head);
		if (!(inc.head ^ inc.tail))
			break;
		cpu_relax();
	}
out:
	barrier(); /* Make sure nothing creeps in before the lock is taken. */
}

static __always_inline void TicketRelease(TicketLock_t *lock)
{
	atomic_increment(&lock->tickets.head);
}

// Dynamic queue based ticketing spin locks

extern int tkt_spin_pass(TicketLock_t *lock, struct __raw_tickets inc);
extern void tkt_q_do_wake(TicketLock_t *lock);

static __always_inline void QTicketAcquire(TicketLock_t *lock) {
	register struct __raw_tickets inc = { .tail = 2 };

	inc = atomic_exchange_and_add(&lock->tickets, inc);
	if (likely(!(inc.head ^ inc.tail) || tkt_spin_pass(lock, inc)))
		goto out;

	for (;;) {
		inc.head = ACCESS_ONCE(lock->tickets.head);
		if (!(inc.head ^ inc.tail) || tkt_spin_pass(lock, inc))
			break;
		cpu_relax();
	}
out:
	barrier(); /* Make sure nothing creeps in before the lock is taken. */
}

static __always_inline void QTicketRelease(TicketLock_t *lock)
{
	__ticket_t head = 2;

	head = atomic_exchange_and_add(&lock->tickets.head, head);
	if (head & 0x1)
		tkt_q_do_wake(lock);
}

END_C_DECLS

#endif
