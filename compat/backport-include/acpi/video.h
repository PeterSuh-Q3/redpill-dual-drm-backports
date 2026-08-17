/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: acpi/video.h
 *
 * DSM 4.4.302 stock 커널(7.2/7.3/7.4 공통, 실기 확인: acpi_video_register/
 * unregister/get_backlight_type 이 /proc/kallsyms 에 전무)은 CONFIG_ACPI_VIDEO
 * 서브시스템이 없음 — i915_drv.c/intel_opregion.c 가 무조건 호출하는 이
 * 세 심볼이 insmod 시 Unknown symbol 로 i915.ko 적재 자체를 막는다.
 *
 * 대상 하드웨어(Synology NAS)는 헤드리스라 ACPI 패널 백라이트 제어가
 * 애초에 무의미 — no-op/no-native 매크로로 치환해 링크만 통과시킨다.
 * (실제 acpi/video.h 자체(구조체/enum)는 host 것을 include_next 로 유지)
 */
#ifndef _BPI_ACPI_VIDEO_H
#define _BPI_ACPI_VIDEO_H

#include_next <acpi/video.h>

#define acpi_video_register() do {} while (0)
#define acpi_video_unregister() do {} while (0)
#define acpi_video_get_backlight_type() acpi_backlight_vendor

#endif /* _BPI_ACPI_VIDEO_H */
