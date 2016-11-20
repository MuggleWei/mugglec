/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "muggle/base/str.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "muggle/base/log.h"

char* StrAllocByDiff(const char* p_start, const char* p_end)
{
	char* buf = NULL;
	ptrdiff_t diff = p_end - p_start + 1;
	buf = (char*)malloc((size_t)diff + 1);
	memcpy(buf, p_start, (size_t)diff);
	buf[(size_t)diff] = '\0';

	return buf;
}
bool StrStartsWith(const char* str, const char* sub_str)
{
	if (str == NULL || sub_str == NULL)
	{
		MUGGLE_DEBUG_WARNING(0, "maybe input error string");
		return false;
	}

	char* p = strstr(str, sub_str);
	if (p == str)
	{
		return true;
	}

	return false;
}
bool StrEndsWith(const char* str, const char* sub_str)
{
	size_t str_len, sub_str_len, i;
	const char *p, *q;

	if (str == NULL || sub_str == NULL)
	{
		MUGGLE_DEBUG_WARNING(0, "maybe input error string");
		return false;
	}

	str_len = strlen(str);
	sub_str_len = strlen(sub_str);
	
	if (sub_str_len > str_len)
	{
		return false;
	}

	p = str + str_len - 1;
	q = sub_str + sub_str_len - 1;
	for (i = 0; i < sub_str_len; ++i)
	{
		if (*p != *q)
		{
			return false;
		}
		--p;
		--q;
	}

	return true;
}

char* StrSplitLineToWords(char* line, char** words, int* word_idx, int max_word_num)
{
	char *q = NULL, *r = NULL;

	// initialize words
	memset(words, 0, sizeof(char*) * max_word_num);
	*word_idx = 0;

	q = line;
	MG_SKIP_BLANK(q);
	r = q;
	while (*q != '\n' && *q != '\r' && *q != '\0')
	{
		if (*word_idx == max_word_num)
		{
			MUGGLE_DEBUG_WARNING(0, "It's has no enough space to split line to words");
			return q;
		}

		if (*q == ' ' || *q == '\t')
		{
			*q = '\0';
			++q;
			words[(*word_idx)++] = r;
			MG_SKIP_BLANK(q);
			r = q;
		}
		else
		{
			++q;
		}
	}

	if (*word_idx == max_word_num)
	{
		MUGGLE_DEBUG_WARNING(0, "It's has no enough space to split line to words");
		return q;
	}
	if (*r != '\r' && *r != '\n' && *r != '\0')
	{
		words[(*word_idx)++] = r;
	}
	while (*q == '\n' || *q == '\r')
	{
		*q = '\0';
		++q;
		r = q;
	}

	return q;
}