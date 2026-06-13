# vendor/drm-5.4 — pinned 5.4 LTS drm 스냅샷 (원자재 브랜치)

이 브랜치는 Linux **v5.4.x** 에서 추출한 drm_core + i915 + amd/{amdgpu,display} 단일
스냅샷만 담는다. (main 의 레시피와 분리된 lineage)

- 생성/갱신: main 의 `scripts/01_extract_drm_core.sh` 산출물을 여기에 커밋
- **단일 커밋 = drm_core 단일버전 원칙(§4.1) 구조적 보장**
- 핀 커밋 해시는 커밋 메시지에 기록 (재현성)

> 아직 비어 있음 — 다음 단계에서 v5.4.x 추출본을 채운다.
