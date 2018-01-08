/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "muggle/c/base/str.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include "muggle/c/base/log.h"

char* MuggleStrAllocByDiff(const char* p_start, const char* p_end)
{
	char* buf = NULL;
	ptrdiff_t diff = p_end - p_start + 1;
	buf = (char*)malloc((size_t)diff + 1);
	memcpy(buf, p_start, (size_t)diff);
	buf[(size_t)diff] = '\0';

	return buf;
}
bool MuggleStrStartsWith(const char* str, const char* sub_str)
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
bool MuggleStrEndsWith(const char* str, const char* sub_str)
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

char* MuggleStrSplitLineToWords(char* line, char** words, int* cnt, int max_word_num)
{
	char *q = NULL, *r = NULL;

	// initialize words
	memset(words, 0, sizeof(char*) * max_word_num);
	*cnt = 0;

	q = line;
	MUGGLE_SKIP_BLANK(q);
	r = q;
	while (*q != '\n' && *q != '\r' && *q != '\0')
	{
		if (*cnt == max_word_num)
		{
			MUGGLE_DEBUG_WARNING(0, "It's has no enough space to split line to words");
			return q;
		}

		if (*q == ' ' || *q == '\t')
		{
			*q = '\0';
			++q;
			words[(*cnt)++] = r;
			MUGGLE_SKIP_BLANK(q);
			r = q;
		}
		else
		{
			++q;
		}
	}

	if (*cnt == max_word_num)
	{
		MUGGLE_DEBUG_WARNING(0, "It's has no enough space to split line to words");
		return q;
	}
	if (*r != '\r' && *r != '\n' && *r != '\0')
	{
		words[(*cnt)++] = r;
	}
	while (*q == '\n' || *q == '\r')
	{
		*q = '\0';
		++q;
		r = q;
	}

	return q;
}

bool MuggleStrToi(const char *str, int *pval, int base)
{
	long ret;
	char *endptr;

	if (str == NULL || pval == NULL)
	{
		return false;
	}

	errno = 0;
    ret = strtol(str, &endptr, base);
	if (endptr == str)
	{
		// failed parse
		return false;
	}
	else if (*endptr != '\0')
	{
		MUGGLE_SKIP_BLANK(endptr);
		if (*endptr != '\0')
		{
			// this function only parse one word
			return false;
		}
	}
	else if ((ret == LONG_MAX || ret == LONG_MIN) && errno == ERANGE)
	{
		// out of range
		return false;
	}
	else if (ret > INT_MAX || ret < INT_MIN)
	{
		// out of integer range
		return false;
	}

	*pval = (int)ret;

	return true;
}
bool MuggleStrToui(const char *str, unsigned int *pval, int base)
{
	unsigned long ret;
	char *endptr;

	if (str == NULL || pval == NULL)
	{
		return false;
	}

	errno = 0;
	ret = strtoul(str, &endptr, base);
	if (endptr == str)
	{
		// failed parse
		return false;
	}
	else if (*endptr != '\0')
	{
		MUGGLE_SKIP_BLANK(endptr);
		if (*endptr != '\0')
		{
			// this function only parse one word
			return false;
		}
	}
	else if (ret == ULONG_MAX && errno == ERANGE)
	{
		// out of range or negative integer
		return false;
	}
	else if (ret > UINT_MAX)
	{
		// out of unsigned integer range
		return false;
	}

	*pval = (unsigned int)ret;

	return true;
}
bool MuggleStrTol(const char *str, long *pval, int base)
{
	char *endptr;

	if (str == NULL || pval == NULL)
	{
		return false;
	}

	errno = 0;
	*pval = strtol(str, &endptr, base);
	if (endptr == str)
	{
		// failed parse
		return false;
	}
	else if (*endptr != '\0')
	{
		MUGGLE_SKIP_BLANK(endptr);
		if (*endptr != '\0')
		{
			// this function only parse one word
			return false;
		}
	}
	else if ((*pval == LONG_MAX || *pval == LONG_MIN) && errno == ERANGE)
	{
		// out of range
		return false;
	}

	return true;
}
bool MuggleStrToul(const char *str, unsigned long *pval, int base)
{
	char *endptr;

	if (str == NULL || pval == NULL)
	{
		return false;
	}

	errno = 0;
    *pval = strtoul(str, &endptr, base);
	if (endptr == str)
	{
		// failed parse
		return false;
	}
	else if (*endptr != '\0')
	{
		MUGGLE_SKIP_BLANK(endptr);
		if (*endptr != '\0')
		{
			// this function only parse one word
			return false;
		}
	}
	else if (*pval == ULONG_MAX && errno == ERANGE)
	{
		// out of range
		return false;
	}

	return true;
}
bool MuggleStrToll(const char *str, long long *pval, int base)
{
	char *endptr;

	if (str == NULL || pval == NULL)
	{
		return false;
	}

	errno = 0;
    *pval = strtoll(str, &endptr, base);
	if (endptr == str)
	{
		// failed parse
		return false;
	}
	else if (*endptr != '\0')
	{
		MUGGLE_SKIP_BLANK(endptr);
		if (*endptr != '\0')
		{
			// this function only parse one word
			return false;
		}
	}
	else if ((*pval == LLONG_MAX || *pval == LLONG_MIN) && errno == ERANGE)
	{
		// out of range
		return false;
	}

	return true;
}
bool MuggleStrToull(const char *str, unsigned long long *pval, int base)
{
	char *endptr;

	if (str == NULL || pval == NULL)
	{
		return false;
	}

	errno = 0;
    *pval = strtoull(str, &endptr, base);
	if (endptr == str)
	{
		// failed parse
		return false;
	}
	else if (*endptr != '\0')
	{
		MUGGLE_SKIP_BLANK(endptr);
		if (*endptr != '\0')
		{
			// this function only parse one word
			return false;
		}
	}
	else if (*pval == ULLONG_MAX && errno == ERANGE)
	{
		// out of range
		return false;
	}

	return true;
}
bool MuggleStrTof(const char *str, float *pval)
{
	char *endptr;

	if (str == NULL || pval == NULL)
	{
		return false;
	}

	errno = 0;
    *pval = strtof(str, &endptr);
	if (endptr == str)
	{
		// failed parse
		return false;
	}
	else if (*endptr != '\0')
	{
		MUGGLE_SKIP_BLANK(endptr);
		if (*endptr != '\0')
		{
			// this function only parse one word
			return false;
		}
	}
	else if ((*pval == HUGE_VAL || *pval == HUGE_VALF || *pval == HUGE_VALL) && errno == ERANGE)
	{
		// out of range
		return false;
	}

	return true;
}
bool MuggleStrTod(const char *str, double *pval)
{
	char *endptr;

	if (str == NULL || pval == NULL)
	{
		return false;
	}

	errno = 0;
    *pval = strtod(str, &endptr);
	if (endptr == str)
	{
		// failed parse
		return false;
	}
	else if (*endptr != '\0')
	{
		MUGGLE_SKIP_BLANK(endptr);
		if (*endptr != '\0')
		{
			// this function only parse one word
			return false;
		}
	}
	else if ((*pval == HUGE_VAL || *pval == HUGE_VALF || *pval == HUGE_VALL) && errno == ERANGE)
	{
		// out of range
		return false;
	}

	return true;
}
bool MuggleStrTold(const char *str, long double *pval)
{
	char *endptr;

	if (str == NULL || pval == NULL)
	{
		return false;
	}

	errno = 0;
    *pval = strtold(str, &endptr);
	if (endptr == str)
	{
		// failed parse
		return false;
	}
	else if (*endptr != '\0')
	{
		MUGGLE_SKIP_BLANK(endptr);
		if (*endptr != '\0')
		{
			// this function only parse one word
			return false;
		}
	}
	else if ((*pval == HUGE_VAL || *pval == HUGE_VALF || *pval == HUGE_VALL) && errno == ERANGE)
	{
		// out of range
		return false;
	}

	return true;
}