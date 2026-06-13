/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: struct pin_cookie (~4.18). 4.4.302 부재 → 빈 구조체. */
#ifndef _BPI_LINUX_LOCKDEP_H
#define _BPI_LINUX_LOCKDEP_H
#include_next <linux/lockdep.h>
#ifndef _BPI_HAVE_PIN_COOKIE
#define _BPI_HAVE_PIN_COOKIE
struct pin_cookie { };
#endif
#endif
