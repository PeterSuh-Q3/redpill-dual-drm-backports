/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: wait_queue rename (4.13). 4.4.302 = wait_queue_t.
 * 타입/함수명만 매핑. 구조체 멤버(.entry/.head) 는 매크로 불가 → i915_sw_fence.c 소스패치. */
#ifndef _BPI_LINUX_WAIT_H
#define _BPI_LINUX_WAIT_H
#include_next <linux/wait.h>
#ifndef _BPI_HAVE_WQ_ENTRY
#define _BPI_HAVE_WQ_ENTRY
typedef wait_queue_t wait_queue_entry_t;
#endif
#ifndef __add_wait_queue_entry_tail
#define __add_wait_queue_entry_tail __add_wait_queue_tail
#endif
#ifndef clear_and_wake_up_bit
static inline void clear_and_wake_up_bit(int bit, void *word)
{
	clear_bit_unlock(bit, word);
	wake_up_bit(word, bit);
}
#endif
#endif
