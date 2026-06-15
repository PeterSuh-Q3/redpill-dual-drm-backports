/* SPDX-License-Identifier: GPL-2.0 */
/*
 * interval_tree_generic.h backport: completely self-contained.
 * Does NOT include linux/rbtree_augmented.h to avoid struct name
 * incompatibilities in DSM 4.4.302 kernel.
 * Provides INTERVAL_TREE_DEFINE using rb_root_cached * API.
 */
#ifndef _BACKPORT_LINUX_INTERVAL_TREE_GENERIC_H
#define _BACKPORT_LINUX_INTERVAL_TREE_GENERIC_H

#include <linux/rbtree.h>

/* rb_root_cached — introduced in 4.15, provide for 4.4 */
#ifndef RB_ROOT_CACHED
struct rb_root_cached {
	struct rb_root rb_root;
	struct rb_node *rb_leftmost;
};
#define RB_ROOT_CACHED (struct rb_root_cached) { {NULL, }, NULL }

static inline struct rb_node *rb_first_cached(const struct rb_root_cached *root)
{
	return root->rb_leftmost;
}

static inline void rb_insert_color_cached(struct rb_node *node,
					  struct rb_root_cached *root,
					  bool leftmost)
{
	if (leftmost)
		root->rb_leftmost = node;
	rb_insert_color(node, &root->rb_root);
}

static inline void rb_erase_cached(struct rb_node *node,
				   struct rb_root_cached *root)
{
	if (root->rb_leftmost == node)
		root->rb_leftmost = rb_next(node);
	rb_erase(node, &root->rb_root);
}
#endif /* RB_ROOT_CACHED */

/*
 * INTERVAL_TREE_DEFINE — 5.4-style, using rb_root_cached *.
 * Augment propagation is done inline without rb_augment_callbacks.
 */
#define INTERVAL_TREE_DEFINE(ITSTRUCT, ITRB, ITTYPE, ITSUBTREE,	\
			     ITSTART, ITLAST, ITSTATIC, ITPREFIX)	\
									\
static void ITPREFIX ## __it_propagate(struct rb_node *rb,		\
				       struct rb_node *stop)		\
{									\
	while (rb != stop) {						\
		ITSTRUCT *n = rb_entry(rb, ITSTRUCT, ITRB);		\
		ITTYPE s = ITLAST(n);					\
		if (n->ITRB.rb_left) {					\
			ITSTRUCT *l = rb_entry(n->ITRB.rb_left,	\
					       ITSTRUCT, ITRB);		\
			if (l->ITSUBTREE > s) s = l->ITSUBTREE;	\
		}							\
		if (n->ITRB.rb_right) {					\
			ITSTRUCT *r = rb_entry(n->ITRB.rb_right,	\
					       ITSTRUCT, ITRB);		\
			if (r->ITSUBTREE > s) s = r->ITSUBTREE;	\
		}							\
		if (n->ITSUBTREE == s) break;				\
		n->ITSUBTREE = s;					\
		rb = rb_parent(&n->ITRB);				\
	}								\
}									\
									\
ITSTATIC void ITPREFIX ## _insert(ITSTRUCT *node,			\
				  struct rb_root_cached *root)		\
{									\
	struct rb_node **link = &root->rb_root.rb_node;			\
	struct rb_node *rb_parent = NULL;				\
	ITTYPE start = ITSTART(node), last = ITLAST(node);		\
	ITSTRUCT *parent;						\
	bool leftmost = true;						\
									\
	node->ITSUBTREE = last;						\
	while (*link) {							\
		rb_parent = *link;					\
		parent = rb_entry(rb_parent, ITSTRUCT, ITRB);		\
		if (parent->ITSUBTREE < last)				\
			parent->ITSUBTREE = last;			\
		if (start < ITSTART(parent))				\
			link = &(*link)->rb_left;			\
		else {							\
			link = &(*link)->rb_right;			\
			leftmost = false;				\
		}							\
	}								\
	rb_link_node(&node->ITRB, rb_parent, link);			\
	rb_insert_color_cached(&node->ITRB, root, leftmost);		\
}									\
									\
ITSTATIC void ITPREFIX ## _remove(ITSTRUCT *node,			\
				  struct rb_root_cached *root)		\
{									\
	struct rb_node *deepest = rb_prev(&node->ITRB);			\
	if (root->rb_leftmost == &node->ITRB)				\
		root->rb_leftmost = rb_next(&node->ITRB);		\
	rb_erase(&node->ITRB, &root->rb_root);				\
	if (deepest)							\
		ITPREFIX ## __it_propagate(deepest, NULL);		\
	else if (root->rb_root.rb_node)					\
		ITPREFIX ## __it_propagate(root->rb_root.rb_node, NULL); \
}									\
									\
static ITSTRUCT *							\
ITPREFIX ## __it_search(ITSTRUCT *node,					\
			ITTYPE start, ITTYPE last)			\
{									\
	while (true) {							\
		if (node->ITRB.rb_left) {				\
			ITSTRUCT *l = rb_entry(node->ITRB.rb_left,	\
					       ITSTRUCT, ITRB);		\
			if (l->ITSUBTREE >= start) {			\
				node = l; continue;			\
			}						\
		}							\
		if (ITSTART(node) <= last) {				\
			if (ITLAST(node) >= start)			\
				return node;				\
			if (node->ITRB.rb_right) {			\
				ITSTRUCT *r = rb_entry(			\
					node->ITRB.rb_right,		\
					ITSTRUCT, ITRB);		\
				if (r->ITSUBTREE >= start) {		\
					node = r; continue;		\
				}					\
			}						\
		}							\
		return NULL;						\
	}								\
}									\
									\
ITSTATIC ITSTRUCT *							\
ITPREFIX ## _iter_first(struct rb_root_cached *root,			\
			ITTYPE start, ITTYPE last)			\
{									\
	ITSTRUCT *node, *leftmost;					\
	if (!root->rb_root.rb_node)					\
		return NULL;						\
	leftmost = rb_entry(root->rb_leftmost, ITSTRUCT, ITRB);	\
	if (leftmost->ITSUBTREE < start)				\
		return NULL;						\
	node = rb_entry(root->rb_root.rb_node, ITSTRUCT, ITRB);	\
	return ITPREFIX ## __it_search(node, start, last);		\
}									\
									\
ITSTATIC ITSTRUCT *							\
ITPREFIX ## _iter_next(ITSTRUCT *node, ITTYPE start, ITTYPE last)	\
{									\
	struct rb_node *rb = rb_next(&node->ITRB);			\
	while (rb) {							\
		node = rb_entry(rb, ITSTRUCT, ITRB);			\
		if (ITSTART(node) > last) return NULL;			\
		if (ITLAST(node) >= start) return node;			\
		rb = rb_next(rb);					\
	}								\
	return NULL;							\
}

#endif /* _BACKPORT_LINUX_INTERVAL_TREE_GENERIC_H */
