// SPDX-License-Identifier: MIT
/*
 * drivers/gpu/drm/i915/intel_device_info_cml_hs.c (cherry-pick 발췌)
 *
 * 설계서 §9.3 / §9.4 기준. CML-H/S device_info 구조체 및 pciidlist[] 엔트리.
 *
 * 적용 위치:
 *   - 아래 intel_cometlake_h_info 정의는 intel_device_info.c 에 병합.
 *   - INTEL_CML_*_IDS(...) 엔트리는 i915_pci.c 의 pciidlist[] 에 병합.
 *
 * cherry-pick 적용 순서 (§9.4):
 *   1. i915_pciids.h        ID 매크로 정의 (의존성 없음, 가장 먼저)
 *   2. intel_device_info.c  device_info 구조체 연결  <-- 이 파일
 *   3. i915_pci.c           pciidlist[] 엔트리 등록
 *   4. intel_crt.c          CML-S VGA 처리 (CONFIG_DRM_I915_CML_S_VGA=n 시 생략)
 *   5. cml_hs_compat.h      추가 심볼 처리 확인
 */

/* ── intel_device_info.c 추가 항목 ──
 * CML-H/S 는 GEN9.5 공용 구조체 재사용 -> 별도 info 최소화.
 * GT2 H variant 만 별도 info 사용.
 */
#if 0 /* 아래는 intel_device_info.c 에 병합할 발췌 (참조용) */

static const struct intel_device_info intel_cometlake_h_info = {
	GEN9_FEATURES,
	PLATFORM(INTEL_COMETLAKE),
	.gt = 2,
};

#endif

/* ── i915_pci.c 의 pciidlist[] 에 추가할 엔트리 ──
 *
 *   INTEL_CML_H_GT1_IDS(&intel_cometlake_info),    // GT1: cometlake_info 공유
 *   INTEL_CML_H_GT2_IDS(&intel_cometlake_h_info),  // GT2: H variant
 *   INTEL_CML_S_GT1_IDS(&intel_cometlake_info),
 *   INTEL_CML_S_GT2_IDS(&intel_cometlake_h_info),
 */
