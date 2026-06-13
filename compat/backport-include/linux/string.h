/* SPDX-License-Identifier: GPL-2.0 */
/* backport shim: match_string (~4.5). */
#ifndef _BPI_LINUX_STRING_H
#define _BPI_LINUX_STRING_H
#include_next <linux/string.h>
#ifndef _BPI_HAVE_MATCH_STRING
#define _BPI_HAVE_MATCH_STRING
static inline int match_string(const char * const *array, size_t n, const char *string)
{
	int i;
	for (i = 0; i < (int)n; i++) {
		if (!array[i]) break;
		if (!strcmp(array[i], string)) return i;
	}
	return -EINVAL;
}
#endif
#endif
