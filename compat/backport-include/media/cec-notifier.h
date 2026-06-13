/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: media/cec-notifier.h (4.4.302 부재). 헤드리스 → 빈 stub. */
#ifndef _BPI_MEDIA_CEC_NOTIFIER_H
#define _BPI_MEDIA_CEC_NOTIFIER_H
struct cec_notifier;
struct device;
struct edid;
static inline struct cec_notifier *cec_notifier_get(struct device *dev) { return NULL; }
static inline void cec_notifier_put(struct cec_notifier *n) {}
static inline void cec_notifier_set_phys_addr(struct cec_notifier *n, u16 pa) {}
static inline void cec_notifier_set_phys_addr_from_edid(struct cec_notifier *n, const struct edid *e) {}
#endif
