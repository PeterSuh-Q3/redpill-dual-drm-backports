/* ★ 2026-08-17: torvalds/linux v5.4 pin(219d54332a09) 원본 이식 — DSM 4.4.302
 * host 에 linux/fbcon.h 자체가 없어 fbmem.c vendoring 컴파일 실패(apollolake
 * 실기 발견). CONFIG_FRAMEBUFFER_CONSOLE 을 켜지 않으므로 #else 분기의
 * no-op 인라인 스텁이 자동 적용 — 헤드리스 NAS 환경에 적합, 실제 텍스트
 * 콘솔(fbcon) 드라이버는 vendoring하지 않음. */
#ifndef _LINUX_FBCON_H
#define _LINUX_FBCON_H

#ifdef CONFIG_FRAMEBUFFER_CONSOLE
void __init fb_console_init(void);
void __exit fb_console_exit(void);
int fbcon_fb_registered(struct fb_info *info);
void fbcon_fb_unregistered(struct fb_info *info);
void fbcon_fb_unbind(struct fb_info *info);
void fbcon_suspended(struct fb_info *info);
void fbcon_resumed(struct fb_info *info);
int fbcon_mode_deleted(struct fb_info *info,
		       struct fb_videomode *mode);
void fbcon_new_modelist(struct fb_info *info);
void fbcon_get_requirement(struct fb_info *info,
			   struct fb_blit_caps *caps);
void fbcon_fb_blanked(struct fb_info *info, int blank);
void fbcon_update_vcs(struct fb_info *info, bool all);
void fbcon_remap_all(struct fb_info *info);
int fbcon_set_con2fb_map_ioctl(void __user *argp);
int fbcon_get_con2fb_map_ioctl(void __user *argp);
#else
static inline void fb_console_init(void) {}
static inline void fb_console_exit(void) {}
static inline int fbcon_fb_registered(struct fb_info *info) { return 0; }
static inline void fbcon_fb_unregistered(struct fb_info *info) {}
static inline void fbcon_fb_unbind(struct fb_info *info) {}
static inline void fbcon_suspended(struct fb_info *info) {}
static inline void fbcon_resumed(struct fb_info *info) {}
static inline int fbcon_mode_deleted(struct fb_info *info,
				     struct fb_videomode *mode) { return 0; }
static inline void fbcon_new_modelist(struct fb_info *info) {}
static inline void fbcon_get_requirement(struct fb_info *info,
					 struct fb_blit_caps *caps) {}
static inline void fbcon_fb_blanked(struct fb_info *info, int blank) {}
static inline void fbcon_update_vcs(struct fb_info *info, bool all) {}
static inline void fbcon_remap_all(struct fb_info *info) {}
static inline int fbcon_set_con2fb_map_ioctl(void __user *argp) { return 0; }
static inline int fbcon_get_con2fb_map_ioctl(void __user *argp) { return 0; }
#endif

#endif /* _LINUX_FBCON_H */
