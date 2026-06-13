/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: component_add_typed (added ~4.19). 4.4.302 부재.
 * i915 audio/hdcp 컴포넌트 바인딩 — DSM 헤드리스 환경에선 non-critical.
 * stub: -ENOSYS 반환. 실제 audio codec binding 없이도 i915 로드 가능. */
#ifndef _BPI_LINUX_COMPONENT_H
#define _BPI_LINUX_COMPONENT_H
#include_next <linux/component.h>
#ifndef _BPI_HAVE_COMPONENT_ADD_TYPED
#define _BPI_HAVE_COMPONENT_ADD_TYPED
struct component_ops;
static inline int
component_add_typed(struct device *dev, const struct component_ops *ops, int subtype)
{
	return component_add(dev, ops);
}
#endif
#endif
