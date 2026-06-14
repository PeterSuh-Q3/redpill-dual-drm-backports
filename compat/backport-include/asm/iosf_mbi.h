/* SPDX-License-Identifier: GPL-2.0 */
/*
 * 4.4.302 compat: CONFIG_IOSF_MBI=y but symbols are NOT EXPORT_SYMBOL'd.
 * Replace entire header with inline stubs so i915 builds without unresolved
 * references. include_next is intentionally absent — the real header would
 * expose extern declarations that cannot be resolved at module load time.
 */
#ifndef _BACKPORT_ASM_IOSF_MBI_H
#define _BACKPORT_ASM_IOSF_MBI_H

#include <linux/notifier.h>

static inline void iosf_mbi_punit_acquire(void) {}
static inline void iosf_mbi_punit_release(void) {}
static inline void iosf_mbi_assert_punit_acquired(void) {}

static inline int
iosf_mbi_register_pmic_bus_access_notifier(struct notifier_block *nb)
{ return 0; }

static inline int
iosf_mbi_unregister_pmic_bus_access_notifier_unlocked(struct notifier_block *nb)
{ return 0; }

/* Prevent the real kernel header from being pulled in again */
#define _ASM_X86_IOSF_MBI_H

#endif /* _BACKPORT_ASM_IOSF_MBI_H */
