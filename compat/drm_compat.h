/* SPDX-License-Identifier: GPL-2.0 */
/*
 * compat/drm_compat.h
 * 4.4.302(Synology geminilake) <-> 5.4 LTS DRM core 호환 레이어 (공통).
 * i915 + amdgpu 공용.
 *
 * ★ Rev: 1차 심볼-갭 분석(docs/SYMBOL-GAP-Rev1.md) 반영.
 *   Synology geminilake 4.4.302 는 i915 를 CannonLake(GEN10)까지 백포트하며
 *   dma_fence / drm_gem_object_put(_unlocked) / drm_atomic / i2c_lock_bus 를
 *   이미 네이티브로 제공한다. 따라서 설계서 §6 원안의 해당 매핑은 **제거**한다
 *   (매크로/inline 재정의 시 충돌). 4.4.302 에 부재해 shim 이 필요한 공통 항목은
 *   dma_resv(타입/accessor) 뿐이다.
 *
 * shim 방향: 5.4 소스가 호출하는 심볼 -> 4.4.302 커널 심볼로 연결.
 */
#ifndef _REDPILL_DRM_COMPAT_H
#define _REDPILL_DRM_COMPAT_H

#include <linux/version.h>
#include <linux/dma-fence.h>      /* 4.4.302 제공: dma_fence_* 네이티브 (매핑 불필요) */
#include <linux/reservation.h>    /* 4.4.302 제공: reservation_object (dma_resv 는 부재) */
#include <drm/drmP.h>
#include <drm/drm_gem.h>          /* 4.4.302 제공: drm_gem_object_put(_unlocked) */

/* ── dma_resv <- reservation_object 매핑 (유일한 필수 공통 shim) ──
 * 5.4 소스: dma_resv / dma_resv_* (313회) 사용.
 * 4.4.302: reservation_object 만 제공, dma_resv 타입/함수 부재.
 * 상세 accessor 는 reservation_compat.h 에서 보강.
 */
#define dma_resv                reservation_object
#define dma_resv_lock           reservation_object_lock
#define dma_resv_unlock         reservation_object_unlock
#define dma_resv_init           reservation_object_init
#define dma_resv_fini           reservation_object_fini
#define dma_resv_trylock        reservation_object_trylock

/* dma_resv accessor/유틸 상세 매핑 (force-include 시 함께 제공) */
#include "reservation_compat.h"

/*
 * 제거됨(4.4.302 네이티브 제공 — 재정의 충돌 방지). 근거: docs/SYMBOL-GAP-Rev1.md
 *   - dma_fence_ops/init/signal/get/put/is_signaled   (linux/dma-fence.h)
 *   - drm_atomic_state_alloc / drm_atomic_state_free   (drm/drm_atomic.h 존재)
 *   - drm_gem_object_put / drm_gem_object_put_unlocked (drm/drm_gem.h)
 */

#endif /* _REDPILL_DRM_COMPAT_H */
