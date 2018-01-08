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
MUGGLE_CC_EXPORT char* MuggleStrAllocByDiff(const char* p_start, const char* p_end);
MUGGLE_CC_EXPORT bool MuggleStrStartsWith(const char* str, const char* sub_str);
MUGGLE_CC_EXPORT bool MuggleStrEndsWith(const char* str, const char* sub_str);

/*
 *  StrUtils_SplitLineToWords: split line to words
 *  @line: input line
 *  @words: output words
 *  @cnt: record number words in this line
 *  @max_word_num: up limit number of words in this line
 *  @return value: return next line
 *	NOTE: This function will break the original data
 */
MUGGLE_CC_EXPORT char* MuggleStrSplitLineToWords(char* line, char** words, int* cnt, int max_word_num);

/*
 *	string to number
 */
MUGGLE_CC_EXPORT bool MuggleStrToi(const char *str, int *pval, int base);
MUGGLE_CC_EXPORT bool MuggleStrToui(const char *str, unsigned int *pval, int base); 
MUGGLE_CC_EXPORT bool MuggleStrTol(const char *str, long *pval, int base);
MUGGLE_CC_EXPORT bool MuggleStrToul(const char *str, unsigned long *pval, int base);
MUGGLE_CC_EXPORT bool MuggleStrToll(const char *str, long long *pval, int base);
MUGGLE_CC_EXPORT bool MuggleStrToull(const char *str, unsigned long long *pval, int base);
MUGGLE_CC_EXPORT bool MuggleStrTof(const char *str, float *pval);
MUGGLE_CC_EXPORT bool MuggleStrTod(const char *str, double *pval);
MUGGLE_CC_EXPORT bool MuggleStrTold(const char *str, long double *pval);

EXTERN_C_END

#endif