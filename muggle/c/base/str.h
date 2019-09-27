/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_STR_H_
#define MUGGLE_C_STR_H_

#include "muggle/c/base/macro.h"
#include <stdbool.h>

EXTERN_C_BEGIN

/*
 * like python string interface
 * */

MUGGLE_CC_EXPORT
int muggle_str_startswith(const char *str, const char *prefix);

MUGGLE_CC_EXPORT
int muggle_str_endswith(const char *str, const char *suffix);

/*
 * NOTE: start and end represent: [start, end)
 * if end == 0, then end = strlen(str)
 * */
MUGGLE_CC_EXPORT
int muggle_str_count(const char *str, const char *sub, int start, int end);

MUGGLE_CC_EXPORT
int muggle_str_find(const char *str, const char *sub, int start, int end);

MUGGLE_CC_EXPORT
int muggle_str_lstrip_idx(const char *str);

MUGGLE_CC_EXPORT
int muggle_str_rstrip_idx(const char *str);


/*
 * convert to number
 * return non-zero represent success, 0 represent failed
 * */
MUGGLE_CC_EXPORT int muggle_str_toi(const char *str, int *pval, int base);
MUGGLE_CC_EXPORT int muggle_str_tou(const char *str, unsigned int *pval, int base); 
MUGGLE_CC_EXPORT int muggle_str_tol(const char *str, long *pval, int base);
MUGGLE_CC_EXPORT int muggle_str_toul(const char *str, unsigned long *pval, int base);
MUGGLE_CC_EXPORT int muggle_str_toll(const char *str, long long *pval, int base);
MUGGLE_CC_EXPORT int muggle_str_toull(const char *str, unsigned long long *pval, int base);
MUGGLE_CC_EXPORT int muggle_str_tof(const char *str, float *pval);
MUGGLE_CC_EXPORT int muggle_str_tod(const char *str, double *pval);
MUGGLE_CC_EXPORT int muggle_str_told(const char *str, long double *pval);

EXTERN_C_END

#endif
