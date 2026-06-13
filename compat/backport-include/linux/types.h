/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: __poll_t (4.16+ — 4.4.302 부재). drm_file.h 등에서 사용. */
#ifndef _BPI_LINUX_TYPES_H
#define _BPI_LINUX_TYPES_H
#include_next <linux/types.h>
#ifndef _BPI_HAVE_POLL_T
#define _BPI_HAVE_POLL_T
typedef unsigned __bitwise __poll_t;
#endif
#endif
