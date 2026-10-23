/* SPDX-License-Identifier: GPL-2.0 OR MIT */
/*
 * K4 backport of the legacy VMware hypercall interface used by Linux v5.4
 * vmwgfx. Synology's 4.4.302 headers do not ship asm/vmware.h. This keeps the
 * VMware port-based ABI supported by Fusion/ESXi; it does not emulate VMware
 * detection or issue hypercalls from non-VMware paths.
 */
#ifndef _BPI_ASM_VMWARE_H
#define _BPI_ASM_VMWARE_H

#define VMWARE_HYPERVISOR_PORT    0x5658
#define VMWARE_HYPERVISOR_PORT_HB 0x5659

#define VMWARE_HYPERVISOR_HB  (1U << 0)
#define VMWARE_HYPERVISOR_OUT (1U << 1)

#define VMWARE_HYPERCALL \
	"movw $0x5658, %%dx; inl (%%dx), %%eax"
#define VMWARE_HYPERCALL_HB_OUT \
	"movw $0x5659, %%dx; rep outsb;"
#define VMWARE_HYPERCALL_HB_IN \
	"movw $0x5659, %%dx; rep insb;"

#endif /* _BPI_ASM_VMWARE_H */
