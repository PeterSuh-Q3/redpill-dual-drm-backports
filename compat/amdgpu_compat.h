/* SPDX-License-Identifier: GPL-2.0 */
/*
 * compat/amdgpu_compat.h
 * amdgpu 전용 4.4 <-> 5.4 호환 레이어. 설계서 §6.3 기준.
 */
#ifndef _REDPILL_AMDGPU_COMPAT_H
#define _REDPILL_AMDGPU_COMPAT_H

#include <linux/fence.h>
#include <linux/mm_types.h>
#include "drm_compat.h"

/* ── ttm_resource 관련 ──
 * 주의: ttm_resource 구조체는 5.10 에서 도입됨.
 * 5.4 OOT 소스는 이미 ttm_mem_reg 사용 -> 이 shim 불필요.
 * (5.10 OOT 적용 시에만 필요. 본 프로젝트는 5.4 기준이므로 비워둠.)
 */

/* ── amdkfd: 4.4 미지원 -> stub 처리 ──
 * HSA(Heterogeneous System Architecture) 미지원 환경 대응.
 */
static inline int  amdgpu_amdkfd_init(void) { return 0; }
static inline void amdgpu_amdkfd_fini(void) {}
static inline bool amdkfd_fence_check_mm(struct fence *f,
					 struct mm_struct *mm)
{
	return false;
}

/* ── amdgpu_vm_bo_update 시그니처 차이 대응 ──
 * 필요 시 버전별 래퍼 추가 (Step 5 단독 빌드에서 undefined 발생 시 보강).
 */

#endif /* _REDPILL_AMDGPU_COMPAT_H */
