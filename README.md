# Redpill Dual DRM Backport

Redpill DSM 커널 **4.4.302** 환경에 인텔 i915 + AMD amdgpu 를 **동시 지원**하는
Dual DRM OOT 백포트. OOT 베이스는 **Linux 5.4 LTS**.

> 전체 설계 비전은 [`CLAUDE.md`](./CLAUDE.md) 참조.
> CLAUDE.md **§0 (Rev.3)** 에 실제 구현 현황 / 설계 대비 변경점이 정리되어 있다.

## 현재 진행 파일럿

설계서의 광범위 목표(GEN5~GEN11, CML-H/S, GCN4~RDNA1) 중 **첫 실증 타깃**을 진행 중:

| 항목 | 값 |
|---|---|
| 플랫폼 | **geminilake / DSM 7.3 / 커널 4.4.302** |
| Intel iGPU | Gemini Lake (GEN9 Atom) — i915 |
| AMD dGPU | Polaris12 (WX 3100, `1002:6985`) — amdgpu |
| 검증기 | ECS H110M4-C2D 실기 |

실기 현황:
- **amdgpu**: ✅ 정상 로드 — `/dev/dri/card1`, `renderD129` 생성, `Initialized amdgpu 3.35.0`
- **i915**: 🔧 부팅 panic 수정 후 재검증 대기 (§0 해결 이슈 5 참조)

## 빌드 (GitHub Actions)

로컬 `scripts/*.sh` 7단계 대신 **CI 워크플로우**로 빌드한다 (Synology 툴체인 컨테이너 사용).

```
.github/workflows/
  step4-i915-geminilake-44302.yml     # i915 단독
  step5-amdgpu-geminilake-44302.yml   # amdgpu 단독
  step6-dual-drm-geminilake-44302.yml # Dual 통합 (정식 산출)
  verify-modules-geminilake-44302.yml # depmod -e 기반 심볼 검증
```

```bash
# integration/dual-drm 브랜치 기준으로 트리거
gh workflow run step6-dual-drm-geminilake-44302.yml --ref integration/dual-drm
gh workflow run verify-modules-geminilake-44302.yml --ref integration/dual-drm
```

산출 `.ko` 는 `module-output` 브랜치(`thirdparty/geminilake-7.3-4.4.302/`)로 push 되며,
거기서 추출해 `mshell-modules` / `tcrp-modules` 릴리즈로 배포한다.

## 브랜치 구조

| 브랜치 | 역할 |
|---|---|
| `main` | 레시피 — compat shim / backport-include / Makefile / 워크플로우 / 문서 |
| `vendor/drm-5.4` | 5.4 LTS DRM 원본 vendoring |
| `integration/dual-drm` | vendor 소스 + main 머지 + **소스 직접 패치**. CI 빌드 대상 |
| `module-output` | CI 산출 `.ko` (orphan) |

> ⚠️ **프로세스 규칙**: `main` 레시피 변경 후 반드시
> `integration/dual-drm` 에 `git merge main` → push → 그 다음 빌드 트리거.
> (머지 누락 시 CI 가 stale integration 을 빌드)
> 소스 자체 패치는 `integration/dual-drm` 에서 직접 수정.

## compat 레이어 (실제 구조)

```
compat/
  backport-include/linux/*.h   시스템 헤더를 가로채 4.4.302 누락 API 제공
                               (wait.h, workqueue.h, xarray.h, dma-resv.h ...)
  kcl_compat.h                 -include force-include — 전 TU 에 shim 주입
  drm_compat.h / i915_compat.h / amdgpu_compat.h / reservation_compat.h
  dma-fence-chain.c            보조 소스
```

> 설계서(§6)의 `dma_fence`→`fence` 매핑 방식은 폐기되었다. 4.4.302 가
> 이미 `dma_fence_*` 를 네이티브로 export 하므로 매핑이 아니라
> **누락 API 보강 + ABI 차이 보정**이 실제 작업이다.

## DSM 환경 제약

- 모듈 서명: Redpill loader hook (서명 미적용 OOT 적재)
- 헤드리스: 디스플레이 출력 미사용 → fbdev 에뮬레이션 제거 (§0 이슈 3)
- device node: 부팅 시 `/dev/dri/card*`, `renderD*` 생성 확인
- amdgpu VM size 64GB 는 GPU 가상주소 공간으로 정상값 (시스템 RAM 무관)

## DRM 보조 모듈 의존성 매트릭스 (4.4.302 계열)

이 저장소가 빌드하는 `drm.ko`/`amdgpu.ko`/`i915.ko`는 완전히 자기완결적이지 않다.
`hdmi.ko`, `drm_mipi_dsi.ko`, `backlight.ko` 등 일부 심볼은 **host(DSM stock) 커널이
빌트인으로 제공하는 플랫폼도 있고, 전무해서 별도 번들이 필요한 플랫폼도 있다** —
획일적으로 처리하면 심볼 중복(host 빌트인 플랫폼) 또는 Unknown symbol(host 미제공
플랫폼) 중 하나로 반드시 실패한다.

| 플랫폼 | hdmi.ko | drm_mipi_dsi.ko | sync_file | backlight.ko/fb.ko | drm_panel_orientation_quirks.ko | 검증 상태 |
|---|---|---|---|---|---|---|
| geminilake / apollolake | host 빌트인 — 번들 금지 | host 빌트인 — 번들 금지 | ✅ `drm.ko` 자체 내장 | `.config` 상 `=m` (레거시 번들, mshell-modules) | ✅ CI 빌드 | ✅ 확정(`.config` CONFIG_HDMI/MIPI_DSI/SYNC_FILE=y, 7.2/7.3/7.4 동일) |
| **broadwellnk / broadwell / broadwellnkv2 / broadwellntbap / denverton / purley / r1000 / v1000** (8개 전부) | **host 없음 — 번들 필요** | **host 없음 — 번들 필요** | ✅ `drm.ko` 자체 내장(2026-08-17 수정) | `.config` 상 `=m`이나 **broadwellnk 7.4 실기엔 파일 자체 부재** — 컴파일 가능≠설치 이미지 포함 보장 | CI 미빌드(CONFIG 미설정, broadwellnk만 2026-08-17 추가) | ✅ **`.config` 전수 확인**(`dante90/syno-compiler:7.2/7.3/7.4`, 8플랫폼×3버전=24조합 전부 CONFIG_HDMI/MIPI_DSI/SYNC_FILE 미설정) + broadwellnk만 실기 kallsyms/insmod 이중검증 |

**DSM 버전(7.2/7.3/7.4)은 이 매트릭스에 영향 없음 — 플랫폼 커널 계열로만 갈린다.**
2026-08-17 `dante90/syno-compiler:7.2/7.3/7.4` 이미지의 각 플랫폼 `.config`를 직접 대조해 확정 —
실기 없이도 `CONFIG_HDMI`/`CONFIG_DRM_MIPI_DSI`/`CONFIG_SYNC_FILE`/`CONFIG_DRM_PANEL_ORIENTATION_QUIRKS`
유무로 host 빌트인 여부를 판별 가능함을 broadwellnk 실기 결과와 대조해 검증 완료.
단 `backlight.ko`/`fb.ko`는 `.config`가 `=m`이라도 실제 DSM 설치 이미지 포함 여부는 별개 —
실기 확인이 계속 필요하다.

**오늘(2026-08-17) 발견/수정된 sync_file 이슈**: DSM 4.4.302 stock 커널은 버전(7.2/7.3/7.4)
무관하게 `CONFIG_SYNC_FILE`이 비활성이라 `sync_file_create`/`sync_file_get_fence`가 host에
전무함 — vendor `drivers/dma-buf/sync_file.c`를 `drm-y`에 fold해 `drm.ko`가 자체 제공하도록
수정(`compat/backport-include/linux/sync_file.h`). **모든 4.4.302 플랫폼에 공통 적용됨.**
단, mshell-modules 쪽에 이미 배포된 구버전 독립 `sync_file.ko`(16개 조합)는 신 `drm.ko`와
심볼이 중복되므로 제거 필요.

**DSM 7.4 특이사항**: 10개 플랫폼 전부 이 저장소 CI 산출물만 존재(레거시 backfill 전무) —
`drm_panel_orientation_quirks.ko`(오늘 CONFIG 추가분 제외)와 `hdmi`/`backlight`/`drm_mipi_dsi`가
broadwellnk 외 9개 플랫폼엔 전혀 없다. `drm_mipi_dsi.c`는 이 저장소 vendor 소스에 이미 존재하므로
`CONFIG_DRM_MIPI_DSI=m` 워크플로우 추가만으로 해결 가능하나, `hdmi.c`/`backlight.c`는
`drivers/video/` 트리 자체가 vendoring 되어 있지 않아 별도 작업이 필요하다.

관련 저장소: [PeterSuh-Q3/mshell-modules](https://github.com/PeterSuh-Q3/mshell-modules) —
이 저장소의 CI 산출물을 배포·패키징하는 쪽. `thirdparty/blacklist.lst`,
`.github/workflows/main-tar-modules.yml`의 플랫폼별 `case` 문에서 host 빌트인 예외를 관리한다.
동일 매트릭스가 그쪽 README에도 있다.
