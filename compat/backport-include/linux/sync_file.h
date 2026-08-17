/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: linux/sync_file.h
 *
 * 4.4.302 DSM stock 커널(7.2/7.3/7.4 공통, 실기 확인)은 CONFIG_SYNC_FILE 자체가
 * 비활성 — struct sync_file / sync_file_create() / sync_file_get_fence() 가
 * 전무하다(host kallsyms 에 없음, /proc/config.gz 로도 미확인). include_next 로
 * host 헤더에 기댈 수 없어 자체 완결형으로 정의한다.
 *
 * struct dma_fence / dma_fence_cb 는 Synology 가 자체적으로 4.4.302 에 이미
 * 백포트해 두었으므로(linux/dma-fence.h 실존, geminilake amdgpu 빌드로 확인됨)
 * host 의 linux/dma-fence.h 를 그대로 사용한다(재정의하지 않음).
 *
 * 구현체는 drivers/dma-buf/sync_file.c (vendor 5.4, 이미 존재) →
 * 최상위 Makefile 에서 독립 sync_file.ko 로 빌드해 drm.ko 보다 먼저 insmod.
 */
#ifndef _BPI_LINUX_SYNC_FILE_H
#define _BPI_LINUX_SYNC_FILE_H

#include <linux/types.h>
#include <linux/kref.h>
#include <linux/ktime.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/dma-fence.h>

struct sync_file {
	struct file		*file;
	struct kref		kref;
	char			name[32];
#ifdef CONFIG_DEBUG_FS
	struct list_head	sync_file_list;
#endif

	wait_queue_head_t	wq;
	unsigned long		flags;

	struct dma_fence	*fence;
	struct dma_fence_cb	cb;
};

#define POLL_ENABLED DMA_FENCE_FLAG_USER_BITS

struct sync_file *sync_file_create(struct dma_fence *fence);
struct dma_fence *sync_file_get_fence(int fd);

#endif /* _BPI_LINUX_SYNC_FILE_H */
