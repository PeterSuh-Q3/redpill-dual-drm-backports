# drm_core/ — 5.4 LTS 단일 소스 추출 대상

이 디렉토리는 **빈 스캐폴드**다. 실제 소스는 Step 1 에서 Linux 5.4 LTS 에서 추출한다.
설계서 §7 / §10 Step1 기준 추출 대상:

    drm_atomic.c
    drm_atomic_helper.c
    drm_gem.c
    drm_dp_helper.c
    drm_dp_mst_topology.c
    dma-fence.c
    ttm/ttm_bo.c
    ttm/ttm_bo_util.c
    ttm/ttm_memory.c

추출 명령은 `scripts/01_extract_drm_core.sh` 실행.

> 원칙(§4.1): drm_core 는 5.4 단일 버전으로 통일. TTM 단일 인스턴스 강제.
