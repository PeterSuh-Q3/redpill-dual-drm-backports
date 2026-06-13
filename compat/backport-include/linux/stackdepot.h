/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: linux/stackdepot.h (~4.6, 4.4.302 부재)
 * drm_mm.h / i915_drv.h 가 디버그(stack tracking)용으로 참조.
 * CONFIG_DRM_DEBUG_MM 등 디버그 옵션 off 시 미사용 → 컴파일용 최소 stub. */
#ifndef _BPI_LINUX_STACKDEPOT_H
#define _BPI_LINUX_STACKDEPOT_H
#include <linux/gfp.h>
typedef u32 depot_stack_handle_t;
static inline depot_stack_handle_t
stack_depot_save(unsigned long *entries, unsigned int nr, gfp_t gfp) { return 0; }
static inline unsigned int
stack_depot_fetch(depot_stack_handle_t handle, unsigned long **entries) { *entries = NULL; return 0; }
static inline int stack_depot_init(void) { return 0; }
#endif
