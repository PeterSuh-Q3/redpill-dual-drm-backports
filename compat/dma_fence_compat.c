// SPDX-License-Identifier: GPL-2.0
/*
 * compat/dma_fence_compat.c
 * fence (4.4) -> dma_fence (5.4) 브릿지.
 *
 * 대부분의 매핑은 drm_compat.h 의 #define 으로 처리되지만,
 * 매크로만으로 해결되지 않는 시그니처 차이(예: dma_fence_wait_timeout
 * 의 반환 타입/인자)는 여기서 래퍼로 보강한다.
 */
#include <linux/module.h>
#include <linux/fence.h>
#include "drm_compat.h"

/* 5.4 소스가 호출하는 dma_fence_wait_timeout 시그니처를
 * 4.4 fence_wait_timeout 으로 연결하는 래퍼 예시.
 */
signed long
dma_fence_wait_timeout_compat(struct fence *fence, bool intr,
			      signed long timeout)
{
	return fence_wait_timeout(fence, intr, timeout);
}
EXPORT_SYMBOL(dma_fence_wait_timeout_compat);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Redpill Dual DRM fence compat bridge");
