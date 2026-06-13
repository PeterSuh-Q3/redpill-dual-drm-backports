# 1차 심볼-갭 분석 (integration bootstrap, 2026-06-13)

> 목적: 5.4 vendor 소스가 쓰는 분기 API ↔ **Synology geminilake 4.4.302** 제공 현황 대조 →
> compat shim/patch 작업의 정확한 범위 산정.
> 방법: (a) 5.4 소스 사용량 grep 카운트, (b) 4.4.302 레퍼런스(`/Users/yousuk/Downloads/linux-4.4.302`) 심볼 제공 확인.
> ※ 정적 분석(파일/심볼 존재). 완전한 Step2 modpost(빌드된 4.4.302 KLIB_BUILD 대상)는 후속.

## ★ 핵심 결론: 설계서 §6 은 **vanilla 4.4 가정** — Synology 4.4.302 는 이미 대량 백포트됨

Synology geminilake 4.4.302 는 i915 를 **CannonLake(GEN10)** 까지 백포트하면서 modern DRM API 도
함께 들여왔다. 따라서 §6 의 일부 shim 은 **불필요**하며, 적용 시 **재정의 충돌**을 일으킨다.

## shim 범위 재산정 (vs Synology 4.4.302)

| §6 shim 항목 | 5.4 사용량 | 4.4.302 제공 | 판정 |
|---|---|---|---|
| `dma_fence_*` ← fence_* 매핑 | 1008회/97파일 | **dma-fence.h 제공** | ❌ **불필요** (적용 시 충돌) |
| `dma_resv` ← reservation_object | 313회/48파일 | reservation_object O / **dma_resv 타입 X** | ✅ **필요** (매핑) ※4.4.302가 alias 이미 제공하는지 재확인 |
| `drm_gem_object_put(_unlocked)` | 62회/17파일 | **drm_gem.h 제공** | ❌ **불필요** |
| `i2c_lock_bus` ← i2c_lock_adapter | (i915 일부) | **i2c.h 제공** | ❌ **불필요** |
| `intel_wakeref_t`/intel_runtime_pm | 248회/67파일 | **부재(0파일)** | ✅ **필요** (i915 전용) |
| `ttm_resource` | 0 (5.4 미사용) | 부재 | ⚪ moot (설계서 §6.3 주석과 일치) |
| `reservation_object` (5.4측) | 0 (5.4는 dma_resv로 이행) | — | — |

→ **실제 필요 shim 2종**: `dma_resv→reservation_object` 매핑, `intel_wakeref_t/intel_runtime_pm`.
나머지(dma_fence/gem_put/i2c)는 **제거 대상** (Synology가 이미 제공).

## i915 세대 / CML-H/S cherry-pick 함의

- 4.4.302 i915 PCI 흔적: **SKYLAKE·KABYLAKE·CANNONLAKE** 까지 확인 (vanilla 4.4 의 Skylake 한계를 넘어섬).
- 단, **ICELAKE/COMETLAKE 미확인** → 10세대(ICL/CML)는 여전히 5.4 OOT 네이티브 + §9 cherry-pick 필요(설계 목표 유효).
- intel_wakeref 부재는 4.4.302 i915 가 GEN10급이어도 5.4 의 runtime_pm 리팩터 이전임을 의미 → shim 필요 확정.

## amdgpu

- 4.4.302 에 `drivers/gpu/drm/amd/amdgpu` 트리 **존재** → Dual 의 amd 측 ABI 참조 가능.
- amdgpu 측 분기 API(dma_resv 등) shim 은 위 공통 항목과 공유.

## 다음 작업(patches/ 시리즈로 연결)

1. `0001-compat-dma_resv-map.patch` — dma_resv→reservation_object (필요 시. 4.4.302 alias 선확인)
2. `0002-compat-intel-wakeref.patch` — intel_wakeref_t/intel_runtime_pm shim (§6.2)
3. `compat/` 에서 **dma_fence/gem_put/i2c 매핑 제거** (Synology 제공분과 충돌 방지) — §6 원안 수정
4. `0100~` CML-H/S (§9) — ICL/CML 미백포트 확인됐으므로 유효
5. 후속: 빌드된 4.4.302 KLIB_BUILD 확보 후 modpost 기반 Step2 완전 검증

## 확인 필요(미해결)

- 4.4.302 `reservation.h` 가 `#define dma_resv reservation_object` alias 를 이미 제공하는지 (제공 시 1번도 불필요)
- 4.4.302 의 정확한 i915 백포트 베이스 커널 버전 (wakeref 외 추가 분기 API 식별용)
