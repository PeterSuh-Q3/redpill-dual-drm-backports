// SPDX-License-Identifier: GPL-2.0
/*
 * compat/drm_compat.c
 * drm_compat.h 에서 선언한 비-inline shim 의 구현.
 *
 * 설계서 §6.1: drm_atomic 은 4.4 미구현 -> stub.
 * 단, i915 5.4 의 atomic modeset 경로가 실제로 호출하는 부분은
 * Step 4 단독 빌드에서 undefined reference 로 드러나므로
 * 그때 stub -> 실제 브릿지로 승격한다.
 */
#include <linux/module.h>
#include <linux/slab.h>
#include "drm_compat.h"

struct drm_atomic_state *drm_atomic_state_alloc(struct drm_device *dev)
{
	struct drm_atomic_state *state;

	state = kzalloc(sizeof(*state), GFP_KERNEL);
	if (!state)
		return NULL;

	/* TODO(Step4): 4.4 drm_modeset infra 와 연결.
	 * 현재는 alloc/free 만 제공하는 최소 stub.
	 */
	return state;
}
EXPORT_SYMBOL(drm_atomic_state_alloc);

void drm_atomic_state_free(struct drm_atomic_state *state)
{
	kfree(state);
}
EXPORT_SYMBOL(drm_atomic_state_free);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Redpill Dual DRM 4.4<->5.4 compat shim (common)");
