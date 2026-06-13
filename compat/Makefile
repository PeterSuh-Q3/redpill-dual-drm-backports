# SPDX-License-Identifier: GPL-2.0
# compat/ shim 레이어 — 헤더 전용 (★Rev: SYMBOL-GAP-Rev1 반영)
#
# 1차 심볼-갭 분석 결과, 4.4.302(Synology geminilake)가 dma_fence/drm_atomic/
# drm_gem_object_put/i2c_lock_bus 를 네이티브 제공하므로 설계서 원안의 .c 브릿지
# (drm_compat.c=atomic stub, dma_fence_compat.c=fence bridge)는 모두 불필요 → 제거됨.
# 남은 shim 은 전부 매크로/inline(헤더) → 빌드할 .o 가 없다.
#
# 헤더 주입: 최상위 Makefile 이 KCPPFLAGS(-include compat/drm_compat.h)로 전 컴파일에 적용.
# i915_compat.h / amdgpu_compat.h 는 드라이버별로 patches/ 에서 #include 주입 예정.
#
# 빌드 대상 없음(header-only). 향후 .c 보강 필요 시 obj-y 추가.
