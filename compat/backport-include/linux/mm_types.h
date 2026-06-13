/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: vm_fault_t (~4.17). */
#ifndef _BPI_LINUX_MM_TYPES_H
#define _BPI_LINUX_MM_TYPES_H
#include_next <linux/mm_types.h>
#ifndef _BPI_HAVE_VM_FAULT_T
#define _BPI_HAVE_VM_FAULT_T
typedef int vm_fault_t;
#endif
#endif
