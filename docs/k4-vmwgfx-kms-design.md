# K4 VMware SVGA II (15ad:0405) DRM/KMS 설계

> **목표 범위**
>
> K4 커널 4.4.302에서 VMware SVGA II의 DRM/KMS를 사용해 Junior 화면을
> 출력하는 것이 목표다. 첫 수용 목표는 PCI `15ad:0405` 장치 바인딩, DRM
> card/connector 생성, Junior KMS helper 화면 출력이다. **3D 가속, Mesa,
> FFmpeg/VA-API, 전체 vmwgfx userspace 기능은 이 작업의 성공 조건이 아니며
> 별도 검증 전에는 지원을 주장하지 않는다.**

## 저장소·ABI 기준

이 저장소의 K4 통합 DRM은 Linux 5.4 LTS에서 가져온 DRM/TTM 소스와 K4.4.302
호스트 ABI용 compat/패치를 조합한다. 따라서 vmwgfx 소스도 우선 **upstream
Linux v5.4**를 기준으로 평가한다. K5 `quad-gpu-backports`에서 동작한 v6.5
vmwgfx는 기능·실기 시험의 참고 자료일 뿐, 소스나 `.ko`를 이 저장소로 그대로
복사하거나 K4에서 적재 가능한 증거로 취급하지 않는다.

upstream v5.4의 vmwgfx Kconfig는 `DRM`, `PCI`, `X86`, `MMU`를 요구하고,
`FB_DEFERRED_IO`, CFB, TTM, FB 및 DRM KMS helper를 선택한다. 드라이버의 PCI
테이블에는 `15ad:0405`가 있다. 이는 소스의 초기 검토 결과일 뿐, 대상 K4의
`.config`, `Module.symvers`, 실제 제공 모듈이 그 요구사항을 충족한다는 뜻은
아니다. 구현 전에 같은 DSM/플랫폼/4.4.302 툴체인의 GPL 소스와 런타임 설정·심볼을
대조한다.

참고 원천은 [Linux v5.4 vmwgfx](https://github.com/torvalds/linux/tree/v5.4/drivers/gpu/drm/vmwgfx),
[v5.4 vmwgfx Kconfig](https://github.com/torvalds/linux/blob/v5.4/drivers/gpu/drm/vmwgfx/Kconfig),
[v5.4 vmwgfx PCI 등록부](https://github.com/torvalds/linux/blob/v5.4/drivers/gpu/drm/vmwgfx/vmwgfx_drv.c)다.
가져오는 경우 원본 커밋, 파일 목록, 라이선스 및 저작권 고지를 기록한다.

## K5 결과의 올바른 승계

K5에서는 Linux v6.5 vmwgfx를 통합했고, EPYC7002 DSM 7.4 / 5.10.55 Junior 파일럿에서
PCI `15ad:0405` 바인딩과 KMS helper 화면 출력을 확인했다. 이는 v6.5 소스와 K5
provider 조합의 KMS 성공 사례다. K4 v5.4 소스의 API 호환성, K4 provider ABI,
플랫폼 전체 적재 가능성은 입증하지 않는다.

K5에서 다룬 `CONFIG_MAPPING_DIRTY_HELPERS` / coherent-surface 제한을 K4에
기계적으로 이식하지 않는다. v5.4 vmwgfx의 실제 dirty/coherent 경로와 심볼을
검토하여, KMS 최소 범위에서 호출되는지 및 K4.4.302에 어떤 ABI가 필요한지 먼저
확정한다. 미지원 API를 빈 shim으로 숨기거나 `KBUILD_MODPOST_WARN=1`로 미해결
심볼을 허용하지 않는다. KMS에 불필요한 3D/coherent 경로는 성공처럼 모의 구현하지
말고, 실제 upstream 옵션·코드 경로를 근거로 제외 가능성을 평가한다.

## 단계별 설계

### 1. 단일 파일럿 대상 고정

- 첫 빌드 대상을 **Gemini Lake / DSM 7.4 / K4 4.4.302**로 고정한다. 이 저장소의
  기존 K4 파일럿과 Docker Desktop `dante90/syno-compiler:7.4` 환경을 따른다.
- 실기 시험 전 실제 VM 제품(지원 목표는 VMware Fusion 또는 ESXi), PCI vendor/device,
  PCI 주소, 가상 GPU 모델, 대상 kernel release를 직접 확인한다. QEMU의 VMware VGA
  에뮬레이션이나 단순히 같은 PCI ID를 내는 장비는 별도 명시 없이 VMware 지원
  성공으로 간주하지 않는다.
- 대상 VM에서 VMware hypervisor 식별이 필요한지는 v5.4 소스의 probe 조건과
  K4.4.302 헤더/런타임 동작으로 판단한다. K5 v6.5의 hypervisor 판별 코드를
  그대로 가져오지 않으며, 식별 조건을 우회하지 않는다.

### 2. 소스·Kconfig 연결

- v5.4 `drivers/gpu/drm/vmwgfx/`의 소스, 하위 Makefile/Kconfig, 필요한 UAPI를
  고정된 upstream snapshot으로 가져온다. 이 저장소의 DRM core/TTM과 ioctl/UAPI가
  충돌하지 않는지 비교한다.
- 기존 `DRM_VMWGFX` Kconfig source 경로를 실제 v5.4 소스로 채우고, `vmwgfx/`
  Makefile 연결을 추가한다. 호스트 커널 `.config`
  접두사(`CONFIG_`)와 외부 DRM 백포트 Kconfig 출력 네임스페이스를 혼동하지 않는다.
- v5.4 vmwgfx가 요구하는 KMS helper, TTM, framebuffer, deferred-I/O, DMA/PCI 및
  fence/dma-buf provider 각각에 대해 실제 `.ko` 또는 커널 내장 여부, export,
  symbol CRC를 확인한다. `modules.dep`만으로 제공자 호환성을 증명하지 않는다.
- K4의 기존 i915+amdgpu+Bochs+AST 공용 DRM provider를 재사용하되, 중복 export
  방지 예외와 built-in provider 가드를 보존한다. `drm.ko`를 임의로 중복 추가하거나
  다른 플랫폼·커널의 provider를 가져오지 않는다.

### 3. 빌드·정적 게이트

- 파일럿 전용 재현 스크립트와 로그를 `scripts/` 및 `out/`에 분리한다. 기존
  all-platform workflow를 파일럿 용도로 바꾸거나 바로 전체 매트릭스를 트리거하지
  않는다.
- 같은 Gemini Lake 7.4 K4 toolchain, kernel source, `.config`, `Module.symvers`로
  vmwgfx와 필요한 provider를 빌드한다. 다른 드라이버의 기존 소스 변경을 자동으로
  섞지 않는다.
- `make` 종료 상태, MODPOST undefined/duplicate symbols, `modinfo` 의존성·alias,
  `vermagic=4.4.302+`, 산출 모듈 해시를 기록한다. `vmwgfx.ko` 단독 빌드 성공은
  적재 가능성이나 KMS 동작 성공을 뜻하지 않는다.
- 첫 통합 산출물의 필수 모듈 목록과 실제 all-modules 추출/선택 로그를 대조한다.
  새 모듈을 필수 체크에 포함하기 전, 모든 pilot provider가 패키지에 포함되는지
  확인한다.

### 4. K4 실기 수용시험

1. 기존 DRM 모듈이 섞이지 않은 clean Junior boot에서 플랫폼, DSM/kernel,
   VM 제품, PCI ID, 초기 `lsmod`, `/dev/dri`, dmesg를 기록한다.
2. 파일럿 팩에 포함된 정확한 DRM/TTM provider를 먼저 적재하고 `vmwgfx`를 적재한다.
   이미 다른 ABI의 같은 이름 모듈이 올라와 있으면 그 위에 혼합 적재하지 않는다.
3. PCI 바인딩, `/dev/dri/card*`, render node, connector/modes, vmwgfx probe 및
   unresolved/duplicate symbol 여부를 확인한다. i915가 공존하면 card 번호를
   고정 가정하지 않는다.
4. KMS helper가 vmwgfx 카드를 driver 기준으로 선택하도록 실행해 초기 패턴,
   반복 갱신, 종료·복원·재실행을 확인한다. i915/Bochs 카드로 자동 우회하지 않는다.
5. Oops, WARN, lockup, TTYD 응답 저하가 발생하면 해당 부팅에서 반복 삽입하지
   않고 로그를 보존한다. KMS 성공과 3D/전체 vGPU 기능 검증은 분리 기록한다.

## 승격 기준 및 미포함 범위

Gemini Lake / DSM 7.4 / K4.4.302의 DS920+ VMware Junior 실기에서 파일럿 `vmwgfx`
적재, PCI `15ad:0405` 바인딩, `/dev/dri/card0` 및 `renderD128` 생성과 Junior KMS
helper 가동까지 확인했다. 이에 따라 STEP6 전체 플랫폼 컴파일 매트릭스에는 vmwgfx를
포함하도록 준비했다. 이는 각 플랫폼의 빌드·MODPOST 결과를 확인하기 위한 확장이지,
모든 플랫폼의 실기 적재 검증을 뜻하지 않는다. 전체 매트릭스 완료 후에도 런타임 수용은
플랫폼별로 구분해 기록한다.

현재 명시적 비목표: vmwgfx 3D acceleration, Mesa userspace 제공, GPU passthrough,
일반 VMware PCI device 지원, K5 바이너리 재사용, 이 KMS 작업을 AMD H/W
트랜스코딩 수용시험으로 간주하는 것.

## 구현 기록 — 2026-10-23

- 고정 upstream v5.4 커밋 `219d54332a09e8d8741c1e1982f5eae56099de85`의
  `drivers/gpu/drm/vmwgfx/` 65개 파일을 vendor 스냅샷과 K4 빌드 소스에 추가했다.
- `drivers/gpu/drm/Kconfig`에는 `DRM_VMWGFX` source 항목이 이미 있었고, DRM
  Makefile에 `obj-$(CONFIG_DRM_VMWGFX) += vmwgfx/`를 연결했다.
- K4.4.302 차이는
  [`patches/0001-vmwgfx-k4-scatterlist-and-vmalloc-compat.patch`](../patches/0001-vmwgfx-k4-scatterlist-and-vmalloc-compat.patch)와
  `compat/backport-include/asm/vmware.h`, `compat/backport-include/linux/frame.h`에
  분리했다: K4 scatterlist iterator/IDA API, 직접 필요한 vmalloc/IRQ 선언, K4에 없는
  objtool annotation, VMware Fusion/ESXi legacy port hypercall ABI.
- 재현 명령: `scripts/build-k4-vmwgfx-pilot.sh` (기본값은 quad DRM + vmwgfx 결합),
  또는 `VMWGFX_ONLY=1 scripts/build-k4-vmwgfx-pilot.sh` (vmwgfx 의존성 컴파일만).
- 기본 Gemini Lake / DSM 7.4 / K4.4.302 전체 결합 파일럿이 통과했다. 결과는
  `out/k4-vmwgfx-kms-pilot/geminilake-7.4-k4.4.302/full-quad/`에 보존되어 있다.
  make/MODPOST 성공, `vmwgfx.ko` 생성, `vermagic=4.4.302+`, PCI alias
  `pci:v000015ADd00000405...`를 확인했다. 미해결/중복 심볼 게이트도 통과했다.
- 2026-10-23 DS920+ (실행 프로필 `synology_geminilake_920+`, VMware SVGA II
  `15ad:0405`) Junior 시험에서 DRM core/helper/FB 의존 모듈과 파일럿 `vmwgfx.ko`를
  `/tmp`에서 적재했다. 초기 부팅에는 i915/DRM이 올라오지 않은 상태였으며, 파일럿
  적재 뒤 vmwgfx가 `0000:00:0f.0`에 바인딩되고 `card0`, `renderD128`, 가상 connector
  모드 목록을 만들었다. 사용자가 Junior helper의 정상 가동도 확인했다. 이 시험에서는
  `/lib/modules`를 수정하지 않았고, 다섯 드라이버 모두의 개별 런타임 적재를 검증한 것은
  아니다.
- 위 결과는 해당 Gemini Lake VMware 시험 구성의 KMS 경로를 검증한다. VMware Fusion/ESXi
  제품별 전체 지원, 다른 플랫폼의 실기 적재, 3D 가속 또는 VA-API 동작까지 입증하지는
  않는다.
