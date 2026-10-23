#!/usr/bin/env bash
# Isolated DSM 7.4 Gemini Lake K4.4.302 vmwgfx compile pilot. Never loads modules.
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/.." && pwd)
IMAGE=${IMAGE:-dante90/syno-compiler:7.4}
JOBS=${JOBS:-4}
VMWGFX_ONLY=${VMWGFX_ONLY:-0}
OUT=${OUT:-"$ROOT/out/k4-vmwgfx-kms-pilot/geminilake-7.4-k4.4.302/$(date -u +%Y%m%dT%H%M%SZ)"}

test -s "$ROOT/drivers/gpu/drm/vmwgfx/vmwgfx_drv.c" || {
  echo "vmwgfx source missing: merge pinned vendor/drm-5.4 snapshot into this tree first" >&2
  exit 2
}
mkdir -p "$OUT"
docker image inspect "$IMAGE" --format '{{.Id}}' > "$OUT/image-id.txt"
git -C "$ROOT" rev-parse HEAD > "$OUT/source-head.txt"
git -C "$ROOT" diff > "$OUT/source-working.diff"

docker run --rm -i --entrypoint bash -e JOBS="$JOBS" -e VMWGFX_ONLY="$VMWGFX_ONLY" \
  -v "$ROOT:/input:ro" -v "$OUT:/output" "$IMAGE" -s <<'CONTAINER' 2>&1 | tee "$OUT/build.log"
set -euo pipefail
mkdir -p /tmp/input
tar -C /input --exclude=.git --exclude=out --exclude=work --exclude=ksrc \
  --exclude=toolchain --exclude='*.o' --exclude='*.ko' --exclude='*.cmd' \
  --exclude=Module.symvers --exclude=modules.order -cf - . | tar -C /tmp/input -xf -
cd /tmp/input
patch --batch --forward -p1 -i patches/0001-vmwgfx-k4-scatterlist-and-vmalloc-compat.patch
sha256sum patches/0001-vmwgfx-k4-scatterlist-and-vmalloc-compat.patch > /output/k4-compat-patch.sha256
tar -czf /output/source-snapshot.tgz .

KDIR=/opt/geminilake/build
export ARCH=x86_64 CROSS_COMPILE=/opt/geminilake/bin/x86_64-pc-linux-gnu-
case "$(cat "$KDIR/include/config/kernel.release")" in
  4.4.302*) ;; *) echo 'Wrong target kernel; expected 4.4.302' >&2; exit 1 ;;
esac
test -s "$KDIR/.config"
test -s "$KDIR/Module.symvers"
cp "$KDIR/.config" /output/kernel.config
cp "$KDIR/Module.symvers" /output/kernel.Module.symvers

FLAGS="-I$PWD/compat/backport-include -I$PWD/include -I$PWD/include/uapi -I$PWD/compat -include $PWD/compat/drm_compat.h -include $PWD/compat/kcl_compat.h -DCONFIG_FB_DEFERRED_IO -DCONFIG_FB_NOTIFY -DCONFIG_DRM_HDMI_IN_KERNEL"
CONFIG_ARGS=(CONFIG_DRM=m CONFIG_DRM_KMS_HELPER=m CONFIG_DRM_TTM=m CONFIG_DRM_VMWGFX=m \
  CONFIG_DRM_HDMI_IN_KERNEL=y CONFIG_DRM_SYNC_FILE_IN_KERNEL=y \
  CONFIG_INTERVAL_TREE_IN_KERNEL=y CONFIG_FB_NOTIFY_IN_KERNEL=y)
REQUIRED=(drm drm_kms_helper ttm vmwgfx)
if [ "$VMWGFX_ONLY" != 1 ]; then
  CONFIG_ARGS+=(CONFIG_DRM_I915=m CONFIG_DRM_I915_USERPTR=y CONFIG_DRM_SCHED=m \
    CONFIG_DRM_AMDGPU=m CONFIG_DRM_VRAM_HELPER=m CONFIG_DRM_BOCHS=m \
    CONFIG_DRM_AST=m CONFIG_DRM_PANEL_ORIENTATION_QUIRKS=m)
  REQUIRED+=(i915 amdgpu bochs-drm ast drm_vram_helper gpu-sched)
else
  CONFIG_ARGS+=(CONFIG_DRM_I915=n CONFIG_DRM_AMDGPU=n CONFIG_DRM_BOCHS=n \
    CONFIG_DRM_AST=n CONFIG_DRM_SCHED=n CONFIG_DRM_VRAM_HELPER=n)
fi
if ! make -j"$JOBS" -C "$KDIR" M="$PWD" \
  "${CONFIG_ARGS[@]}" KCPPFLAGS="$FLAGS" modules > /output/make.log 2>&1; then
  tail -n 180 /output/make.log
  exit 1
fi
grep -E '(^  (CC|LD|MODPOST) \[M\]|WARNING:.*undefined|WARNING:.*exported twice|Unknown symbol)' \
  /output/make.log | tail -n 120 || true

if grep -E 'WARNING:.*undefined|WARNING:.*exported twice|Unknown symbol' /output/make.log; then
  echo 'Unresolved/duplicate symbols: reject pilot' >&2
  exit 1
fi

mkdir -p /output/modules
while IFS= read -r ko; do
  "${CROSS_COMPILE}strip" --strip-debug "$ko"
  cp "$ko" /output/modules/
done < <(find . -name '*.ko' -type f)

for name in "${REQUIRED[@]}"; do
  test -s "/output/modules/$name.ko"
done

cd /output/modules
for ko in *.ko; do
  modinfo "$ko"
  modinfo -F vermagic "$ko" | grep -q '^4.4.302'
done > /output/modinfo.txt
modinfo -F alias vmwgfx.ko | grep -qi 'v000015ADd00000405'
sha256sum *.ko > /output/SHA256SUMS
if [ "$VMWGFX_ONLY" = 1 ]; then
  tar -czf /output/geminilake-7.4-4.4.302-vmwgfx-dependency-pilot.tgz ./*.ko
else
  tar -czf /output/geminilake-7.4-4.4.302-quad-drm-vmwgfx-pilot.tgz ./*.ko
fi
CONTAINER

printf 'Validated compile pilot: %s\n' "$OUT"
