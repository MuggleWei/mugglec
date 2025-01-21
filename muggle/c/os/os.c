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
#include "muggle/c/log/log.h"
#include <stdlib.h>

#if MUGGLE_PLATFORM_WINDOWS

#include <windows.h>
#include <stdio.h>
#include <stdbool.h>

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

muggle_file_list_node_t *muggle_os_listdir(const char *dirpath, int ftype)
{
	HANDLE hFind;
	WIN32_FIND_DATAA ffd;

	char win_dirpath[MUGGLE_MAX_PATH];
	muggle_path_join(dirpath, "\\*", win_dirpath, sizeof(win_dirpath));

	hFind = FindFirstFileA(win_dirpath, &ffd);
	if (hFind == INVALID_HANDLE_VALUE) {
		return NULL;
	}

	muggle_file_list_node_t *head = NULL;
	muggle_file_list_node_t *tail = NULL;
	do {
		if ((strcmp(ffd.cFileName, ".") == 0) ||
			(strcmp(ffd.cFileName, "..") == 0)) {
			continue;
		}

		bool filter_pass = true;
		if (ftype != MUGGLE_FILE_TYPE_NULL) {
			switch (ftype) {
				case MUGGLE_FILE_TYPE_REGULAR: {
					if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)) {
						filter_pass = false;
					}
				}break;
				case MUGGLE_FILE_TYPE_DIR: {
					if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
						filter_pass = false;
					}
				}break;
				case MUGGLE_FILE_TYPE_LINK: {
					if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
						filter_pass = false;
					}
				}break;
				default: {
					filter_pass = false;
				}break;
			}
		}

		if (!filter_pass) {
			continue;
		}

		muggle_file_list_node_t *node =
			(muggle_file_list_node_t *)malloc(sizeof(muggle_file_list_node_t));
		if (node == NULL) {
			MUGGLE_LOG_FATAL("failed allocate file_list_node");
			break;
		}

		node->next = NULL;
		size_t len = strlen(ffd.cFileName);
		node->filepath = (char *)malloc(len + 1);
		memcpy(node->filepath, ffd.cFileName, len);
		node->filepath[len] = '\0';

		if (tail == NULL) {
			head = node;
			tail = node;
		} else {
			tail->next = node;
			tail = node;
		}
	} while (FindNextFileA(hFind, &ffd) != 0);
	FindClose(hFind);

	return head;
}

#else

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
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

muggle_file_list_node_t *muggle_os_listdir(const char *dirpath, int ftype)
{
	DIR *dirp = opendir(dirpath);
	if (dirp == NULL) {
		return NULL;
	}

	muggle_file_list_node_t *head = NULL;
	muggle_file_list_node_t *tail = NULL;
	struct dirent *direntp = NULL;
	struct stat stat_buf;
	char filepath[MUGGLE_MAX_PATH];
	while ((direntp = readdir(dirp)) != NULL) {
		if ((strcmp(direntp->d_name, ".") == 0) ||
			(strcmp(direntp->d_name, "..") == 0)) {
			continue;
		}

		if (muggle_path_join(dirpath, direntp->d_name, filepath,
							 sizeof(filepath)) != 0) {
			MUGGLE_LOG_FATAL("failed join path: %s, %s",
				dirpath, direntp->d_name);
			continue;
		}

		if (stat(filepath, &stat_buf) == -1) {
			MUGGLE_LOG_ERROR("failed stat filepath: %s", filepath);
			continue;
		}

		bool filter_pass = true;
		if (ftype != MUGGLE_FILE_TYPE_NULL) {
			switch (ftype) {
				case MUGGLE_FILE_TYPE_REGULAR: {
					if (!S_ISREG(stat_buf.st_mode)) {
						filter_pass = false;
					}
				}break;
				case MUGGLE_FILE_TYPE_DIR: {
					if (!S_ISDIR(stat_buf.st_mode)) {
						filter_pass = false;
					}
				}break;
				case MUGGLE_FILE_TYPE_LINK: {
					if (!S_ISLNK(stat_buf.st_mode)) {
						filter_pass = false;
					}
				}break;
				default: {
					filter_pass = false;
				}break;
			}
		}
		if (!filter_pass) {
			continue;
		}

		muggle_file_list_node_t *node =
			(muggle_file_list_node_t *)malloc(sizeof(muggle_file_list_node_t));
		if (node == NULL) {
			MUGGLE_LOG_FATAL("failed allocate file_list_node");
			continue;
		}

		node->next = NULL;
		size_t len = strlen(direntp->d_name);
		node->filepath = (char *)malloc(len + 1);
		memcpy(node->filepath, direntp->d_name, len);
		node->filepath[len] = '\0';

		if (tail == NULL) {
			head = node;
			tail = node;
		} else {
			tail->next = node;
			tail = node;
		}
	}

	closedir(dirp);

	return head;
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

void muggle_os_free_file_nodes(muggle_file_list_node_t *node)
{
	muggle_file_list_node_t *next = NULL;
	while (node) {
		next = (muggle_file_list_node_t *)node->next;
		if (node->filepath) {
			free(node->filepath);
		}
		free(node);

		node = next;
	}
}
