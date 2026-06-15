/* SPDX-License-Identifier: GPL-2.0 */
/*
 * compat/reservation_compat.h
 * dma_resv (5.4) -> reservation_object (4.4) accessor 전체 매핑.
 *
 * 설계서 §11 리스크: "dma_resv shim 누락 -> GEM 크래시".
 * reservation_object 전체 accessor 를 빠짐없이 매핑해야 함.
 */
#ifndef _REDPILL_RESERVATION_COMPAT_H
#define _REDPILL_RESERVATION_COMPAT_H

#include <linux/reservation.h>

/* 핵심 lifecycle 은 drm_compat.h 에서 매핑. 여기서는 accessor/유틸 보강. */

#define dma_resv_held(obj) \
	reservation_object_held(obj)

#define dma_resv_assert_held(obj) \
	reservation_object_assert_held(obj)

#define dma_resv_get_excl(obj) \
	reservation_object_get_excl(obj)

#define dma_resv_get_list(obj) \
	reservation_object_get_list(obj)

static inline void dma_resv_add_excl_fence(struct reservation_object *obj,
					    struct dma_fence *fence)
{
	struct dma_fence *old = reservation_object_get_excl(obj);
	/* 4.4.302 backport: 4.4 dma_fence_release WARNs if cb_list is
	 * non-empty at release time. Signal old fence first to drain it. */
	if (old)
		dma_fence_signal(old);
	reservation_object_add_excl_fence(obj, fence);
}

#define dma_resv_add_shared_fence(obj, fence) \
	reservation_object_add_shared_fence(obj, fence)

#define dma_resv_reserve_shared(obj, num_fences) \
	reservation_object_reserve_shared(obj)

#define dma_resv_get_fences_rcu(obj, pfence_excl, pshared_count, pshared) \
	reservation_object_get_fences_rcu(obj, pfence_excl, pshared_count, pshared)

#define dma_resv_wait_timeout_rcu(obj, wait_all, intr, timeout) \
	reservation_object_wait_timeout_rcu(obj, wait_all, intr, timeout)

#define dma_resv_test_signaled_rcu(obj, test_all) \
	reservation_object_test_signaled_rcu(obj, test_all)

#define dma_resv_copy_fences(dst, src) \
	reservation_object_copy_fences(dst, src)

#endif /* _REDPILL_RESERVATION_COMPAT_H */
