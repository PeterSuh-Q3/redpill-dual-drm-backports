/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: __GFP_RETRY_MAYFAIL (~4.13) → __GFP_NORETRY 근사. */
#ifndef _BPI_LINUX_GFP_H
#define _BPI_LINUX_GFP_H
#include_next <linux/gfp.h>
#ifndef __GFP_RETRY_MAYFAIL
#define __GFP_RETRY_MAYFAIL __GFP_NORETRY
#endif
#endif
