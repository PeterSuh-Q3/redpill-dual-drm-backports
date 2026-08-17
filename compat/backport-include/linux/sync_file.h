/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: linux/sync_file.h
 *
 * 4.4.302 DSM stock 커널(7.2/7.3/7.4 실기 확인, 헤더는 7.3/7.4 동일)은
 * linux/sync_file.h 헤더 자체는 갖고 있으나 CONFIG_SYNC_FILE 이 비활성이라
 * sync_file.o 구현체가 빌드/링크되지 않음 → sync_file_create/get_fence 등이
 * kallsyms 에 전무(insmod 시 drm.ko "Unknown symbol").
 *
 * 이 파일은 host include/linux/sync_file.h(7.4 toolkit dev.txz 에서 실측 추출,
 * 7.3 과 바이트 동일) 를 그대로 이식한 것 — 손으로 재구성하지 않음
 * (최초 버전은 user_name 필드명을 name 으로, POLL_ENABLED 를
 * DMA_FENCE_FLAG_USER_BITS 로 잘못 추정해 broadwellnk 7.4 파일럿 빌드에서
 * 컴파일 에러 12건 발생 → host 원본 대조로 교정, 2026-08-17).
 *
 * struct dma_fence / dma_fence_array 는 host 가 이미 백포트 보유(4.4.302 공통) →
 * 재정의하지 않고 host 헤더를 그대로 사용.
 *
 * 구현체는 drivers/dma-buf/sync_file.c (vendor 5.4) →
 * drivers/gpu/drm/Makefile 의 drm-y 에 fold(dma-fence-chain.c 와 동일 패턴).
 */
#ifndef _LINUX_SYNC_FILE_H
#define _LINUX_SYNC_FILE_H

#include <linux/types.h>
#include <linux/ktime.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/dma-fence.h>
#include <linux/dma-fence-array.h>

struct sync_file {
	struct file		*file;
	char			user_name[32];
#ifdef CONFIG_DEBUG_FS
	struct list_head	sync_file_list;
#endif

	wait_queue_head_t	wq;
	unsigned long		flags;

	struct dma_fence	*fence;
	struct dma_fence_cb cb;
};

#define POLL_ENABLED 0

struct sync_file *sync_file_create(struct dma_fence *fence);
struct dma_fence *sync_file_get_fence(int fd);
char *sync_file_get_name(struct sync_file *sync_file, char *buf, int len);

#endif /* _LINUX_SYNC_FILE_H */
