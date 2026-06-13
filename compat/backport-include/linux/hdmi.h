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

#define HDMI_DRM_INFOFRAME_SIZE 26
static inline int hdmi_drm_infoframe_init(struct hdmi_drm_infoframe *frame)
{
	memset(frame, 0, sizeof(*frame));
	frame->type    = HDMI_INFOFRAME_TYPE_DRM;
	frame->version = 1;
	frame->length  = HDMI_DRM_INFOFRAME_SIZE;
	return 0;
}
static inline ssize_t hdmi_drm_infoframe_pack(struct hdmi_drm_infoframe *frame,
					      void *buffer, size_t size)
{
	return 0; /* HDR infoframe packing stub — headless DSM 환경에서 미사용 */
}
static inline int hdmi_drm_infoframe_check(struct hdmi_drm_infoframe *frame)
{
	return 0;
}
/* hdmi_infoframe_pack_only: added ~5.0, equivalent to pack without checksum invalidation */
#ifndef hdmi_infoframe_pack_only
#define hdmi_infoframe_pack_only(frame, buf, size) hdmi_infoframe_pack(frame, buf, size)
#endif
/* hdmi_avi_infoframe_check: added ~5.0; validates AVI infoframe fields */
static inline int hdmi_avi_infoframe_check(struct hdmi_avi_infoframe *frame)
{
	return 0;
}
/* hdmi_infoframe_unpack: 5.0 added size arg; 4.4 proto uses 2-arg.
 * Variadic macro to absorb the extra argument silently. */
#ifdef hdmi_infoframe_unpack
#undef hdmi_infoframe_unpack
#endif
/* Prototype already in include_next — wrap to drop 3rd arg */
static inline int
_bpi_hdmi_infoframe_unpack(union hdmi_infoframe *frame, void *buffer, size_t size)
{
	(void)size;
	return hdmi_infoframe_unpack(frame, buffer);
}
#define hdmi_infoframe_unpack(f, b, s) _bpi_hdmi_infoframe_unpack(f, b, s)

/* hdmi_spd_infoframe_check / hdmi_vendor_infoframe_check: added ~5.0 */
static inline int hdmi_spd_infoframe_check(struct hdmi_spd_infoframe *frame)
{ return 0; }
static inline int hdmi_vendor_infoframe_check(struct hdmi_vendor_infoframe *frame)
{ return 0; }

/* 4.4.302 enum hdmi_infoframe_type 에 DRM(HDR) 타입 부재 → 보강 */
#ifndef _BPI_HDMI_TYPE_DRM
#define _BPI_HDMI_TYPE_DRM
#define HDMI_INFOFRAME_TYPE_DRM        0x87
#define HDMI_PACKET_TYPE_GENERAL_CONTROL 0x03
#define HDMI_PACKET_TYPE_GAMUT_METADATA  0x0a
#endif

#endif /* HDMI_EOTF_TRADITIONAL_GAMMA_SDR */

#endif /* _BPI_LINUX_HDMI_H */
