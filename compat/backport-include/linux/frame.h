/* SPDX-License-Identifier: GPL-2.0 */
/*
 * K4.4 compatibility: STACK_FRAME_NON_STANDARD is consumed by newer objtool
 * tooling and has no runtime effect. Synology's 4.4.302 headers predate it.
 */
#ifndef _BPI_LINUX_FRAME_H
#define _BPI_LINUX_FRAME_H

#define STACK_FRAME_NON_STANDARD(func)

#endif /* _BPI_LINUX_FRAME_H */
