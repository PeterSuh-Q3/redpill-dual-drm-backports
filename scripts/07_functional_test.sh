#!/usr/bin/env bash
# Step 7. 기능 검증 (§10 Step7)
set -euo pipefail
echo "[*] i915 QSV"
vainfo --display drm --device /dev/dri/renderD128 || true
echo "[*] amdgpu VCN"
vainfo --display drm --device /dev/dri/renderD129 || true

IN="${1:-input.mkv}"
echo "[*] Dual 동시 트랜스코딩 (입력: $IN)"
ffmpeg -hwaccel vaapi -hwaccel_device /dev/dri/renderD128 \
       -i "$IN" -vf "format=nv12,hwupload" \
       -c:v h264_vaapi output_intel.mp4 &
ffmpeg -hwaccel vaapi -hwaccel_device /dev/dri/renderD129 \
       -i "$IN" -vf "format=nv12,hwupload" \
       -c:v h264_vaapi output_amd.mp4 &
wait
echo "[done] Step7. 기능 검증 완료 — dmesg 에 GPU hang 없는지 확인"
dmesg | grep -iE "GPU HANG|ring timeout" && echo "[NG] hang 발견" || echo "    hang 없음"
