/* Shim for <linux/pfn_t.h> (pfn_t added ~4.5). On 4.4.302 vm_insert_mixed()
 * still takes a raw unsigned long pfn, so degrade pfn_t to that. */
#ifndef _MSHELL_SHIM_LINUX_PFN_T_H
#define _MSHELL_SHIM_LINUX_PFN_T_H
#include <linux/pfn.h>
typedef unsigned long pfn_t;
#define PFN_DEV 0
#define PFN_MAP 0
#define __pfn_to_pfn_t(pfn, flags) ((unsigned long)(pfn))
#define pfn_t_to_pfn(x)            ((unsigned long)(x))
#endif
