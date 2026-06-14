/* SPDX-License-Identifier: GPL-2.0 */
/*
 * 4.4.302 compat: iosf_mbi symbols not exported by DSM kernel.
 * Stub all PMIC bus access functions — headless VAAPI needs no PMIC arbitration.
 */
#ifndef _BACKPORT_ASM_IOSF_MBI_H
#define _BACKPORT_ASM_IOSF_MBI_H

#include_next <asm/iosf_mbi.h>

/* Override extern declarations with static inline stubs */
#undef iosf_mbi_punit_acquire
#undef iosf_mbi_punit_release
#undef iosf_mbi_assert_punit_acquired
#undef iosf_mbi_register_pmic_bus_access_notifier
#undef iosf_mbi_unregister_pmic_bus_access_notifier_unlocked

static inline void iosf_mbi_punit_acquire(void) {}
static inline void iosf_mbi_punit_release(void) {}
static inline void iosf_mbi_assert_punit_acquired(void) {}
static inline int iosf_mbi_register_pmic_bus_access_notifier(
		struct notifier_block *nb) { return 0; }
static inline int iosf_mbi_unregister_pmic_bus_access_notifier_unlocked(
		struct notifier_block *nb) { return 0; }

#endif /* _BACKPORT_ASM_IOSF_MBI_H */
