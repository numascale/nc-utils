/*
 * Queued ticket spinlocks.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright IBM Corporation, 2013
 *
 * Authors: Paul E. McKenney <pau...@linux.vnet.ibm.com>
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "ncutils_atomic.h"
#include "ncutils_lock.h"

#define NR_CPUS 4096

struct tkt_q {
    int cpu;
    __ticket_t tail;
    struct tkt_q *next;
};

struct tkt_q_head {
    TicketLock_t *ref;           /* Pointer to spinlock if in use. */
    int64_t head_tkt;                   /* Head ticket when started queuing. */
    struct tkt_q *spin;             /* Head of queue. */
    struct tkt_q **spin_tail;       /* Tail of queue. */
};

/*
 * TKT_Q_SWITCH is twice the number of CPUs that must be spinning on a
 * given ticket lock to motivate switching to spinning on a queue.
 * The reason that it is twice the number is because the bottom bit of
 * the ticket is reserved for the bit that indicates that a queue is
 * associated with the lock.
 */
#define TKT_Q_SWITCH  (8 * 2)

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

/*
 * TKT_Q_NQUEUES is the number of queues to maintain.  Large systems
 * might have multiple highly contended locks, so provide more queues for
 * systems with larger numbers of CPUs.
 */
#define TKT_Q_NQUEUES (DIV_ROUND_UP(NR_CPUS + TKT_Q_SWITCH - 1, TKT_Q_SWITCH) * 2)

/* The queues themselves. */
struct tkt_q_head tkt_q_heads[TKT_Q_NQUEUES];

/* Advance to the next queue slot, wrapping around to the beginning. */
static int tkt_q_next_slot(int i)
{
    return (++i < TKT_Q_NQUEUES) ? i : 0;
}

/* Very crude hash from lock address to queue slot number. */
static unsigned long tkt_q_hash(TicketLock_t *lock)
{
    return (((unsigned long)lock) >> 8) % TKT_Q_NQUEUES;
}

/*
 * Return a pointer to the queue header associated with the specified lock,
 * or return NULL if there is no queue for the lock or if the lock's queue
 * is in transition.
 */
static struct tkt_q_head *tkt_q_find_head(TicketLock_t *lock)
{
    int i;
    int start;

    start = i = tkt_q_hash(lock);
    do {
	if (tkt_q_heads[i].ref == lock)
	    return &tkt_q_heads[i];
    } while ((i = tkt_q_next_slot(i)) != start);
    return NULL;
}

/*
 * Try to stop queuing, reverting back to normal ticket-lock operation.
 * We can only stop queuing when the queue is empty, which means that
 * we need to correctly handle races where someone shows up in the queue
 * just as we are trying to dispense with the queue.  They win, we lose.
 */
static int tkt_q_try_unqueue(TicketLock_t *lock, struct tkt_q_head *tqhp)
{
    TicketLock_t asold;
    TicketLock_t asnew;

    /* Pick up the ticket values. */
    asold = ACCESS_ONCE(*lock);
    if ((asold.tickets.head & ~0x1) == asold.tickets.tail) {

	/* Attempt to mark the lock as not having a queue. */
	asnew = asold;
	asnew.tickets.head &= ~0x1;
	if (atomic_compare_and_exchange(&lock->head_tail,
					asold.head_tail,
					asnew.head_tail) == asold.head_tail) {

	    /* Succeeded, mark the queue as unused. */
	    ACCESS_ONCE(tqhp->ref) = NULL;
	    return 1;
	}
    }

    /* Failed, tell the caller there is still a queue to pass off to. */
    return 0;
}

/*
 * Hand the lock off to the first CPU on the queue.
 */
void tkt_q_do_wake(TicketLock_t *lock)
{
    struct tkt_q_head *tqhp;
    struct tkt_q *tqp;

    /* If the queue is still being set up, wait for it. */
    while ((tqhp = tkt_q_find_head(lock)) == NULL)
	cpu_relax();

    for (;;) {

	/* Find the first queue element. */
	tqp = ACCESS_ONCE(tqhp->spin);
	if (tqp != NULL)
	    break;  /* Element exists, hand off lock. */
	if (tkt_q_try_unqueue(lock, tqhp))
	    return; /* No element, successfully removed queue. */
	cpu_relax();
    }
    if (ACCESS_ONCE(tqhp->head_tkt) != -1)
	ACCESS_ONCE(tqhp->head_tkt) = -1;
    barrier(); /* Order pointer fetch and assignment against handoff. */
    ACCESS_ONCE(tqp->cpu) = -1;
}

/*
 * Given a lock that already has a queue associated with it, spin on
 * that queue.  Return false if there was no queue (which means we do not
 * hold the lock) and true otherwise (meaning we -do- hold the lock).
 */
int tkt_q_do_spin(TicketLock_t *lock, struct __raw_tickets inc)
{
    struct tkt_q **oldtail;
    struct tkt_q tq;
    struct tkt_q_head *tqhp;

    /*
     * Ensure that accesses to queue header happen after sensing
     * the lock's have-queue bit.
     */
    barrier();  /* See above block comment. */

    /* If there no longer is a queue, leave. */
    tqhp = tkt_q_find_head(lock);
    if (tqhp == NULL)
	return 0;

    /* Initialize our queue element. */
    tq.cpu = 0; /*raw_smp_processor_id();*/
    tq.tail = inc.tail;
    tq.next = NULL;

    /* Check to see if we already hold the lock. */
    if (ACCESS_ONCE(tqhp->head_tkt) == inc.tail) {
	/* The last holder left before queue formed, we hold lock. */
	tqhp->head_tkt = -1;
	return 1;
    }

    /*
     * Add our element to the tail of the queue.  Note that if the
     * queue is empty, the ->spin_tail pointer will reference
     * the queue's head pointer, namely ->spin.
     */
    oldtail = atomic_exchange(&tqhp->spin_tail, &tq.next);
    ACCESS_ONCE(*oldtail) = &tq;

    /* Spin until handoff. */
    while (ACCESS_ONCE(tq.cpu) != -1)
	cpu_relax();

    /*
     * Remove our element from the queue.  If the queue is now empty,
     * update carefully so that the next acquisition will enqueue itself
     * at the head of the list.  Of course, the next enqueue operation
     * might be happening concurrently, and this code needs to handle all
     * of the possible combinations, keeping in mind that the enqueue
     * operation happens in two stages: (1) update the tail pointer and
     * (2) update the predecessor's ->next pointer.  With this in mind,
     * the following code needs to deal with three scenarios:
     *
     * 1.   tq is the last entry.  In this case, we use cmpxchg to
     *      point the list tail back to the list head (->spin).  If
     *      the cmpxchg fails, that indicates that we are instead
     *      in scenario 2 below.  If the cmpxchg succeeds, the next
     *      enqueue operation's tail-pointer exchange will enqueue
     *      the next element at the queue head, because the ->spin_tail
     *      pointer now references the queue head.
     *
     * 2.   tq is the last entry, and the next entry has updated the
     *      tail pointer but has not yet updated tq.next.  In this
     *      case, tq.next is NULL, the cmpxchg will fail, and the
     *      code will wait for the enqueue to complete before completing
     *      removal of tq from the list.
     *
     * 3.   tq is not the last pointer.  In this case, tq.next is non-NULL,
     *      so the following code simply removes tq from the list.
     */
    if (tq.next == NULL) {

	/* Mark the queue empty. */
	tqhp->spin = NULL;

	/* Try to point the tail back at the head. */
	if (atomic_compare_and_exchange(&tqhp->spin_tail,
					&tq.next,
					&tqhp->spin) == &tq.next)
	    return 1; /* Succeeded, queue is now empty. */

	/* Failed, if needed, wait for the enqueue to complete. */
	while (tq.next == NULL)
	    cpu_relax();

	/* The following code will repair the head. */
    }
    barrier(); /* Force ordering between handoff and critical section. */

    /*
     * Advance list-head pointer.  This same task will be the next to
     * access this when releasing the lock, so no need for a memory
     * barrier after the following assignment.
     */
    ACCESS_ONCE(tqhp->spin) = tq.next;
    return 1;
}

/*
 * Given a lock that does not have a queue, attempt to associate the
 * i-th queue with it, returning true if successful (meaning we hold
 * the lock) or false otherwise (meaning we do -not- hold the lock).
 * Note that the caller has already filled in ->ref with 0x1, so we
 * own the queue.
 */
static int
tkt_q_init_contend(int i, TicketLock_t *lock, struct __raw_tickets inc)
{
    TicketLock_t asold;
    TicketLock_t asnew;
    struct tkt_q_head *tqhp;

    /* Initialize the i-th queue header. */
    tqhp = &tkt_q_heads[i];
    tqhp->spin = NULL;
    tqhp->spin_tail = &tqhp->spin;

    /* Each pass through this loop attempts to mark the lock as queued. */
    do {
	asold.head_tail = ACCESS_ONCE(lock->head_tail);
	asnew = asold;
	if (asnew.tickets.head & 0x1) {

	    /* Someone beat us to it, back out. */
	    barrier();
	    ACCESS_ONCE(tqhp->ref) = NULL;

	    /* Spin on the queue element they set up. */
	    return tkt_q_do_spin(lock, inc);
	}

	/*
	 * Record the head counter in case one of the spinning
	 * CPUs already holds the lock but doesn't realize it yet.
	 */
	tqhp->head_tkt = asold.tickets.head;

	/* The low-order bit in the head counter says "queued". */
	asnew.tickets.head |= 0x1;
    } while (atomic_compare_and_exchange(&lock->head_tail,
					 asold.head_tail,
					 asnew.head_tail) != asold.head_tail);

    /* Point the queue at the lock and go spin on it. */
    ACCESS_ONCE(tqhp->ref) = lock;
    return tkt_q_do_spin(lock, inc);
}

/*
 * Start handling a period of high contention by finding a queue to associate
 * with this lock.  Returns true if successful (in which case we hold the
 * lock) and false otherwise (in which case we do -not- hold the lock).
 */
int tkt_q_start_contend(TicketLock_t *lock, struct __raw_tickets inc)
{
    int i;
    int start;

    /* Hash the lock address to find a starting point. */
    start = i = tkt_q_hash(lock);

    /*
     * Each pass through the following loop attempts to associate
     * the lock with the corresponding queue.
     */
    do {
	/*
	 * Use 0x1 to mark the queue in use, but also avoiding
	 * any spinners trying to use it before we get it all
	 * initialized.
	 */
	if (!tkt_q_heads[i].ref &&
	    atomic_compare_and_exchange(&tkt_q_heads[i].ref,
					NULL,
					(TicketLock_t *)0x1) == NULL) {

	    /* Succeeded, now go initialize it. */
	    return tkt_q_init_contend(i, lock, inc);
	}

	/* If someone beat us to it, go spin on their queue. */
	if (ACCESS_ONCE(lock->tickets.head) & 0x1)
	    return tkt_q_do_spin(lock, inc);
    } while ((i = tkt_q_next_slot(i)) != start);

    /* All the queues are in use, revert to spinning on the ticket lock. */
    return 0;
}

int tkt_spin_pass(TicketLock_t *ap, struct __raw_tickets inc)
{
    if (inc.head & 0x1) {

	/* This lock has a queue, so go spin on the queue. */
	if (tkt_q_do_spin(ap, inc))
	    return 1;

	/* Get here if the queue is in transition: Retry next time. */

    } else if (inc.tail - TKT_Q_SWITCH == inc.head) {

	/*
	 * This lock has lots of spinners, but no queue.
	 * Go create a queue to spin on.
	 */
	if (tkt_q_start_contend(ap, inc))
	    return 1;

	/* Get here if the queue is in transition: Retry next time. */
    }

    /* Either no need for a queue or the queue is in transition.  Spin. */
    return 0;
}
