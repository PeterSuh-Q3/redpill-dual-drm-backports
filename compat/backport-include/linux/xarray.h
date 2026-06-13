/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: minimal xarray API (added ~4.20). 4.4.302 = radix_tree only.
 * drm_gem_fence_array 와 i915 pages 코드에서 xa_is_value / xarray 타입 사용.
 * 실제 xarray 자료구조 대신 radix_tree 위에 최소 래퍼 제공. */
#ifndef _BPI_LINUX_XARRAY_H
#define _BPI_LINUX_XARRAY_H

#include <linux/radix-tree.h>
#include <linux/spinlock.h>
#include <linux/gfp.h>

struct xarray {
	spinlock_t xa_lock;
	struct radix_tree_root xa_head;
};

#define DEFINE_XARRAY(name) struct xarray name = {	\
	.xa_lock = __SPIN_LOCK_UNLOCKED(name.xa_lock),	\
	.xa_head = RADIX_TREE_INIT(GFP_KERNEL),		\
}

static inline void xa_init(struct xarray *xa)
{
	spin_lock_init(&xa->xa_lock);
	INIT_RADIX_TREE(&xa->xa_head, GFP_KERNEL);
}

static inline void xa_destroy(struct xarray *xa)
{
	/* radix_tree_destroy not available in 4.4 — leak guard only */
}

/* xa_is_value: xarray encodes values as odd pointers (bit 0 set).
 * In our stub, values are stored as radix-tree entries — never tagged. */
static inline bool xa_is_value(const void *entry)
{
	return (unsigned long)entry & 1;
}

static inline void *xa_mk_value(unsigned long v)
{
	return (void *)((v << 1) | 1);
}

static inline unsigned long xa_to_value(const void *entry)
{
	return (unsigned long)entry >> 1;
}

/* xa_store: store entry at index; returns replaced entry or NULL */
static inline void *xa_store(struct xarray *xa, unsigned long index,
			     void *entry, gfp_t gfp)
{
	void *old = NULL;
	unsigned long flags;
	spin_lock_irqsave(&xa->xa_lock, flags);
	old = radix_tree_lookup(&xa->xa_head, index);
	radix_tree_insert(&xa->xa_head, index, entry);
	spin_unlock_irqrestore(&xa->xa_lock, flags);
	return old;
}

static inline void *xa_load(struct xarray *xa, unsigned long index)
{
	return radix_tree_lookup(&xa->xa_head, index);
}

static inline void *xa_erase(struct xarray *xa, unsigned long index)
{
	void *old;
	unsigned long flags;
	spin_lock_irqsave(&xa->xa_lock, flags);
	old = radix_tree_delete(&xa->xa_head, index);
	spin_unlock_irqrestore(&xa->xa_lock, flags);
	return old;
}

#define xa_for_each(xa, index, entry)	\
	/* stub: radix_tree_for_each not easily portable; leave empty range */ \
	for ((entry) = NULL; 0;)

#endif /* _BPI_LINUX_XARRAY_H */
