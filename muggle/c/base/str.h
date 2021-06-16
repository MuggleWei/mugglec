/******************************************************************************
 *  @file         str.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-12
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        function prototypes for mugglec string processing
 *
 *  This file contains some simple functions for processing strings
 *****************************************************************************/

#ifndef MUGGLE_C_STR_H_
#define MUGGLE_C_STR_H_

#include "muggle/c/base/macro.h"
#include <stdbool.h>

EXTERN_C_BEGIN

/**
 * @brief Check whether string starts with prefix
 *
 * @param str    the whole string
 * @param prefix the prefix string to be checked
 *
 * @return  return nonzero if string starts with the prefix, otherwise return zero
 */
MUGGLE_C_EXPORT
int muggle_str_startswith(const char *str, const char *prefix);

/**
 * @brief Check whether string ends with suffix
 *
 * @param str    the whole string
 * @param suffix the suffix string to be checked
 *
 * @return  return nonzero if found matching string, otherwise return zero
 */
MUGGLE_C_EXPORT
int muggle_str_endswith(const char *str, const char *suffix);

/**
 * @brief The number of non-overlapping occurrences of substring sub in the range [start, end)
 *
 * @param str   the whole string
 * @param sub   substring to be searched
 * @param start search starts from this index
 * @param end   search ends from this index - 1, NOTE: 0 represet end = strlen(str)
 *
 * @return  the number of occurrences
 */
MUGGLE_C_EXPORT
int muggle_str_count(const char *str, const char *sub, int start, int end);

/**
 * @brief Return the lowest index in the string where the substring sub is found
 *
 * @param str the whole string
 * @param sub substring to be searched
 * @param start search starts from this index
 * @param end search ends from this index - 1, NOTE: 0 represet end = strlen(str)
 *
 * @return the lowest index when found, return -1 if sub is not found
 */
MUGGLE_C_EXPORT
int muggle_str_find(const char *str, const char *sub, int start, int end);

/**
 * @brief find string without leading blank
 *
 * @param str the whole string
 *
 * @return offset skip leading blank
 */
MUGGLE_C_EXPORT
int muggle_str_lstrip_idx(const char *str);

/**
 * @brief find string without trailling blank
 *
 * @param str the whole string
 *
 * @return offset of last character that is not blank
 */
MUGGLE_C_EXPORT
int muggle_str_rstrip_idx(const char *str);


/**
 * @brief convert string to integer
 *
 * @param str  string
 * @param pval integer pointer
 * @param base numerical base
 *
 * @return
 *     non-zero - success
 *     0 - failed
 */
MUGGLE_C_EXPORT int muggle_str_toi(const char *str, int *pval, int base);

/**
 * @brief convert string to unsigned integer
 *
 * @param str  string
 * @param pval unsigned integer pointer
 * @param base numerical base
 *
 * @return
 *     non-zero - success
 *     0 - failed
 */
MUGGLE_C_EXPORT int muggle_str_tou(const char *str, unsigned int *pval, int base);

/**
 * @brief convert string to long integer
 *
 * @param str  string
 * @param pval long integer pointer
 * @param base numerical base
 *
 * @return
 *     non-zero - success
 *     0 - failed
 */
MUGGLE_C_EXPORT int muggle_str_tol(const char *str, long *pval, int base);

/**
 * @brief convert string to unsigned long integer
 *
 * @param str  string
 * @param pval unsigned long integer pointer
 * @param base numerical base
 *
 * @return
 *     non-zero - success
 *     0 - failed
 */
MUGGLE_C_EXPORT int muggle_str_toul(const char *str, unsigned long *pval, int base);

/**
 * @brief convert string to long long integer
 *
 * @param str  string
 * @param pval long long integer pointer
 * @param base numerical base
 *
 * @return
 *     non-zero - success
 *     0 - failed
 */
MUGGLE_C_EXPORT int muggle_str_toll(const char *str, long long *pval, int base);

/**
 * @brief convert string to unsigned long long integer
 *
 * @param str  string
 * @param pval unsigned long long integer pointer
 * @param base numerical base
 *
 * @return
 *     non-zero - success
 *     0 - failed
 */
MUGGLE_C_EXPORT int muggle_str_toull(const char *str, unsigned long long *pval, int base);

/**
 * @brief convert string to float
 *
 * @param str  string
 * @param pval float pointer
 *
 * @return
 *     non-zero - success
 *     0 - failed
 */
MUGGLE_C_EXPORT int muggle_str_tof(const char *str, float *pval);

/**
 * @brief convert string to double
 *
 * @param str  string
 * @param pval double pointer
 *
 * @return
 *     non-zero - success
 *     0 - failed
 */
MUGGLE_C_EXPORT int muggle_str_tod(const char *str, double *pval);

/**
 * @brief convert string to long double
 *
 * @param str  string
 * @param pval long double pointer
 *
 * @return
 *     non-zero - success
 *     0 - failed
 */
MUGGLE_C_EXPORT int muggle_str_told(const char *str, long double *pval);

EXTERN_C_END

#endif
