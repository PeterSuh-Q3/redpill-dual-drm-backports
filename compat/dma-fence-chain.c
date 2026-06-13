// SPDX-License-Identifier: GPL-2.0-only
/*
 * dma-fence-chain: chain fences in a timeline (minimal backport for 4.4.302)
 *
 * Ported from Linux 5.4 drivers/dma-buf/dma-fence-chain.c
 * Copyright (C) 2018 Advanced Micro Devices, Inc.  (Christian König)
 *
 * 4.4.302 적합화:
 *   - dma_fence_ops.use_64bit_seqno 필드 없음 → 제거
 *   - dma_fence_free() 없음 → kfree() 직접 사용
 *   - dma_fence_context_alloc(1) 은 4.4.302 에도 존재
 */
#include <linux/export.h>
#include <linux/slab.h>
#include <linux/irq_work.h>
#include <linux/dma-fence.h>
#include "backport-include/linux/dma-fence-chain.h"

static const char *dma_fence_chain_get_driver_name(struct dma_fence *fence)
{
	return "dma_fence_chain";
}

static const char *dma_fence_chain_get_timeline_name(struct dma_fence *fence)
{
	return "unbound";
}

static void dma_fence_chain_irq_work(struct irq_work *work)
{
	struct dma_fence_chain *chain =
		container_of(work, struct dma_fence_chain, work);

	dma_fence_signal(&chain->base);
	dma_fence_put(&chain->base);
}

static void dma_fence_chain_cb(struct dma_fence *f, struct dma_fence_cb *cb)
{
	struct dma_fence_chain *chain =
		container_of(cb, struct dma_fence_chain, cb);

	irq_work_queue(&chain->work);
	dma_fence_put(f);
}

static bool dma_fence_chain_enable_signaling(struct dma_fence *fence)
{
	struct dma_fence_chain *chain = to_dma_fence_chain(fence);
	struct dma_fence *prev;

	dma_fence_get(&chain->base);
	prev = rcu_dereference_protected(chain->prev,
					 lockdep_is_held(&chain->lock));
	if (!prev || dma_fence_add_callback(prev, &chain->cb,
					    dma_fence_chain_cb)) {
		dma_fence_put(&chain->base);
		return false;
	}
	return true;
}

static bool dma_fence_chain_signaled(struct dma_fence *fence)
{
	struct dma_fence_chain *chain = to_dma_fence_chain(fence);
	struct dma_fence *prev = rcu_dereference_protected(chain->prev,
					lockdep_is_held(&chain->lock));

	if (!dma_fence_is_signaled(chain->fence))
		return false;
	return !prev || dma_fence_is_signaled(prev);
}

static void dma_fence_chain_release(struct dma_fence *fence)
{
	struct dma_fence_chain *chain = to_dma_fence_chain(fence);
	struct dma_fence *prev;

	prev = rcu_dereference_protected(chain->prev, true);
	rcu_assign_pointer(chain->prev, NULL);
	dma_fence_put(prev);
	dma_fence_put(chain->fence);
	kfree(fence);   /* 4.4.302: dma_fence_free() 미존재 */
}

const struct dma_fence_ops dma_fence_chain_ops = {
	.get_driver_name   = dma_fence_chain_get_driver_name,
	.get_timeline_name = dma_fence_chain_get_timeline_name,
	.enable_signaling  = dma_fence_chain_enable_signaling,
	.signaled          = dma_fence_chain_signaled,
	.release           = dma_fence_chain_release,
};
EXPORT_SYMBOL(dma_fence_chain_ops);

/**
 * dma_fence_chain_walk - advance to the next fence in the chain
 *
 * Walk the chain: GC signaled nodes, return next unsignaled node (ref held).
 * Returns NULL if no more fences in chain.
 */
struct dma_fence *dma_fence_chain_walk(struct dma_fence *fence)
{
	struct dma_fence_chain *chain;
	struct dma_fence *prev, *tmp, *result = NULL;

	chain = to_dma_fence_chain(fence);
	if (!chain) {
		dma_fence_put(fence);
		return NULL;
	}

	while (true) {
		prev = rcu_dereference_protected(chain->prev,
						 lockdep_is_held(&chain->lock));
		if (!prev)
			break;

		prev = dma_fence_get(prev);
		if (!prev)
			break;

		if (!dma_fence_is_signaled(prev)) {
			result = prev;
			break;
		}

		/* GC: replace prev with prev->prev */
		tmp = to_dma_fence_chain(prev) ?
			rcu_dereference_protected(
				to_dma_fence_chain(prev)->prev, true) :
			NULL;
		if (tmp)
			dma_fence_get(tmp);
		rcu_assign_pointer(chain->prev, tmp);
		dma_fence_put(prev);

		if (!tmp)
			break;
		prev = tmp;
		chain = to_dma_fence_chain(prev);
		if (!chain) {
			result = prev;
			break;
		}
	}

	dma_fence_put(fence);
	return result;
}
EXPORT_SYMBOL(dma_fence_chain_walk);

/**
 * dma_fence_chain_find_seqno - find fence at or after seqno in chain
 *
 * Updates *pfence to point to the chain node whose prev_seqno < seqno.
 * Returns -EINVAL if seqno is beyond the chain head.
 */
int dma_fence_chain_find_seqno(struct dma_fence **pfence, uint64_t seqno)
{
	struct dma_fence_chain *chain;

	if (!seqno)
		return 0;

	chain = to_dma_fence_chain(*pfence);
	if (!chain || chain->base.seqno < seqno)
		return -EINVAL;

	dma_fence_chain_for_each(*pfence, &chain->base) {
		if ((*pfence)->context != chain->base.context ||
		    to_dma_fence_chain(*pfence)->prev_seqno < seqno)
			break;
	}
	return 0;
}
EXPORT_SYMBOL(dma_fence_chain_find_seqno);

/**
 * dma_fence_chain_init - initialize a fence chain node
 */
void dma_fence_chain_init(struct dma_fence_chain *chain,
			  struct dma_fence *prev,
			  struct dma_fence *fence,
			  uint64_t seqno)
{
	static u64 context;
	static bool ctx_init;

	spin_lock_init(&chain->lock);
	init_irq_work(&chain->work, dma_fence_chain_irq_work);

	if (!ctx_init) {
		context = dma_fence_context_alloc(1);
		ctx_init = true;
	}

	rcu_assign_pointer(chain->prev, prev);
	chain->fence = fence;
	chain->prev_seqno = prev ? prev->seqno : 0;

	dma_fence_init(&chain->base, &dma_fence_chain_ops,
		       &chain->lock, context, seqno);

	if (dma_fence_is_signaled(fence) &&
	    (!prev || dma_fence_is_signaled(prev)))
		dma_fence_signal(&chain->base);
}
EXPORT_SYMBOL(dma_fence_chain_init);
