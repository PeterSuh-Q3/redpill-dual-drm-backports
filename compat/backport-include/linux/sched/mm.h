/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: linux/sched/mm.h (4.11) + fs_reclaim_acquire/release(~4.14 lockdep stub) */
#ifndef _BPI_SCHED_MM_H
#define _BPI_SCHED_MM_H
#include <linux/sched.h>
#include <linux/mm.h>
#ifndef _BPI_HAVE_FS_RECLAIM
#define _BPI_HAVE_FS_RECLAIM
static inline void fs_reclaim_acquire(gfp_t gfp) {}
static inline void fs_reclaim_release(gfp_t gfp) {}
#endif
#endif
