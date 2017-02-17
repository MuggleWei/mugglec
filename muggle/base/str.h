/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef __MUGGLE_STR_H__
#define __MUGGLE_STR_H__

#include "muggle/base/macro.h"
#include <stdbool.h>

EXTERN_C_BEGIN

#define MUGGLE_SKIP_BLANK(p) \
while (*p == ' ' || *p == '\t') \
{ \
	++p; \
}

#define MUGGLE_SKIP_BLANK_AND_LFCR(p) \
while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') \
{ \
	++p; \
}

#define MUGGLE_SKIP_WORD(p) \
while (*p != ' ' && *p != '\t' && *p != '\r' && *p != '\n' && *p != '\0') \
{ \
	++p; \
}

#define MUGGLE_SKIP_TO_NEXT_WORD(p) \
MUGGLE_SKIP_WORD(p) \
MUGGLE_SKIP_BLANK_AND_LFCR(p)

#define MUGGLE_SKIP_TO_LINE_END(p) \
while (*p != '\n' && *p != '\r' && *p != '\0') \
{ \
	++p; \
}

#define MUGGLE_SKIP_TO_NEXT_LINE(p) \
MUGGLE_SKIP_TO_LINE_END(p); \
if (*p != '\0') \
{ \
	MUGGLE_SKIP_TO_NEXT_WORD(p); \
}

// note: the range is [p_start, p_end], so generate char* is "p_start ... p_end '\0'"
MUGGLE_BASE_EXPORT char* StrAllocByDiff(const char* p_start, const char* p_end);
MUGGLE_BASE_EXPORT bool StrStartsWith(const char* str, const char* sub_str);
MUGGLE_BASE_EXPORT bool StrEndsWith(const char* str, const char* sub_str);

/*
 *  StrUtils_SplitLineToWords: split line to words
 *  @line: input line
 *  @words: output words
 *  @cnt: record number words in this line
 *  @max_word_num: up limit number of words in this line
 *  @return value: return next line
 *	NOTE: This function will break the original data
 */
MUGGLE_BASE_EXPORT char* StrSplitLineToWords(char* line, char** words, int* cnt, int max_word_num);

/*
 *	string to number
 */
MUGGLE_BASE_EXPORT bool StrToi(const char *str, int *pval, int base);
MUGGLE_BASE_EXPORT bool StrToui(const char *str, unsigned int *pval, int base); 
MUGGLE_BASE_EXPORT bool StrTol(const char *str, long *pval, int base);
MUGGLE_BASE_EXPORT bool StrToul(const char *str, unsigned long *pval, int base);
MUGGLE_BASE_EXPORT bool StrToll(const char *str, long long *pval, int base);
MUGGLE_BASE_EXPORT bool StrToull(const char *str, unsigned long long *pval, int base);
MUGGLE_BASE_EXPORT bool StrTof(const char *str, float *pval);
MUGGLE_BASE_EXPORT bool StrTod(const char *str, double *pval);
MUGGLE_BASE_EXPORT bool StrTold(const char *str, long double *pval);

EXTERN_C_END

#endif