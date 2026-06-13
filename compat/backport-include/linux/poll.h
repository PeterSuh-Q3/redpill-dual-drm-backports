/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: EPOLL* 타입상수(~4.16) → POLL* 매핑. */
#ifndef _BPI_LINUX_POLL_H
#define _BPI_LINUX_POLL_H
#include_next <linux/poll.h>
#ifndef EPOLLIN
#define EPOLLIN   POLLIN
#define EPOLLOUT  POLLOUT
#define EPOLLERR  POLLERR
#define EPOLLHUP  POLLHUP
#define EPOLLPRI  POLLPRI
#define EPOLLRDNORM POLLRDNORM
#define EPOLLWRNORM POLLWRNORM
#endif
#endif
