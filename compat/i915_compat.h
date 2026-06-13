/* SPDX-License-Identifier: GPL-2.0 */
/*
 * compat/i915_compat.h
 * i915 전용 4.4.302 <-> 5.4 호환 레이어. 설계서 §6.2 기준.
 *
 * ★ Rev: 심볼-갭 분석(docs/SYMBOL-GAP-Rev1.md) 반영.
 *   - intel_wakeref_t / intel_runtime_pm: 4.4.302 부재(0파일) → shim 유지(필수).
 *   - i2c_lock_bus: 4.4.302 네이티브 제공 → 매핑 제거(충돌 방지).
 *   - icl_display_core_init/uninit: 5.4 i915 소스가 자체 구현(intel_display_power.c)
 *     → shim 선언 불필요, 제거.
 */
#ifndef _REDPILL_I915_COMPAT_H
#define _REDPILL_I915_COMPAT_H

#include <linux/pm_runtime.h>
#include "drm_compat.h"

struct intel_runtime_pm;

/* ── intel_runtime_pm wakeref wrapper (필수 shim) ──
 * 5.4: intel_wakeref_t 타입 + rpm 구조체 기반 API (248회/67파일)
 * 4.4.302: 해당 타입/리팩터 부재 → pm_runtime_get_sync() 직접 호출 방식으로 매핑
 */
typedef int intel_wakeref_t;

static inline intel_wakeref_t
intel_runtime_pm_get(struct intel_runtime_pm *rpm)
{
	pm_runtime_get_sync(rpm->kdev);
	return 1;
}

static inline void
intel_runtime_pm_put(struct intel_runtime_pm *rpm, intel_wakeref_t wref)
{
	pm_runtime_put_autosuspend(rpm->kdev);
}

/*
 * 제거됨(근거: docs/SYMBOL-GAP-Rev1.md):
 *   - i2c_lock_bus / i2c_unlock_bus  (4.4.302 linux/i2c.h 네이티브 제공)
 *   - icl_display_core_init/uninit   (5.4 i915 소스 자체 구현)
 */

#endif /* _REDPILL_I915_COMPAT_H */
