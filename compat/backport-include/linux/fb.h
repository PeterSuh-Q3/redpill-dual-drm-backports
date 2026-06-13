/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: remove_conflicting_pci_framebuffers (5.x; 4.4.302 부재)
 * 헤드리스 환경 — efifb/vesafb 인계 불필요 → stub(0). */
#ifndef _BPI_LINUX_FB_H
#define _BPI_LINUX_FB_H
#include_next <linux/fb.h>
#ifndef FBINFO_HIDE_SMEM_START
#define FBINFO_HIDE_SMEM_START 0
#endif
#ifndef _BPI_HAVE_REMOVE_CONFLICTING_PCI_FB
#define _BPI_HAVE_REMOVE_CONFLICTING_PCI_FB
struct pci_dev;
static inline int
remove_conflicting_pci_framebuffers(struct pci_dev *pdev, int res_id, const char *name)
{
	return 0;
}
#endif
#endif
