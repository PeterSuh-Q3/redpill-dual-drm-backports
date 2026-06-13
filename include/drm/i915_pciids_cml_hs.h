/* SPDX-License-Identifier: MIT */
/*
 * include/drm/i915_pciids_cml_hs.h
 *
 * Comet Lake-H/S PCI Device ID 매크로 (Cherry-pick from Linux 5.5-rc1).
 * 설계서 §9.2 기준.
 *
 * 적용 방식:
 *   include/drm/i915_pciids.h 의 CML 정의 블록 직후에 이 헤더를 include 하거나,
 *   해당 매크로들을 i915_pciids.h 본문에 직접 cherry-pick 한다.
 *   (scripts/03_cherrypick_cml_hs.sh 참조)
 *
 * GEN9.5 (CML-U 와 동일 아키텍처) -> intel_cometlake_info 공유.
 * GT2 H variant 만 intel_cometlake_h_info 사용 (intel_device_info.c).
 */
#ifndef _I915_PCIIDS_CML_HS_H
#define _I915_PCIIDS_CML_HS_H

/* Comet Lake-H GT1 */
#define INTEL_CML_H_GT1_IDS(info) \
	INTEL_VGA_DEVICE(0x9BC8, info)

/* Comet Lake-H GT2 */
#define INTEL_CML_H_GT2_IDS(info) \
	INTEL_VGA_DEVICE(0x9BC4, info), \
	INTEL_VGA_DEVICE(0x9BC5, info), \
	INTEL_VGA_DEVICE(0x9BC6, info), \
	INTEL_VGA_DEVICE(0x9BCA, info), \
	INTEL_VGA_DEVICE(0x9BCB, info), \
	INTEL_VGA_DEVICE(0x9BCC, info)

/* Comet Lake-S GT1 */
#define INTEL_CML_S_GT1_IDS(info) \
	INTEL_VGA_DEVICE(0x9BC0, info)

/* Comet Lake-S GT2 */
#define INTEL_CML_S_GT2_IDS(info) \
	INTEL_VGA_DEVICE(0x9B41, info), \
	INTEL_VGA_DEVICE(0x9BA4, info), \
	INTEL_VGA_DEVICE(0x9BA8, info), \
	INTEL_VGA_DEVICE(0x9BAA, info), \
	INTEL_VGA_DEVICE(0x9BAB, info), \
	INTEL_VGA_DEVICE(0x9BAC, info)

#endif /* _I915_PCIIDS_CML_HS_H */
