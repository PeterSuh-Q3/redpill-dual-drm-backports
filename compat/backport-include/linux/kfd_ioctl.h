/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: linux/kfd_ioctl.h
 * KFD(Kernel Fusion Driver) HSA ioctl 헤더. 4.4.302에 없음.
 * CONFIG_HSA_AMD=n 환경에서는 실제로 include되지 않으나 선제 shim 제공. */
#ifndef _BPI_LINUX_KFD_IOCTL_H
#define _BPI_LINUX_KFD_IOCTL_H
/* empty stub — HSA compute path is not built without CONFIG_HSA_AMD */
#endif
