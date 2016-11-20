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

#define MG_SKIP_BLANK(p) \
while (*p == ' ' || *p == '\t') \
{ \
	++p; \
}

#define MG_SKIP_BLANK_AND_LFCR(p) \
while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') \
{ \
	++p; \
}

#define MG_SKIP_WORD(p) \
while (*p != ' ' && *p != '\t' && *p != '\r' && *p != '\n' && *p != '\0') \
{ \
	++p; \
}

#define MG_SKIP_TO_NEXT_WORD(p) \
MG_SKIP_WORD(p) \
MG_SKIP_BLANK_AND_LFCR(p)

#define MG_SKIP_TO_LINE_END(p) \
while (*p != '\n' && *p != '\r' && *p != '\0') \
{ \
	++p; \
}

#define MG_SKIP_TO_NEXT_LINE(p) \
MG_SKIP_TO_LINE_END(p); \
if (*p != '\0') \
{ \
	MG_SKIP_TO_NEXT_WORD(p); \
}

// note: the range is [p_start, p_end], so generate char* is "p_start ... p_end '\0'"
MUGGLE_BASE_EXPORT char* StrAllocByDiff(const char* p_start, const char* p_end);
MUGGLE_BASE_EXPORT bool StrStartsWith(const char* str, const char* sub_str);
MUGGLE_BASE_EXPORT bool StrEndsWith(const char* str, const char* sub_str);

/*
*  StrUtils_SplitLineToWords: split line to words
*  @line: input line
*  @words: output words
*  @word_idx: record number words in this line
*  @max_word_num: up limit number of words in this line
*  @return value: return next line
*/
MUGGLE_BASE_EXPORT char* StrSplitLineToWords(char* line, char** words, int* word_idx, int max_word_num);

EXTERN_C_END

#endif