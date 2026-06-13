#!/usr/bin/env bash
# Step 5. amdgpu 단독 빌드 검증 (§10 Step5)
set -euo pipefail
: "${KSRC:?KSRC 미지정}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
make -C "$KSRC" M="$ROOT/drivers/gpu/drm/amd/amdgpu" modules
sudo insmod "$ROOT/drivers/gpu/drm/drm_core/ttm/ttm.ko" || true
sudo insmod "$ROOT/drivers/gpu/drm/amd/amdgpu/amdgpu.ko"
dmesg | grep -E "amdgpu.*navi|RDNA|GFX10" | tail -20
ls -l /dev/dri/renderD129 || echo "[NG] renderD129 없음"
