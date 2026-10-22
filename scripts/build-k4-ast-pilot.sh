#!/usr/bin/env bash
# Local, isolated DSM 7.4 Gemini Lake quad-DRM compile pilot. Never loads modules.
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
IMAGE=${IMAGE:-dante90/syno-compiler:7.4}
JOBS=${JOBS:-4}
OUT=${OUT:-"$ROOT/out/junior-ast-kms-pilot/$(date -u +%Y%m%dT%H%M%SZ)"}
mkdir -p "$OUT"
docker image inspect "$IMAGE" --format '{{.Id}}' > "$OUT/image-id.txt"
git -C "$ROOT" rev-parse HEAD > "$OUT/source-head.txt"
git -C "$ROOT" diff > "$OUT/source-working.diff"
docker run --rm -i --entrypoint bash -e JOBS="$JOBS" \
  -v "$ROOT:/input:ro" -v "$OUT:/output" "$IMAGE" -s <<'CONTAINER' 2>&1 | tee "$OUT/build.log"
set -euo pipefail
mkdir -p /tmp/input
tar -C /input --exclude=.git --exclude=out --exclude=work --exclude=ksrc \
  --exclude=toolchain --exclude='*.o' --exclude='*.ko' --exclude='*.cmd' \
  --exclude=Module.symvers --exclude=modules.order -cf - . | tar -C /tmp/input -xf -
cd /tmp/input
tar -czf /output/source-snapshot.tgz .
KDIR=/opt/geminilake/build
export ARCH=x86_64 CROSS_COMPILE=/opt/geminilake/bin/x86_64-pc-linux-gnu-
case "$(cat "$KDIR/include/config/kernel.release")" in
  4.4.302*) ;; *) echo 'Wrong target kernel' >&2; exit 1 ;;
esac
cp "$KDIR/.config" /output/kernel.config
cp "$KDIR/Module.symvers" /output/kernel.Module.symvers
FLAGS="-I$PWD/compat/backport-include -I$PWD/include -I$PWD/include/uapi -I$PWD/compat -include $PWD/compat/drm_compat.h -include $PWD/compat/kcl_compat.h -DCONFIG_FB_DEFERRED_IO -DCONFIG_FB_NOTIFY -DCONFIG_DRM_HDMI_IN_KERNEL"
make -j"$JOBS" -C "$KDIR" M="$PWD" \
  CONFIG_DRM=m CONFIG_DRM_KMS_HELPER=m CONFIG_DRM_I915=m CONFIG_DRM_I915_USERPTR=y \
  CONFIG_DRM_TTM=m CONFIG_DRM_SCHED=m CONFIG_DRM_AMDGPU=m \
  CONFIG_DRM_VRAM_HELPER=m CONFIG_DRM_BOCHS=m CONFIG_DRM_AST=m \
  CONFIG_DRM_PANEL_ORIENTATION_QUIRKS=m CONFIG_FB_NOTIFY_IN_KERNEL=y \
  CONFIG_DRM_SYNC_FILE_IN_KERNEL=y CONFIG_INTERVAL_TREE_IN_KERNEL=y \
  CONFIG_DRM_HDMI_IN_KERNEL=y KCPPFLAGS="$FLAGS" modules 2>&1 | tee /tmp/build.log
if grep -E 'WARNING:.*undefined|WARNING:.*exported twice|Unknown symbol' /tmp/build.log; then
  echo 'Unresolved/duplicate symbols: reject pilot' >&2; exit 1
fi
mkdir /output/modules
while IFS= read -r ko; do
  "${CROSS_COMPILE}strip" --strip-debug "$ko"
  cp "$ko" /output/modules/
done < <(find . -name '*.ko')
for name in drm drm_kms_helper i915 amdgpu bochs-drm ast drm_vram_helper ttm gpu-sched; do
  test -s "/output/modules/$name.ko"
done
cp Module.symvers /output/quad.Module.symvers
cd /output/modules
for ko in *.ko; do
  modinfo "$ko"
  modinfo -F vermagic "$ko" | grep -q '^4.4.302'
done > /output/modinfo.txt
sha256sum *.ko > /output/SHA256SUMS
tar -czf /output/geminilake-7.4-4.4.302-quad-drm.tgz ./*.ko
CONTAINER
printf 'Validated compile pilot: %s\n' "$OUT"
