/* SPDX-License-Identifier: GPL-2.0 */
/* backport stub: linux/ascii85.h added in 5.x; not in 4.4.302 */
#ifndef _BPI_LINUX_ASCII85_H
#define _BPI_LINUX_ASCII85_H
#include <linux/types.h>

/* ASCII85 인코딩 버퍼 크기 상수: 4 바이트 → 5 바이트 + nul */
#define ASCII85_BUFSZ  6

static inline size_t ascii85_encode_len(size_t len) { 
    return (len * 5 / 4) + 2;  /* 4 바이트 그룹 + nul */
}

static inline char *ascii85_encode(u32 v, char *buf) { 
    buf[0] = '?'; 
    buf[1] = '\0'; 
    return buf; 
}
#endif
