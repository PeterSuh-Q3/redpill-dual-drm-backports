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
