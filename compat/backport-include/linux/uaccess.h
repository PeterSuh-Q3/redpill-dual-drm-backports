/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim:
 *  - access_ok 2-arg(5.0) ↔ 4.4.302 3-arg(type,addr,size). variadic 으로 양쪽 수용.
 *  - user_access_begin/end (~4.19) stub.
 * 4.4.302 x86 의 __range_not_ok / user_addr_max 사용(존재 확인). */
#ifndef _BPI_LINUX_UACCESS_H
#define _BPI_LINUX_UACCESS_H
#include_next <linux/uaccess.h>

#ifdef access_ok
#undef access_ok
#endif
#define _BPI_ACK_PICK(_1, _2, _3, NAME, ...) NAME
#define _bpi_access_ok2(addr, size) \
	likely(!__range_not_ok((unsigned long)(addr), (size), user_addr_max()))
#define _bpi_access_ok3(type, addr, size) \
	likely(!__range_not_ok((unsigned long)(addr), (size), user_addr_max()))
#define access_ok(...) \
	_BPI_ACK_PICK(__VA_ARGS__, _bpi_access_ok3, _bpi_access_ok2)(__VA_ARGS__)

/* user_access_begin: 4.4.302 Synology may define 1-arg version; 5.4 uses 2-arg.
 * Force 2-arg version regardless. */
#ifdef user_access_begin
#undef user_access_begin
#endif
#define user_access_begin(ptr, len) access_ok((ptr), (len))
#ifndef user_access_end
#define user_access_end() do { } while (0)
#endif
#endif
