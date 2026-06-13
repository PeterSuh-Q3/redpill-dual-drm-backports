#!/usr/bin/env bash
# Step 6. Dual DRM 통합 빌드 (§10 Step6)
# 로드 순서: drm_core -> TTM -> i915 -> amdgpu
set -euo pipefail
: "${KSRC:?KSRC 미지정}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
D="$ROOT/drivers/gpu/drm"

echo "[*] TTM 중복 로드 방지 확인 (결과 없어야 정상)"
grep -r "MODULE_ALIAS.*ttm" "$D/i915" "$D/amd" 2>/dev/null \
  | grep -v "drm_core" && echo "[NG] TTM 중복 alias 발견" || echo "    OK"

make -C "$KSRC" M="$ROOT" modules

echo "[*] 로드 순서: drm_core -> TTM -> i915 -> amdgpu"
sudo insmod "$D/drm_core/drm.ko"
sudo insmod "$D/drm_core/ttm/ttm.ko"
sudo insmod "$D/i915/i915.ko"
sudo insmod "$D/amd/amdgpu/amdgpu.ko"

echo "[*] Pass 기준"
lsmod | grep ttm
ls -l /dev/dri/
