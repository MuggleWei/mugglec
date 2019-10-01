/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "path.h"

#if MUGGLE_PLATFORM_WINDOWS
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <stdio.h>
#include <string.h>
#include "muggle/c/base/err.h"
#include "muggle/c/base/str.h"
#include "muggle/c/os/os.h"

int muggle_path_abspath(const char *path, char *ret, unsigned int size)
{
	if (size <= 1)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	if (muggle_path_isabs(path))
	{
		if (strlen(path) > size - 1)
		{
			return MUGGLE_ERR_INVALID_PARAM;
		}

		strncpy(ret, path, size - 1);
		return MUGGLE_OK;
	}

	char cur_dir[MUGGLE_MAX_PATH];
	int r;
	r = muggle_os_curdir(cur_dir, sizeof(cur_dir));
	if (r != MUGGLE_OK)
	{
		return r;
	}

	char full_path[MUGGLE_MAX_PATH];
	r = muggle_path_join(cur_dir, path, full_path, sizeof(full_path));
	if (r != MUGGLE_OK)
	{
		return r;
	}

	r = muggle_path_normpath(full_path, ret, size);
	if (r != MUGGLE_OK)
	{
		return r;
	}

	return MUGGLE_OK;
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

int muggle_path_isabs(const char *path)
{
	size_t len = strlen(path);

	if (len > 1 && path[0] == '/')
	{
		return 1;
	}

	if (len > 2 &&
		((path[0] >= 'a' && path[0] <= 'z') || (path[0] >= 'A' && path[0] <= 'Z')) &&
		path[1] == ':' &&
		(path[2] == '/' || path[2] == '\\'))
	{
		return 1;
	}

	return 0;
}

int muggle_path_exists(const char *path)
{
#if MUGGLE_PLATFORM_WINDOWS
	// convert to utf16 characters
	WCHAR unicode_buf[MUGGLE_MAX_PATH] = { 0 };
	MultiByteToWideChar(CP_UTF8, 0, path, -1, unicode_buf, MUGGLE_MAX_PATH);

	// get file attributes
	DWORD attr = GetFileAttributesW(unicode_buf);
	if (attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY))
	{
		return 0;
	}

	return 1;
#else
	if (access(path, F_OK) != -1)
		return 1;

	return 0;
#endif
}

int muggle_path_join(const char *path1, const char *path2, char *ret, unsigned int size)
{
	int len_path1 = (int)strlen(path1);
	int len_path2 = (int)strlen(path2);
	if (len_path1 <= 0 || len_path2 <= 0)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	if (len_path1 >= size)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}
	strncpy(ret, path1, size - 1);

	if (*path2 == '\0')
	{
		return MUGGLE_OK;
	}

	int pos = len_path1;
	if (!muggle_str_endswith(ret, "/") && !muggle_str_endswith(ret, "\\"))
	{
		if (pos >= size)
		{
			return MUGGLE_ERR_INVALID_PARAM;
		}

		ret[pos] = '/';
		pos++;
		ret[pos] = '\0';

		len_path1 += 1;
	}

	const char *p = path2;
	if (*p == '/')
	{
		if (len_path2 == 1)
		{
			return MUGGLE_ERR_INVALID_PARAM;
		}
		p += 1;

		len_path2 -= 1;
	}

	if (len_path1 + len_path2 >= size)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	strncpy(ret + pos, p, len_path2);

	return MUGGLE_OK;
}

int muggle_path_normpath(const char *path, char *ret, unsigned int size)
{
	int r;
	int pos = 0;
	const char *cursor = path;
	int len = (int)strlen(path);

	// cal max len
	if (len >= size)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}

	if (!muggle_path_isabs(path))
	{
		if (muggle_str_startswith(path, "./") || muggle_str_startswith(path, ".\\"))
		{
			cursor += 2;
		}
	}

	while (*cursor != '\0')
	{
		if (*cursor == '.' && *(cursor + 1) == '.')
		{
			if (*(cursor + 2) != '\0' && *(cursor + 2) != '/' && *(cursor + 2) != '\\')
			{
				return MUGGLE_ERR_INVALID_PARAM;
			}

			cursor += 2;
			if (pos == 0)
			{
				ret[pos++] = '.';
				ret[pos++] = '.';
				ret[pos++] = *cursor;
			}
			else
			{
				if (pos >= 3 && ret[pos-3] == '.' && ret[pos-2] == '.' &&
					(ret[pos-1] == '/' || ret[pos-1] == '\\'))
				{
					ret[pos++] = '.';
					ret[pos++] = '.';
					ret[pos++] = *cursor;
				}
				else
				{
					if (ret[pos-1] != '/' && ret[pos-1] != '\\')
					{
						return MUGGLE_ERR_INVALID_PARAM;
					}

					pos -= 2;
					if (pos < 0)
					{
						return MUGGLE_ERR_INVALID_PARAM;
					}

					int i = pos;
					while (i >= 0)
					{
						if (ret[i] == '/' || ret[i] == '\\')
						{
							break;
						}
						--i;
					}

					pos = i + 1;
				}
			}

		}
		else
		{
			ret[pos++] = *cursor;
		}

		if (*cursor != '\0')
		{
			++cursor;
		}
	}

	if (pos == 0)
	{
		if (size <= 2)
		{
			return MUGGLE_ERR_INVALID_PARAM;
		}

		ret[pos++] = '.';
		ret[pos++] = '/';
	}
	ret[pos] = '\0';

	return MUGGLE_OK;
}
