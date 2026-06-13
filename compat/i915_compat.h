/* SPDX-License-Identifier: GPL-2.0 */
/*
 * compat/i915_compat.h
 * i915 전용 4.4 <-> 5.4 호환 레이어. 설계서 §6.2 기준.
 */
#ifndef _REDPILL_I915_COMPAT_H
#define _REDPILL_I915_COMPAT_H

#include <linux/pm_runtime.h>
#include "drm_compat.h"

struct intel_runtime_pm;
struct drm_i915_private;

/* ── intel_runtime_pm wakeref wrapper ──
 * 5.4: intel_wakeref_t 타입 + rpm 구조체 기반 API
 * 4.4: pm_runtime_get_sync() 직접 호출 방식
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

/* ── ICL Display Core 초기화 ──
 * 주의: stub 이 아닌 실제 구현 필요 (i915_compat.c bridge).
 * 4.4 의 drm_display infra 와 연결하는 브릿지 함수로 작성.
 */
void icl_display_core_init(struct drm_i915_private *dev_priv, bool resume);
void icl_display_core_uninit(struct drm_i915_private *dev_priv);

/* ── i2c adapter lock API 변경 대응 ──
 * 5.4: i2c_lock_bus() 사용
 * 4.4: i2c_lock_adapter() 사용
 */
#define i2c_lock_bus(adap, flags)   i2c_lock_adapter(adap)
#define i2c_unlock_bus(adap, flags) i2c_unlock_adapter(adap)

#endif /* _REDPILL_I915_COMPAT_H */
