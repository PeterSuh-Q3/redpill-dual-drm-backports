/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: linux/sched/task.h
 * 4.11에서 linux/sched.h 가 서브헤더로 분리됨.
 * 4.4.302에는 존재하지 않으므로 linux/sched.h 로 리다이렉트. */
#ifndef _BPI_LINUX_SCHED_TASK_H
#define _BPI_LINUX_SCHED_TASK_H
#include <linux/sched.h>
#endif
