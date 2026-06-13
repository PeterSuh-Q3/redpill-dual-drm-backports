/* SPDX-License-Identifier: GPL-2.0 */
/* backport stub: CEC notifier API (4.14+) */
#ifndef _BPI_MEDIA_CEC_NOTIFIER_H
#define _BPI_MEDIA_CEC_NOTIFIER_H

struct cec_notifier;
struct edid;

static inline struct cec_notifier *
cec_notifier_get_conn(struct device *dev, const char *conn) { return NULL; }
static inline void cec_notifier_put(struct cec_notifier *n) { }
static inline void
cec_notifier_set_phys_addr_from_edid(struct cec_notifier *n,
				     const struct edid *edid) { }
static inline void
cec_notifier_phys_addr_invalidate(struct cec_notifier *n) { }

#endif
