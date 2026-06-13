# 브랜치 / 소스트리 전략 — redpill-dual-drm-backports

> 4.4.302 베이스 + 5.4 LTS OOT + CML-H/S 오버레이. dual-gpu-backports(5.10.55/mt65)와
> **완전 분리된 별개 트랙**. 빌드 타깃 커널/툴체인(4.4.302 dev.txz)·dsm_linux 는 외부 읽기전용 레퍼런스.

## 3계층 모델

```
vendor/drm-5.4 ──(pinned snapshot)──┐
                                    ├─→ integration/dual-drm ──(검증)──→ main ──→ tag/release
main(recipe: compat+patches+        │           ↑
     scripts+Kconfig/Makefile) ─────┘      feat/* 병합
                                                │
        CI 빌드 → module-output/<platform>-<dsm>-4.4.302  (gitout 쿼터우회)
```

## 브랜치 목록

| 브랜치 | 수명 | 내용 | 고정 worktree |
|---|---|---|---|
| `main` | 영속 | **레시피**: compat/, patches/, scripts/, cml_hs 오버레이, Kconfig/Makefile, 설계서(CLAUDE.md) | `~/redpill-dual-drm-backports` |
| `vendor/drm-5.4` | 영속(점진 bump) | **원자재**: v5.4.x 추출 drm_core+i915+amd 단일 스냅샷(1커밋) | `~/redpill-dual-drm-backports-vendor` |
| `integration/dual-drm` | 영속 | vendor + main patch 적용·Dual 빌드 검증장 | `~/redpill-dual-drm-backports-int` |
| `feat/compat-shim` | 단기 | §6 Step2 | — |
| `feat/cml-hs` | 단기 | §9 Step3 patch-series 작성 | — |
| `feat/i915-mt54` / `feat/amdgpu-dc` | 단기 | Step4 / Step5 단독 bring-up | — |
| `module-output/<plat>-<dsm>` | CI 생성 | 빌드 .ko (아티팩트 쿼터 우회) | — |

## 핵심 설계 결정

1. **drm_core 단일버전(§4.1)** → `vendor/drm-5.4` 단일 스냅샷으로 구조적 강제. i915·amd 가 동일 커밋에서 추출됨이 보장됨.
2. **추출 모델(EXTRACT.md)** → thin 추출 대신 **vendoring**(GPL-2.0 재배포 합법, CI 헤르메틱). `01_extract` 는 vendor 스냅샷 *생성/bump* 시에만 사용.
3. **cherry-pick(§9)** → `git cherry-pick`/수동병합 대신 `patches/` **결정적 시리즈**(`git am`). `03_cherrypick` 의 수동 단계 대체.

## 빌드 Step ↔ 위치

| Step | 작업 | 위치 |
|---|---|---|
| 1 추출 | 5.4 drm_core | `vendor/drm-5.4` 갱신 |
| 2 shim 검증 | compat 심볼 | `feat/compat-shim` → main, `02_*` CI 게이트 |
| 3 cherry-pick | CML-H/S | `feat/cml-hs` → `patches/` → main |
| 4 i915 단독 | 단독 빌드 | CI `build-i915` (integration) |
| 5 amdgpu 단독 | 단독 빌드 | CI `build-amdgpu` |
| 6 Dual 통합 | drm_core→TTM→i915→amdgpu | CI `build-dual` → `module-output/*` |
| 7 기능검증 | QSV/VCN | 실기(수동); CI 는 심볼충돌 정적검증까지 |

## 경계

- **dual-gpu-backports(5.10.55/mt65)**: 불간섭. CI 패턴(gitout, Node24 강제)만 차용.
- **4.4.302 dev.txz/toolchain, dsm_linux**: 외부 읽기전용 레퍼런스(브랜치 아님).
