# SPDX-License-Identifier: GPL-2.0
# compat/ shim 레이어 Makefile
#
# 4.4.302 <-> 5.4 LTS API gap 보정 모듈.
# 헤더(.h)는 각 드라이버가 -I$(src)/compat 로 참조.
# .c 는 별도 compat 모듈로 빌드되어 export symbol 제공.

obj-$(CONFIG_DRM_BACKPORT_COMPAT) += drm_compat_shim.o

drm_compat_shim-y := \
	drm_compat.o \
	dma_fence_compat.o

ccflags-y += -I$(src) -I$(src)/../include
