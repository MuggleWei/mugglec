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
 * check whether str starts with prefix
 * @str: the whole string
 * @prefix: the prefix string to be checked
 * RETURN: return nonzero if found matching string otherwise zero
 * */
MUGGLE_CC_EXPORT
int muggle_str_startswith(const char *str, const char *prefix);

/*
 * check whether str ends with suffix
 * @str: the whole string
 * @prefix: the suffix string to be checked
 * RETURN: return nonzero if found matching string otherwise zero
 * */
MUGGLE_CC_EXPORT
int muggle_str_endswith(const char *str, const char *suffix);

/*
 * the number of non-overlapping occurrences of substring sub in the range [start, end)
 * @str: the whole string
 * @sub: substring to be searched
 * @start: search starts from this index
 * @end: search ends from this index - 1, NOTE: 0 represet end = strlen(str)
 * RETURN: the number of occurrences 
 * */
MUGGLE_CC_EXPORT
int muggle_str_count(const char *str, const char *sub, int start, int end);

/*
 * return the lowest index in the str where the subsequence sub is found
 * @str: the whole string
 * @sub: substring to be searched
 * @start: search starts from this index
 * @end: search ends from this index - 1, NOTE: 0 represet end = strlen(str)
 * RETURN: the lowest index when found, return -1 if sub is not found
 * */
MUGGLE_CC_EXPORT
int muggle_str_find(const char *str, const char *sub, int start, int end);

/*
 * return index with blank leading bytes removed
 * @str: the whole string
 * RETURN: index after blank leading bytes removed
 * */
MUGGLE_CC_EXPORT
int muggle_str_lstrip_idx(const char *str);

/*
 * return index with blank trailling bytes removed
 * @str: the whole string
 * RETURN: index after blank trailling bytes removed
 * */
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
