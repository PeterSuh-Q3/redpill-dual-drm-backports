# K4 AST quad-DRM 구현

## 목적과 소스 기준

K4의 i915 + AMDGPU + Bochs에 ASPEED AST 디스플레이/KMS를 추가한다.
기존 i915 mmap/HDMI 안정화와 Bochs 코드는 유지한다. AST 추가는 하드웨어
트랜스코딩 기능 추가를 뜻하지 않는다.

K5 `quad-gpu-backports/drivers/gpu/drm/ast`를 참조하되 직접 복사하지 않는다.
K4의 DRM/TTM API는 Linux 5.4이므로 **upstream v5.4 AST**를
`drivers/gpu/drm/ast-v54/`에 가져왔다. K5의 newer aperture/fbdev API를
K4에 스텁으로 만드는 방식을 피한다. 기존 drm, drm_kms_helper,
drm_vram_helper, ttm을 공유하고 외부 provider 바이너리를 반입하지 않는다.

원본: https://github.com/torvalds/linux/tree/v5.4/drivers/gpu/drm/ast

| 파일 | 원본 Git blob SHA-1 |
| --- | --- |
| Kconfig | `829620d5326cce6e25e2254d13dbf48dc7eb0841` |
| Makefile | `561f7c4199e4a354ec6de7d9c3841561bf1241af` |
| ast_dp501.c | `98cd69269263f48c1a49e609ef27366d5e885b37` |
| ast_dram_tables.h | `1e9ac9d6d26c7daa4d4f1d43ee9c124fa6eabf81` |
| ast_drv.c | `6ed6ff49efc0707fbaa607468c1038f86159ac56` |
| ast_drv.h | `244cc7c382afbbd257edcdf3fcf87852ed558094` |
| ast_main.c | `50de8e47659c7f62c272b068adf2d0bad9138956` |
| ast_mode.c | `d349c721501c6cb5ee890b5c2906479e0b865daf` |
| ast_post.c | `2d1b186197432ea42be84d55e0472dde950ef6cf` |
| ast_tables.h | `d665dd5af5dd80f2348dd1290c41ecb4756ab9d7` |
| ast_ttm.c | `c52d9229417158d5b2d4c1846b41b94754f4f237` |

라이선스와 저작권을 유지한다. 초기 가져오기에서 파일 끝의 여분 빈 줄만
정리했으며 동작 변경은 없다. AST2000~2500 계열의 upstream 탐지를 사용한다.
AST2600 및 K5와 동일한 지원범위를 주장하지 않는다. PCI alias와 chip/revision
탐지는 별개다. AST1180은 upstream PCI 테이블에서 비활성화되어 있다.

DP501 DisplayPort는 장비 내 펌웨어 백업 또는 `ast_dp501_fw.bin` 요청을
사용한다. 이 변경에는 펌웨어 바이너리를 포함하지 않는다. VGA와 DP501
시험은 구분하고 DP501 장비의 펌웨어 로드 결과를 별도 확인한다.

## 구현과 재현

- DRM Makefile/Kconfig에 AST 연결, `CONFIG_DRM_AST=m`.
- Gemini Lake 로컬 설정과 정규 STEP6 all-platform workflow에 AST 활성화.
- workflow 업로드 전/병합 전 필수 산출물 검사에 `ast.ko` 추가.
- 로컬 파일럿: `bash scripts/build-k4-ast-pilot.sh`.
  Docker Desktop `dante90/syno-compiler:7.4`의 Gemini Lake 4.4.302
  설정/Module.symvers를 사용한다. 기본 4 jobs, `JOBS`로 조절한다.
- 입력 트리를 임시 경로에 복사하고 기존 .o/.ko는 제외한다.
  네 드라이버 전체 빌드, make 실패, MODPOST undefined/duplicate를 검사한다.
- 이미지 ID, 실제 소스 스냅샷, 커널 설정/심볼, 로그, modinfo, SHA256SUMS와
  모듈팩은 타임스탬프별 `out/junior-ast-kms-pilot/`에 남긴다.

## 실기 수용시험 — 미실행

1. AST PCI ID, 플랫폼/DSM/kernel, 현재 모듈팩 출처를 기록한다.
2. clean boot에서 동일 팩의 공용 의존성을 사용한다. 다른 버전의 이미
   로딩된 DRM 위에 혼합 삽입하지 않는다.
3. `modprobe ast` 후 PCI 바인딩, DRM driver/connector/modes와 dmesg를
   확인한다. card 번호를 고정 가정하지 않는다.
4. AST 카드를 명시 선택한 헬퍼로 mmap, modeset, 반복 redraw,
   종료/복원/재실행을 시험하고 물리 화면을 확인한다.
5. Oops, soft lockup, unknown symbol, WARN을 기록한다.
   기존 i915/Bochs 및 AMDGPU 회귀는 별도 장비에서 검증한다.

컴파일 성공과 실기 성공을 구분한다. 커밋/푸시/Actions 실행/배포는
별도 요청 전까지 수행하지 않는다.

## 2026-10-22 첫 통합 컴파일 결과

- 경로: `out/junior-ast-kms-pilot/20261022T134245Z/`
- 첫 빌드에서 make exit 0, MODPOST 19 modules, 19개 .ko 생성.
- AST 소스의 추가 API 수정 없이 컴파일 성공. MODPOST undefined/duplicate 없음.
- 모든 모듈 vermagic: `4.4.302+ SMP mod_unload`.
- `ast.ko` SHA-256:
  `82b9f068a729544a6aae8d1648d78e829af6f4083c035dd02d75938b58117678`
- 팩 SHA-256:
  `ca80894d153d1dd75b8acc1f15be62876d1dd831e1be6521e522f044c3eaf854`
- AST PCI alias: ASPEED `1a03:2000`, `1a03:2010`, display class.
- AST 직접 의존: drm, drm_kms_helper, drm_vram_helper, fb, i2c-algo-bit.
  TTM은 VRAM helper의 의존성이다. `i2c-algo-bit.ko`는 본 OOT 팩에 없으며
  대상 DSM/로더의 일치하는 모듈이 필요하다. Gemini Lake host config는
  `CONFIG_I2C_ALGOBIT=m`; host Module.symvers에 `i2c_bit_add_bus`가 있다.
  컴파일 이미지에 해당 .ko는 없어 물리적 provider 확보/적재는 실기 전 확인한다.
- 기존 공용 DRM/i915/AMDGPU 코드에는 incompatible-pointer 경고가 남아 있다
  (drm_prime dma-buf attach, amdgpu_dma_buf attach, i915 mmu notifier 등).
  AST 파일 자체의 경고는 발견되지 않았으나 공용 경로의 안전성을 보증하지는
  않는다. 이번 변경에서 무관한 콜백을 기계적으로 변환하지 않았다.
- 아직 미실행: AST 장비 적재/KMS/물리 출력, 전체 30-target workflow.
