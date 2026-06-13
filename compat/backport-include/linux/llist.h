/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: llist_for_each_safe (added ~4.15). 4.4.302 부재. */
#ifndef _BPI_LINUX_LLIST_H
#define _BPI_LINUX_LLIST_H
#include_next <linux/llist.h>
#ifndef llist_for_each_safe
#define llist_for_each_safe(pos, n, node)			\
	for ((pos) = (node);					\
	     (pos) && ((n) = (pos)->next, true);		\
	     (pos) = (n))
#endif
#endif
