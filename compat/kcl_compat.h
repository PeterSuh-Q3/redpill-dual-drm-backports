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

/* --- MODULE_FIRMWARE / MODULE_PARM_DESC: Synology 4.4.302 fails on string
 * concatenation args. Replace with no-ops (modinfo annotations only). */
#include <linux/module.h>
#ifdef MODULE_FIRMWARE
#undef MODULE_FIRMWARE
#endif
#define MODULE_FIRMWARE(_fw) /* noop */

/* --- math: DIV64_U64_ROUND_UP (5.2+) --- */
#ifndef DIV64_U64_ROUND_UP
#define DIV64_U64_ROUND_UP(x, y) div64_u64((x) + (y) - 1, (y))
#endif

/* --- wait: wake_up_var / wait_var_event_killable (4.19+) --- */
#include <linux/wait.h>
#ifndef wake_up_var
#define wake_up_var(var) wake_up_bit(var, 0)
#define wait_var_event_killable(var, cond) \
    wait_on_bit(var, 0, TASK_KILLABLE)
#define init_var_waiter(wq) /* noop */
#endif

/* --- refcount: refcount_dec_and_lock_irqsave (5.1+) --- */
#include <linux/refcount.h>
#ifndef refcount_dec_and_lock_irqsave
#define refcount_dec_and_lock_irqsave(r, lock, flags) \
    ({ (void)(flags); refcount_dec_and_lock(r, lock); })
#endif

/* --- xarray (4.20+): force-include shim so all files get xa_* --- */
#include <linux/xarray.h>

/* --- mm: totalram_pages() function (5.1+); 4.4 uses a global variable --- */
#ifndef _BPI_TOTALRAM_PAGES_FN
#define _BPI_TOTALRAM_PAGES_FN
#define totalram_pages() (totalram_pages)
#endif

/* --- mm: mmgrab (4.11+) --- */
#include <linux/sched.h>
#ifndef mmgrab
static inline void mmgrab(struct mm_struct *mm)
{
    atomic_inc(&mm->mm_count);
}
#endif

/* --- i915: CONFIG_DRM_I915_USERFAULT_AUTOSUSPEND (not in 4.4) --- */
#ifndef CONFIG_DRM_I915_USERFAULT_AUTOSUSPEND
#define CONFIG_DRM_I915_USERFAULT_AUTOSUSPEND 0
#endif

/* --- vm: call_mmap (5.x VM API) --- */
#ifndef call_mmap
static inline int call_mmap(struct file *file, struct vm_area_struct *vma)
{
    return file->f_op->mmap(file, vma);
}
#endif

/* --- rcu: rcu_swap_protected (4.19+) --- */
#ifndef rcu_swap_protected
#define rcu_swap_protected(rcu, ptr, cond) \
    ({ typeof(ptr) _ptr = rcu_dereference_protected((rcu), (cond)); \
       rcu_assign_pointer((rcu), (ptr)); _ptr; })
#endif

/* --- radix_tree: radix_tree_iter_delete (4.14+) --- */
#include <linux/radix-tree.h>
#ifndef radix_tree_iter_delete
static inline void
radix_tree_iter_delete(struct radix_tree_root *root,
                       struct radix_tree_iter *iter, void **slot)
{
    radix_tree_delete(root, iter->index);
}
#endif

/* --- workqueue: flush_rcu_work (5.x) --- */
#include <linux/workqueue.h>
struct rcu_work;
#ifndef flush_rcu_work
#define flush_rcu_work(w) flush_work(&(w)->work)
#endif

/* --- dma: dma_map_page_attrs (4.6+) --- */
#include <linux/dma-mapping.h>
#ifndef dma_map_page_attrs
#define dma_map_page_attrs(dev, page, off, sz, dir, attr) \
    dma_map_page(dev, page, off, sz, dir)
#endif

/* --- lockdep: lockdep_is_held_type / lockdep_assert_irqs_disabled (5.x) --- */
#ifndef lockdep_is_held_type
#define lockdep_is_held_type(lock, type) 1
#endif
#ifndef lockdep_assert_irqs_disabled
#define lockdep_assert_irqs_disabled() do { } while (0)
#endif

/* --- time: time_after32 (5.x) --- */
#ifndef time_after32
#define time_after32(a, b) ((u32)(b) - (u32)(a) > (1U << 31))
#endif

/* --- atomic: try_cmpxchg (5.5+) --- */
#ifndef try_cmpxchg
#define try_cmpxchg(p, old, new) \
    ({ typeof(*old) _o = *(old); \
       *(old) = cmpxchg((p), _o, (new)); \
       *(old) == _o; })
#endif

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
