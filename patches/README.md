# patches/ — 5.4 vendor 스냅샷 위에 적용되는 결정적 patch-series

설계서 §9의 "CML-H/S Cherry-pick" 및 §6 shim 연결을 **git cherry-pick(불안정·수동병합)**
대신 **순서 있는 패치 시리즈**로 캡처한다. `vendor/drm-5.4` 스냅샷 위에 `git am`(또는 quilt)
으로 결정적으로 적용 → CI 무인 빌드 보장.

## 네이밍 규약

```
00xx-*  공통/shim 연결 (compat 레이어 hook)
01xx-*  CML-H/S (§9: pciids → device_info → pci_entries → vga-guard)
02xx-*  amdgpu DC / RDNA1 보정
09xx-*  DSM 환경(펌웨어 경로·서명) 빌드측 보정
```

예시(작성 예정):
```
0001-compat-shim-hooks.patch
0100-cml-hs-pciids.patch          # include/drm/i915_pciids.h
0101-cml-hs-device-info.patch     # intel_device_info.c
0102-cml-hs-pci-entries.patch     # i915_pci.c pciidlist[]
0103-cml-s-vga-guard.patch        # CONFIG_DRM_I915_CML_S_VGA
```

## 적용 (integration 브랜치 빌드 시)

```bash
# vendor/drm-5.4 스냅샷 워킹트리에서
git am ../patches/0*.patch          # 또는 quilt push -a
```

> `scripts/03_cherrypick_cml_hs.sh` 의 수동병합 단계는 이 시리즈로 대체된다.
