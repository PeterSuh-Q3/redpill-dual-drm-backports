/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: asm/set_memory.h (~4.12 분리, 4.4.302 엔 set_memory_* 가
 * asm/cacheflush.h 에 있음) */
#ifndef _BPI_ASM_SET_MEMORY_H
#define _BPI_ASM_SET_MEMORY_H
#include <asm/cacheflush.h>
#endif
