/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: __is_constexpr (~5.3). bit-field/BUILD_BUG 매크로가 사용. */
#ifndef _BPI_LINUX_CONST_H
#define _BPI_LINUX_CONST_H
#include_next <linux/const.h>
#ifndef __is_constexpr
#define __is_constexpr(x) \
	(sizeof(int) == sizeof(*(8 ? ((void *)((long)(x) * 0l)) : (int *)8)))
#endif
#endif
