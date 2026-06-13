/*
 * Shim for <asm/set_memory.h> which was split out in ~4.12.
 * On the 4.4.302 kernel the set_memory_* helpers live in <asm/cacheflush.h>.
 * Provided on the include path via Makefile: ccflags-y += -I.../kcl/shim
 */
#ifndef _MSHELL_SHIM_ASM_SET_MEMORY_H
#define _MSHELL_SHIM_ASM_SET_MEMORY_H
#include <asm/cacheflush.h>
#endif
