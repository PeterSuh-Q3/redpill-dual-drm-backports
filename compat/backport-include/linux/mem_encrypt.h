/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: linux/mem_encrypt.h (SME/SEV, ~4.14 — 4.4.302 부재)
 * drm_gem.c / drm_vm.c 는 pgprot_decrypted 만 사용. 암호화 미지원 환경 = identity. */
#ifndef _BPI_LINUX_MEM_ENCRYPT_H
#define _BPI_LINUX_MEM_ENCRYPT_H
#ifndef pgprot_decrypted
#define pgprot_decrypted(prot) (prot)
#endif
#endif
