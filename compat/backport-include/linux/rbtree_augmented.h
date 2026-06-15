/* SPDX-License-Identifier: GPL-2.0 */
/*
 * rbtree_augmented backport: add rb_erase_augmented_cached (5.4 API)
 * for 4.4.302 which lacks rb_root_cached support in rbtree_augmented.
 */
#ifndef _BACKPORT_LINUX_RBTREE_AUGMENTED_H
#define _BACKPORT_LINUX_RBTREE_AUGMENTED_H

#include <linux/rbtree.h>
#include_next <linux/rbtree_augmented.h>

#ifndef rb_erase_augmented_cached
static inline void
rb_erase_augmented_cached(struct rb_node *node, struct rb_root_cached *root,
			   const struct rb_augment_callbacks *augment)
{
	if (root->rb_leftmost == node)
		root->rb_leftmost = rb_next(node);
	rb_erase_augmented(node, &root->rb_root, augment);
}
#endif

#endif /* _BACKPORT_LINUX_RBTREE_AUGMENTED_H */
