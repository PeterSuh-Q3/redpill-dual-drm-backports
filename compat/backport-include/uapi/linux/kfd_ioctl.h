/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/* backport shim: uapi/linux/kfd_ioctl.h
 * 4.4.302 kfd_ioctl.h lacks MMIO remap constants added ~5.0.
 * Values match linux 5.4 include/uapi/linux/kfd_ioctl.h (HW-fixed offsets). */
#ifndef _BPI_UAPI_LINUX_KFD_IOCTL_H
#define _BPI_UAPI_LINUX_KFD_IOCTL_H

#include_next <uapi/linux/kfd_ioctl.h>

#ifndef KFD_MMIO_REMAP_HDP_MEM_FLUSH_CNTL
#define KFD_MMIO_REMAP_HDP_MEM_FLUSH_CNTL 0xa80
#endif
#ifndef KFD_MMIO_REMAP_HDP_REG_FLUSH_CNTL
#define KFD_MMIO_REMAP_HDP_REG_FLUSH_CNTL 0xa84
#endif

#endif
