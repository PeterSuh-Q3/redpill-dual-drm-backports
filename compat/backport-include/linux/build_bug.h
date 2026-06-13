/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: linux/build_bug.h (~4.13 분리) + static_assert(~4.16). */
#ifndef _BPI_LINUX_BUILD_BUG_H
#define _BPI_LINUX_BUILD_BUG_H
#include <linux/bug.h>
#ifndef static_assert
#define static_assert(expr, ...) _Static_assert(expr, #expr)
#endif
#endif
