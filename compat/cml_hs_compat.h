/* SPDX-License-Identifier: GPL-2.0 */
/*
 * compat/cml_hs_compat.h
 * CML-H/S Cherry-pick 커밋 적용 후 발생하는 추가 심볼 처리.
 * 설계서 §6.4 기준.
 */
#ifndef _REDPILL_CML_HS_COMPAT_H
#define _REDPILL_CML_HS_COMPAT_H

struct drm_i915_private;

/* CML-H/S 는 GEN9.5 공용 경로 사용 -> 별도 shim 최소화.
 * intel_cometlake_info 구조체를 공유하므로 초기화 shim 불필요.
 * PCI ID 추가만으로 동작 예상.
 */

/* CML-S 데스크탑의 아날로그 출력(VGA) 비활성화 처리.
 * DSM 환경에서는 CONFIG_DRM_I915_CML_S_VGA=n 이 기본.
 */
#ifdef CONFIG_DRM_I915_CML_S_VGA
/* CML-S VGA 출력: intel_crt.c 초기화 경로 선택적 포함 */
#else
static inline void intel_crt_init(struct drm_i915_private *dev_priv) {}
#endif

#endif /* _REDPILL_CML_HS_COMPAT_H */
