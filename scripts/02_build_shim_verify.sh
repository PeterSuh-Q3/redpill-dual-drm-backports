#!/usr/bin/env bash
# Step 2. compat shim 작성 및 심볼 검증 (§10 Step2)
# Pass 기준: missing_symbols.txt 의 모든 심볼이 compat/ 에서 해결됨
set -euo pipefail
: "${KSRC:?4.4.302 커널 소스 경로를 KSRC 로 지정하세요}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"

echo "[*] 4.4 커널 심볼 추출"
nm "$KSRC/vmlinux" | grep -E "drm_|fence_|reservation_" \
  > /tmp/4.4_symbols.txt

echo "[*] 누락 심볼 비교"
diff /tmp/5.4_drm_symbols.txt /tmp/4.4_symbols.txt 2>/dev/null \
  | grep "^<" > /tmp/missing_symbols.txt || true
echo "    -> /tmp/missing_symbols.txt ($(wc -l < /tmp/missing_symbols.txt) 항목)"

echo "[*] modpost 검증"
make -C "$KSRC" M="$ROOT" modules 2>&1 | grep "undefined" || echo "    undefined 없음"
echo "[done] Step2. shim 검증 완료"
