/* SPDX-License-Identifier: GPL-2.0 */
/*
 * compat/i915_compat.h — i915 전용 shim (현재 비어 있음)
 *
 * ★ Rev2: Step4 빌드로 판명 — intel_wakeref_t / intel_runtime_pm 은 커널 API 가 아니라
 *   **5.4 i915 내부 타입**(intel_wakeref.h / intel_runtime_pm.h 에서 자체 정의).
 *   우리는 5.4 i915 소스를 그대로 컴파일하므로 이 타입들이 함께 제공된다.
 *   설계서 §6.2 / SYMBOL-GAP-Rev1 의 "4.4.302 부재 → shim 필요" 판정은
 *   host(구형 i915) 기준 오판이었음 — 5.4 소스가 공급하므로 shim 불필요.
 *   (force-include 시 5.4 정의보다 앞서 incomplete struct 를 deref → 컴파일 파괴했음)
 *
 *   i2c_lock_bus 도 4.4.302 네이티브 제공으로 제거됨(Rev1).
 *
 * → i915 가 호출하는 *커널* API 중 4.4.302 에 없는 것이 Step4 빌드에서 추가로
 *   드러나면 그때 표적 shim 을 여기에 추가한다. 현재는 없음.
 */
#ifndef _REDPILL_I915_COMPAT_H
#define _REDPILL_I915_COMPAT_H

#include "drm_compat.h"   /* dma_resv 매핑 등 공통 shim */

/* (i915 전용 추가 shim 없음) */

#endif /* _REDPILL_I915_COMPAT_H */
