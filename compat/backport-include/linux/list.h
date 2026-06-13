/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: list_is_first (4.19+) and list_for_each_entry_from_reverse (4.12+).
 * 4.4.302 에 미존재. list_prev_entry 는 4.4에 있으므로 그것을 활용. */
#ifndef _BPI_LINUX_LIST_H
#define _BPI_LINUX_LIST_H
#include_next <linux/list.h>
#ifndef list_is_first
static inline int list_is_first(const struct list_head *list,
				const struct list_head *head)
{
	return list->prev == head;
}
#endif
#ifndef list_for_each_entry_from_reverse
#define list_for_each_entry_from_reverse(pos, head, member)		\
	for (; &pos->member != (head);					\
	     pos = list_prev_entry(pos, member))
#endif
#endif
