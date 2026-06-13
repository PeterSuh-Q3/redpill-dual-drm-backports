/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: mul_u32_u32 (~4.16). include_next + inline. */
#ifndef _BPI_LINUX_MATH64_H
#define _BPI_LINUX_MATH64_H
#include_next <linux/math64.h>
#ifndef _BPI_HAVE_MUL_U32_U32
#define _BPI_HAVE_MUL_U32_U32
static inline u64 mul_u32_u32(u32 a, u32 b) { return (u64)a * b; }
#endif
#endif
