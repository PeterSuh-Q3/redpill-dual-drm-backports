/* SPDX-License-Identifier: GPL-2.0 */
/*
 * 4.4.302 compat: CONFIG_IOSF_MBI=y but symbols not EXPORT_SYMBOL'd.
 * Force the #else (stub) branch in the original header by undefining
 * CONFIG_IOSF_MBI before include_next.
 */
#ifndef _BACKPORT_ASM_IOSF_MBI_H
#define _BACKPORT_ASM_IOSF_MBI_H

#ifdef CONFIG_IOSF_MBI
#undef CONFIG_IOSF_MBI
#endif

#include_next <asm/iosf_mbi.h>

#endif /* _BACKPORT_ASM_IOSF_MBI_H */
