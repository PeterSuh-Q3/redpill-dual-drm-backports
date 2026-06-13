/*
 * kcl_compat.h - kernel compatibility shim for building DC-era amdgpu/ttm
 * on the Synology 4.4.302 kernel (geminilakeamdgpu virtual platform).
 *
 * Force-included via Makefile ccflags-y += -include .../kcl_compat.h
 * Each block includes the real kernel header FIRST (guarded), then provides
 * the missing newer API as inline/macro shims.
 */
#ifndef _MSHELL_KCL_COMPAT_H
#define _MSHELL_KCL_COMPAT_H

#include <linux/version.h>

/* --- mm: get_user_pages / release_pages signature backport --- */
#include <linux/mm.h>
#include <linux/pagemap.h>
/*
 * 4.4: long get_user_pages(tsk, mm, start, nr_pages, gup_flags, pages, vmas)
 * amdgpu(4.15): get_user_pages(start, nr_pages, gup_flags, pages, vmas)
 */
#ifndef MSHELL_KCL_GUP
#define MSHELL_KCL_GUP
#define get_user_pages(start, nr_pages, gup_flags, pages, vmas) \
	get_user_pages(current, current->mm, start, nr_pages, gup_flags, pages, vmas)
#endif
/*
 * 4.4: void release_pages(pages, nr, cold)
 * amdgpu(4.15): release_pages(pages, nr)
 */
#ifndef MSHELL_KCL_RELPAGES
#define MSHELL_KCL_RELPAGES
#define release_pages(pages, nr) release_pages(pages, nr, false)
#endif

/* --- reservation: _lock_interruptible (added ~4.13) --- */
#include <linux/reservation.h>
static inline int
mshell_reservation_object_lock_interruptible(struct reservation_object *obj,
					     struct ww_acquire_ctx *ctx)
{
	return ww_mutex_lock_interruptible(&obj->lock, ctx);
}
#define reservation_object_lock_interruptible(obj, ctx) \
	mshell_reservation_object_lock_interruptible(obj, ctx)

/* --- idr_remove: 4.4 returns void; 4.11+ returns the removed pointer --- */
#include <linux/idr.h>
static inline void *mshell_idr_remove(struct idr *idr, int id)
{
	void *p = idr_find(idr, id);
	idr_remove(idr, id);
	return p;
}
#define idr_remove(idr, id) mshell_idr_remove(idr, id)

/* --- reservation_object_copy_fences (added ~4.13) --- */
#include <linux/dma-fence.h>
static inline int
mshell_reservation_object_copy_fences(struct reservation_object *dst,
				      struct reservation_object *src)
{
	struct dma_fence *excl = NULL, **shared = NULL;
	unsigned int shared_count = 0, i;
	int r;

	r = reservation_object_get_fences_rcu(src, &excl, &shared_count, &shared);
	if (r)
		return r;

	for (i = 0; i < shared_count; i++) {
		r = reservation_object_reserve_shared(dst);
		if (r)
			goto out;
	}
	for (i = 0; i < shared_count; i++)
		reservation_object_add_shared_fence(dst, shared[i]);
	reservation_object_add_excl_fence(dst, excl);
	r = 0;
out:
	if (excl)
		dma_fence_put(excl);
	for (i = 0; i < shared_count; i++)
		dma_fence_put(shared[i]);
	kfree(shared);
	return r;
}
#define reservation_object_copy_fences(dst, src) \
	mshell_reservation_object_copy_fences(dst, src)

/* --- i915 5.4-specific Kconfig strings absent in 4.4.302 --- */
#ifndef CONFIG_DRM_I915_FORCE_PROBE
#define CONFIG_DRM_I915_FORCE_PROBE ""
#endif
#ifndef CONFIG_DRM_I915_SPIN_REQUEST
#define CONFIG_DRM_I915_SPIN_REQUEST 0
#endif

/* --- slab: SLAB_TYPESAFE_BY_RCU renamed from SLAB_DESTROY_BY_RCU (4.14+) --- */
#include <linux/slab.h>
#ifndef SLAB_TYPESAFE_BY_RCU
#define SLAB_TYPESAFE_BY_RCU SLAB_DESTROY_BY_RCU
#endif

/* --- pm: DPM_FLAG_NEVER_SKIP (5.1+), dev_pm_set_driver_flags (5.1+) --- */
#include <linux/pm.h>
#ifndef DPM_FLAG_NEVER_SKIP
#define DPM_FLAG_NEVER_SKIP 0
#endif
static inline void dev_pm_set_driver_flags(struct device *dev, u32 flags) { }

/* --- ratelimit: RATELIMIT_MSG_ON_RELEASE + ratelimit_set_flags (4.15+) --- */
#include <linux/ratelimit.h>
#ifndef RATELIMIT_MSG_ON_RELEASE
#define RATELIMIT_MSG_ON_RELEASE (1 << 0)
#endif
static inline void ratelimit_set_flags(struct ratelimit_state *rs, unsigned long flags) { }

/* --- sysctl: SYSCTL_ZERO / SYSCTL_ONE (5.2+) --- */
#include <linux/sysctl.h>
#ifndef SYSCTL_ZERO
extern int sysctl_vals[];
#define SYSCTL_ZERO (&sysctl_vals[0])
#define SYSCTL_ONE  (&sysctl_vals[1])
#endif

/* --- device links: DL_FLAG_STATELESS (4.13+) --- */
#include <linux/device.h>
#ifndef DL_FLAG_STATELESS
#define DL_FLAG_STATELESS 0
#endif

/* --- cpuhp: CPU hotplug states/helpers (4.7+) --- */
#include <linux/cpu.h>
#ifndef CPUHP_INVALID
#define CPUHP_INVALID    -1
#define CPUHP_AP_ONLINE_DYN 130
enum cpuhp_state;
static inline int cpuhp_setup_state_multi(int state, const char *name,
	int (*startup)(unsigned int cpu, struct hlist_node *node),
	int (*teardown)(unsigned int cpu, struct hlist_node *node))
{ return CPUHP_INVALID; }
static inline int cpuhp_state_add_instance(int state, struct hlist_node *node)
{ return 0; }
static inline int cpuhp_state_remove_instance(int state, struct hlist_node *node)
{ return 0; }
static inline void cpuhp_remove_multi_state(int state) { }
#endif

/* --- pm_runtime: pm_runtime_suspended_time (4.6+) --- */
#include <linux/pm_runtime.h>
static inline u64 pm_runtime_suspended_time(struct device *dev) { return 0; }

/* --- pci: resizable BAR (added ~4.15) — stub out (disable rebar) --- */
#include <linux/pci.h>
static inline void mshell_pci_release_resource(struct pci_dev *dev, int resno) { }
static inline int  mshell_pci_resize_resource(struct pci_dev *dev, int resno, int size)
{
	return -ENOTSUPP; /* amdgpu treats -ENOTSUPP as "no resizable BAR", continues */
}
#define pci_release_resource(dev, resno)        mshell_pci_release_resource(dev, resno)
#define pci_resize_resource(dev, resno, size)   mshell_pci_resize_resource(dev, resno, size)

#endif /* _MSHELL_KCL_COMPAT_H */
