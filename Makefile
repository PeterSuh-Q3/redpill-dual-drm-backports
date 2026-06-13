# SPDX-License-Identifier: GPL-2.0
# Redpill Dual DRM OOT 최상위 Makefile
#
# 빌드 순서 (설계서 §4.1 / §10 Step6):
#   drm_core -> TTM -> i915 -> amdgpu
#   drm_core 는 5.4 단일 버전. TTM 단일 인스턴스 강제.
#
# 사용:
#   make -C /path/to/4.4.302/kernel M=$(PWD) modules
#
# KDIR 미지정 시 현재 실행 커널 헤더 사용.

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

# ── compat shim 레이어 ──
obj-$(CONFIG_DRM_BACKPORT_COMPAT) += compat/

# ── DRM core (5.4 단일 소스) ──
obj-$(CONFIG_DRM)        += drivers/gpu/drm/drm_core/

# ── 드라이버 ──
obj-$(CONFIG_DRM_I915)   += drivers/gpu/drm/i915/
obj-$(CONFIG_DRM_AMDGPU) += drivers/gpu/drm/amd/amdgpu/

# OOT 빌드 시 compat 헤더 우선 탐색
ccflags-y += -I$(src)/compat -I$(src)/include

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

.PHONY: all clean
