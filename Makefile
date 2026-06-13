# SPDX-License-Identifier: GPL-2.0
# Redpill Dual DRM OOT 최상위 Makefile (★Rev: upstream 레이아웃 + header-only compat)
#
# 빌드 순서(§4.1/§10 Step6): drm(core) -> TTM -> scheduler(gpu-sched) -> i915 -> amdgpu
#   → 모두 upstream drivers/gpu/drm/Makefile 이 CONFIG 게이팅으로 처리.
#   drm 단일 버전은 vendor/drm-5.4 단일 스냅샷으로 보장(별도 drm_core/ rename 안 함).
#   ※ 실제 소스(drivers/gpu/drm/*)는 integration/dual-drm 브랜치(vendor 병합본)에 존재.
#      main 은 레시피(compat/, patches/, Makefile, Kconfig)만 보유.
#
# 사용(integration 워킹트리에서):
#   make all KDIR=/path/to/4.4.302/build
#   CONFIG 주입 필수(4.4.302 .config 에 우리 OOT 옵션 없음) — BUILD-NOTES 참조.

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

# ── DRM 스택 (upstream 네이티브 descent) ──
# drivers/gpu/drm/Makefile 이 CONFIG 로 drm.o / drm_kms_helper.o / ttm/ /
# scheduler/(gpu-sched, §설계 Makefile 누락분 보완) / i915/ / amd/amdgpu/ 게이팅.
obj-y += drivers/gpu/drm/

# ── compat shim (header-only) ──
# .c 빌드 없음. drm_compat.h 를 전 컴파일에 force-include (OOT 하위 디렉토리 전파용).
# (최상위 ccflags-y 는 drivers/gpu/drm/* 로 전파되지 않으므로 KCPPFLAGS 사용)
COMPAT_CPPFLAGS := -I$(PWD)/compat -I$(PWD)/include -include $(PWD)/compat/drm_compat.h

all:
	$(MAKE) -C $(KDIR) M=$(PWD) KCPPFLAGS="$(COMPAT_CPPFLAGS)" modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

.PHONY: all clean
