/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_FILE_H_
#define MUGGLE_C_FILE_H_

#include "muggle/c/base/macro.h"
#include <stdbool.h>

EXTERN_C_BEGIN

typedef struct MuggleFile_tag
{
#if MUGGLE_PLATFORM_WINDOWS
	void* fd;
	char file_path[MUGGLE_MAX_PATH];
#else
	int fd;
	char file_path[MUGGLE_MAX_PATH];
#endif
}MuggleFile;

enum eFileFlags
{
	MUGGLE_FILE_WRITE	= 0x01,		// open file with write
	MUGGLE_FILE_READ	= 0x02,		// open file with read
	MUGGLE_FILE_APPEND	= 0x04,		// open file with append
	MUGGLE_FILE_CREAT	= 0x08,		// if file is not exist, create it
	MUGGLE_FILE_EXCL	= 0x10,		// ensure this call create file
	MUGGLE_FILE_TRUNC	= 0x20,		// Opens a file and truncates its size to zero bytes
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
MUGGLE_CC_EXPORT bool MuggleGetAbsolutePath(const char* in_file_name, char* out_file_path);
MUGGLE_CC_EXPORT bool MuggleGetDirectory(const char* file_path, char* dir);
MUGGLE_CC_EXPORT bool MuggleReadFromFile(const char* file_path, char** ptr_bytes, long* ptr_num);

MUGGLE_CC_EXPORT void MuggleGetProcessPath(char* file_path);
MUGGLE_CC_EXPORT bool MuggleIsFileExist(const char* file_path);
MUGGLE_CC_EXPORT bool MuggleIsAbsolutePath(const char* file_path);
MUGGLE_CC_EXPORT bool MuggleDeleteFile(const char *file_path);

MUGGLE_CC_EXPORT bool MuggleFileIsValid(MuggleFile *fh);
MUGGLE_CC_EXPORT bool MuggleFileOpen(MuggleFile *fh, const char* file_path, int flags, int attr);
MUGGLE_CC_EXPORT bool MuggleFileClose(MuggleFile *fh);
MUGGLE_CC_EXPORT long long MuggleFileSeek(MuggleFile *fh, long long offset, int whence);
MUGGLE_CC_EXPORT long MuggleFileWrite(MuggleFile *fh, const void *buf, long cnt_bytes);
MUGGLE_CC_EXPORT long MuggleFileRead(MuggleFile *fh, void *buf, long cnt_bytes);

EXTERN_C_END

#endif