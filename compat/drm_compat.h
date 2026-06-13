/* SPDX-License-Identifier: GPL-2.0 */
/*
 * compat/drm_compat.h — 공통 DRM compat (현재 거의 비어 있음)
 *
 * ★ Rev3: backport-include 헤더 교체 방식으로 전환.
 *   - dma_fence: 4.4.302 네이티브.
 *   - dma_resv 매핑: compat/backport-include/linux/dma-resv.h 로 이전(헤더 교체).
 *   - drmP.h/drm_gem.h 전역 force-include 제거(매 TU 에 drm 체인을 끌어와
 *     linux/sched/signal.h·hdr_sink_metadata cascade 를 유발했음).
 *   더 이상 force-include 하지 않는다. 필요한 shim 은 backport-include 가 담당.
 */
#ifndef _REDPILL_DRM_COMPAT_H
#define _REDPILL_DRM_COMPAT_H

/* (공통 추가 shim 없음 — backport-include 헤더 레이어가 처리) */

#endif /* _REDPILL_DRM_COMPAT_H */
