// SPDX-License-Identifier: GPL-2.0-only
/*
 * interval_tree backport for 4.4.302: provide rb_root_cached * API.
 * Manual implementation — avoids INTERVAL_TREE_DEFINE macro and
 * rb_augment_callbacks struct incompatibilities with DSM 4.4.302 kernel.
 */
#include <linux/export.h>
#include <linux/rbtree.h>
#include "backport-include/linux/interval_tree.h"

static void
it_augment(struct rb_node *rb, struct rb_node *stop)
{
	while (rb != stop) {
		struct interval_tree_node *n =
			rb_entry(rb, struct interval_tree_node, rb);
		unsigned long subtree_last = n->last;

		if (n->rb.rb_left) {
			struct interval_tree_node *l =
				rb_entry(n->rb.rb_left,
					 struct interval_tree_node, rb);
			if (l->__subtree_last > subtree_last)
				subtree_last = l->__subtree_last;
		}
		if (n->rb.rb_right) {
			struct interval_tree_node *r =
				rb_entry(n->rb.rb_right,
					 struct interval_tree_node, rb);
			if (r->__subtree_last > subtree_last)
				subtree_last = r->__subtree_last;
		}
		if (n->__subtree_last == subtree_last)
			break;
		n->__subtree_last = subtree_last;
		rb = rb_parent(&n->rb);
	}
}

void interval_tree_insert(struct interval_tree_node *node,
			  struct rb_root_cached *root)
{
	struct rb_node **link = &root->rb_root.rb_node;
	struct rb_node *rb_parent = NULL;
	bool leftmost = true;

	node->__subtree_last = node->last;

	while (*link) {
		struct interval_tree_node *parent =
			rb_entry(*link, struct interval_tree_node, rb);

		rb_parent = *link;
		if (parent->__subtree_last < node->last)
			parent->__subtree_last = node->last;

		if (node->start < parent->start) {
			link = &(*link)->rb_left;
		} else {
			link = &(*link)->rb_right;
			leftmost = false;
		}
	}

	rb_link_node(&node->rb, rb_parent, link);
	rb_insert_color(&node->rb, &root->rb_root);
	if (leftmost)
		root->rb_leftmost = &node->rb;
}
EXPORT_SYMBOL_GPL(interval_tree_insert);

void interval_tree_remove(struct interval_tree_node *node,
			  struct rb_root_cached *root)
{
	struct rb_node *deepest;

	if (root->rb_leftmost == &node->rb)
		root->rb_leftmost = rb_next(&node->rb);

	deepest = rb_prev(&node->rb);
	rb_erase(&node->rb, &root->rb_root);

	if (deepest)
		it_augment(deepest, NULL);
	else if (root->rb_root.rb_node)
		it_augment(root->rb_root.rb_node, NULL);
}
EXPORT_SYMBOL_GPL(interval_tree_remove);

static struct interval_tree_node *
it_subtree_search(struct interval_tree_node *node,
		  unsigned long start, unsigned long last)
{
	while (true) {
		if (node->rb.rb_left) {
			struct interval_tree_node *l =
				rb_entry(node->rb.rb_left,
					 struct interval_tree_node, rb);
			if (l->__subtree_last >= start) {
				node = l;
				continue;
			}
		}
		if (node->start <= last) {
			if (node->last >= start)
				return node;
			if (node->rb.rb_right) {
				struct interval_tree_node *r =
					rb_entry(node->rb.rb_right,
						 struct interval_tree_node, rb);
				if (r->__subtree_last >= start) {
					node = r;
					continue;
				}
			}
		}
		return NULL;
	}
}

struct interval_tree_node *
interval_tree_iter_first(struct rb_root_cached *root,
			 unsigned long start, unsigned long last)
{
	struct interval_tree_node *node;

	if (!root->rb_root.rb_node)
		return NULL;

	node = rb_entry(root->rb_leftmost, struct interval_tree_node, rb);
	if (node->__subtree_last < start)
		return NULL;

	node = rb_entry(root->rb_root.rb_node, struct interval_tree_node, rb);
	return it_subtree_search(node, start, last);
}
EXPORT_SYMBOL_GPL(interval_tree_iter_first);

struct interval_tree_node *
interval_tree_iter_next(struct interval_tree_node *node,
			unsigned long start, unsigned long last)
{
	struct rb_node *rb = rb_next(&node->rb);

	while (rb) {
		node = rb_entry(rb, struct interval_tree_node, rb);
		if (node->start > last)
			return NULL;
		if (node->last >= start)
			return node;
		rb = rb_next(rb);
	}
	return NULL;
}
EXPORT_SYMBOL_GPL(interval_tree_iter_next);
