/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "os.h"
#include "muggle/c/base/err.h"

#if MUGGLE_PLATFORM_WINDOWS

#include <windows.h>

int muggle_os_process_path(char *path, unsigned int size)
{
	// get module file name in unicode characters
	WCHAR unicode_buf[MUGGLE_MAX_PATH] = { 0 };
	DWORD ret = GetModuleFileNameW(NULL, unicode_buf, MUGGLE_MAX_PATH);
	if (ret == 0)
	{
		return MUGGLE_ERR_SYS_CALL;
	}

	// convert to utf8
	ret = WideCharToMultiByte(CP_UTF8, 0, unicode_buf, -1, path, size - 1, NULL, FALSE);
	if (ret == 0)
	{
		return MUGGLE_ERR_SYS_CALL;
	}

	return MUGGLE_OK;
}

int muggle_os_curdir(char *path, unsigned int size)
{
	// get current dir in unicode characters
	WCHAR unicode_buf[MUGGLE_MAX_PATH] = { 0 };
	DWORD ret = GetCurrentDirectoryW(MUGGLE_MAX_PATH, unicode_buf);
	if (ret == 0)
	{
		return MUGGLE_ERR_SYS_CALL;
	}

	// convert to utf8
	ret = WideCharToMultiByte(CP_UTF8, 0, unicode_buf, -1, path, size - 1, NULL, FALSE);
	if (ret == 0)
	{
		return MUGGLE_ERR_SYS_CALL;
	}

	return MUGGLE_OK;
}

int muggle_os_chdir(const char *path)
{
	return SetCurrentDirectoryA(path) ? MUGGLE_OK : MUGGLE_ERR_SYS_CALL;
}

int muggle_os_mkdir(const char *path)
{
	// adapted from: https://gist.github.com/JonathonReinhart/8c0d90191c38af2dcadb102c4e202950
	const size_t len = strlen(path);
	char _path[MUGGLE_MAX_PATH];
	char *p;

	if (len > sizeof(_path)-1)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}
	strcpy(_path, path);

	/* Iterate the string */
	for (p = _path + 1; *p; p++)
	{
		if (*p == '/')
		{
			/* Temporarily truncate */
			*p = '\0';

			if (!CreateDirectory(_path, NULL))
			{
				DWORD err = GetLastError();
				if(err != ERROR_ALREADY_EXISTS)
				{
					return MUGGLE_ERR_SYS_CALL;
				}
			}

			*p = '/';
		}
	}

	if (!CreateDirectory(_path, NULL))
	{
		DWORD err = GetLastError();
		if(err != ERROR_ALREADY_EXISTS)
		{
			return MUGGLE_ERR_SYS_CALL;
		}
	}

	return MUGGLE_OK;
}

#else

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

int muggle_os_process_path(char *path, unsigned int size)
{
	char sz_tmp[64], buf[MUGGLE_MAX_PATH];
	ssize_t len;

	snprintf(sz_tmp, 63, "/proc/%ld/exe", (long)getpid());
	len = readlink(sz_tmp, path, size - 1);
	if (len >= 0)
	{
		path[len] = '\0';
		return MUGGLE_OK;
	}

	return MUGGLE_ERR_SYS_CALL;
}

int muggle_os_curdir(char *path, unsigned int size)
{
	return getcwd(path, (size_t)size) != NULL ? MUGGLE_OK : MUGGLE_ERR_SYS_CALL;
}

int muggle_os_chdir(const char *path)
{
	return chdir(path) == 0 ? MUGGLE_OK : MUGGLE_ERR_SYS_CALL;
}

int muggle_os_mkdir(const char *path)
{
	// adapted from: https://gist.github.com/JonathonReinhart/8c0d90191c38af2dcadb102c4e202950
	const size_t len = strlen(path);
	char _path[MUGGLE_MAX_PATH];
	char *p;

	if (len > sizeof(_path)-1)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}
	strcpy(_path, path);

	/* Iterate the string */
	for (p = _path + 1; *p; p++)
	{
		if (*p == '/')
		{
			/* Temporarily truncate */
			*p = '\0';

			if (mkdir(_path, S_IRWXU) != 0)
			{
				if (errno != EEXIST)
				{
					return MUGGLE_ERR_SYS_CALL;
				}
			}

			*p = '/';
		}
	}

	if (mkdir(_path, S_IRWXU) != 0)
	{
		if (errno != EEXIST)
		{
			return MUGGLE_ERR_SYS_CALL;
		}
	}

	return MUGGLE_OK;
}

#endif
