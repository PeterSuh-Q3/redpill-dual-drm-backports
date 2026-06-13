# Redpill Dual DRM Backport

Redpill DSM 커널 **4.4.302** 환경에 인텔 i915(GEN5~GEN11) + AMD amdgpu(GCN4~RDNA1)를
동시 지원하는 **Dual DRM OOT 백포트 모듈**. OOT 베이스는 **Linux 5.4 LTS**,
CML-H/S 확장은 **5.5-rc1 Cherry-pick**.

> 설계 전문은 [`CLAUDE.md`](./CLAUDE.md) 참조 (Rev.2).

## 핵심 전략

- **OOT 베이스**: Linux 5.4 LTS 단일 소스 (i915 GEN11 + amdgpu RDNA1 동시 커버)
- **PCI Device ID Override 패치 제거**: MT54 네이티브 인식으로 대체. CML-H/S 는 Cherry-pick
- **Dual DRM 단일 버전 원칙**: drm_core / TTM 을 5.4 단일 인스턴스로 통일 (심볼 충돌 방지)
- **compat shim**: 4.4 ↔ 5.4 API gap 보정 레이어

## 디렉토리 구조

```
compat/                  4.4.302 전용 shim 레이어 (§6)
  drm_compat.{h,c}         공통 DRM API 매핑 + drm_atomic stub
  dma_fence_compat.c       fence -> dma_fence 브릿지
  reservation_compat.h     dma_resv -> reservation_object 전체 accessor
  i915_compat.h            i915 전용 shim
  amdgpu_compat.h          amdgpu 전용 shim
  cml_hs_compat.h          CML-H/S cherry-pick 전용
include/drm/
  i915_pciids_cml_hs.h     CML-H/S PCI ID 매크로 (§9.2)
drivers/gpu/drm/
  drm_core/                5.4 LTS 단일 소스 (Step1 에서 추출)
  i915/                    MT54 OOT + cherry-pick 산출물
  amd/{amdgpu,display}/    5.4 amdgpu + DC
firmware/{i915,amdgpu}/    GuC/HuC/RDNA1 펌웨어 (§12.2)
scripts/                   빌드 7단계 (§10)
Kconfig / Makefile         빌드 설정 (§8)
```

> `drm_core/`, `i915/`, `amd/` 의 실제 커널 소스는 라이선스/용량 문제로 동봉하지
> 않으며 `scripts/01_extract_drm_core.sh` 가 5.4 LTS 에서 추출한다.
> 각 디렉토리의 `EXTRACT.md` 참조.

## 빌드 절차 (§10)

```bash
# 0) 4.4.302 커널 소스 경로 지정
export KSRC=/path/to/4.4.302/kernel

# 1) 5.4 LTS drm_core 추출
scripts/01_extract_drm_core.sh

# 2) compat shim 심볼 검증
scripts/02_build_shim_verify.sh

# 3) CML-H/S cherry-pick 적용
scripts/03_cherrypick_cml_hs.sh

# 4~6) 단독 -> 통합 빌드
scripts/04_build_i915.sh
scripts/05_build_amdgpu.sh
scripts/06_build_dual.sh

# 7) 기능 검증 (QSV / VCN / Dual 트랜스코딩)
scripts/07_functional_test.sh input.mkv
```

## DSM 환경 제약 (§12)

- 모듈 서명: Redpill loader hook 또는 `module.sig_enforce=0`
- 펌웨어 경로: `/usr/lib/firmware/` 또는 심링크 보정
- device node: udev 부재 시 `rc.local` 에서 수동 mknod
- 권장 파라미터: `i915.enable_guc=2 amdgpu.vm_size=64 amdgpu.vram_limit=256`
