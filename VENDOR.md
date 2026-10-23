# vendor/drm-5.4 — pinned 5.4 LTS drm 스냅샷 (원자재 브랜치)

Linux **v5.4** (tag) 에서 추출한 drm 서브트리. main 의 레시피(compat/patches)와 분리된 lineage.
integration/dual-drm 이 이 스냅샷 위에 patches/ 를 적용해 Dual 빌드한다.

## 핀 (재현성)

| 항목 | 값 |
|---|---|
| upstream | torvalds/linux |
| tag | **v5.4** |
| commit | `219d54332a09e8d8741c1e1982f5eae56099de85` |
| 추출일 | 2026-06-13 |

> **단일 커밋 = drm_core 단일버전 원칙(§4.1) 구조적 보장** — i915·amd 가 동일 v5.4 에서 추출됨.

## 포함 경로 (faithful upstream 레이아웃)

```
include/drm/            include/uapi/drm/
drivers/dma-buf/        (dma-fence.c, dma-resv 등 — i915/amdgpu/ttm 의존)
drivers/gpu/drm/        코어 *.c/*.h (88개)
drivers/gpu/drm/ttm/    drivers/gpu/drm/scheduler/
drivers/gpu/drm/i915/   drivers/gpu/drm/amd/  (amdgpu + display DC)
drivers/gpu/drm/vmwgfx/ (VMware SVGA II KMS; Linux v5.4 exact source)
```

설계서 §7 의 `drm_core/` 재배치(rename)는 integration 단계에서 Makefile/patch 로 수행.
nouveau/radeon 등 타 드라이버는 sparse-checkout 으로 제외(불필요).

## VMware SVGA II 추가 경로

`drivers/gpu/drm/vmwgfx/`는 K4.4.302 KMS pilot을 위해 같은 upstream v5.4
고정 커밋(`219d54332a09e8d8741c1e1982f5eae56099de85`)에서 가져온 65개 원본 파일이다.
K4 호환성 수정은 integration 브랜치에서 별도로 관리하며, 이 소스 추가만으로
K4 ABI 적재 가능성이나 실기 KMS 성공을 의미하지 않는다.

## 갱신 (5.4.x bump)

main 의 `scripts/01_extract_drm_core.sh` 를 새 v5.4.y 로 돌려 이 브랜치를 교체 커밋하고
위 표의 commit/tag/추출일을 갱신한다.
