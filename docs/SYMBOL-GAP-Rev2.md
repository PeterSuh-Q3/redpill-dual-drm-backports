# 2차 심볼-갭 (전수 인벤토리 + 로컬 정적 정밀화, 2026-06-13)

> Rev1(샘플링) 과신 정정. 방법 = **A** CI 전수 인벤토리(keep-going+-Wno-error 1회) +
> **B** 로컬 정적 분석(vendor 5.4 헤더 #include ↔ 4.4.302 제공 대조, 빌드 불필요).
> 결론: gap 은 산발적 API 델타가 아니라 **bounded 한 backport-include 헤더 레이어**(~10).

## A) CI 전수 인벤토리 결과 — cascade 2종이 모든 파일 초기 차단

drm_compat.h 가 force-include 하는 `<drm/drmP.h>` 체인이 매 TU 초입에서:
1. `linux/sched/signal.h: No such file` (4.4.302 = 4.11 헤더 재편 이전)
2. `struct hdr_sink_metadata incomplete` (4.4.302 linux/hdmi.h 에 타입 부재)
→ 이 둘 때문에 거의 모든 파일이 동일 지점에서 멈춰, 더 깊은 델타는 가려짐.
부수 교훈: drm_compat.h 가 drmP.h 를 전역 force-include → cascade 증폭(수정 대상).

## B) 로컬 정적 — 5.4 drm/i915 가 요구하나 4.4.302에 없는 커널 헤더 (확정)

vendor 5.4 헤더/소스의 `#include <linux/*>` 124개 중 **누락 8** (asm/* 는 arch 경로라 제외):

| 누락 헤더 | 사용처(빌드 경로) | shim 전략 |
|---|---|---|
| `linux/sched/signal.h` | drm_os_linux.h, drm core 다수 | → `#include <linux/sched.h>` |
| `linux/dma-resv.h` | drm_gem.h (+ttm) | → `#include <linux/reservation.h>` + dma_resv 매핑 |
| `linux/dma-fence-chain.h` | drm_syncobj.h (i915 syncobj) | 백포트/stub |
| `linux/mem_encrypt.h` | drm_gem.c, drm_vm.c | stub: `sme_active()→0` 등 |
| `linux/pseudo_fs.h` | drm_drv.c | 백포트: init_pseudo/pseudo_fs_context(~5.3) |
| `linux/stackdepot.h` | drm_mm.h, i915_drv.h | stub(디버그 기능) |
| `linux/media-bus-format.h` | drm_crtc.h | MEDIA_BUS_FMT_* 정의 제공 |
| `asm/set_memory.h` | (주로 amdgpu) | → `#include <asm/cacheflush.h>` |

## B') present 헤더의 누락 타입/멤버

| 타입/멤버 | 헤더 | 사용처 | shim |
|---|---|---|---|
| `struct hdr_sink_metadata` | linux/hdmi.h | drm_connector.h | 구조체 정의 제공(force-include) |

## 구현 계획 (backport-include 레이어 = MT54 의 실체, bounded)

1. `compat/backport-include/` 신설 → LINUXINCLUDE 최우선 prepend.
2. 위 누락 헤더별 shim 파일 작성:
   - 단순 redirect: sched/signal.h, dma-resv.h, asm/set_memory.h
   - stub: mem_encrypt.h, stackdepot.h
   - 백포트(실로직): pseudo_fs.h, dma-fence-chain.h(syncobj 사용 시)
   - 정의 제공: media-bus-format.h, struct hdr_sink_metadata
3. drm_compat.h 에서 `<drm/drmP.h>`/`<drm/drm_gem.h>` 전역 force-include 제거(cascade 차단).
4. 재빌드 → 다음 층(드라이버 내부 델타) 인벤토리.

## 정정 요지

- Rev1 "shim 2개면 충분"은 **표본 편향 과신**. 실제 = backport-include 헤더 ~10.
- 단, **여전히 bounded**: 수백 패치가 아니라 헤더 레이어 한 묶음(대부분 redirect/stub).
  이것이 i915-backports 의 `backport-include/` 와 동일 성격 = "MT54 백포트 인프라"의 실체.
- 불변 결론: §9 소거, GEN11 네이티브, 제거한 매크로 shim 들.
