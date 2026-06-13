/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: swiotlb_max_segment (~4.18) → 제한없음 의미 0. */
#ifndef _BPI_LINUX_SWIOTLB_H
#define _BPI_LINUX_SWIOTLB_H
#include_next <linux/swiotlb.h>
#ifndef _BPI_HAVE_SWIOTLB_MAX_SEGMENT
#define _BPI_HAVE_SWIOTLB_MAX_SEGMENT
static inline unsigned int swiotlb_max_segment(void) { return 0; }
#endif
#endif
