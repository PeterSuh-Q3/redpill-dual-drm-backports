/* Shim for <linux/mem_encrypt.h> (AMD SME, added ~4.14). 4.4.302 has no SME;
 * pgprot_decrypted is identity on non-SME systems. */
#ifndef _MSHELL_SHIM_LINUX_MEM_ENCRYPT_H
#define _MSHELL_SHIM_LINUX_MEM_ENCRYPT_H
#ifndef pgprot_decrypted
#define pgprot_decrypted(prot) (prot)
#endif
#ifndef sme_active
#define sme_active() 0
#endif
#ifndef mem_encrypt_active
#define mem_encrypt_active() 0
#endif
#endif
