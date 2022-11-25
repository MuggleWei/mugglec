/******************************************************************************
 *  @file         str.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-15
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec string processing
 *****************************************************************************/

#include "str.h"
#include "muggle/c/base/err.h"
#include <errno.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>

int muggle_str_startswith(const char *str, const char *prefix)
{
	if (str == NULL || prefix == NULL)
	{
		return 0;
	}

	size_t str_len = strlen(str);
	size_t prefix_len = strlen(prefix);

	if (str_len < prefix_len)
	{
		return 0;
	}

	for (size_t i = 0; i < prefix_len; ++i)
	{
		if (str[i] != prefix[i])
		{
			return 0;
		}
	}

	if (str_len != 0 && prefix_len == 0)
	{
		return 0;
	}


	return 1;
}

int muggle_str_endswith(const char *str, const char *suffix)
{
	if (str == NULL || suffix == NULL)
	{
		return 0;
	}

	size_t str_len = strlen(str);
	size_t suffix_len = strlen(suffix);

	if (str_len < suffix_len)
	{
		return 0;
	}

	for (size_t i = 0; i < suffix_len; ++i)
	{
		if (str[str_len - 1 - i] != suffix[suffix_len - 1 - i])
		{
			return 0;
		}
	}

	if (str_len != 0 && suffix_len == 0)
	{
		return 0;
	}

	return 1;
}

int muggle_str_count(const char *str, const char *sub, int start, int end)
{
	if (str == NULL || sub == NULL)
	{
		return 0;
	}

	size_t str_len = strlen(str);
	size_t sub_len = strlen(sub);

	if (start < 0 || end < 0)
	{
		return 0;
	}

	if (start >= (int)str_len)
	{
		return 0;
	}

	if (end == 0)
	{
		end = (int)str_len;
	}

	if (end > (int)str_len)
	{
		end = (int)str_len;
	}

	if (end <= start)
	{
		return 0;
	}

	int cnt = 0;
	const char *pos = str + start;
	const char *end_pos = str + end;
	while (1)
	{
		pos = strstr(pos, sub);
		if (pos)
		{
			if (pos + sub_len > end_pos)
			{
				break;
			}
			++cnt;
			pos = pos + sub_len;
			if (pos >= end_pos)
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

	return cnt;
}

int muggle_str_find(const char *str, const char *sub, int start, int end)
{
	if (str == NULL || sub == NULL)
	{
		return -1;
	}

	size_t str_len = strlen(str);
	size_t sub_len = strlen(sub);

	if (start < 0 || end < 0)
	{
		return -1;
	}

	if (start >= (int)str_len)
	{
		return -1;
	}

	if (end == 0)
	{
		end = (int)str_len;
	}

	if (end > (int)str_len)
	{
		end = (int)str_len;
	}

	if (end <= start)
	{
		return -1;
	}

	const char *pos = strstr(str + start, sub);
	if (pos == NULL || pos + sub_len > str + end)
	{
		return -1;
	}

	return (int)(pos - str);
}

int muggle_str_lstrip_idx(const char *str)
{
	if (str == NULL)
	{
		return -1;
	}

	int str_len = (int)strlen(str);
	int idx = 0;
	while (isspace(str[idx]))
	{
		if (++idx >= str_len)
		{
			return -1;
		}
	}

	return idx;
}

int muggle_str_rstrip_idx(const char *str)
{
	if (str == NULL)
	{
		return -1;
	}

	int str_len = (int)strlen(str);
	int idx = str_len - 1;
	while (isspace(str[idx]))
	{
		if (--idx < 0)
		{
			return -1;
		}
	}

	return idx;
}

// static int in_split_sep(int ch, const char *sep, int sep_len)
// {
//     if (sep == NULL)
//     {
//         return isspace(ch);
//     }

//     for (int i = 0; i < sep_len; ++i)
//     {
//         if (ch == (int)sep[i])
//         {
//             return 1;
//         }
//     }

//     return 0;
// }

int muggle_str_toi(const char *str, int *pval, int base)
{
	long ret;
	char *endptr;

	if (str == NULL || pval == NULL)
	{
		return 0;
	}

	errno = 0;
    ret = strtol(str, &endptr, base);
	if (endptr == str)
	{
		// failed parse
		return 0;
	}
	else if (*endptr != '\0')
	{
		// this function only parse one word
		if (muggle_str_lstrip_idx(endptr) != -1)
		{
			return 0;
		}
	}
	else if ((ret == LONG_MAX || ret == LONG_MIN) && errno == ERANGE)
	{
		// out of range
		return 0;
	}
	else if (ret > INT_MAX || ret < INT_MIN)
	{
		// out of integer range
		return 0;
	}

	*pval = (int)ret;

	return 1;
}
int muggle_str_tou(const char *str, unsigned int *pval, int base)
{
	unsigned long ret;
	char *endptr;

	if (str == NULL || pval == NULL)
	{
		return 0;
	}

	errno = 0;
    ret = strtoul(str, &endptr, base);
	if (endptr == str)
	{
		// failed parse
		return 0;
	}
	else if (*endptr != '\0')
	{
		// this function only parse one word
		if (muggle_str_lstrip_idx(endptr) != -1)
		{
			return 0;
		}
	}

	if (ret == ULONG_MAX || ret == ULLONG_MAX)
	{
		// out of range or negative integer
		return 0;
	}

	*pval = (unsigned int)ret;

	return 1;
}
int muggle_str_tol(const char *str, long *pval, int base)
{
	char *endptr;

	if (str == NULL || pval == NULL)
	{
		return 0;
	}

	errno = 0;
	*pval = strtol(str, &endptr, base);
	if (endptr == str)
	{
		// failed parse
		return 0;
	}
	else if (*endptr != '\0')
	{
		// this function only parse one word
		if (muggle_str_lstrip_idx(endptr) != -1)
		{
			return 0;
		}
	}

	if (*pval == LONG_MAX || *pval == LONG_MIN || *pval == LLONG_MAX || *pval == LLONG_MIN)
	{
		// out of range
		return 0;
	}

	return 1;
}
int muggle_str_toul(const char *str, unsigned long *pval, int base)
{
	char *endptr;

	if (str == NULL || pval == NULL)
	{
		return 0;
	}

	errno = 0;
    *pval = strtoul(str, &endptr, base);
	if (endptr == str)
	{
		// failed parse
		return 0;
	}
	else if (*endptr != '\0')
	{
		// this function only parse one word
		if (muggle_str_lstrip_idx(endptr) != -1)
		{
			return 0;
		}
	}

	if (*pval == ULONG_MAX || *pval == ULLONG_MAX)
	{
		// out of range
		return 0;
	}

	return 1;
}
int muggle_str_toll(const char *str, long long *pval, int base)
{
	char *endptr;

	if (str == NULL || pval == NULL)
	{
		return 0;
	}

	errno = 0;
    *pval = strtoll(str, &endptr, base);
	if (endptr == str)
	{
		// failed parse
		return 0;
	}
	else if (*endptr != '\0')
	{
		// this function only parse one word
		if (muggle_str_lstrip_idx(endptr) != -1)
		{
			return 0;
		}
	}

	if (*pval == LLONG_MAX || *pval == LLONG_MIN)
	{
		// out of range
		return 0;
	}

	return 1;
}
int muggle_str_toull(const char *str, unsigned long long *pval, int base)
{
	char *endptr;

	if (str == NULL || pval == NULL)
	{
		return 0;
	}

	errno = 0;
    *pval = strtoull(str, &endptr, base);
	if (endptr == str)
	{
		// failed parse
		return 0;
	}
	else if (*endptr != '\0')
	{
		// this function only parse one word
		if (muggle_str_lstrip_idx(endptr) != -1)
		{
			return 0;
		}
	}

	if (*pval == ULLONG_MAX)
	{
		// out of range
		return 0;
	}

	return 1;
}
int muggle_str_tof(const char *str, float *pval)
{
	char *endptr;

	if (str == NULL || pval == NULL)
	{
		return 0;
	}

	errno = 0;
    *pval = strtof(str, &endptr);
	if (endptr == str)
	{
		// failed parse
		return 0;
	}
	else if (*endptr != '\0')
	{
		// this function only parse one word
		if (muggle_str_lstrip_idx(endptr) != -1)
		{
			return 0;
		}
	}

	if (*pval == HUGE_VAL || *pval == HUGE_VALF || *pval == HUGE_VALL)
	{
		// out of range
		return 0;
	}

	return 1;
}
int muggle_str_tod(const char *str, double *pval)
{
	char *endptr;

	if (str == NULL || pval == NULL)
	{
		return 0;
	}

	errno = 0;
    *pval = strtod(str, &endptr);
	if (endptr == str)
	{
		// failed parse
		return 0;
	}
	else if (*endptr != '\0')
	{
		// this function only parse one word
		if (muggle_str_lstrip_idx(endptr) != -1)
		{
			return 0;
		}
	}
	else if ((*pval == HUGE_VAL || *pval == HUGE_VALF || *pval == HUGE_VALL) && errno == ERANGE)
	{
		// out of range
		return 0;
	}

	return 1;

}
int muggle_str_told(const char *str, long double *pval)
{
	char *endptr;

	if (str == NULL || pval == NULL)
	{
		return 0;
	}

	errno = 0;
    *pval = strtold(str, &endptr);
	if (endptr == str)
	{
		// failed parse
		return 0;
	}
	else if (*endptr != '\0')
	{
		// this function only parse one word
		if (muggle_str_lstrip_idx(endptr) != -1)
		{
			return 0;
		}
	}
	else if ((*pval == HUGE_VAL || *pval == HUGE_VALF || *pval == HUGE_VALL) && errno == ERANGE)
	{
		// out of range
		return 0;
	}

	return 1;
}

