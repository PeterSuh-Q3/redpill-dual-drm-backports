# Junior i915 KMS 안정화 설계

작성: 2026-10-21. 상태: geminilake K4 콜백 ABI 파일럿 적재 및 helper 실행 확인.
이번 실행에서 이전 Oops는 관찰되지 않았으나 HDMI InfoFrame 경고가 남았다.
bochs 가상화면 출력은 별도 드라이버 준비가 필요하다.
추가 설계: [Junior bochs 가상 디스플레이](junior-bochs-kms-display-design.md).

## 구현 기준 저장소

커널 모듈 수정·빌드는 사용자가 지정한
`/Volumes/GITHUB/redpill-dual-drm-backports`를 기준으로 한다.
`drivers/gpu/drm/i915/i915_mm.c`, `compat/`와 실제 DSM K4 빌드 헤더를
먼저 대조한다. 기존 geminilake DSM 7.4 파일럿
`.github/workflows/pilot-geminilake-dsm74-i915-pmu.yml`의 툴체인·헤더
주입 경로를 참조한다. 이 설계만으로 해당 워크플로우를 실행하지 않는다.
quad-gpu-backports는 이번 K4 수정의 구현 대상이 아니다.

## 목적과 확인된 장애

Junior 상태 화면 출력 때문에 시스템이 Oops/soft-lockup에 빠지는 문제를
해결한다. helper의 제한 운용과 i915의 근본 수정을 구분하고, 최종 목표는
현재 실패하는 장비에서도 KMS 출력을 안정적으로 유지하는 것이다.

확인한 대상은 `synology_geminilake_920+`, `4.4.302+ #90080`, i915 PCI
`8086:1912`이다. 커널은 하드웨어를 QEMU로 보고한다. GPU 전달 방식은 별도
확인이 필요하며, 가상화 자체를 원인으로 단정하지 않는다.

- i915는 적재되어 card0와 renderD128을 생성했다.
- uptime 27.5초 무렵 helper의 SETCRTC에서 i915 InfoFrame 경고가 발생했다.
- 27.755초 `remap_pfn+0x3/0x60 [i915]`에서 Oops가 났다. 호출 경로는
  `i915_gem_fault → remap_io_mapping → apply_to_page_range → remap_pfn`이다.
- 이후 같은 `kms-display` 태스크에서 soft-lockup이 반복됐다. fatal panic
  문구는 확보되지 않았다. InfoFrame 경고와 페이지 폴트의 인과관계는 미확정이다.
- 실기 helper의 cksum `2280859959 967120`은 로컬 tcrp-addons 바이너리와
  일치한다. cksum/크기 비교는 배포 추적의 출발점이며 최종 식별에는 SHA-256을 쓴다.

기존 화면 복원·다중 카드 출력·표시 개선 이력은 있다. 사용자가 언급한
1차 안정화 변경의 정확한 커밋/배포 경로는 아직 특정하지 못했으므로,
그 변경이 없었다고 단정하지 않는다. 이번 근거와 대조할 항목으로 유지한다.

## 우선 조사할 콜백 ABI 불일치

로컬 소스에서 다음 차이가 확인됐다. 현재 실기 모듈의 원천 소스가 이 파일이라고
확정한 것은 아니다.

| 로컬 소스 | remap_pfn 콜백 인자 |
| --- | --- |
| mshell-modules/src/4.4.302/drivers/gpu/drm/i915/i915_mm.c | pte, token, addr, data (4개) |
| redpill-dual-drm-backports/drivers/gpu/drm/i915/i915_mm.c | pte, addr, data (3개) |
| quad-gpu-backports/drivers/gpu/drm/i915/i915_mm.c | pte, addr, data (3개) |

Oops 레지스터는 `RDX=0x7f7fe3c39000`, `RCX=0xffff8800ad313d00`,
`CR2=0x7f7fe3c39008`이다. x86-64에서 4인자 콜백 호출의 addr는 RDX,
data는 RCX로 전달된다. 3인자 구현이 이를 받으면 RDX를 data로 읽고,
`r->pfn`의 +8 접근에서 이번 CR2와 같은 주소를 참조할 수 있다.
이는 우선순위가 높은 가설이며 위 저장소의 소스와 실제 타깃 헤더·호환 계층을
대조하고 콜백 타입 컴파일 검사로 확인한다. 역어셈블리는 필수 단계가 아니다.

확정 절차:

1. 실기 i915/DRM 의존 모듈 SHA-256, vermagic, srcversion(있을 때), 로더
   포함 팩 및 all-modules 설치 경로를 대조한다. 파일과 적재 모듈의 동일성도
   확인한다. modules.dep만으로 공급 모듈의 호환성을 판단하지 않는다.
2. redpill-dual-drm-backports 소스의 remap_pfn과 빌드에 실제 적용되는
   커널 pte_fn_t 정의 및 apply_to_page_range 호출부를 대조한다. 전처리 결과와
   콜백 타입 컴파일 검사로 compat 헤더 적용 후에도 불일치가 남는지 확인한다.
3. 기존 빌드의 전처리 헤더, pte_fn_t 정의, 경고 억제 옵션, 관련 패치를
   수집한다. 다른 소스 트리의 함수 모양만 보고 원천 저장소를 지정하지 않는다.

배포 바이너리와 소스/빌드 기록이 불일치해 출처를 확인할 수 없을 때에만
역어셈블리를 보조 증거로 사용한다. 소스 수정 설계를 시작하는 선행 조건으로
요구하지 않는다.

### geminilake K4 파일럿 빌드 (2026-10-21)

- Docker Desktop `dante90/syno-compiler:7.4`의 `/opt/geminilake/build`를 사용했다.
  헤더의 `pte_fn_t`는 `(pte, token, addr, data)` 4인자이며 커널 릴리스는
  `4.4.302+`다.
- `drivers/gpu/drm/i915/i915_mm.c`의 `remap_pfn`에 `pgtable_t token`을 추가하고,
  `__builtin_types_compatible_p` 기반 컴파일 타임 검사를 두었다. 이 검사가
  타깃 typedef와 정확히 일치해야 컴파일된다.
- 전체 빌드가 `MODPOST 17 modules`까지 성공했고 17개 모듈의 vermagic이
  `4.4.302+`임을 확인했다. i915.ko SHA-256:
  `d589db8fae44f8a25ae1de2d99ca29ab16fcd2952db5b83037e695374557773f`.
  빌드 산출물은 별도 파일럿 묶음에 보관한다.
- `i915_mm.c`의 콜백 타입 불일치 경고는 발생하지 않았다. 다만 같은 빌드에서
  `drm_prime.c`, `drm_connector.c`, `intel_wakeref.c`, `i915_gem_userptr.c`,
  `i915_request.c`, `intel_acpi.c`, `intel_gmbus.c`에 다른
  incompatible-pointer 경고가 관찰됐다. 이 경고들은 이번 콜백 수정과 별도이며,
  해당 바이너리들을 실기에 적재하기 전에 원인을 분류해야 한다.
- 후속 실기에서 기존 i915/DRM 스택을 정상 언로드하고 파일럿으로 교체했다.
  card0/renderD128이 생성되고 DMC/GuC/HuC 초기화 로그를 확인했다.
  drm_mipi_dsi와 intel-gtt는 런타임 커널 내장 심볼과 중복되어 별도 삽입하지 않았다.
- 사용자가 helper를 실행한 뒤 uptime 약 1298.5초에 kms-display PID 1771이
  intel_write_infoframe의 WARN_ON(len < 0)을 두 번, pipe state 불일치 경고를
  한 번 발생시켰다. 해당 조회 범위에 새 Oops/panic은 없었다. 이전 remap_pfn
  장애의 장기 해소 또는 물리 화면 정상 출력까지 검증한 것은 아니다.

## 근본 수정과 빌드 기준

콜백 불일치가 확정되면 redpill-dual-drm-backports의 i915 호환 계층을 수정한다.
대상 헤더에 대한 컴파일 검사로 3/4인자 규약을 선택하고, 구형 규약에서는
token을 받는 어댑터가 공통 구현에 pte/addr/data를 전달하도록 한다.
함수 포인터 캐스트나 경고 억제로 규약 차이를 숨기지 않는다.

같은 apply_to_page_range를 사용하는 remap_sg 및 해당 트리의 다른 콜백도
검사한다. 타깃 커널 버전 숫자만으로 분기하지 않는다. 콜백 타입 불일치는
빌드 실패로 처리하며, 각 규약의 컴파일 검사를 재현 가능한 스크립트에 둔다.

Docker Desktop의 DSM 7.4 툴체인과 해당 플랫폼/런타임에 정확히 맞는
GPL 소스·.config·Module.symvers를 사용한다. 파일럿은 geminilake K4부터
진행한다. i915 단독 교체는 기존 DRM 공급 모듈과의 심볼/CRC 호환성 확인 후에만
허용하고, 필요하면 같은 빌드의 의존 모듈 묶음으로 시험한다.

공식 자산은 실기 통과 전 유지한다. 소스 커밋, 도구 이미지 식별자, 설정 및
심볼 해시, 빌드 명령, 산출물 해시와 경고 로그를 파일럿 manifest에 보존한다.

## helper의 제한 운용 및 개선

소유 경로: tcrp-addons/junior-realtime-helper/src/{selftest.sh,kms-display.c}.

- 문제가 확인된 i915/런타임 조합에서는 KMS 자동 시작을 보류하는 정책을
  둔다. 네트워크·디스크 상태 수집과 기존 콘솔 출력은 계속한다. K4 전체나
  모든 i915를 영구 차단하는 정책으로 확대하지 않는다.
- sysfs의 드라이버/플랫폼 식별 후 정책을 적용한다. probe 자체도 DRM ioctl을
  실행하면 위험 경로를 자극할 수 있으므로, 허용 전에는 DRM 장치를 열지 않는다.
- 수동 시험 옵션과 단계 로그를 추가한다: create-dumb, map, first-touch,
  addfb, setcrtc, render, cleanup. 시작/완료를 각각 기록하고 즉시 flush한다.
- 타임스탬프 변화가 redraw 간격 제한을 우회하지 않도록 최소 갱신 간격을
  독립적으로 강제한다. 현재 `changed || elapsed >= 2`는 changed일 때 2초
  미만 갱신도 허용한다. 정적 영역은 최초 작성 후 필요한 부분만 갱신한다.
- 현재 CRTC 모드를 우선 사용하고 불필요한 해상도 변경/커넥터 재탐색을 줄인다.
  새 framebuffer를 연결하는 SETCRTC 자체가 사라지는 것은 아니다.
- msync/DIRTYFB는 드라이버별 필요성을 확인해 사용한다. 호출 축소만으로
  최초 GEM page fault가 해결됐다고 판단하지 않는다.
- 카드별 초기화 실패에는 재시도 간격과 상한을 둔다. Oops 이후 프로세스
  재시작이나 SIGKILL이 커널을 복구한다고 가정하지 않는다. 종료 시 CRTC 복원도
  ioctl을 호출하므로 동일하게 검증한다.

## 단계별 검증

현재 Oops 부팅에서 적재/출력 재시험을 계속하지 않는다. 클린 부팅에서 helper
자동 KMS 실행을 제한하고, 독립된 TTYD 로그 수집을 먼저 시작한다. 전체 dmesg와
단계 로그를 호스트로 저장한다. 연결 단절 시 마지막 로그 보존을 보장할 수 없다는
한계를 기록하고, 가능하면 별도 커널 로그 전달 경로를 사용한다.

| 단계 | 작업 | 통과 조건 |
| --- | --- | --- |
| 0 | helper KMS 없이 i915 로딩·상태 관찰 | 5분간 새 Oops/lockup 없음 |
| 1 | 최소 C probe로 dumb buffer 생성·map·한 페이지 최초 접근 | modeset 없이 fault 경로 통과 |
| 2 | 버퍼 전체 읽기/쓰기 및 해제 | 새 Oops/lockup 없고 정상 종료 |
| 3 | 기존 모드로 1회 SETCRTC·정적 화면 | 출력 확인, InfoFrame 경고 재평가 |
| 4 | 2초 이상 간격으로 상태 갱신 | 30분 정상 출력·응답 유지 |
| 5 | 정상 종료/복원, 재시작, DSM 전환 | 3회 반복 및 전환 중 장애 없음 |
| 6 | 실사용 회귀 확인 | 기존 GPU 기능·상태 수집 유지, 2시간 출력 관찰 |

각 단계는 앞 단계 통과 후 진행한다. 최초 Oops/soft-lockup에서 중단하고
다음 시험은 새 부팅에서 한다. timeout만으로 커널 장애를 격리할 수는 없다.
InfoFrame 경고가 남으면 mmap 수정 성공과 디스플레이 안정화 완료를 구분한다.
단기 통과를 모든 플랫폼 안정성 보장으로 확대하지 않는다.

## 진행 순서와 승격 조건

1. 실기 모듈과 원천 빌드를 식별하고 콜백 ABI 가설을 확정한다.
2. helper 제한 운용/진단 단계와 콜백 호환 패치를 각각 준비한다.
3. geminilake K4 파일럿을 빌드하고 위 검증을 순서대로 수행한다.
4. 동일 코드 경로를 사용하는 다른 K4 플랫폼 및 K5 회귀를 확인한다.
5. 통과한 조합에 대해 KMS 자동 실행을 다시 허용하고 정규 빌드에 반영한다.

설계 완료 시점에는 코드, 패키지, 실기 상태를 변경하지 않는다. 커밋/푸시와
릴리즈는 별도 요청에 따른다.
