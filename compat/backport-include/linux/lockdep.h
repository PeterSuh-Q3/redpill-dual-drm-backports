/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: pin_cookie + lockdep_pin/unpin_lock for 4.4.302 → 5.4 */
#ifndef _BPI_LINUX_LOCKDEP_H
#define _BPI_LINUX_LOCKDEP_H
#include_next <linux/lockdep.h>

/* pin_cookie struct added ~4.18 */
#ifndef _BPI_HAVE_PIN_COOKIE
#define _BPI_HAVE_PIN_COOKIE
struct pin_cookie { };
#endif

/* lockdep_pin_lock: 4.4 = do-while void; 5.4 returns struct pin_cookie.
 * Must be expression-context safe (used as: cookie = lockdep_pin_lock(l)) */
#ifdef lockdep_pin_lock
#undef lockdep_pin_lock
#endif
static inline struct pin_cookie __bpi_lockdep_pin_lock(struct lockdep_map *l)
{ (void)l; return (struct pin_cookie){}; }
#define lockdep_pin_lock(l) __bpi_lockdep_pin_lock(l)

/* lockdep_unpin_lock: 4.4 = 1-arg; 5.4 = 2-arg */
#ifdef lockdep_unpin_lock
#undef lockdep_unpin_lock
#endif
#define lockdep_unpin_lock(l, c) ((void)(l), (void)(c))

#endif
