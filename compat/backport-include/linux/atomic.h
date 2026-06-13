/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: atomic_fetch_inc (~4.8). include_next + 정의. */
#ifndef _BPI_LINUX_ATOMIC_H
#define _BPI_LINUX_ATOMIC_H
#include_next <linux/atomic.h>
#ifndef atomic_fetch_inc
#define atomic_fetch_inc(v) (atomic_inc_return(v) - 1)
#endif
#endif
