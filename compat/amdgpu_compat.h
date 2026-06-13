/* SPDX-License-Identifier: GPL-2.0 */
/*
 * compat/amdgpu_compat.h
 * amdgpu 전용 4.4.302 <-> 5.4 호환 레이어. 설계서 §6.3 기준.
 *
 * ★ Rev: 심볼-갭 분석(docs/SYMBOL-GAP-Rev1.md) 반영.
 *   - amdkfd stub 제거: 5.4 amdgpu 가 amdgpu_amdkfd.c 에서 amdgpu_amdkfd_init 등을
 *     자체 정의 → shim stub 은 이중정의 충돌. (HSA off 경로도 5.4 소스가 처리)
 *     또한 설계서 원안의 amdkfd_fence_check_mm(struct fence*) 은 4.4.302/5.4 모두
 *     dma_fence 를 쓰므로 시그니처 자체가 부정확.
 *   - <linux/fence.h> -> dma_fence 는 drm_compat.h(linux/dma-fence.h)에서 제공.
 *   - ttm_resource: 5.4 는 ttm_mem_reg 사용 → shim 불필요(설계서 §6.3 주석과 동일).
 */
#ifndef _REDPILL_AMDGPU_COMPAT_H
#define _REDPILL_AMDGPU_COMPAT_H

#include <linux/mm_types.h>
#include "drm_compat.h"          /* dma_fence(네이티브) + dma_resv 매핑 */

/*
 * 현재 amdgpu 전용 추가 shim 없음.
 * Step5(amdgpu 단독 빌드)에서 undefined/시그니처 불일치가 드러나면
 * 그 시점에 표적 래퍼를 여기에 추가한다. (추측 기반 stub 금지)
 *
 * 제거됨(근거: docs/SYMBOL-GAP-Rev1.md):
 *   - amdgpu_amdkfd_init/fini, amdkfd_fence_check_mm  (5.4 amdgpu 자체 정의와 충돌)
 */

#endif /* _REDPILL_AMDGPU_COMPAT_H */
