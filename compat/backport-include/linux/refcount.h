/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: linux/refcount.h (~4.11). 4.4.302 부재 → atomic_t 매핑. */
#ifndef _BPI_LINUX_REFCOUNT_H
#define _BPI_LINUX_REFCOUNT_H
#include <linux/atomic.h>
#include <linux/spinlock.h>
typedef struct refcount_struct { atomic_t refs; } refcount_t;
#define REFCOUNT_INIT(n)            { .refs = ATOMIC_INIT(n) }
static inline void refcount_set(refcount_t *r, unsigned int n) { atomic_set(&r->refs, n); }
static inline unsigned int refcount_read(const refcount_t *r) { return atomic_read(&r->refs); }
static inline void refcount_inc(refcount_t *r) { atomic_inc(&r->refs); }
static inline void refcount_dec(refcount_t *r) { atomic_dec(&r->refs); }
static inline __must_check bool refcount_dec_and_test(refcount_t *r) { return atomic_dec_and_test(&r->refs); }
static inline __must_check bool refcount_inc_not_zero(refcount_t *r) { return atomic_inc_not_zero(&r->refs); }
static inline __must_check bool refcount_add_not_zero(unsigned int i, refcount_t *r) { return atomic_add_unless(&r->refs, i, 0); }
static inline void refcount_add(unsigned int i, refcount_t *r) { atomic_add(i, &r->refs); }
static inline __must_check bool refcount_sub_and_test(unsigned int i, refcount_t *r) { return atomic_sub_and_test(i, &r->refs); }
static inline __must_check bool refcount_dec_and_lock(refcount_t *r, spinlock_t *lock) { return atomic_dec_and_lock(&r->refs, lock); }
#endif
