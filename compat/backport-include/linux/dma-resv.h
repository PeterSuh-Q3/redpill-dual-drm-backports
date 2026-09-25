/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: linux/dma-resv.h (5.4 rename of reservation.h)
 * 4.4.302 는 reservation_object 만 제공. dma_resv_* → reservation_object_* 매핑.
 * (구 drm_compat.h / reservation_compat.h 의 매핑을 여기로 통합 — 헤더 교체 방식) */
#ifndef _BPI_LINUX_DMA_RESV_H
#define _BPI_LINUX_DMA_RESV_H

#include <linux/reservation.h>
#include <linux/slab.h>

#define dma_resv                 reservation_object
#define dma_resv_list            reservation_object_list
#define dma_resv_lock            reservation_object_lock
#define dma_resv_unlock          reservation_object_unlock
#define dma_resv_trylock         reservation_object_trylock
#define dma_resv_init            reservation_object_init
/* 4.4.302 backport: signal all fences before fini to drain cb_list,
 * preventing dma_fence_release WARN in __i915_gem_free_object_rcu path. */
static inline void dma_resv_fini(struct reservation_object *obj)
{
	struct reservation_object_list *flist;
	struct dma_fence *excl;
	unsigned int i;

	excl = reservation_object_get_excl(obj);
	if (excl)
		dma_fence_signal(excl);

	flist = reservation_object_get_list(obj);
	if (flist) {
		for (i = 0; i < flist->shared_count; i++)
			dma_fence_signal(flist->shared[i]);
	}

	reservation_object_fini(obj);
}
#define dma_resv_held            reservation_object_held
#define dma_resv_assert_held     reservation_object_assert_held
#define dma_resv_get_excl        reservation_object_get_excl
#define dma_resv_get_list        reservation_object_get_list
/* 4.4.302 backport: signal old excl AND shared fences before replacing to
 * drain cb_list. reservation_object_add_excl_fence() drops shared fences
 * too, so all must be signaled first to prevent dma_fence_release WARN. */
static inline void dma_resv_add_excl_fence(struct reservation_object *obj,
					    struct dma_fence *fence)
{
	struct reservation_object_list *flist;
	struct dma_fence *old = reservation_object_get_excl(obj);
	unsigned int i;

	if (old)
		dma_fence_signal(old);

	flist = reservation_object_get_list(obj);
	if (flist) {
		for (i = 0; i < flist->shared_count; i++)
			dma_fence_signal(flist->shared[i]);
	}

	reservation_object_add_excl_fence(obj, fence);
}
#define dma_resv_add_shared_fence reservation_object_add_shared_fence
/* Unlike dma_resv_get_fences_rcu(), the 4.4 reservation API requires a
 * non-NULL exclusive-fence output. When callers pass NULL, the modern API
 * appends that fence to the shared array; preserve that ownership contract. */
static inline int k4_dma_resv_get_fences_rcu(struct reservation_object *obj,
					     struct dma_fence **pfence_excl,
					     unsigned int *pshared_count,
					     struct dma_fence ***pshared)
{
	struct dma_fence *excl = NULL, **shared = NULL, **all;
	unsigned int count = 0, i;
	int ret;

	ret = reservation_object_get_fences_rcu(obj, &excl, &count, &shared);
	if (ret)
		return ret;
	if (pfence_excl) {
		*pfence_excl = excl;
		*pshared_count = count;
		*pshared = shared;
		return 0;
	}
	if (!excl) {
		*pshared_count = count;
		*pshared = shared;
		return 0;
	}

	all = krealloc(shared, sizeof(*all) * (count + 1), GFP_KERNEL);
	if (!all) {
		for (i = 0; i < count; i++)
			dma_fence_put(shared[i]);
		dma_fence_put(excl);
		kfree(shared);
		return -ENOMEM;
	}
	all[count++] = excl;
	*pshared_count = count;
	*pshared = all;
	return 0;
}
#define dma_resv_get_fences_rcu k4_dma_resv_get_fences_rcu
#define dma_resv_wait_timeout_rcu reservation_object_wait_timeout_rcu
#define dma_resv_test_signaled_rcu reservation_object_test_signaled_rcu
#define dma_resv_copy_fences     reservation_object_copy_fences
#define dma_resv_get_excl_rcu    reservation_object_get_excl_rcu
#define dma_resv_lock_interruptible(obj, ctx) ww_mutex_lock_interruptible(&(obj)->lock, (ctx))

/* 5.4: dma_resv_reserve_shared(obj, num) — 4.4: reservation_object_reserve_shared(obj) */
#define dma_resv_reserve_shared(obj, num) reservation_object_reserve_shared(obj)

#endif
