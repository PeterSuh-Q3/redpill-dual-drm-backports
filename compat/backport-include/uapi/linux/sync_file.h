/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: uapi/linux/sync_file.h — host CONFIG_SYNC_FILE=n 이라 부재.
 * upstream(Linux 5.4) 그대로 이식. sync_file.c 의 ioctl 구현이 참조. */
#ifndef _BPI_UAPI_LINUX_SYNC_FILE_H
#define _BPI_UAPI_LINUX_SYNC_FILE_H

#include <linux/ioctl.h>
#include <linux/types.h>

struct sync_merge_data {
	char	name[32];
	__s32	fd2;
	__s32	fence;
	__u32	flags;
	__u32	pad;
};

struct sync_fence_info {
	char	obj_name[32];
	char	driver_name[32];
	__s32	status;
	__u32	flags;
	__u64	timestamp_ns;
};

struct sync_file_info {
	char	name[32];
	__s32	status;
	__u32	flags;
	__u32	num_fences;
	__u32	pad;

	__u64	sync_fence_info;
};

#define SYNC_IOC_MAGIC		'>'

#define SYNC_IOC_MERGE		_IOWR(SYNC_IOC_MAGIC, 3, struct sync_merge_data)

#define SYNC_IOC_FILE_INFO	_IOWR(SYNC_IOC_MAGIC, 4, struct sync_file_info)

#endif /* _BPI_UAPI_LINUX_SYNC_FILE_H */
