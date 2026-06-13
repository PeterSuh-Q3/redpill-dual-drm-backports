/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: linux/pseudo_fs.h (5.3+; 4.4.302 엔 fs_context 인프라 부재)
 *
 * ★주의: 4.4.302 는 fs_context/init_fs_context 가 없고 mount_pseudo 기반이다.
 *   여기서는 컴파일 진행용 최소 타입만 제공한다. drm_drv.c 의
 *   file_system_type.init_fs_context 사용부는 .mount 기반으로 별도 patch 필요
 *   (다음 인벤토리 층에서 'file_system_type has no member init_fs_context' 로 드러남).
 *   → patches/ 에 drm_fs 등록 백포트 패치로 처리 예정. */
#ifndef _BPI_LINUX_PSEUDO_FS_H
#define _BPI_LINUX_PSEUDO_FS_H

#include <linux/fs.h>

struct pseudo_fs_context {
	const struct super_operations *ops;
	const struct dentry_operations *dops;
	unsigned long magic;
};

/* 실제 마운트 브릿지는 drm_drv.c 패치에서 mount_pseudo 로 연결한다(여기선 타입만). */
struct pseudo_fs_context *init_pseudo(struct fs_context *fc, unsigned long magic);

#endif
