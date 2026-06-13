/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: struct rcu_work / queue_rcu_work (~4.18). */
#ifndef _BPI_LINUX_WORKQUEUE_H
#define _BPI_LINUX_WORKQUEUE_H
#include_next <linux/workqueue.h>
#ifndef _BPI_HAVE_RCU_WORK
#define _BPI_HAVE_RCU_WORK
#include <linux/rcupdate.h>
struct rcu_work {
	struct work_struct work;
	struct rcu_head rcu;
	struct workqueue_struct *wq;
};
static inline struct rcu_work *to_rcu_work(struct work_struct *w)
{ return container_of(w, struct rcu_work, work); }
#define INIT_RCU_WORK(_work, _func) INIT_WORK(&(_work)->work, (_func))
static inline bool queue_rcu_work(struct workqueue_struct *wq, struct rcu_work *rwork)
{
	rwork->wq = wq;
	return queue_work(wq, &rwork->work);
}
#endif
#endif
