#!/usr/bin/env bash
# Step 4. i915 단독 빌드 검증 (§10 Step4)
set -euo pipefail
: "${KSRC:?KSRC 미지정}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
make -C "$KSRC" M="$ROOT/drivers/gpu/drm/i915" modules
echo "[*] 로드 테스트"
sudo insmod "$ROOT/drivers/gpu/drm/drm_core/drm.ko" || true
sudo insmod "$ROOT/drivers/gpu/drm/i915/i915.ko"
echo "[*] Pass 기준 확인"
dmesg | grep -E "i915.*GEN(9|11)|ICL|CML" | tail -20
dmesg | grep -i "i915.*initialized" || true
ls -l /dev/dri/ || echo "[NG] /dev/dri 노드 없음 — §12.3 device node 확인"
