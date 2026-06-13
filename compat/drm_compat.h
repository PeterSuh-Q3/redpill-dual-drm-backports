/* SPDX-License-Identifier: GPL-2.0 */
/*
 * compat/drm_compat.h
 * 4.4.302 <-> 5.4 LTS DRM core API 호환 레이어 (공통)
 * 설계서 §6.1 기준. i915 + amdgpu 공용.
 *
 * shim 방향: 5.4 소스가 호출하는 심볼 -> 4.4 커널 심볼로 연결.
 */
#ifndef _REDPILL_DRM_COMPAT_H
#define _REDPILL_DRM_COMPAT_H

#include <linux/version.h>
#include <linux/fence.h>
#include <linux/reservation.h>
#include <drm/drmP.h>
#include <drm/drm_gem.h>

/* ── dma_fence: 4.4 fence API 매핑 ──
 * 5.4 소스: dma_fence_* 호출
 * 4.4 커널: fence_* 존재
 */
#define dma_fence_ops           fence_ops
#define dma_fence_init          fence_init
#define dma_fence_signal        fence_signal
#define dma_fence_get           fence_get
#define dma_fence_put           fence_put
#define dma_fence_is_signaled   fence_is_signaled

/* ── drm_atomic: 4.4 미구현 -> stub (drm_compat.c 에서 구현) ── */
struct drm_atomic_state *drm_atomic_state_alloc(struct drm_device *dev);
void drm_atomic_state_free(struct drm_atomic_state *state);

/* ── reservation_object <-> dma_resv 매핑 ──
 * 5.4 소스: dma_resv_* 사용 (전환기, reservation_object 병존)
 * 4.4 커널: reservation_object 전용
 * 상세 accessor 매핑은 reservation_compat.h 참조.
 */
#define dma_resv                reservation_object
#define dma_resv_lock           reservation_object_lock
#define dma_resv_unlock         reservation_object_unlock
#define dma_resv_init           reservation_object_init
#define dma_resv_fini           reservation_object_fini
#define dma_resv_trylock        reservation_object_trylock

/* ── GEM object ref/put 통합 shim ──
 * 5.4: drm_gem_object_put() 및 drm_gem_object_put_unlocked() 혼용
 * 4.4: drm_gem_object_unreference_unlocked() 단독
 */
static inline void drm_gem_object_put(struct drm_gem_object *obj)
{
	drm_gem_object_unreference_unlocked(obj);
}
static inline void drm_gem_object_put_unlocked(struct drm_gem_object *obj)
{
	drm_gem_object_unreference_unlocked(obj);
}

#endif /* _REDPILL_DRM_COMPAT_H */
