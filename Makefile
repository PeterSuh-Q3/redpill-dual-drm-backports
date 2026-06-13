# SPDX-License-Identifier: GPL-2.0
# Redpill Dual DRM OOT 최상위 Makefile (integration — upstream drm 레이아웃)
#
# 빌드 순서(§4.1/§10 Step6): drm(core) -> TTM -> scheduler(gpu-sched) -> i915 -> amdgpu
#   → 모두 upstream drivers/gpu/drm/Makefile 이 CONFIG 게이팅으로 처리.
#   drm 단일 버전은 vendor/drm-5.4 단일 스냅샷으로 보장(별도 drm_core/ rename 안 함).
#
# 사용:
#   make -C /path/to/4.4.302/kernel M=$(PWD) modules \
#        $(cat config.oot)        # CONFIG_* 주입 (§8) — BUILD-NOTES.md 참조
#   또는: make all KDIR=/path/to/4.4.302/build
#
# KDIR 미지정 시 현재 실행 커널 헤더 사용.

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

# ── compat shim 레이어 (본 프로젝트 신설) ──
obj-$(CONFIG_DRM_BACKPORT_COMPAT) += compat/

# ── DRM 스택 (upstream 네이티브 descent) ──
# drivers/gpu/drm/Makefile 이 아래를 CONFIG 로 게이팅:
#   obj-$(CONFIG_DRM) += drm.o            (drm.ko)
#   obj-$(CONFIG_DRM_KMS_HELPER) += drm_kms_helper.o
#   obj-$(CONFIG_DRM_TTM)   += ttm/       (ttm.ko, 단일 인스턴스)
#   obj-$(CONFIG_DRM_SCHED) += scheduler/ (gpu-sched.ko — amdgpu 의존, §설계 Makefile 누락분 보완)
#   obj-$(CONFIG_DRM_I915)  += i915/
#   obj-$(CONFIG_DRM_AMDGPU)+= amd/amdgpu/
obj-y += drivers/gpu/drm/

# compat 헤더 전파:
#   OOT M= 빌드에서 최상위 ccflags-y 는 하위 디렉토리(drivers/gpu/drm/*)로 전파되지 않음.
#   → 전 컴파일에 적용되는 KCPPFLAGS 로 주입(아래 all: 타깃). BUILD-NOTES.md §전파 참조.
COMPAT_CPPFLAGS := -I$(PWD)/compat -I$(PWD)/include -include $(PWD)/compat/drm_compat.h

all:
	$(MAKE) -C $(KDIR) M=$(PWD) KCPPFLAGS="$(COMPAT_CPPFLAGS)" modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

.PHONY: all clean
