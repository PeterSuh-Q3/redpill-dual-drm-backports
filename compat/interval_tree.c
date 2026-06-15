// SPDX-License-Identifier: GPL-2.0-only
/*
 * interval_tree backport for 4.4.302: provide rb_root_cached * API
 * that the 5.4 OOT DRM/i915/amdgpu code expects.
 * Built into the OOT module instead of relying on kernel export.
 */
#include <linux/interval_tree_generic.h>
#include <linux/compiler.h>
#include <linux/export.h>

/* Use our backport header (rb_root_cached * variant) */
#include "backport-include/linux/interval_tree.h"

#define START(node) ((node)->start)
#define LAST(node)  ((node)->last)

INTERVAL_TREE_DEFINE(struct interval_tree_node, rb,
		     unsigned long, __subtree_last,
		     START, LAST,, interval_tree)

EXPORT_SYMBOL_GPL(interval_tree_insert);
EXPORT_SYMBOL_GPL(interval_tree_remove);
EXPORT_SYMBOL_GPL(interval_tree_iter_first);
EXPORT_SYMBOL_GPL(interval_tree_iter_next);
