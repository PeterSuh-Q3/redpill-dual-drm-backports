/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: vga_remove_vgacon (5.x; 4.4.302 부재) → 헤드리스 stub(0). */
#ifndef _BPI_LINUX_VGAARB_H
#define _BPI_LINUX_VGAARB_H
#include_next <linux/vgaarb.h>
#ifndef _BPI_HAVE_VGA_REMOVE_VGACON
#define _BPI_HAVE_VGA_REMOVE_VGACON
struct pci_dev;
static inline int vga_remove_vgacon(struct pci_dev *pdev) { return 0; }
#endif
#endif
