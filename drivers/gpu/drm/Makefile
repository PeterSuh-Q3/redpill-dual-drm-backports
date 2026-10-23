# SPDX-License-Identifier: GPL-2.0

# Makefile for the drm device driver.  This driver provides support for the
# Direct Rendering Infrastructure (DRI) in XFree86 4.1.0 and higher.

drm-y       :=	drm_auth.o drm_cache.o \
		drm_file.o drm_gem.o drm_ioctl.o drm_irq.o \
		drm_memory.o drm_drv.o drm_pci.o \
		drm_sysfs.o drm_hashtab.o drm_mm.o \
		drm_crtc.o drm_fourcc.o drm_modes.o drm_edid.o \
		drm_encoder_slave.o \
		drm_trace_points.o drm_prime.o \
		drm_rect.o drm_vma_manager.o drm_flip_work.o \
		drm_modeset_lock.o drm_atomic.o drm_bridge.o \
		drm_framebuffer.o drm_connector.o drm_blend.o \
		drm_encoder.o drm_mode_object.o drm_property.o \
		drm_plane.o drm_color_mgmt.o drm_print.o \
		drm_dumb_buffers.o drm_mode_config.o drm_vblank.o \
		drm_syncobj.o drm_lease.o drm_writeback.o drm_client.o \
		drm_client_modeset.o drm_atomic_uapi.o drm_hdcp.o \
		../../../compat/dma-fence-chain.o

# sync_file: DSM 4.4.302 stock 커널 대다수(broadwellnk 등)는 CONFIG_SYNC_FILE=n 이라
# host 에 sync_file_create/get_fence 가 없어 우리 drm.ko 가 자체 제공해야 하지만,
# geminilake/apollolake host 는 CONFIG_SYNC_FILE=y(빌트인) 라 우리가 또 만들면
# 심볼 중복 export 충돌 — CONFIG_DRM_SYNC_FILE_IN_KERNEL=y 로 이 두 플랫폼만 제외
# (interval_tree/amdgpu 가드와 동일 패턴).
ifneq ($(CONFIG_DRM_SYNC_FILE_IN_KERNEL),y)
drm-y += ../../dma-buf/sync_file.o
endif

# hdmi: drm_edid.c 가 hdmi_avi_infoframe_init/hdmi_vendor_infoframe_init 를
# 무조건 호출하는데, DSM 4.4.302 stock 커널 대다수(broadwellnk 등)는
# CONFIG_HDMI=n 이라 host 에 없음(실기 확인) — sync_file 과 동일 패턴으로
# 우리가 직접 제공. geminilake/apollolake host 는 CONFIG_HDMI=y(빌트인) 라
# CONFIG_DRM_HDMI_IN_KERNEL=y 로 제외.
ifneq ($(CONFIG_DRM_HDMI_IN_KERNEL),y)
drm-y += ../../video/hdmi.o
endif

drm-$(CONFIG_DRM_LEGACY) += drm_legacy_misc.o drm_bufs.o drm_context.o drm_dma.o drm_scatter.o drm_lock.o
drm-$(CONFIG_DRM_LIB_RANDOM) += lib/drm_random.o
drm-$(CONFIG_DRM_VM) += drm_vm.o
drm-$(CONFIG_COMPAT) += drm_ioc32.o
drm-$(CONFIG_DRM_GEM_CMA_HELPER) += drm_gem_cma_helper.o
drm-$(CONFIG_DRM_GEM_SHMEM_HELPER) += drm_gem_shmem_helper.o
drm-$(CONFIG_DRM_ATI_PCIGART) += ati_pcigart.o
drm-$(CONFIG_DRM_PANEL) += drm_panel.o
drm-$(CONFIG_OF) += drm_of.o
drm-$(CONFIG_AGP) += drm_agpsupport.o
drm-$(CONFIG_DEBUG_FS) += drm_debugfs.o drm_debugfs_crc.o
drm-$(CONFIG_DRM_LOAD_EDID_FIRMWARE) += drm_edid_load.o

# Linux v5.4 VRAM helper is one composite module; keep the GEM helper and
# common/MM helpers together so bochs-drm resolves the same exported symbols.
obj-$(CONFIG_DRM_VRAM_HELPER) += drm_vram_helper.o
drm_vram_helper-y := drm_gem_vram_helper.o drm_vram_helper_common.o \
		drm_vram_mm_helper.o

drm_kms_helper-y := drm_crtc_helper.o drm_dp_helper.o drm_dsc.o drm_probe_helper.o \
		drm_plane_helper.o drm_dp_mst_topology.o drm_atomic_helper.o \
		drm_kms_helper_common.o drm_dp_dual_mode_helper.o \
		drm_simple_kms_helper.o drm_modeset_helper.o \
		drm_scdc_helper.o drm_gem_framebuffer_helper.o \
		drm_atomic_state_helper.o drm_damage_helper.o \
		drm_format_helper.o drm_self_refresh_helper.o

# drm_kms_helper-$(CONFIG_DRM_PANEL_BRIDGE) += bridge/panel.o  # bridge/ vendor 미보유
drm_kms_helper-$(CONFIG_DRM_FBDEV_EMULATION) += drm_fb_helper.o
drm_kms_helper-$(CONFIG_DRM_KMS_CMA_HELPER) += drm_fb_cma_helper.o
drm_kms_helper-$(CONFIG_DRM_DP_AUX_CHARDEV) += drm_dp_aux_dev.o
drm_kms_helper-$(CONFIG_DRM_DP_CEC) += drm_dp_cec.o

# ── redpill-dual-drm: vendor/drm-5.4 subset 만 빌드 ──
# upstream drivers/gpu/drm/Makefile 의 벤더 드라이버 디렉토리(arm/omapdrm/tilcdc/
# i2c/panel/bridge/hisilicon/tiny/rcar-du 등 무조건 obj-y + 나머지 CONFIG 게이팅)는
# vendor 추출본에 소스가 없어 M=$PWD 전체 descent 시 "No rule to make target" 으로
# 빌드가 실패한다. vendor 가 실제 보유한 drm core + ttm/scheduler/amd/i915 만 남긴다.
# (Step4=i915 단독은 CONFIG_DRM_I915=m 만, Step6=Dual 은 +amdgpu/ttm/sched 게이팅)
obj-$(CONFIG_DRM_KMS_HELPER) += drm_kms_helper.o

obj-$(CONFIG_DRM)	+= drm.o
obj-$(CONFIG_DRM_MIPI_DBI) += drm_mipi_dbi.o
obj-$(CONFIG_DRM_MIPI_DSI) += drm_mipi_dsi.o
obj-$(CONFIG_DRM_PANEL_ORIENTATION_QUIRKS) += drm_panel_orientation_quirks.o
obj-$(CONFIG_DRM_BOCHS) += bochs-v54/
obj-$(CONFIG_DRM_AST) += ast-v54/
obj-$(CONFIG_DRM_VMWGFX) += vmwgfx/
obj-$(CONFIG_DRM_TTM)	+= ttm/
obj-$(CONFIG_DRM_SCHED)	+= scheduler/
obj-$(CONFIG_DRM_AMDGPU)+= amd/amdgpu/
obj-$(CONFIG_DRM_I915)	+= i915/
