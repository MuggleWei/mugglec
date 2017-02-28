/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "muggle/base_c/file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <errno.h>
#include "muggle/base_c/log.h"

bool FileGetAbsolutePath(const char* in_file_name, char* out_file_path)
{
	if (FileIsAbsolutePath(in_file_name))
	{
		return true;
	}

	char module_path[MUGGLE_MAX_PATH];
	FileGetProcessPath(module_path);

	FileGetDirectory(module_path, out_file_path);
	size_t len = strlen(out_file_path);

	memcpy(&out_file_path[len], in_file_name, strlen(in_file_name) + 1);

	return true;
}
bool FileGetDirectory(const char* file_path, char* dir)
{
	size_t len = strlen(file_path);
	while (len > 0)
	{
		if (file_path[len - 1] == '/' || file_path[len - 1] == '\\')
		{
			break;
		}
		--len;
	}

	if (len < 0)
	{
		return false;
	}

	memmove(dir, file_path, len);
	dir[len] = '\0';

	return true;
}
bool FileRead(const char* file_path, char** ptr_bytes, long* ptr_num)
{
	long file_size;
	size_t read_size;

	// note : why use "rb"
	// if use "r", the return value of ftell and fread maybe not equal
	FILE* fp = fopen(file_path, "rb");
	if (fp == NULL)
	{
		return false;
	}

	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if (file_size == 0)
	{
		*ptr_bytes = NULL;
		*ptr_num = 0;
		return true;
	}

	*ptr_bytes = (char*)malloc(file_size + 1);
	read_size = fread(*ptr_bytes, 1, (size_t)file_size, fp);
	(*ptr_bytes)[file_size] = '\0';
	fclose(fp);

	MUGGLE_ASSERT(read_size == file_size);
	*ptr_num = (long)read_size;

	return true;
}

#if MUGGLE_PLATFORM_WINDOWS

#include <windows.h>

void FileGetProcessPath(char* file_path)
{
	// convert to unicode characters
	WCHAR unicode_buf[MUGGLE_MAX_PATH] = { 0 };
	GetModuleFileNameW(NULL, unicode_buf, MUGGLE_MAX_PATH);

	// convert to utf8
	WideCharToMultiByte(CP_UTF8, 0, unicode_buf, -1, file_path, MUGGLE_MAX_PATH, NULL, FALSE);
}
bool FileIsExist(const char* file_path)
{
	// convert to utf16 characters
	WCHAR unicode_buf[MUGGLE_MAX_PATH] = { 0 };
	MultiByteToWideChar(CP_UTF8, 0, file_path, -1, unicode_buf, MUGGLE_MAX_PATH);

	// get file attributes
	DWORD attr = GetFileAttributesW(unicode_buf);
	if (attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY))
		return false;
	return true;
}
bool FileIsAbsolutePath(const char* file_path)
{
	size_t len = strlen(file_path);
	if (len > 2 &&
		((file_path[0] >= 'a' && file_path[0] <= 'z') || (file_path[0] >= 'A' && file_path[0] <= 'Z')) &&
		file_path[1] == ':' &&
		(file_path[2] == '/' || file_path[2] == '\\'))
	{
		return true;
	}

	return false;
}
bool FileDelete(const char *file_path)
{
	// convert to utf16 characters
	WCHAR unicode_buf[MUGGLE_MAX_PATH] = { 0 };
	MultiByteToWideChar(CP_UTF8, 0, file_path, -1, unicode_buf, MUGGLE_MAX_PATH);

	return DeleteFileW(unicode_buf);
}

bool FileHandleIsValid(FileHandle *fh)
{
	MUGGLE_ASSERT_MSG(fh != NULL, "File handle is NULL\n");
	if (fh == NULL)
	{
		return false;
	}

	return fh->fd != NULL;
}
bool FileHandleOpen(FileHandle *fh, const char* file_path, int flags, int mode)
{
	DWORD dwDesiredAccess, dwShareMode, dwCreationDisposition;
	WCHAR unicode_buf[MUGGLE_MAX_PATH] = { 0 };
	size_t len;

	MUGGLE_ASSERT_MSG(fh != NULL, "File handle is NULL\n");
	if (fh == NULL)
	{
		return false;
	}

	len = strlen(file_path);
	if (len >= MUGGLE_MAX_PATH)
	{
		MUGGLE_DEBUG_WARNING("path name is too long\n");
		return false;
	}

	// convert to utf16 characters
	MultiByteToWideChar(CP_UTF8, 0, file_path, -1, unicode_buf, MUGGLE_MAX_PATH);

	// access mode
	dwDesiredAccess =
		(((flags &  MUGGLE_FILE_WRITE) && !(flags &  MUGGLE_FILE_APPEND)) ? GENERIC_WRITE : 0) |
		((flags &  MUGGLE_FILE_READ) ? GENERIC_READ : 0) |
		((flags &  MUGGLE_FILE_APPEND) ? FILE_APPEND_DATA : 0);

	// share mode, alway allow subsequent open operations to request read, write and delete
	dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

	// creation disposition
	if (flags & MUGGLE_FILE_CREAT)
	{
		if (flags & MUGGLE_FILE_EXCL)
		{
			dwCreationDisposition = CREATE_NEW;
		}
		else if (flags & MUGGLE_FILE_TRUNC)
		{
			dwCreationDisposition = CREATE_ALWAYS;
		}
		else
		{
			dwCreationDisposition = OPEN_ALWAYS;
		}
	}
	else
	{
		if (flags & MUGGLE_FILE_TRUNC)
		{
			dwCreationDisposition = TRUNCATE_EXISTING;
			dwDesiredAccess |= GENERIC_WRITE;
		}
		else
		{
			dwCreationDisposition = OPEN_EXISTING;
		}		
	}

	fh->fd = (void*)CreateFile(
		unicode_buf, dwDesiredAccess, dwShareMode,
		NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (fh->fd == INVALID_HANDLE_VALUE)
	{
		fh->fd = NULL;
		MUGGLE_DEBUG_WARNING("Failed open file %s - error code %ld\n", file_path, (long)GetLastError());
		return false;
	}

	memcpy(fh->file_path, file_path, len);
	fh->file_path[len] = '\0';

	return true;
}
bool FileHandleClose(FileHandle *fh)
{
	MUGGLE_ASSERT_MSG(fh != NULL, "File handle is NULL\n");
	if (fh == NULL)
	{
		return false;
	}

	if (!CloseHandle((HANDLE)fh->fd))
	{
		MUGGLE_DEBUG_WARNING("Failed close file handle: %d - error code %ld\n", fh->fd, (long)GetLastError());
		return false;
	}

	fh->fd = NULL;
	fh->file_path[0] = '\0';

	return true;
}
long long FileHandleSeek(FileHandle *fh, long long offset, int whence)
{
	LARGE_INTEGER li, li_ret;
	DWORD move_method;

	MUGGLE_ASSERT_MSG(fh != NULL, "File handle is NULL\n");
	if (fh == NULL)
	{
		return -1;
	}

	li.QuadPart = (LONGLONG)offset;
	move_method =
		(whence == MUGGLE_FILE_SEEK_BEGIN) ? FILE_BEGIN :
		((whence == MUGGLE_FILE_SEEK_CURRENT) ? FILE_CURRENT : FILE_END);

	if (!SetFilePointerEx(fh->fd, li, &li_ret, move_method))
	{
		MUGGLE_DEBUG_WARNING("Failed file seek - error code %ld\n", (long)GetLastError());
		return -1;
	}

	return (long long)li_ret.QuadPart;
}
long FileHandleWrite(FileHandle *fh, const void *buf, long cnt_bytes)
{
	DWORD num_write;

	MUGGLE_ASSERT_MSG(fh != NULL, "File handle is NULL\n");
	if (fh == NULL)
	{
		return -1;
	}

	if (!WriteFile((HANDLE)fh->fd, (LPCVOID)buf, (DWORD)cnt_bytes, &num_write, NULL))
	{
		MUGGLE_DEBUG_WARNING("Failed write data into file - error code: %ld\n", (long)GetLastError());
		return (long)-1;
	}

	return (long)num_write;
}
long FileHandleRead(FileHandle *fh, void *buf, long cnt_bytes)
{
	DWORD num_read;

	MUGGLE_ASSERT_MSG(fh != NULL, "File handle is NULL\n");
	if (fh == NULL)
	{
		return -1;
	}

	if (!ReadFile((HANDLE)fh->fd, buf, (DWORD)cnt_bytes, &num_read, NULL))
	{
		MUGGLE_DEBUG_WARNING("Failed read data from file - error code: %ld\n", (long)GetLastError());
		return (long)-1;
	}

	return (long)num_read;
}

#else

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void FileGetProcessPath(char* file_path)
{
	char sz_tmp[64], buf[MUGGLE_MAX_PATH];
	ssize_t len;

	snprintf(sz_tmp, 63, "/proc/%ld/exe", (long)getpid());
	len = readlink(sz_tmp, file_path, MUGGLE_MAX_PATH);
	MUGGLE_ASSERT_MSG(len >= 0, "something wrong in readlink function");
	if (len >= 0)
		file_path[len] = '\0';
}
bool FileIsExist(const char* file_path)
{
	if (access(file_path, F_OK) != -1)
		return true;

	return false;
}
bool FileIsAbsolutePath(const char* file_path)
{
	size_t len = strlen(file_path);
	if (len > 1 && file_path[0] == '/')
	{
		return true;
	}

	return false;
}
bool FileDelete(const char *file_path)
{
	return remove(file_path) != -1;
}

bool FileHandleIsValid(FileHandle *fh)
{
	MUGGLE_ASSERT_MSG(fh != NULL, "File handle is NULL\n");
	if (fh == NULL)
	{
		return false;
	}

	return fh->fd != -1;
}
bool FileHandleOpen(FileHandle *fh, const char* file_path, int flags, int attr)
{
	int access_mode, addition_flags;
	mode_t mode;
	size_t len;

	MUGGLE_ASSERT_MSG(fh != NULL, "File handle is NULL\n");
	if (fh == NULL)
	{
		return false;
	}

	len = strlen(file_path);
	if (len >= MUGGLE_MAX_PATH)
	{
		MUGGLE_DEBUG_WARNING("path name is too long\n");
		return false;
	}

	// flags
	if ((flags & MUGGLE_FILE_WRITE) || (flags & MUGGLE_FILE_APPEND))
	{
		if (flags & MUGGLE_FILE_READ)
		{
			access_mode = O_RDWR;
		}
		else
		{
			access_mode = O_WRONLY;
		}
	}
	else if (flags & MUGGLE_FILE_READ)
	{
		access_mode = O_RDONLY;
	}
	else
	{
		MUGGLE_DEBUG_WARNING("Must Contain write or read mode\n");
		fh->fd = -1;
		return false;
	}

	// addition flags
	addition_flags =
		((flags & MUGGLE_FILE_APPEND) ? O_APPEND : 0) |
		((flags & MUGGLE_FILE_CREAT) ? O_CREAT : 0) |
		((flags & MUGGLE_FILE_EXCL) ? O_EXCL : 0) |
		((flags & MUGGLE_FILE_TRUNC) ? O_TRUNC : 0);

	// mode
	mode =
		((attr & MUGGLE_FILE_ATTR_USER_READ) ? S_IRUSR : 0) |
		((attr & MUGGLE_FILE_ATTR_USER_WRITE) ? S_IWUSR : 0) |
		((attr & MUGGLE_FILE_ATTR_USER_EXECUTE) ? S_IXUSR : 0) |
		((attr & MUGGLE_FILE_ATTR_GRP_READ) ? S_IRGRP : 0) |
		((attr & MUGGLE_FILE_ATTR_GRP_WRITE) ? S_IWGRP : 0) |
		((attr & MUGGLE_FILE_ATTR_GRP_EXECUTE) ? S_IXGRP : 0) |
		((attr & MUGGLE_FILE_ATTR_OTHER_READ) ? S_IROTH : 0) |
		((attr & MUGGLE_FILE_ATTR_OTHER_WRITE) ? S_IWOTH : 0) |
		((attr & MUGGLE_FILE_ATTR_OTHER_EXECUTE) ? S_IXOTH : 0);
	
	fh->fd = open(file_path, access_mode | addition_flags, mode);
	if (fh->fd == -1)
	{
		MUGGLE_DEBUG_WARNING("Failed open file %s - %s\n", file_path, strerror(errno));
		return false;
	}

	memcpy(fh->file_path, file_path, len);
	fh->file_path[len] = '\0';

	return true;
}
bool FileHandleClose(FileHandle *fh)
{
	MUGGLE_ASSERT_MSG(fh != NULL, "File handle is NULL\n");
	if (fh == NULL)
	{
		return false;
	}

	if (close(fh->fd) == -1)
	{
		MUGGLE_DEBUG_WARNING("Failed close file handle: %d - %s\n", fh->fd, strerror(errno));
		return false;
	}

	fh->fd = -1;
	fh->file_path[0] = '\0';

	return true;
}
long long FileHandleSeek(FileHandle *fh, long long offset, int whence)
{
	off_t new_pos;

	MUGGLE_ASSERT_MSG(fh != NULL, "File handle is NULL\n");
	if (fh == NULL)
	{
		return -1;
	}

	int w = 
		(whence == MUGGLE_FILE_SEEK_BEGIN) ? SEEK_SET :
		((whence == MUGGLE_FILE_SEEK_CURRENT) ? SEEK_CUR : SEEK_END);

	new_pos = lseek(fh->fd, (off_t)offset, w);
	if (new_pos == -1)
	{
		MUGGLE_DEBUG_WARNING("Failed file seek - %s\n", strerror(errno));
	}

	return (long long)new_pos;
}
long FileHandleWrite(FileHandle *fh, const void *buf, long cnt_bytes)
{
	ssize_t num_write;

	MUGGLE_ASSERT_MSG(fh != NULL, "File handle is NULL\n");
	if (fh == NULL)
	{
		return -1;
	}

	num_write = write(fh->fd, buf, (size_t)cnt_bytes);
	if (num_write == -1)
	{
		MUGGLE_DEBUG_WARNING("Failed write data into file - %s\n", strerror(errno));
	}

	return (long)num_write;
}
long FileHandleRead(FileHandle *fh, void *buf, long cnt_bytes)
{
	ssize_t num_read;

	MUGGLE_ASSERT_MSG(fh != NULL, "File handle is NULL\n");
	if (fh == NULL)
	{
		return -1;
	}

	num_read = read(fh->fd, buf, cnt_bytes);
	if (num_read == -1)
	{
		MUGGLE_DEBUG_WARNING("Failed read data from file - %s\n", strerror(errno));
	}

	return num_read;
}

#endif
