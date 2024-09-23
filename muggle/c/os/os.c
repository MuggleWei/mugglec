/******************************************************************************
 *  @file         os.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-21
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec os utils
 *****************************************************************************/

#include "os.h"
#include "muggle/c/base/err.h"
#include "muggle/c/os/path.h"

#if MUGGLE_PLATFORM_WINDOWS

#include <windows.h>
#include <stdio.h>

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

	if (len == 0)
	{
		return MUGGLE_OK;
	}

	if (len > sizeof(_path)-1)
	{
		return MUGGLE_ERR_INVALID_PARAM;
	}
	strcpy(_path, path);

	/* Iterate the string */
	for (p = _path + 1; *p; p++)
	{
		if (*p == '/' || *p == '\\')
		{
			char c = *p;

			/* Temporarily truncate */
			*p = '\0';

			if (strlen(_path) == 2 && _path[1] == ':')
			{
				// don't need to create windows drive letter
			}
			else if (!CreateDirectoryA(_path, NULL))
			{
				DWORD err = GetLastError();
				if(err != ERROR_ALREADY_EXISTS)
				{
					return MUGGLE_ERR_SYS_CALL;
				}
			}

			*p = c;
		}
	}

	if (strlen(_path) == 2 && _path[1] == ':')
	{
		// don't need to create windows drive letter
	}
	else if (!CreateDirectoryA(_path, NULL))
	{
		DWORD err = GetLastError();
		if(err != ERROR_ALREADY_EXISTS)
		{
			return MUGGLE_ERR_SYS_CALL;
		}
	}

	return MUGGLE_OK;
}

int muggle_os_remove(const char *path)
{
	return DeleteFileA(path) ? MUGGLE_OK : MUGGLE_ERR_SYS_CALL;
}

int muggle_os_rmdir(const char *path)
{
	return RemoveDirectoryA(path) ? MUGGLE_OK : MUGGLE_ERR_SYS_CALL;
}

int muggle_os_rename(const char *src, const char *dst)
{
	return rename(src, dst) == 0 ? MUGGLE_OK : MUGGLE_ERR_SYS_CALL;
}

#else

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#if MUGGLE_PLATFORM_APPLE
#include <libproc.h>
#endif

int muggle_os_process_path(char *path, unsigned int size)
{
#if MUGGLE_PLATFORM_APPLE
	memset(path, 0, size);

	pid_t pid = getpid();
	int ret = proc_pidpath(pid, path, size);
	if (ret <= 0)
	{
		return MUGGLE_ERR_SYS_CALL;
	}

	return MUGGLE_OK;
#else
	char sz_tmp[64];
	ssize_t len;

	snprintf(sz_tmp, 63, "/proc/%ld/exe", (long)getpid());
	len = readlink(sz_tmp, path, size - 1);
	if (len >= 0)
	{
		path[len] = '\0';
		return MUGGLE_OK;
	}

	return MUGGLE_ERR_SYS_CALL;
#endif
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

	if (len == 0)
	{
		return MUGGLE_OK;
	}

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

int muggle_os_remove(const char *path)
{
	return remove(path) == 0 ? MUGGLE_OK : MUGGLE_ERR_SYS_CALL;
}

int muggle_os_rmdir(const char *path)
{
	return rmdir(path) == 0 ? MUGGLE_OK : MUGGLE_ERR_SYS_CALL;
}

int muggle_os_rename(const char *src, const char *dst)
{
	return rename(src, dst) == 0 ? MUGGLE_OK : MUGGLE_ERR_SYS_CALL;
}

#endif

FILE* muggle_os_fopen(const char *filepath, const char *mode)
{
	int ret = 0;
	const char *abs_filepath = NULL;
	char tmp_path[MUGGLE_MAX_PATH];
	if (muggle_path_isabs(filepath)) {
		abs_filepath = filepath;
	} else {
		char cur_path[MUGGLE_MAX_PATH];
		ret = muggle_os_curdir(cur_path, sizeof(cur_path));
		if (ret != 0) {
			return NULL;
		}

		ret = muggle_path_join(cur_path, filepath, tmp_path, sizeof(tmp_path));
		if (ret != 0) {
			return NULL;
		}

		abs_filepath = tmp_path;
	}

	char file_dir[MUGGLE_MAX_PATH];
	ret = muggle_path_dirname(abs_filepath, file_dir, sizeof(file_dir));
	if (ret != 0) {
		return NULL;
	}

	if (!muggle_path_exists(file_dir)) {
		ret = muggle_os_mkdir(file_dir);
		if (ret != 0) {
			return NULL;
		}
	}

	return fopen(abs_filepath, mode);
}
