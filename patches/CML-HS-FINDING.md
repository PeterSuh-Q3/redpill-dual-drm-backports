# CML-H/S Cherry-pick(§9) 검증 결과: **v5.4 base 에서 불필요**

> 결론: vendor/drm-5.4(pin 219d5433, v5.4) 는 CML-H/S PCI Device ID 를 **이미 전부 네이티브로
> 포함**한다. 설계서 §9 cherry-pick(0100~0103 패치)은 이 base 에서 **생성하지 않는다**.
> 강행 시 pciidlist[] 에 **중복 ID 등록** → 잘못된/무의미한 변경.

## 검증 (integration vendor 소스 대조, 2026-06-13)

설계서 §9.2 가 추가하려는 CML-H/S ID 14개 ↔ 5.4 `include/drm/i915_pciids.h`:

| 설계서 매크로 | ID | 5.4 위치 |
|---|---|---|
| INTEL_CML_H_GT1 | 9BC8 | INTEL_CML_GT2_IDS |
| INTEL_CML_H_GT2 | 9BC4,9BC5,9BC6,9BCA,9BCB,9BCC | INTEL_CML_GT2_IDS |
| INTEL_CML_S_GT1 | 9BC0 | INTEL_CML_GT2_IDS |
| INTEL_CML_S_GT2 | 9B41 | INTEL_CML_GT2_IDS |
| INTEL_CML_S_GT2 | 9BA4,9BA8,9BAA,9BAB,9BAC | INTEL_CML_GT1_IDS |

→ **누락 0개.** 5.4 의 `INTEL_CML_GT1_IDS`/`GT2_IDS` 는 CML-U 뿐 아니라 CML-H/S SKU 까지
통합 열거하며, `i915_pci.c:868-869` 에서 `intel_coffeelake_gt1/gt2_info`(GEN9.5) 로 등록됨.

## 설계서 §9.3 의 추가 부정확성

- 설계서는 `intel_cometlake_h_info` + `PLATFORM(INTEL_COMETLAKE)` 를 추가하라 하나,
  **5.4 vendor 에는 `INTEL_COMETLAKE` 플랫폼 enum 도 `intel_cometlake_info` 도 없다**
  (intel_device_info.h 에 INTEL_COFFEELAKE 만 존재). 적용 시 컴파일 실패.
- 5.4 는 CML 을 coffeelake info 로 처리하므로 별도 struct 도 불필요.

## 조치

1. patches/ 에 CML-H/S 패치(0100~0103) **생성하지 않음**.
2. 레시피 오버레이 `include/drm/i915_pciids_cml_hs.h`, `drivers/gpu/drm/i915/intel_device_info_cml_hs.c`
   는 v5.4 base 기준 **obsolete**. (다른 base 로 bump 시 재평가)
3. Kconfig `CONFIG_DRM_I915_CML_HS` 는 no-op 처리 권장(default n).

## 함의

설계서 §3/§5 의 "CML-H/S = cherry-pick 필요" 전제는 더 낮은 base(또는 4.4 vanilla) 기준이었음.
**v5.4 LTS pin 에서는 ICL·CML-U·CML-H/S 전부 네이티브** → §9 단계 자체가 소거됨.
10세대 커버리지는 vendor 그대로 충족.
