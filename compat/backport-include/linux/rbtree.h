/* SPDX-License-Identifier: GPL-2.0 */
/*
 * rbtree backport: add rb_root_cached (introduced in Linux 4.15)
 * for 4.4.302 kernel which only has rb_root.
 */
#ifndef _BACKPORT_LINUX_RBTREE_H
#define _BACKPORT_LINUX_RBTREE_H

#include_next <linux/rbtree.h>

#ifndef RB_ROOT_CACHED
struct rb_root_cached {
	struct rb_root rb_root;
	struct rb_node *rb_leftmost;
};

#define RB_ROOT_CACHED (struct rb_root_cached) { {NULL, }, NULL }

static inline void rb_insert_color_cached(struct rb_node *node,
					  struct rb_root_cached *root,
					  bool leftmost)
{
	if (leftmost)
		root->rb_leftmost = node;
	rb_insert_color(node, &root->rb_root);
}

static inline struct rb_node *
rb_erase_cached(struct rb_node *node, struct rb_root_cached *root)
{
	struct rb_node *leftmost = NULL;
	if (root->rb_leftmost == node)
		leftmost = root->rb_leftmost = rb_next(node);
	rb_erase(node, &root->rb_root);
	return leftmost;
}

static inline void rb_replace_node_cached(struct rb_node *victim,
					  struct rb_node *new,
					  struct rb_root_cached *root)
{
	if (root->rb_leftmost == victim)
		root->rb_leftmost = new;
	rb_replace_node(victim, new, &root->rb_root);
}

static inline struct rb_node *rb_first_cached(const struct rb_root_cached *root)
{
	return root->rb_leftmost;
}
#endif /* RB_ROOT_CACHED */

#endif /* _BACKPORT_LINUX_RBTREE_H */
