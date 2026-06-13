# CLAUDE.md — Redpill Dual DRM Backport 설계서 (Rev.2)

> **프로젝트**: Redpill i915 + amdgpu Dual DRM OOT Backport
> **작성일**: 2026-06-13
> **리비전**: Rev.2 (cherry-pick 전략 반영 / 리뷰 수정사항 적용)
> **기준 커널**: 4.4.302 (베이스), 5.10.55 (레퍼런스 성공 빌드)
> **목표 OOT 소스**: Linux 5.4 LTS + CML-H/S Cherry-pick

---

## 용어 정의

| 용어 | 정의 |
|---|---|
| **mt65 OOT** | 5.10.55 기반 i915 OOT 패치셋. 기존 레퍼런스 성공 빌드에 사용 |
| **MT54 OOT** | 5.4 LTS 기반 i915 OOT 패치셋. 본 설계의 신규 목표 |
| **PCI Device ID Override** | GPU 드라이버 pciidlist[]에 미등록 Device ID를 강제 추가하는 패치 |
| **Cherry-pick** | 특정 커밋만 선별하여 베이스 소스에 적용하는 방식 |
| **compat shim** | 커널 버전 간 API 차이를 메우는 호환 레이어 |
| **GEN#** | Intel GPU 마이크로아키텍처 세대 (GEN11 = Ice Lake iGPU) |

---

## 1. 프로젝트 개요

### 1.1 배경

Redpill 프로젝트에서 DSM 커널 4.4.302 기반 환경에 인텔 10세대 이하 iGPU(i915) 및
AMD GPU(amdgpu)를 동시에 지원하는 Dual DRM OOT 백포트 모듈을 구성한다.

기존 PCI Device ID Override 패치(임시방편) 방식을 탈피하여 OOT 네이티브 인식으로
전환하고, Comet Lake-H/S(10세대 데스크탑·고성능)까지 지원 범위를 확장한다.

### 1.2 기존 현황

| 항목 | 내용 | 상태 |
|---|---|---|
| 레퍼런스 빌드 | 5.10.55 + **mt65 OOT** | ✅ 빌드 성공 |
| 신규 목표 베이스 | 4.4.302 + **MT54 OOT** (5.4 LTS) | 🔧 설계 중 |
| PCI ID Override 패치 | 미지원 세대 Device ID 강제 주입 | ⚠️ 임시방편 → 제거 목표 |

> **mt65 / MT54 관계**: 동일 OOT 패치 패밀리의 커널 버전별 변형.
> mt65 = 5.10.x 대응, MT54 = 5.4.x 대응.

### 1.3 목표

- 커널 4.4.302 기반 **i915 + amdgpu Dual DRM** 병합 빌드 성공
- PCI Device ID Override 패치 완전 제거 → MT54 OOT 네이티브 인식으로 대체
- **인텔 10세대 이하 전 라인업** iGPU 완전 지원
  - Ice Lake (GEN11) U/Y 계열
  - Comet Lake-U (GEN9.5) 계열
  - **Comet Lake-H/S (GEN9.5) → 5.4 기반 Cherry-pick으로 추가**
- AMD GPU RDNA1(RX 5000) 이하 동시 지원

---

## 2. 커널 4.4 기본 i915 지원 범위

### 2.1 세대별 지원 현황

| 세대 | 코드명 | iGPU | GEN# | 지원 여부 |
|---|---|---|---|---|
| 1세대 | Ironlake | HD Graphics (2010) | GEN5 | ✅ 완전 지원 |
| 2세대 | Sandy Bridge | HD 2000/3000 | GEN6 | ✅ 완전 지원 |
| 3세대 | Ivy Bridge | HD 2500/4000 | GEN7 | ✅ 완전 지원 |
| 4세대 | Haswell | HD 4600 / Iris 5100 | GEN7.5 | ✅ 완전 지원 |
| 5세대 | Broadwell | HD 5500 / Iris 6100 | GEN8 | ✅ 완전 지원 |
| 6세대 | Skylake | HD 510~580 / Iris 540 | GEN9 | ⚠️ 포함, 일부 버그 |
| 7세대 | Kaby Lake | HD 610~650 / Iris 640 | GEN9.5 | ❌ 미포함 (4.7+) |
| 8세대+ | Coffee Lake 이상 | — | GEN9.5+ | ❌ 미포함 |
| 10세대 | Ice Lake / Comet Lake | — | GEN11/9.5 | ❌ 미포함 (5.1+) |

### 2.2 커널 4.4의 실용 지원 한계

```
커널 4.4 i915 실용 범위:
  Ironlake(1세대) ~ Skylake(6세대) 까지

NAS/서버 환경 주요 미지원 대상:
  - Apollo Lake / Gemini Lake (Atom 계열)  → OOT 필요
  - Denverton (Atom C3000)                → OOT 필요
  - Ice Lake / Comet Lake (10세대)         → OOT 필요
```

### 2.3 세대별 최소 커널 요구사항

| 세대 | 코드명 | 최소 커널 | 본 프로젝트 대응 |
|---|---|---|---|
| 7세대 | Kaby Lake | 4.7+ | MT54 포함 |
| 8세대 | Coffee Lake | 4.12+ | MT54 포함 |
| 8세대 | Whiskey Lake | 4.18+ | MT54 포함 |
| 9세대 | Coffee Lake Refresh | 4.20+ | MT54 포함 |
| **10세대** | **Ice Lake** | **5.1+** | **MT54 네이티브** |
| **10세대** | **Comet Lake-U** | **5.4+** | **MT54 네이티브** |
| **10세대** | **Comet Lake-H/S** | **5.6+** | **✅ Cherry-pick 적용** |
| 11세대 | Tiger Lake | 5.9+ | 범위 외 |
| 12세대 | Alder Lake | 5.15+ | 범위 외 |

---

## 3. OOT 전략: 5.4 LTS + CML-H/S Cherry-pick

### 3.1 전략 선택 근거

```
목표: CML-H/S 지원을 포함한 10세대 이하 완전 커버

선택지 비교:
┌─────────────────┬──────────────┬───────────┬──────────┐
│ 항목            │ 5.4 + CP     │ 5.6 OOT   │ 5.10 LTS │
├─────────────────┼──────────────┼───────────┼──────────┤
│ CML-H/S 지원    │ ✅ (Cherry)  │ ✅ 네이티브│ ✅ 완전  │
│ LTS 안정성      │ ✅           │ ❌ EOL 3개월│ ✅       │
│ shim 작업량     │ 최소         │ 중간       │ 최대     │
│ amdgpu 추가이득 │ 없음         │ 없음       │ 있음     │
│ 유지보수성      │ 높음         │ 낮음       │ 높음     │
│ 보안 패치       │ ✅           │ ❌         │ ✅       │
│ 전체 리스크     │ 낮음         │ 중-높음    │ 높음     │
└─────────────────┴──────────────┴───────────┴──────────┘
CP = Cherry-pick
```

> **5.6 선택 불가 이유**: 5.6은 LTS가 아님 (2020.03 릴리스, 2020.06 EOL).
> 이미 6년 전 EOL로 보안 패치·errata 수정이 없음.
> CML-H/S만을 위해 비LTS를 선택하는 것은 유지보수 관점에서 비합리적.

### 3.2 5.4 LTS 기반 선택 근거

```
- 4.4 → 5.4 infra delta: 5.10 대비 관리 가능한 수준
- Ice Lake (GEN11 / 10세대 U·Y): 5.4에 네이티브 완전 포함
- Comet Lake-U (GEN9.5): 5.4에 포함, 일부 변종 확인 필요
- drm_atomic / dma_buf shim 레이어 부담 최소
- dma_resv ↔ reservation_object 병존 전환기 (5.4) → shim 단순
- LTS 장기 유지보수 및 보안 패치 지속
- amdgpu RDNA1 지원 5.4에서 안정화 완료
```

### 3.3 Cherry-pick 전략 개요

CML-H/S는 GEN9.5 아키텍처(CML-U와 동일)를 사용하므로
초기화 경로 자체는 5.4에 이미 존재하며,
**미등록 PCI Device ID 추가가 핵심 작업**이다.

```
5.4 OOT 베이스
  └─ intel_cometlake_info 구조체: 이미 존재 (CML-U용)
  └─ GEN9.5 초기화 경로: 이미 존재
  └─ CML-H/S PCI Device ID: 미등록 ← Cherry-pick으로 추가

Cherry-pick 작업량 추정:
  대상 파일: 3~4개
  변경 라인: 약 100~200줄
  → 5.6 전체 OOT 전환 대비 작업량 1/10 이하
```

### 3.4 타깃 범위 확정

```
[인텔 iGPU]
  MT54 OOT 네이티브: GEN5 ~ GEN11 (1세대 Ironlake ~ 10세대 Ice Lake)
  Cherry-pick 추가:  CML-H GT1/GT2, CML-S GT1/GT2

[AMD GPU]
  MT54 OOT 네이티브: GCN4 (Polaris) ~ RDNA1 (Navi / RX 5000)
```

---

## 4. Dual DRM 병합 빌드 설계

### 4.1 핵심 제약: DRM Core 단일 버전 원칙

```
┌─────────────────────────────────────────────────┐
│              drm_core (공유 레이어)               │
│    dma_fence / drm_atomic / TTM / dma_buf       │
│           ← 반드시 단일 버전 통일 →              │
├──────────────────┬──────────────────────────────┤
│   i915 driver    │       amdgpu driver          │
│   (5.4 소스)     │       (5.4 소스)             │
└──────────────────┴──────────────────────────────┘
          ↕                      ↕
      동일 OOT 소스 버전에서 추출 필수
      버전 불일치 시: 심볼 충돌 → 링크 실패 또는 런타임 panic
```

### 4.2 AMD GPU 세대별 최소 커널 요구

| AMD GPU 세대 | 최소 커널 | 비고 |
|---|---|---|
| GCN 1 (Southern Islands) | 3.10+ | 타깃 외, 비활성화 |
| GCN 2 (Sea Islands) | 3.15+ | 타깃 외, 비활성화 |
| GCN 3 (Volcanic Islands) | 4.2+ | amdgpu 인프라 도입 |
| GCN 4 (Polaris / RX 400·500) | **4.7+** | Polaris Device ID 4.7에서 추가 |
| GCN 5 / Vega | 4.15+ | DC(Display Core) 필요 |
| RDNA 1 (Navi / RX 5000) | **5.4+** | DC 안정화, VCN2 지원 |
| RDNA 2 (RX 6000) | **5.10+** | GFX10.3, VCN3 → 범위 외 |
| RDNA 3 (RX 7000) | 5.19+ | 백포트 불가 |

> **주의**: GCN4(Polaris)는 4.4 커널에 amdgpu 인프라(드라이버 골격)만 있으며
> Polaris Device ID 등록은 4.7에서 이루어짐. "4.4 기본 포함" 표현은 오해 소지 있음.

> **5.4 OOT 기준 amdgpu 지원 범위: GCN4 (Polaris) ~ RDNA1 (Navi)**

### 4.3 Dual DRM 버전 최종 선택

| 구분 | 선택 | 근거 |
|---|---|---|
| OOT 베이스 소스 | **Linux 5.4 LTS** | i915 GEN11 + amdgpu RDNA1 동시 커버 |
| CML-H/S 확장 | **Cherry-pick** (5.5~5.6 커밋) | LTS 안정성 유지하며 범위 확장 |
| drm_core | 5.4 단일 추출 | 심볼 충돌 방지 |
| TTM | 5.4 단일 인스턴스 | 중복 로드 금지 |
| 4.4 compat shim | 별도 레이어 구성 | API gap 보정 |

---

## 5. PCI Device ID Override 패치 vs MT54 OOT 비교

> **용어 수정**: 구 설계서의 "CPUID 패치"는 실제로 i915 드라이버의
> `pciidlist[]` 테이블에 미지원 GPU의 PCI Device ID를 강제 주입하는 방식이며,
> CPU 식별 명령어인 CPUID(ring0)와 무관함. 이하 "PCI Device ID Override"로 표기.

### 5.1 PCI Device ID Override 방식 (기존)

```
문제 원인:
  커널 4.4 i915 pciidlist[] → GEN9(Skylake)까지만 Device ID 등록
  10세대 ICL/CML PCI Device ID 미등록
        ↓
  pci_register_driver() → 하드웨어 매칭 실패 → 드라이버 바인딩 거부
        ↓
  pciidlist[]에 Device ID를 강제 추가하는 패치로 우회
  (또는 ids_override 커널 파라미터 사용)
```

**방식의 한계:**

```
- GEN 판별 로직 우회로 인한 불안정한 초기화 경로 진입
- 잘못된 intel_device_info 구조체 참조 위험
- GuC/HuC 펌웨어 버전 불일치 위험
- GEN11 전용 Display Engine 미초기화 (ICL의 경우)
- 커널·드라이버 업데이트 시마다 패치 재적용 필요
- 임시방편 (근본적 해결 아님)
```

### 5.2 MT54 OOT 적용 후 변화

```c
// 5.4 기반 i915 소스 pciidlist[] 발췌 (intel_device_info.c 기반)

// Ice Lake GEN11 — 10세대 U/Y
INTEL_ICL_IDS(&intel_icelake_info),
// PCI ID 예: 0x8A51, 0x8A52, 0x8A53, 0x8A5A, 0x8A5C 등 네이티브 포함

// Comet Lake GEN9.5 — 10세대 U 시리즈
INTEL_CML_GT1_IDS(&intel_cometlake_info),
INTEL_CML_GT2_IDS(&intel_cometlake_info),
// → PCI Device ID Override 패치 불필요

// CML-H/S는 Cherry-pick으로 추가 (§9 참조)
// INTEL_CML_H_GT1_IDS, INTEL_CML_H_GT2_IDS
// INTEL_CML_S_GT1_IDS, INTEL_CML_S_GT2_IDS
```

### 5.3 항목별 비교

| 항목 | PCI ID Override | MT54 + Cherry-pick |
|---|---|---|
| PCI Device ID 인식 | 강제 주입 우회 | ✅ 네이티브 인식 |
| intel_device_info 참조 | 잘못된 구조체 위험 | ✅ 정확한 구조체 매핑 |
| GEN11 초기화 경로 | 강제 실행 (불안정) | ✅ 정상 ICL 초기화 |
| Display Engine | 불안정 (ICL DC 미적용) | ✅ ICL DC 포함 |
| GuC/HuC 펌웨어 | 버전 불일치 위험 | ✅ 버전 매칭 |
| CML-H/S 지원 | 부분적·불안정 | ✅ Cherry-pick 네이티브 |
| 유지보수성 | 패치 반복 필요 | ✅ OOT 단위 관리 |
| 전체 안정성 | ⚠️ 임시방편 | ✅ 근본 해결 |

### 5.4 PCI ID Override 패치 제거 가능 여부

| 10세대 세부 | MT54 대체 | 비고 |
|---|---|---|
| Ice Lake (GEN11) U/Y | ✅ 완전 대체 | 5.4에 완전 포함 |
| Comet Lake-U (CML-U) | ✅ 대부분 대체 | 일부 GT 변종 확인 필요 |
| Comet Lake-H (CML-H) | **✅ Cherry-pick 대체** | §9 커밋 적용 후 완전 지원 |
| Comet Lake-S (CML-S) | **✅ Cherry-pick 대체** | §9 커밋 적용 후 완전 지원 |

> **결론: 10세대 전 라인업의 PCI Device ID Override 패치 완전 제거 가능**
> (Ice Lake/CML-U: MT54 네이티브, CML-H/S: Cherry-pick 적용)

---

## 6. compat Shim 레이어 설계

4.4 커널 API와 5.4 OOT 소스 간의 불일치를 보정하는 레이어.
shim은 5.4 소스가 호출하는 함수를 4.4 커널 함수로 연결하는 방향으로 작성.

### 6.1 공통 필수 Shim (i915 + amdgpu 공용)

```c
// compat/drm_compat.h

/* ── dma_fence: 4.4 fence API 매핑 ── */
// 5.4 소스: dma_fence_* 호출
// 4.4 커널: fence_* 존재
#define dma_fence_ops           fence_ops
#define dma_fence_init          fence_init
#define dma_fence_signal        fence_signal
#define dma_fence_get           fence_get
#define dma_fence_put           fence_put
#define dma_fence_is_signaled   fence_is_signaled

/* ── drm_atomic: 4.4 미구현 → stub ── */
struct drm_atomic_state *drm_atomic_state_alloc(struct drm_device *dev);
void drm_atomic_state_free(struct drm_atomic_state *state);

/* ── reservation_object ↔ dma_resv 매핑 ── */
// 5.4 소스: dma_resv_* 사용 (전환기, reservation_object 병존)
// 4.4 커널: reservation_object 전용
#define dma_resv                reservation_object
#define dma_resv_lock           reservation_object_lock
#define dma_resv_unlock         reservation_object_unlock
#define dma_resv_init           reservation_object_init
#define dma_resv_fini           reservation_object_fini
#define dma_resv_trylock        reservation_object_trylock

/* ── GEM object ref/put 통합 shim ── */
// 5.4: drm_gem_object_put() 및 drm_gem_object_put_unlocked() 혼용
// 4.4: drm_gem_object_unreference_unlocked() 단독
static inline void drm_gem_object_put(struct drm_gem_object *obj)
{
    drm_gem_object_unreference_unlocked(obj);
}
static inline void drm_gem_object_put_unlocked(struct drm_gem_object *obj)
{
    drm_gem_object_unreference_unlocked(obj);
}
```

### 6.2 i915 전용 추가 Shim

```c
// compat/i915_compat.h

/* ── intel_runtime_pm wakeref wrapper ── */
// 5.4: intel_wakeref_t 타입 + rpm 구조체 기반 API
// 4.4: pm_runtime_get_sync() 직접 호출 방식
typedef int intel_wakeref_t;

static inline intel_wakeref_t
intel_runtime_pm_get(struct intel_runtime_pm *rpm)
{
    pm_runtime_get_sync(rpm->kdev);
    return 1;
}

static inline void
intel_runtime_pm_put(struct intel_runtime_pm *rpm, intel_wakeref_t wref)
{
    pm_runtime_put_autosuspend(rpm->kdev);
}

/* ── ICL Display Core 초기화 ── */
// 주의: stub이 아닌 실제 구현 필요
// 4.4의 drm_display infra와 연결하는 브릿지 함수로 작성
void icl_display_core_init(struct drm_i915_private *dev_priv, bool resume);
void icl_display_core_uninit(struct drm_i915_private *dev_priv);

/* ── i2c adapter lock API 변경 대응 ── */
// 5.4: i2c_lock_bus() 사용
// 4.4: i2c_lock_adapter() 사용
#define i2c_lock_bus(adap, flags)   i2c_lock_adapter(adap)
#define i2c_unlock_bus(adap, flags) i2c_unlock_adapter(adap)
```

### 6.3 amdgpu 전용 추가 Shim

```c
// compat/amdgpu_compat.h

// ── ttm_resource 관련 ──
// 주의: ttm_resource 구조체는 5.10에서 도입됨
// 5.4 OOT 소스는 이미 ttm_mem_reg 사용 → 이 shim 불필요
// (5.10 OOT 적용 시에만 필요)

/* ── amdkfd: 4.4 미지원 → stub 처리 ── */
// HSA(Heterogeneous System Architecture) 미지원 환경 대응
static inline int  amdgpu_amdkfd_init(void) { return 0; }
static inline void amdgpu_amdkfd_fini(void) {}
static inline bool amdkfd_fence_check_mm(struct fence *f,
                                          struct mm_struct *mm)
{
    return false;
}

/* ── amdgpu_vm_bo_update 시그니처 차이 대응 ── */
// 필요 시 버전별 래퍼 추가
```

### 6.4 CML-H/S Cherry-pick 전용 Shim

```c
// compat/cml_hs_compat.h
// Cherry-pick 커밋 적용 후 발생하는 추가 심볼 처리

/* CML-H/S는 GEN9.5 공용 경로 사용 → 별도 shim 최소화
   intel_cometlake_info 구조체를 공유하므로 초기화 shim 불필요
   PCI ID 추가만으로 동작 예상                              */

/* 단, CML-S 데스크탑의 아날로그 출력(VGA) 비활성화 처리 */
#ifdef CONFIG_DRM_I915_CML_S_VGA
/* CML-S VGA 출력: intel_crt.c 초기화 경로 선택적 포함 */
#else
static inline void intel_crt_init(struct drm_i915_private *dev_priv) {}
#endif
```

---

## 7. 디렉토리 구조

```
drivers/gpu/drm/
├── drm_core/                       ← 5.4 LTS 단일 소스 (핵심 공유)
│   ├── drm_atomic.c
│   ├── drm_atomic_helper.c
│   ├── drm_gem.c
│   ├── drm_dp_helper.c
│   ├── drm_dp_mst_topology.c
│   ├── dma-fence.c
│   └── ttm/
│       ├── ttm_bo.c
│       ├── ttm_bo_util.c
│       └── ttm_memory.c
│
├── i915/                           ← 5.4 기반 MT54 OOT
│   ├── intel_device_info.c         ← ICL/CML-U Device ID 포함
│   │                                  + CML-H/S Cherry-pick 적용
│   ├── i915_pci.c                  ← pciidlist[] Cherry-pick 적용
│   ├── intel_display.c
│   ├── i915_gem.c
│   ├── intel_guc.c
│   └── intel_huc.c
│
└── amd/
    ├── display/                    ← 5.4 DC (Display Core)
    │   └── amdgpu_dm/
    └── amdgpu/                     ← 5.4 기반
        ├── amdgpu_drv.c
        ├── amdgpu_ttm.c            ← ttm_mem_reg 사용 (ttm_resource 아님)
        └── amdgpu_gfx.c

compat/                             ← 4.4.302 전용 shim 레이어
├── drm_compat.h                    ← 공통 DRM API 매핑
├── drm_compat.c
├── dma_fence_compat.c              ← fence → dma_fence 브릿지
├── reservation_compat.h            ← dma_resv → reservation_object 매핑
├── i915_compat.h                   ← i915 전용 shim
├── amdgpu_compat.h                 ← amdgpu 전용 shim
└── cml_hs_compat.h                 ← CML-H/S cherry-pick 전용

firmware/                           ← DSM 환경 펌웨어 경로 설정
├── i915/                           ← GuC/HuC 펌웨어 (5.4 대응 버전)
│   ├── icl_guc_33.0.0.bin
│   └── icl_huc_9.0.0.bin
└── amdgpu/                         ← RDNA1 펌웨어
    └── navi10_*.bin
```

---

## 8. Kconfig 설정

```kconfig
# ================================================================
# Redpill Dual DRM OOT 병합 빌드 설정
# 베이스: 커널 4.4.302 / OOT 소스: Linux 5.4 LTS + Cherry-pick
# ================================================================

# ── 공유 DRM 레이어 (단일 인스턴스 필수) ──
CONFIG_DRM=m
CONFIG_DRM_TTM=m                    # TTM: 중복 로드 절대 금지
                                    # i915, amdgpu 양쪽이 이 단일 인스턴스 공유

# ── i915 ──
CONFIG_DRM_I915=m
CONFIG_DRM_I915_GVT=n               # GVT-g(가상화): Redpill 환경 불필요
CONFIG_DRM_I915_USERPTR=y           # userptr GEM: QSV 활용 시 필요

# ── amdgpu ──
CONFIG_DRM_AMDGPU=m
CONFIG_DRM_AMDGPU_SI=n              # GCN1 (Southern Islands): 타깃 외 비활성화
                                    # (5.4에 옵션 존재하나 타깃 하드웨어 없음)
CONFIG_DRM_AMDGPU_CIK=n             # GCN2 (Sea Islands): 타깃 외 비활성화
CONFIG_DRM_AMD_DC=y                 # Display Core: amdgpu 출력 필수
CONFIG_DRM_AMD_DC_DCN1_0=y          # Vega DCN 1.0
CONFIG_DRM_AMD_DC_DCN2_0=y          # RDNA1 DCN 2.0

# ── compat shim 레이어 ──
# 주의: CONFIG_DRM_BACKPORT_COMPAT 는 커널 표준 옵션이 아님
# 아래 옵션은 본 프로젝트에서 신설하는 커스텀 Kconfig 항목
# Makefile에서 obj-$(CONFIG_DRM_BACKPORT_COMPAT) += compat/ 로 연결
CONFIG_DRM_BACKPORT_COMPAT=y        # [신설 옵션] 4.4 호환 레이어 활성화
CONFIG_DRM_I915_CML_HS=y            # [신설 옵션] CML-H/S Cherry-pick 활성화
CONFIG_DRM_I915_CML_S_VGA=n        # CML-S VGA 아날로그 출력 (DSM 환경 불필요)
```

---

## 9. CML-H/S Cherry-pick 커밋 목록

### 9.1 Cherry-pick 대상 파일 및 작업 내용

| 파일 | 작업 내용 | 출처 커널 |
|---|---|---|
| `include/drm/i915_pciids.h` | CML-H/S PCI ID 매크로 정의 추가 | 5.5-rc1 |
| `drivers/gpu/drm/i915/intel_device_info.c` | CML-H/S ID 매크로 선언 및 device_info 연결 | 5.5-rc1 |
| `drivers/gpu/drm/i915/i915_pci.c` | pciidlist[] 엔트리 추가 | 5.5-rc1 |
| `drivers/gpu/drm/i915/intel_crt.c` | CML-S 아날로그 출력 처리 (선택적) | 5.5-rc1 |

### 9.2 추가할 PCI Device ID 목록

```c
/* include/drm/i915_pciids.h 추가 항목 */

/* Comet Lake-H GT1 */
#define INTEL_CML_H_GT1_IDS(info) \
    INTEL_VGA_DEVICE(0x9BC8, info)

/* Comet Lake-H GT2 */
#define INTEL_CML_H_GT2_IDS(info) \
    INTEL_VGA_DEVICE(0x9BC4, info), \
    INTEL_VGA_DEVICE(0x9BC5, info), \
    INTEL_VGA_DEVICE(0x9BC6, info), \
    INTEL_VGA_DEVICE(0x9BCA, info), \
    INTEL_VGA_DEVICE(0x9BCB, info), \
    INTEL_VGA_DEVICE(0x9BCC, info)

/* Comet Lake-S GT1 */
#define INTEL_CML_S_GT1_IDS(info) \
    INTEL_VGA_DEVICE(0x9BC0, info)

/* Comet Lake-S GT2 */
#define INTEL_CML_S_GT2_IDS(info) \
    INTEL_VGA_DEVICE(0x9B41, info), \
    INTEL_VGA_DEVICE(0x9BA4, info), \
    INTEL_VGA_DEVICE(0x9BA8, info), \
    INTEL_VGA_DEVICE(0x9BAA, info), \
    INTEL_VGA_DEVICE(0x9BAB, info), \
    INTEL_VGA_DEVICE(0x9BAC, info)
```

### 9.3 intel_device_info.c 연결 (발췌)

```c
/* drivers/gpu/drm/i915/intel_device_info.c 추가 항목 */

/* CML-H/S는 GEN9.5 공용 구조체 재사용 → 별도 info 불필요 */
static const struct intel_device_info intel_cometlake_h_info = {
    GEN9_FEATURES,
    PLATFORM(INTEL_COMETLAKE),
    .gt = 2,
};

/* pciidlist[] 추가 */
INTEL_CML_H_GT1_IDS(&intel_cometlake_info),   /* GT1: cometlake_info 공유 */
INTEL_CML_H_GT2_IDS(&intel_cometlake_h_info), /* GT2: H variant */
INTEL_CML_S_GT1_IDS(&intel_cometlake_info),
INTEL_CML_S_GT2_IDS(&intel_cometlake_h_info),
```

### 9.4 Cherry-pick 적용 순서

```
1. i915_pciids.h     → ID 매크로 정의 (의존성 없음, 가장 먼저)
2. intel_device_info.c → device_info 구조체 연결
3. i915_pci.c        → pciidlist[] 엔트리 등록
4. intel_crt.c       → CML-S VGA 처리 (CONFIG_DRM_I915_CML_S_VGA=n 시 생략 가능)
5. cml_hs_compat.h   → 추가 심볼 처리 확인
```

---

## 10. 빌드 단계별 절차

### Step 1. 5.4 LTS drm_core 추출

```bash
# 5.4 커널 소스에서 필요 파일 추출
git clone --depth=1 --branch v5.4 \
    https://github.com/torvalds/linux.git linux-5.4

cp -r linux-5.4/drivers/gpu/drm/{drm_*.c,ttm} \
    drivers/gpu/drm/drm_core/

# 심볼 목록 사전 추출 (shim 작업 참조용)
nm linux-5.4/vmlinux 2>/dev/null | grep "drm_\|dma_fence\|reservation" \
    > /tmp/5.4_drm_symbols.txt
```

**Pass 기준**: drm_core/ 디렉토리 빌드 시 undefined reference 없음

---

### Step 2. compat shim 레이어 작성 및 심볼 검증

```bash
# 4.4 커널 심볼 추출
nm /path/to/4.4.302/vmlinux | grep "drm_\|fence_\|reservation_" \
    > /tmp/4.4_symbols.txt

# 누락 심볼 비교 (shim 작성 대상 목록)
diff /tmp/5.4_drm_symbols.txt /tmp/4.4_symbols.txt | grep "^<" \
    > /tmp/missing_symbols.txt

# modpost 검증
make -C /path/to/kernel M=$(pwd) modules 2>&1 | grep "undefined"
```

**Pass 기준**: `missing_symbols.txt` 내 모든 심볼이 compat/ 레이어에서 해결됨

---

### Step 3. CML-H/S Cherry-pick 적용

```bash
# 5.5-rc1 기준 커밋 cherry-pick
cd drivers/gpu/drm/i915
git cherry-pick <CML-H commit hash>   # intel_device_info.c, i915_pciids.h
git cherry-pick <CML-S commit hash>   # i915_pci.c

# 충돌 확인
git diff --stat HEAD~2
```

**Pass 기준**: cherry-pick 후 컴파일 에러 없음, CML-H/S PCI ID 매크로 정상 확장

---

### Step 4. i915 단독 빌드 검증

```bash
make -C /path/to/kernel M=$(pwd)/drivers/gpu/drm/i915 modules

# 로드 테스트 (PCI ID 인식 확인)
insmod drm.ko && insmod i915.ko

# Pass 기준 확인
dmesg | grep -E "i915.*GEN(9|11)|ICL|CML" | head -20
dmesg | grep -i "i915.*initialized"
```

**Pass 기준**:
```
[OK] dmesg: "i915: GEN11 (ICL)" 또는 "GEN9.5 (CML)" 초기화 메시지
[OK] /dev/dri/card0, /dev/dri/renderD128 생성 확인
[NG] NULL pointer dereference in drm_* → shim 재검토
[NG] "i915: No hardware" → PCI ID cherry-pick 미적용
```

---

### Step 5. amdgpu 단독 빌드 검증

```bash
make -C /path/to/kernel M=$(pwd)/drivers/gpu/drm/amd modules

insmod ttm.ko && insmod amdgpu.ko

dmesg | grep -E "amdgpu.*navi|RDNA|GFX10" | head -20
```

**Pass 기준**:
```
[OK] dmesg: "amdgpu: NAVI10" 또는 "GFX10.1" 초기화 메시지
[OK] /dev/dri/renderD129 생성 확인
[NG] "amdgpu: ttm_bo_init failed" → TTM 단일 인스턴스 재확인
```

---

### Step 6. Dual DRM 통합 빌드

```bash
# TTM 중복 로드 방지 확인
grep -r "MODULE_ALIAS.*ttm" drivers/gpu/drm/{i915,amd}/ \
    | grep -v "drm_core"  # 결과 없어야 함

make -C /path/to/kernel M=$(pwd) modules

# 로드 순서: drm_core → TTM → i915 → amdgpu
insmod drm_core/drm.ko
insmod drm_core/ttm/ttm.ko
insmod i915/i915.ko
insmod amd/amdgpu/amdgpu.ko
```

**Pass 기준**:
```
[OK] lsmod | grep ttm → 단일 인스턴스 (refcount 2 이상)
[OK] /dev/dri/ → card0, card1, renderD128, renderD129 모두 생성
[NG] "disagrees about version of symbol drm_*" → 버전 불일치, drm_core 재추출
```

---

### Step 7. 기능 검증

```bash
# i915 QSV (Intel 동영상 엔진)
vainfo --display drm --device /dev/dri/renderD128
# 기대: VAProfileH264*, VAProfileHEVC* 등 출력

# amdgpu VCN (AMD 동영상 엔진)
vainfo --display drm --device /dev/dri/renderD129
# 기대: VAProfileH264*, VAProfileHEVC* 등 출력

# Dual 동시 트랜스코딩 안정성 (30분 연속)
ffmpeg -hwaccel vaapi -hwaccel_device /dev/dri/renderD128 \
       -i input.mkv -vf "format=nv12,hwupload" \
       -c:v h264_vaapi output_intel.mp4 &

ffmpeg -hwaccel vaapi -hwaccel_device /dev/dri/renderD129 \
       -i input.mkv -vf "format=nv12,hwupload" \
       -c:v h264_vaapi output_amd.mp4 &

wait
```

**Pass 기준**:
```
[OK] 양쪽 vainfo 정상 출력
[OK] 30분 Dual 트랜스코딩 완료, dmesg에 GPU hang 없음
[NG] "GPU HANG" 또는 "ring timeout" → compat shim 안정성 재검토
```

---

## 11. 리스크 및 대응

| 리스크 | 가능성 | 영향도 | 대응 방안 |
|---|---|---|---|
| shim 불완전 → DRM 초기화 크래시 | 중 | 높음 | Step 4/5 단독 빌드 조기 검출 |
| TTM 이중 로드 → 심볼 충돌 | 중 | 높음 | Kconfig TTM 단일 선언, Step 6 lsmod 확인 |
| Cherry-pick 충돌 → 컴파일 에러 | 중 | 중간 | 5.4↔5.5 delta 최소화, 수동 conflict 해결 |
| CML-H/S cherry-pick 후 초기화 실패 | 저 | 중간 | GEN9.5 공용 경로 확인, intel_device_info 구조체 검증 |
| amdgpu DC 초기화 실패 | 중 | 중간 | 5.4 DC 버전 고정, atomic commit path 검증 |
| GuC/HuC 펌웨어 로드 실패 | 중 | 중간 | firmware/ 경로 DSM 환경 맞춤 설정 (§12 참조) |
| DSM 모듈 서명 검증 실패 | 높음 | 높음 | Redpill hook으로 검증 우회 확인 (§12 참조) |
| dma_resv shim 누락 → GEM 크래시 | 중 | 높음 | reservation_object 전체 accessor 매핑 완료 확인 |

---

## 12. DSM 환경 특유 제약

### 12.1 모듈 서명 검증

```
DSM은 커널 모듈 서명 검증을 수행함.
OOT 모듈은 서명되지 않아 기본 로드 불가.

대응:
  Redpill loader hook을 통한 서명 검증 우회 확인 필수.
  module.sig_enforce=0 커널 파라미터 적용 여부 확인.
```

### 12.2 펌웨어 로드 경로

```
일반 Linux:    /lib/firmware/i915/
DSM 환경:      경로 상이할 수 있음

대응:
  /usr/lib/firmware/ 또는 /run/firmware/ 경로 확인.
  firmware_class 커널 파라미터 또는 심링크로 경로 보정.

  GuC/HuC 대응 버전 (5.4 기준):
    icl_guc_33.0.0.bin   (ICL GEN11)
    icl_huc_9.0.0.bin    (ICL GEN11)
    cml_guc_33.0.0.bin   (CML-H/S, 동일 바이너리 사용 가능)
```

### 12.3 DRM Device Node 생성

```
DSM은 일반 udev 규칙을 사용하지 않음.
/dev/dri/card*, /dev/dri/renderD* 자동 생성 여부 확인 필요.

대응:
  mdev 또는 DSM devtmpfs 확인.
  필요 시 rc.local 등 부팅 스크립트에서 수동 mknod 처리.
```

### 12.4 메모리 제약

```
DSM NAS 환경의 제한된 RAM 환경에서:
  - amdgpu VRAM aperture 설정 최소화
  - i915 stolen memory 크기 조정
  - TTM eviction 정책 확인

권장 커널 파라미터:
  i915.enable_guc=2          # GuC 제출 모드
  i915.enable_fbc=1          # 프레임버퍼 압축
  amdgpu.vm_size=64          # VM 크기 최소화 (기본 256)
  amdgpu.vram_limit=256      # VRAM 사용 상한 (MB)
```

---

## 13. 선행 레퍼런스

| 프로젝트 | 설명 | 참고 용도 |
|---|---|---|
| **linux-backports** | kernel.org 공식 백포트 프레임워크 | compat shim 패턴 참조 |
| **backport-iwlwifi** | Intel WiFi 드라이버 OOT 백포트 | compat 레이어 구조 참조 |
| **DKMS i915-sriov** | i915 SR-IOV OOT (Proxmox용) | 최신 i915 OOT 패치 패턴 |
| **amdgpu-pro linux** | AMD 공식 OOT DKMS 모듈 | amdgpu compat 구조 참조 |

```
linux-backports 활용 시 이미 구현된 shim 항목 (재활용 권장):
  - compat/backport-4.4.h  → dma_fence 매핑
  - compat/backport-4.9.h  → drm_atomic 일부
  - compat/backport-5.1.h  → dma_resv 매핑
```

---

## 14. 참고 버전 정보

| 항목 | 버전 | 비고 |
|---|---|---|
| 베이스 커널 | 4.4.302 | DSM 기반 |
| 레퍼런스 성공 빌드 | 5.10.55 + **mt65 OOT** | 기존 성공 사례 |
| 목표 OOT 베이스 | Linux **5.4 LTS** (5.4.x 최신 stable) | MT54 OOT |
| Cherry-pick 출처 | Linux 5.5-rc1 (CML-H/S 커밋) | i915_pciids.h 등 |
| 타깃 i915 세대 | GEN5 ~ GEN11 (1세대 Ironlake ~ 10세대 ICL/CML) | CML-H/S 포함 |
| 타깃 amdgpu 세대 | GCN4 (Polaris 4.7+) ~ RDNA1 (Navi) | |
| linux-firmware | 5.4 LTS 대응 버전 고정 | GuC/HuC 버전 매칭 필수 |

---

## 15. 결론 요약

> 1. **OOT 전략**: Linux **5.4 LTS** 단일 소스 + **CML-H/S Cherry-pick** (5.5-rc1 커밋)
>    → 10세대 전 라인업(ICL, CML-U/H/S) 완전 커버, LTS 안정성 유지
>
> 2. **PCI ID Override 패치**: MT54 + Cherry-pick 적용 시 **10세대 전 라인업 완전 제거 가능**
>    → ICL/CML-U: MT54 네이티브, CML-H/S: Cherry-pick 적용
>
> 3. **Dual DRM**: drm_core를 5.4 단일 버전으로 통일, TTM 단일 인스턴스 강제
>    → 심볼 충돌 방지, amdgpu RDNA1 동시 지원
>
> 4. **compat shim**: 4.4 API gap 보정 레이어 필수 구성
>    → dma_fence, dma_resv, drm_gem_object_put 양방향 매핑 완료
>
> 5. **빌드 순서**: drm_core 추출 → shim 검증 → Cherry-pick → i915 단독 → amdgpu 단독 → Dual 통합
>
> 6. **DSM 환경 제약**: 모듈 서명, 펌웨어 경로, device node 생성 별도 확인 필수

