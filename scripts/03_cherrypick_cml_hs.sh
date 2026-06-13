#!/usr/bin/env bash
# Step 3. CML-H/S Cherry-pick 적용 (§9.4 / §10 Step3)
# Pass 기준: cherry-pick 후 컴파일 에러 없음, CML-H/S PCI ID 매크로 정상 확장
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
I915="$ROOT/drivers/gpu/drm/i915"
PCIIDS="$ROOT/include/drm/i915_pciids.h"

echo "[*] §9.4 적용 순서: i915_pciids.h -> intel_device_info.c -> i915_pci.c -> intel_crt.c -> cml_hs_compat.h"
echo "[1] i915_pciids.h 에 CML-H/S 매크로 병합"
if [ -f "$PCIIDS" ] && ! grep -q "INTEL_CML_H_GT1_IDS" "$PCIIDS"; then
  cat "$ROOT/include/drm/i915_pciids_cml_hs.h" >> "$PCIIDS"
  echo "    매크로 추가 완료"
else
  echo "    (i915_pciids.h 미추출 또는 이미 적용됨 — Step1 후 재실행)"
fi

echo "[2] intel_device_info.c 병합 안내"
echo "    intel_device_info_cml_hs.c 의 intel_cometlake_h_info 구조체를"
echo "    intel_device_info.c 에 병합 (#if 0 가드 제거 후)"

echo "[3] i915_pci.c pciidlist[] 엔트리 추가:"
cat <<'IDS'
    INTEL_CML_H_GT1_IDS(&intel_cometlake_info),
    INTEL_CML_H_GT2_IDS(&intel_cometlake_h_info),
    INTEL_CML_S_GT1_IDS(&intel_cometlake_info),
    INTEL_CML_S_GT2_IDS(&intel_cometlake_h_info),
IDS

echo "[*] 충돌 확인"
( cd "$I915" && git diff --stat HEAD~2 2>/dev/null ) || true
echo "[done] Step3. cherry-pick 적용 (수동 병합 항목 확인 요망)"
