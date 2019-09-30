/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "path.h"
#include <stdio.h>
#include <string.h>
#include "muggle/c/base/err.h"

void muggle_path_abspath(const char *path, char *ret, unsigned int size)
{
	// TODO:
}

int muggle_path_basename(const char *path, char *ret, unsigned int size)
{
	if (size <= 1)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	int total_len = (int)strlen(path);
	if (total_len <= 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	int pos = total_len - 1;
	while (pos >= 0)
	{
		if (path[pos] == '/' || path[pos] == '\\')
		{
			break;
		}
		--pos;
	}

	if (pos < 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	int len = total_len - 1 - pos;
	if (len <= 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	if (len >= size)
	{
		len = size - 1;
	}
	memcpy(ret, path + pos + 1, len);
	ret[len] = '\0';

	return MUGGLE_OK;
}

int muggle_path_dirname(const char *path, char *ret, unsigned int size)
{
	if (size <= 1)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	int total_len = (int)strlen(path);
	if (total_len <= 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	int pos = total_len - 1;
	while (pos >= 0)
	{
		if (path[pos] == '/' || path[pos] == '\\')
		{
			break;
		}
		--pos;
	}

	if (pos < 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	// handle path = "/"
	if (pos == 0)
	{
		pos = 1;
	}

	// handle path = "c:/"
	if (pos - 1 > 0 && path[pos - 1] == ':')
	{
		pos = pos + 1;
	}

	if (pos >= (int)size)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	memcpy(ret, path, pos);
	ret[pos] = '\0';

	return MUGGLE_OK;
}

#if MUGGLE_PLATFORM_WINDOWS

int muggle_path_isabs(const char *path)
{
	size_t len = strlen(path);
	if (len > 2 &&
		((path[0] >= 'a' && path[0] <= 'z') || (path[0] >= 'A' && path[0] <= 'Z')) &&
		path[1] == ':' &&
		(path[2] == '/' || path[2] == '\\'))
	{
		return 1;
	}

	return 0;
}

#else

int muggle_path_isabs(const char *path)
{
	size_t len = strlen(path);
	if (len > 1 && path[0] == '/')
	{
		return 1;
	}

	return 0;
}

#endif
