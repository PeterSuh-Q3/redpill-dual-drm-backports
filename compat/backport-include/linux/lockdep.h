/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: struct pin_cookie (~4.18). 4.4.302 부재 → 빈 구조체. */
#ifndef _BPI_LINUX_LOCKDEP_H
#define _BPI_LINUX_LOCKDEP_H
#include_next <linux/lockdep.h>
#ifndef _BPI_HAVE_PIN_COOKIE
#define _BPI_HAVE_PIN_COOKIE
struct pin_cookie { };
#endif
/* lockdep_unpin_lock: 4.4.302 = 1-arg macro; 5.4 uses 2-arg. Re-define with 2 args. */
#ifdef lockdep_unpin_lock
#undef lockdep_unpin_lock
#endif
#define lockdep_unpin_lock(l, c) do { (void)(l); (void)(c); } while (0)
#endif
