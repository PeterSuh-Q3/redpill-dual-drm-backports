/* SPDX-License-Identifier: GPL-2.0 */
/*
 * interval_tree_generic.h backport: 5.4 version using rb_root_cached *
 * Replaces 4.4.302 kernel version which uses rb_root *.
 *
 * Based on linux/include/linux/interval_tree_generic.h (v5.4)
 */
#ifndef _BACKPORT_LINUX_INTERVAL_TREE_GENERIC_H
#define _BACKPORT_LINUX_INTERVAL_TREE_GENERIC_H

#include <linux/rbtree_augmented.h>

/*
 * Template for implementing interval trees
 *
 * ITSTRUCT:   struct type of the interval tree nodes
 * ITRB:       name of struct rb_node field within ITSTRUCT
 * ITTYPE:     type of the interval endpoints
 * ITSUBTREE:  name of ITTYPE field within ITSTRUCT holding last-in-subtree
 * ITSTART(n): start endpoint of ITSTRUCT node n
 * ITLAST(n):  last endpoint of ITSTRUCT node n
 * ITSTATIC:   'static' or empty
 * ITPREFIX:   function name prefix
 */

#define INTERVAL_TREE_DEFINE(ITSTRUCT, ITRB, ITTYPE, ITSUBTREE,	      \
			     ITSTART, ITLAST, ITSTATIC, ITPREFIX)	      \
									      \
static inline void ITPREFIX ## _augment_propagate(struct rb_node *rb, struct rb_node *stop) \
{									      \
	while (rb != stop) {						      \
		ITSTRUCT *node = rb_entry(rb, ITSTRUCT, ITRB);		      \
		ITTYPE subtree_last = ITLAST(node);			      \
		if (node->ITRB.rb_left) {				      \
			ITSTRUCT *left = rb_entry(node->ITRB.rb_left, ITSTRUCT, ITRB); \
			if (left->ITSUBTREE > subtree_last)		      \
				subtree_last = left->ITSUBTREE;		      \
		}							      \
		if (node->ITRB.rb_right) {				      \
			ITSTRUCT *right = rb_entry(node->ITRB.rb_right, ITSTRUCT, ITRB); \
			if (right->ITSUBTREE > subtree_last)		      \
				subtree_last = right->ITSUBTREE;	      \
		}							      \
		if (node->ITSUBTREE == subtree_last)			      \
			break;						      \
		node->ITSUBTREE = subtree_last;				      \
		rb = rb_parent(&node->ITRB);				      \
	}								      \
}									      \
									      \
static inline void ITPREFIX ## _augment_copy(struct rb_node *rb_old,	      \
					     struct rb_node *rb_new)	      \
{									      \
	ITSTRUCT *old = rb_entry(rb_old, ITSTRUCT, ITRB);		      \
	ITSTRUCT *new = rb_entry(rb_new, ITSTRUCT, ITRB);		      \
	new->ITSUBTREE = old->ITSUBTREE;				      \
}									      \
									      \
static inline void ITPREFIX ## _augment_rotate(struct rb_node *rb_old,	      \
					       struct rb_node *rb_new)	      \
{									      \
	ITSTRUCT *old = rb_entry(rb_old, ITSTRUCT, ITRB);		      \
	ITSTRUCT *new = rb_entry(rb_new, ITSTRUCT, ITRB);		      \
	new->ITSUBTREE = old->ITSUBTREE;				      \
	ITPREFIX ## _augment_propagate(rb_old, NULL);			      \
}									      \
									      \
static const struct rb_augment_callbacks ITPREFIX ## _augment_callbacks = {   \
	.propagate = ITPREFIX ## _augment_propagate,			      \
	.copy      = ITPREFIX ## _augment_copy,				      \
	.rotate    = ITPREFIX ## _augment_rotate			      \
};									      \
									      \
ITSTATIC void ITPREFIX ## _insert(ITSTRUCT *node,			      \
				  struct rb_root_cached *root)		      \
{									      \
	struct rb_node **link = &root->rb_root.rb_node, *rb_parent = NULL;   \
	ITTYPE start = ITSTART(node), last = ITLAST(node);		      \
	ITSTRUCT *parent;						      \
	bool leftmost = true;						      \
									      \
	while (*link) {							      \
		rb_parent = *link;					      \
		parent = rb_entry(rb_parent, ITSTRUCT, ITRB);		      \
		if (parent->ITSUBTREE < last)				      \
			parent->ITSUBTREE = last;			      \
		if (start < ITSTART(parent))				      \
			link = &parent->ITRB.rb_left;			      \
		else {							      \
			link = &parent->ITRB.rb_right;			      \
			leftmost = false;				      \
		}							      \
	}								      \
									      \
	node->ITSUBTREE = last;						      \
	rb_link_node(&node->ITRB, rb_parent, link);			      \
	rb_insert_color_cached(&node->ITRB, root, leftmost);		      \
}									      \
									      \
ITSTATIC void ITPREFIX ## _remove(ITSTRUCT *node,			      \
				  struct rb_root_cached *root)		      \
{									      \
	rb_erase_augmented_cached(&node->ITRB, root,			      \
				  &ITPREFIX ## _augment_callbacks);	      \
}									      \
									      \
static ITTYPE ITPREFIX ## _subtree_search(ITSTRUCT *node,		      \
					  ITTYPE start, ITTYPE last)	      \
{									      \
	while (true) {							      \
		if (node->ITRB.rb_left) {				      \
			ITSTRUCT *left = rb_entry(node->ITRB.rb_left,	      \
						  ITSTRUCT, ITRB);	      \
			if (left->ITSUBTREE >= start) {			      \
				node = left;				      \
				continue;				      \
			}						      \
		}							      \
		if (ITSTART(node) <= last) {				      \
			if (ITLAST(node) >= start)			      \
				return 1;				      \
			if (node->ITRB.rb_right) {			      \
				node = rb_entry(node->ITRB.rb_right,	      \
						ITSTRUCT, ITRB);	      \
				if (node->ITSUBTREE >= start)		      \
					continue;			      \
			}						      \
		}							      \
		return 0;						      \
	}								      \
}									      \
									      \
ITSTATIC ITSTRUCT *							      \
ITPREFIX ## _iter_first(struct rb_root_cached *root,			      \
			ITTYPE start, ITTYPE last)			      \
{									      \
	ITSTRUCT *node, *leftmost;					      \
									      \
	if (!root->rb_root.rb_node)					      \
		return NULL;						      \
									      \
	leftmost = rb_entry(root->rb_leftmost, ITSTRUCT, ITRB);	      \
	if (leftmost->ITSUBTREE < start)				      \
		return NULL;						      \
									      \
	node = rb_entry(root->rb_root.rb_node, ITSTRUCT, ITRB);	      \
	return ITPREFIX ## _subtree_search(node, start, last) ?	      \
		rb_entry(rb_next(&node->ITRB) ?: &node->ITRB,		      \
			 ITSTRUCT, ITRB) : NULL;			      \
}									      \
									      \
ITSTATIC ITSTRUCT *							      \
ITPREFIX ## _iter_next(ITSTRUCT *node, ITTYPE start, ITTYPE last)	      \
{									      \
	struct rb_node *rb = rb_next(&node->ITRB);			      \
	while (rb) {							      \
		node = rb_entry(rb, ITSTRUCT, ITRB);			      \
		if (ITSTART(node) > last)				      \
			return NULL;					      \
		if (ITLAST(node) >= start)				      \
			return node;					      \
		rb = rb_next(rb);					      \
	}								      \
	return NULL;							      \
}

#endif /* _BACKPORT_LINUX_INTERVAL_TREE_GENERIC_H */
