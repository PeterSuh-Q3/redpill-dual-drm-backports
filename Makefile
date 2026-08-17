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

# ── 헤더 우선순위 (★핵심: backport 인프라) ──
# 5.4 drm 소스(.c)는 5.4 drm 헤더로 컴파일돼야 한다. 그러나 OOT M= 빌드에서
# Kbuild 는 커널(4.4.302) include 를 먼저 탐색하므로 우리 vendor 5.4 헤더가 무시된다.
# → LINUXINCLUDE 앞에 vendor include/ 와 compat/ 를 prepend 해 <drm/*.h> 가 5.4 로 해석되게 한다.
#   (linux/·asm/ 등 그 외 헤더는 그대로 4.4.302 커널 것을 사용 = 의도된 동작)
# backport-include(커널헤더 shim) 최우선 → vendor include(5.4 drm) → compat
# backport-include(shim) → vendor include/uapi(5.4 drm) → i915 루트(내부 헤더) → compat
# i915 서브디렉토리(gt/gem/display) 파일이 "i915_drv.h" 등 루트 헤더를 찾도록 i915 루트 -I 추가.
LINUXINCLUDE := -I$(src)/compat/backport-include -I$(src)/include -I$(src)/include/uapi \
                -I$(src)/drivers/gpu/drm/i915 -I$(src)/compat $(LINUXINCLUDE)

# ── DRM 스택 (upstream 네이티브 descent) ──
# drivers/gpu/drm/Makefile 이 CONFIG 로 drm.o / drm_kms_helper.o / ttm/ /
# scheduler/(gpu-sched, §설계 Makefile 누락분 보완) / i915/ / amd/amdgpu/ 게이팅.
obj-y += drivers/gpu/drm/
obj-$(CONFIG_AGP_INTEL) += drivers/char/agp/

# ── compat shim ──
# interval_tree.ko: 최상위에서 obj-m으로 선언해야 독립 .ko로 링크됨
# (obj-y 하위 디렉토리의 obj-m은 OOT M= 빌드에서 standalone .ko 미생성)
# geminilake/apollolake host 는 interval_tree_insert 등을 이미 빌트인 export
# (실기 확인: /proc/kallsyms) — 우리가 또 만들면 "invalid module format"으로
# insmod 자체가 실패(심볼 중복). CONFIG_INTERVAL_TREE_IN_KERNEL=y 로 스킵.
ifneq ($(CONFIG_INTERVAL_TREE_IN_KERNEL),y)
obj-m += interval_tree.o
interval_tree-y := compat/interval_tree.o
endif

# backlight.ko: DSM 4.4.302 전 플랫폼(geminilake/apollolake 포함) 공통 CONFIG_BACKLIGHT_
# CLASS_DEVICE=m(host 빌트인 아님, .config 전수확인) — 항상 우리가 생산, 가드 불필요.
# module_init/exit(postcore_initcall+module_exit)이 있는 정식 독립 모듈이라
# sync_file/hdmi 처럼 drm.ko 에 fold하지 않고 interval_tree 와 동일하게 standalone .ko로 링크.
obj-m += backlight.o
backlight-y := drivers/video/backlight/backlight.o
# drm_compat.h 를 전 컴파일에 force-include (OOT 하위 디렉토리 전파용).
# (최상위 ccflags-y 는 drivers/gpu/drm/* 로 전파되지 않으므로 KCPPFLAGS 사용)
COMPAT_CPPFLAGS := -I$(PWD)/compat -I$(PWD)/include -include $(PWD)/compat/drm_compat.h

all:
	$(MAKE) -C $(KDIR) M=$(PWD) KCPPFLAGS="$(COMPAT_CPPFLAGS)" modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

.PHONY: all clean
