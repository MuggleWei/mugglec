/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef __MUGGLE_FILE_H__
#define __MUGGLE_FILE_H__

#include "muggle/base/macro.h"
#include <stdbool.h>

EXTERN_C_BEGIN

typedef struct FileHandle_tag
{
#if MUGGLE_PLATFORM_WINDOWS
	void* fd;
#else
	int fd;
#endif
}FileHandle;

enum eFileFlags
{
	MUGGLE_FILE_WRITE	= 0x01,		// open file with write
	MUGGLE_FILE_READ	= 0x02,		// open file with read
	MUGGLE_FILE_APPEND	= 0x04,		// open file with append
	MUGGLE_FILE_CREAT	= 0x08,		// if file is not exist, create it
	MUGGLE_FILE_EXCL	= 0x10,		// ensure this call create file
	MUGGLE_FILE_TRUNC	= 0x20,		// Opens a file and truncates its size to zero bytes
};

enum eFileAttribute
{
	MUGGLE_FILE_ATTR_USER_READ		= 0x01,		// user can read
	MUGGLE_FILE_ATTR_USER_WRITE		= 0x02,		// user can write
	MUGGLE_FILE_ATTR_USER_EXECUTE	= 0x04,		// user can execute
	MUGGLE_FILE_ATTR_GRP_READ		= 0x10,		// group can read
	MUGGLE_FILE_ATTR_GRP_WRITE		= 0x20,		// group can write
	MUGGLE_FILE_ATTR_GRP_EXECUTE	= 0x40,		// group can execute
	MUGGLE_FILE_ATTR_OTHER_READ		= 0x100,	// other can read
	MUGGLE_FILE_ATTR_OTHER_WRITE	= 0x200,	// other can write
	MUGGLE_FILE_ATTR_OTHER_EXECUTE	= 0x400,	// other can execute
};

enum eFileSeekWhence
{
	MUGGLE_FILE_SEEK_BEGIN		= 0x01,
	MUGGLE_FILE_SEEK_CURRENT	= 0x02,
	MUGGLE_FILE_SEEK_END		= 0x03,
};

/*
 *  Get current process path
 *  @file_path: out path represent current process path, use MUGGLE_MAX_PATH char buffer
 */
MUGGLE_BASE_EXPORT bool FileGetAbsolutePath(const char* in_file_name, char* out_file_path);
MUGGLE_BASE_EXPORT bool FileGetDirectory(const char* file_path, char* dir);
MUGGLE_BASE_EXPORT bool FileRead(const char* file_path, char** ptr_bytes, long* ptr_num);

MUGGLE_BASE_EXPORT void FileGetProcessPath(char* file_path);
MUGGLE_BASE_EXPORT bool FileIsExist(const char* file_path);
MUGGLE_BASE_EXPORT bool FileIsAbsolutePath(const char* file_path);

MUGGLE_BASE_EXPORT bool FileHandleIsValid(FileHandle fh);
MUGGLE_BASE_EXPORT FileHandle FileHandleOpen(const char* file_path, int flags, int attr);
MUGGLE_BASE_EXPORT bool FileHandleClose(FileHandle fh);
MUGGLE_BASE_EXPORT long long FileHandleSeek(FileHandle fh, long long offset, int whence);
MUGGLE_BASE_EXPORT long FileHandleWrite(FileHandle fh, void *buf, long cnt_bytes);
MUGGLE_BASE_EXPORT long FileHandleRead(FileHandle fh, void *buf, long cnt_bytes);

EXTERN_C_END

#endif