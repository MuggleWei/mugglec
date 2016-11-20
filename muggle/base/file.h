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

/*
 *  Get current process path
 *  @file_path: out path represent current process path, use MG_MAX_PATH char buffer
 */
MUGGLE_BASE_EXPORT void FileGetProcessPath(char* file_path);
MUGGLE_BASE_EXPORT bool FileIsExist(const char* file_path);
MUGGLE_BASE_EXPORT bool FileIsAbsolutePath(const char* file_path);

MUGGLE_BASE_EXPORT bool FileGetAbsolutePath(const char* in_file_name, char* out_file_path);
MUGGLE_BASE_EXPORT bool FileGetDirectory(const char* file_path, char* dir);
MUGGLE_BASE_EXPORT bool FileRead(const char* file_path, char** ptr_bytes, long* ptr_num);

MUGGLE_BASE_EXPORT void* FileGetHandle(const char* file_path, const char* mode);
MUGGLE_BASE_EXPORT void FileCloseHandle(void* file_handle);

EXTERN_C_END

#endif