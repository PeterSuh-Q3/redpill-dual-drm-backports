# i915/ — 5.4 기반 MT54 OOT 추출 대상

5.4 LTS 에서 i915 전체 트리를 추출 후, CML-H/S cherry-pick 을 적용한다.
cherry-pick 산출물은 이 트리에 이미 동봉:

    - intel_device_info_cml_hs.c      (§9.3 발췌, intel_device_info.c 에 병합)
    - ../../../include/drm/i915_pciids_cml_hs.h  (§9.2 ID 매크로)

적용: `scripts/03_cherrypick_cml_hs.sh`
