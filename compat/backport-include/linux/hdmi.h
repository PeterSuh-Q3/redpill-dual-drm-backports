/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: linux/hdmi.h
 * 4.4.302 의 linux/hdmi.h 를 include_next 로 가져온 뒤, 4.4.302 에 없는 HDR 관련
 * 타입(~4.15+)을 append 한다. (drm_connector.h 의 struct hdr_sink_metadata 등)
 *
 * ★ hdmi_drm_infoframe_init/pack 은 5.4 drivers/video/hdmi.c 구현(미vendoring) →
 *   프로토타입만. 링크 시 undefined(modpost-warn) → 다음 층에서 stub/백포트 결정. */
#ifndef _BPI_LINUX_HDMI_H
#define _BPI_LINUX_HDMI_H

#include_next <linux/hdmi.h>

#ifndef HDMI_EOTF_TRADITIONAL_GAMMA_SDR
enum hdmi_eotf {
	HDMI_EOTF_TRADITIONAL_GAMMA_SDR,
	HDMI_EOTF_TRADITIONAL_GAMMA_HDR,
	HDMI_EOTF_SMPTE_ST2084,
	HDMI_EOTF_BT_2100_HLG,
};

enum hdmi_metadata_type {
	HDMI_STATIC_METADATA_TYPE1 = 1,
};

struct hdr_static_metadata {
	__u8 eotf;
	__u8 metadata_type;
	__u16 max_cll;
	__u16 max_fall;
	__u16 min_cll;
};

struct hdmi_drm_infoframe {
	enum hdmi_infoframe_type type;
	unsigned char version;
	unsigned char length;
	enum hdmi_eotf eotf;
	enum hdmi_metadata_type metadata_type;
	struct {
		u16 x, y;
	} display_primaries[3];
	struct {
		u16 x, y;
	} white_point;
	u16 max_display_mastering_luminance;
	u16 min_display_mastering_luminance;
	u16 max_cll;
	u16 max_fall;
};

struct hdr_sink_metadata {
	__u32 metadata_type;
	union {
		struct hdr_static_metadata hdmi_type1;
	};
};

int hdmi_drm_infoframe_init(struct hdmi_drm_infoframe *frame);
ssize_t hdmi_drm_infoframe_pack(struct hdmi_drm_infoframe *frame, void *buffer, size_t size);
#endif /* HDMI_EOTF_TRADITIONAL_GAMMA_SDR */

#endif /* _BPI_LINUX_HDMI_H */
