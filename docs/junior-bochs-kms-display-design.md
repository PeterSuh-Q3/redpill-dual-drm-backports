# Junior bochs 가상 디스플레이 출력 추가 설계

작성: 2026-10-21. 상태: 설계·구현 및 K4 컴파일 완료, 실기 적재/출력 검증 대기.
기준 저장소는 redpill-dual-drm-backports이며 기존
[i915 안정화 설계](junior-i915-kms-stability-design.md)를 보완한다.

## 목적과 확인된 차이

QEMU 가상 VGA를 bochs DRM으로 구동해 Junior 상태 화면을 가상 콘솔에
출력한다. Intel GPU가 함께 존재하더라도 가상화면 출력 대상을 정확히 선택한다.

현재 실기에서 직접 확인한 사실:

| 항목 | 관측 결과 |
| --- | --- |
| 런타임 | geminilake DSM 7.4, 4.4.302+ #90080, QEMU |
| 가상 VGA | PCI 0000:00:02.0, 1234:1111, driver 심볼릭 링크 없음 |
| Intel GPU | PCI 0000:00:10.0, 8086:1912, i915에 바인딩 |
| DRM 노드 | card0/renderD128만 존재하며 i915에 연결 |
| bochs 모듈 | /proc/modules에 없음; 조사한 /exts·/tmp·/lib/modules 파일 검색에도 없음 |
| 이번 파일럿 | 17개 .ko 목록에 bochs.ko 없음 |
| 소스 트리 | drivers/gpu/drm/Kconfig에는 bochs/Kconfig 참조가 있으나 해당 디렉터리와 DRM Makefile 연결이 없음 |

이번 17개 파일럿을 bochs까지 포함된 완성 모듈팩으로 취급하면 안 된다.
압축된 원래 배포 팩에 bochs가 존재하는지까지 위 파일 검색으로 판정할 수는 없다.
원래 팩 목록·해시와 all-modules 추출/선택 로그는 구현 전 추가 대조한다.

현 상태에서 가상 VGA는 DRM 드라이버에 바인딩되지 않아 helper가 사용할
bochs 카드가 없다. i915 HDMI 경고가 bochs 출력을 직접 중단시켰다는 증거는
없다. 먼저 bochs 드라이버 확보와 바인딩을 해결해야 한다.

## helper가 i915를 건드린 이유

tcrp-addons/junior-realtime-helper/src/kms-display.c의 main은 모든 cardN을
순회하고 장치를 연 뒤 SET_MASTER와 setup_display를 호출한다. 현재 선택
인자는 STATUS_FILE 하나이며, 로컬 코드에는 bochs 전용 대상 지정이 없다.
따라서 유일하게 존재한 i915 card0를 설정하는 과정에서 HDMI 경고가 발생했다.
로컬 소스와 실제 배포 helper의 버전/해시는 구현 직전에 다시 맞춘다.

구현한 선택 정책:

- 기존 일반 다중 출력 동작을 유지하면서 선택적 driver 인자를 추가했다.
  `kms-display STATUS_FILE bochs-drm`처럼 호출하면 sysfs에서 카드의 driver를
  확인해 `bochs-drm`에 바인딩된 카드만 연다.
  모듈명 bochs와 PCI driver명 bochs-drm을 구별한다.
- `selftest.sh`는 PCI vendor/device `1234:1111`을 찾으면 helper에 `bochs-drm`
  필터를 전달한다. 카드 번호를 고정하지 않으며, Bochs 카드가 아직 없거나
  바인딩되지 않은 경우 i915 카드로 자동 우회하지 않는다.
- 해당 QEMU VGA 식별자가 없는 시스템은 기존의 무필터 다중 카드 동작을 유지한다.
- 다중 출력 시험에서는 카드별 worker와 단계 로그를 둔다. 한 카드의 실패로
  다른 카드 초기화가 막히는 범위를 줄이되 커널 Oops 격리가 보장된다고 표현하지 않는다.
- 로그에 선택된 PCI/driver/card/connector/mode, create/map/first-touch/addfb/
  setcrtc/render/restore 결과와 errno를 기록한다.

## bochs 소스와 모듈팩 준비

### 확인한 K5 참조 구현

사용자가 지정한 [quad-gpu-backports](https://github.com/PeterSuh-Q3/quad-gpu-backports)의
로컬 main과 원격 main은 조사 시점에 모두
`c3a07f405e47e7c40c87d5bd4ad648f9fab1071a`였다.
bochs 도입 커밋은 `76bd223` (`feat: add QEMU Bochs DRM pilot driver`)이다.

- 구현: `drivers/gpu/drm/bochs-v65/bochs.c`, GPL-2.0-or-later.
  PCI 테이블에 1234:1111과 QEMU 하위 장치 식별자가 있고 PCI driver명은
  `bochs-drm`이다. 현재 실기 가상 VGA와 매칭된다.
- 빌드: `drivers/gpu/drm/Makefile`의 `CPTCFG_DRM_BOCHS` 분기가
  bochs-v65를 연결한다. `.github/scripts/build-unified-drm.sh`는
  `CPTCFG_DRM_BOCHS=m`을 설정·검증하고 bochs를 필수 산출물로 검사한다.
- 통합: `.github/workflows/build-74-only-mt65.yml`은 위 스크립트를 사용하며
  bochs, drm_vram_helper, drm_ttm_helper 등 의존 모듈을 배포 목록에 포함한다.
- K4 이식 시 확인할 API: drmm_mode_config_init/drmm_kzalloc,
  drmm_vram_helper_init, DRM_GEM_VRAM_DRIVER, drm_aperture 제거 함수,
  drm_fbdev_generic_setup 및 simple-display-pipe 준비/해제 콜백.
  현재 K4 트리에는 simple KMS/GEM VRAM helper 소스가 있지만 K5와 동일한
  managed/aperture 인터페이스가 준비돼 있다고 볼 수 없다.

K5 소스의 장치 초기화·VBE/MMIO·화면 설정 및 필수 산출물 검사를 참조해
K4의 DRM/TTM API에 맞게 이식한다. managed 자원 처리의 대체가 필요하면
probe 실패·remove·shutdown에서 해제 순서까지 구현한다. K5 헤더만 복사해
컴파일을 통과시키거나 K5 바이너리를 K4에 적재하는 방식은 사용하지 않는다.
소스상의 통합 이력 확인과 K4 실기 성공 여부를 구분한다.

### K4 구현 순서

1. 기존 배포 팩의 bochs 포함 여부와 원천 커밋을 확인하고, 위 K5 구현을
   기준으로 이 저장소의 DRM API에 맞는 이식 패치와 원천·라이선스를 기록한다.
   다른 플랫폼에서 빌드된 .ko를 복사해 해결하지 않는다.
2. geminilake DSM 7.4의 정확한 GPL 소스/.config/Module.symvers와 기존
   Docker Desktop 7.4 툴체인으로 bochs Kconfig/Makefile 연결을 완성한다.
   CONFIG_DRM_BOCHS=m 및 PCI/MMIO/framebuffer/TTM 요구사항은 선택한 소스로 확인한다.
3. bochs와 DRM/TTM 등의 실제 의존 모듈을 같은 빌드 기준으로 생성한다.
   modinfo 의존 목록, 미정의 심볼/제공자, MODVERSIONS 사용 시 CRC를 검사한다.
   built-in인 drm_mipi_dsi/intel-gtt를 중복 적재하지 않도록 런타임 제공자도 기록한다.
4. PCI modalias 1234:1111의 매칭과 bochs probe를 확인한다. 기존 VGA framebuffer가
   aperture를 소유하면 소유자·해제 경로를 먼저 확인하고 무조건 unbind하지 않는다.
5. all-modules가 bochs.ko와 의존 파일을 추출하고 depmod 이후 modalias에 따라
   적재하는지 확인한다. 파일 포함과 자동 적재 성공은 별도 검증한다.

### 구현 및 K4 빌드 결과 (2026-10-21)

- Linux v5.4의 Bochs DRM 구현을 `drivers/gpu/drm/bochs-v54/`에 이식하고,
  DRM Kconfig/Makefile에 연결했다. upstream v5.4와 동일하게 GEM VRAM helper,
  VRAM common/MM helper를 별도 모듈로 쪼개지 않고 `drm_vram_helper.ko` 하나로
  링크한다.
- Docker Desktop의 `dante90/syno-compiler:7.4` 이미지에서 geminilake의
  `/opt/geminilake/build` (`4.4.302+`)를 사용했다. `KCFLAGS`에 공용 K4
  `kcl_compat.h`와 `CONFIG_DRM_HDMI_IN_KERNEL` 분기를 지정하고, Kconfig 선택은
  `defines.geminilake`로 주입했다. 빌드 로그에는 `MODPOST 15 modules`가 있고,
  최종 산출 디렉터리에는 17개의 `.ko`가 있다.
- 재현 명령(저장소 루트에서):
  `docker run --rm -e 'KCFLAGS=-include /tmp/input/compat/kcl_compat.h -DCONFIG_DRM_HDMI_IN_KERNEL' -v "$PWD:/input:ro" -v "$PWD/out/junior-bochs-kms-pilot/geminilake-7.4-k4.4.302:/output" dante90/syno-compiler:7.4 compile-module geminilake`.
- `bochs-drm.ko`의 vermagic은 `4.4.302+ SMP mod_unload`; PCI alias에
  `1234:1111`이 포함된다. 의존성은 `drm`, `drm_kms_helper`,
  `drm_vram_helper`, `fb`; VRAM helper는 `ttm`, `drm`에 의존한다.
  MODPOST 미해결 심볼 오류는 확인되지 않았다.
- 산출물과 로그: `out/junior-bochs-kms-pilot/geminilake-7.4-k4.4.302/`.
  helper C 코드는 `gcc -static -Wall -Wextra -Werror`로 컴파일했고,
  결과 바이너리는 정적 ELF임을 확인했다. `selftest.sh`의 `sh -n` 검사도 통과했다.
- 아직 실기에서 새 모듈을 적재하거나 `bochs-drm` 바인딩, DRM 카드 생성,
  실제 화면 출력을 확인하지 않았다. 이 결과는 컴파일 검증이지 KMS 동작 성공이
  아니다. 기존 i915 HDMI InfoFrame 경고도 별도 미해결 항목으로 남는다.

## 남아 있는 i915 HDMI 경고의 별도 보완

실행 로그는 intel_hdmi.c:654의 WARN_ON(len < 0), AVI/SPD infoframe 불일치,
infoframes.enable 기대값 0x19/실제값 0x01, pipe state 불일치를 보여준다.
소스상 compat/backport-include/linux/hdmi.h는 CONFIG_DRM_HDMI_IN_KERNEL이
정의되면 hdmi_infoframe_pack_only()를 항상 -ENOSYS로 반환한다.
기존 pilot-geminilake-dsm74-i915-pmu.yml도 해당 옵션을 지정한다.

이는 이번 경고와 직접 연결될 수 있는 구체적인 후보다. 실제 파일럿 빌드의
전처리 결과/명령을 확보해 이 분기의 적용 여부를 확정한다. 이후 커널 내장
HDMI 함수의 기능·자료구조 ABI를 확인하고, 필요한 pack-only 구현을 같은
헤더 기준의 private/namespaced 호환 함수로 제공하는 방안을 우선 검토한다.
정상 구현이 이미 커널에 존재하면 호환성을 확인해 사용한다. 중복 전역 심볼을
생성하거나 WARN_ON만 제거하거나 성공값을 반환하는 방식으로 숨기지 않는다.
AVI/SPD의 올바른 패킹 길이·체크섬과 오류 입력을 검사하고 실기 상태 비교도 통과해야 한다.

bochs 전용 가상화면 출력은 i915의 이 HDMI 경로를 호출할 필요가 없다.
i915 경고 수정과 bochs 출력 복구는 각각 검증한다.

## 검증 순서와 완료 조건

| 단계 | 시험 | 통과 조건 |
| --- | --- | --- |
| 1 | 패키지 정적 검사 | bochs.ko 포함, 대상 vermagic/의존성/PCI alias 일치 |
| 2 | 클린 Junior에서 bochs 적재 | 가상 VGA driver=bochs-drm, 해당 PCI의 DRM card 생성 |
| 3 | bochs 대상 buffer/map/first-touch | 각 단계 정상 반환, 새 Oops/lockup 없음 |
| 4 | bochs만 SETCRTC·정적 패턴 | 가상 콘솔에 실제 패턴 출력, 사용자가 화면 확인 |
| 5 | 상태 화면 30분 갱신 | 정상 갱신, 새 경고/오류 없음, TTYD 응답 유지 |
| 6 | 종료·복원·재시작 3회 | 가상 콘솔 및 DRM master 정상 복원 |
| 7 | i915 공존 | card 순서가 달라도 bochs 선택, bochs 시험 중 i915 SETCRTC 없음 |
| 8 | loader 통합 재부팅·DSM 전환 | bochs 자동 적재/출력 및 정상 전환 확인 |

파일럿의 기존 i915 콜백 수정은 유지한다. bochs 단독 출력 성공만으로 i915
InfoFrame 문제까지 해결됐다고 판단하지 않는다. 새로운 Oops/lockup 발생 시
해당 부팅에서 반복 적재하지 않고 로그를 보존한다. 이 문서는 추가 설계이며
컴파일 완료는 실기 적재·출력 검증 또는 배포 완료를 의미하지 않는다.
