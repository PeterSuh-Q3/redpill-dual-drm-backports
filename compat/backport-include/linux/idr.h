/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: idr_init_base (~4.16). include_next + base 무시 매핑. */
#ifndef _BPI_LINUX_IDR_H
#define _BPI_LINUX_IDR_H
#include_next <linux/idr.h>
#ifndef idr_init_base
#define idr_init_base(idr, base) idr_init(idr)
#endif
#endif
