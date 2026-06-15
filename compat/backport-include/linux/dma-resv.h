/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: linux/dma-resv.h (5.4 rename of reservation.h)
 * 4.4.302 는 reservation_object 만 제공. dma_resv_* → reservation_object_* 매핑.
 * (구 drm_compat.h / reservation_compat.h 의 매핑을 여기로 통합 — 헤더 교체 방식) */
#ifndef _BPI_LINUX_DMA_RESV_H
#define _BPI_LINUX_DMA_RESV_H

#include <linux/reservation.h>

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
/* 4.4.302 backport: signal old excl fence before replacing to drain cb_list,
 * preventing dma_fence_release WARN flood on QuickSync workloads. */
static inline void dma_resv_add_excl_fence(struct reservation_object *obj,
					    struct dma_fence *fence)
{
	struct dma_fence *old = reservation_object_get_excl(obj);
	if (old)
		dma_fence_signal(old);
	reservation_object_add_excl_fence(obj, fence);
}
#define dma_resv_add_shared_fence reservation_object_add_shared_fence
#define dma_resv_get_fences_rcu  reservation_object_get_fences_rcu
#define dma_resv_wait_timeout_rcu reservation_object_wait_timeout_rcu
#define dma_resv_test_signaled_rcu reservation_object_test_signaled_rcu
#define dma_resv_copy_fences     reservation_object_copy_fences
#define dma_resv_get_excl_rcu    reservation_object_get_excl_rcu
#define dma_resv_lock_interruptible(obj, ctx) ww_mutex_lock_interruptible(&(obj)->lock, (ctx))

/* 5.4: dma_resv_reserve_shared(obj, num) — 4.4: reservation_object_reserve_shared(obj) */
#define dma_resv_reserve_shared(obj, num) reservation_object_reserve_shared(obj)

#endif
