#!/usr/bin/env bash
# Step 1. 5.4 LTS drm_core 추출 (설계서 §10 Step1)
# Pass 기준: drm_core/ 빌드 시 undefined reference 없음
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
WORK="${WORK:-/tmp/redpill-build}"
mkdir -p "$WORK"

echo "[*] Linux 5.4 LTS clone (shallow)"
if [ ! -d "$WORK/linux-5.4" ]; then
  git clone --depth=1 --branch v5.4 \
    https://github.com/torvalds/linux.git "$WORK/linux-5.4"
fi

echo "[*] drm_core 소스 복사"
cp -v "$WORK/linux-5.4"/drivers/gpu/drm/drm_*.c \
      "$ROOT/drivers/gpu/drm/drm_core/" || true
cp -rv "$WORK/linux-5.4"/drivers/gpu/drm/ttm/* \
       "$ROOT/drivers/gpu/drm/drm_core/ttm/" || true

echo "[*] 5.4 심볼 목록 사전 추출 (shim 참조용)"
if [ -f "$WORK/linux-5.4/vmlinux" ]; then
  nm "$WORK/linux-5.4/vmlinux" 2>/dev/null \
    | grep -E "drm_|dma_fence|reservation" > /tmp/5.4_drm_symbols.txt || true
fi
echo "[done] Step1. drm_core 추출 완료"
