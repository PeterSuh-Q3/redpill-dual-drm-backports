/* SPDX-License-Identifier: GPL-2.0 */
/*
 * interval_tree backport shim: 4.4.302 kernel exports interval_tree_* with
 * struct rb_root *, while 5.4 OOT code passes struct rb_root_cached *.
 * Wrap the 4.4.302 symbols so callers can use rb_root_cached transparently.
 */
#ifndef _BACKPORT_LINUX_INTERVAL_TREE_H
#define _BACKPORT_LINUX_INTERVAL_TREE_H

#include_next <linux/interval_tree.h>
#include <linux/rbtree.h>

/* 4.4.302 kernel provides interval_tree_* with rb_root * — wrap for rb_root_cached callers */
static inline void
__bp_interval_tree_insert(struct interval_tree_node *node, struct rb_root_cached *root)
{
	interval_tree_insert(node, &root->rb_root);
}

static inline void
__bp_interval_tree_remove(struct interval_tree_node *node, struct rb_root_cached *root)
{
	interval_tree_remove(node, &root->rb_root);
}

static inline struct interval_tree_node *
__bp_interval_tree_iter_first(struct rb_root_cached *root,
			      unsigned long start, unsigned long last)
{
	return interval_tree_iter_first(&root->rb_root, start, last);
}

#undef  interval_tree_insert
#undef  interval_tree_remove
#undef  interval_tree_iter_first
#define interval_tree_insert(node, root)           __bp_interval_tree_insert(node, root)
#define interval_tree_remove(node, root)           __bp_interval_tree_remove(node, root)
#define interval_tree_iter_first(root, start, end) __bp_interval_tree_iter_first(root, start, end)

#endif /* _BACKPORT_LINUX_INTERVAL_TREE_H */
