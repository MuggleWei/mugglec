/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_OS_H_
#define MUGGLE_C_OS_H_

#include "muggle/c/base/macro.h"

EXTERN_C_BEGIN

/*
 * get current process file path
 * @path: the returned path
 * @size: the max size of path (include '\0')
 * RETURN: on success returns 0, otherwise return errno in err.h
 * */
MUGGLE_CC_EXPORT
int muggle_os_process_path(char *path, unsigned int size);

/*
 * get current working directory
 * @path: the returned path
 * @size: the max size of path (include '\0')
 * RETURN: on success returns 0, otherwise return errno in err.h
 * */
MUGGLE_CC_EXPORT
int muggle_os_curdir(char *path, unsigned int size);

/*
 * change working directory
 * @path: target working directory
 * RETURN: on success returns 0, otherwise return errno in err.h
 * */
MUGGLE_CC_EXPORT
int muggle_os_chdir(const char *path);

/*
 * recursive create directory named path
 * @path: path need to create
 * RETURN: success returns 0, otherwise return errno in err.h
 * */
MUGGLE_CC_EXPORT
int muggle_os_mkdir(const char *path);

/*
 * remove the file path
 * @path: the file need to be remove
 * RETURN: success returns 0, otherwise return errno in err.h
 * */
MUGGLE_CC_EXPORT
int muggle_os_remove(const char *path);

/*
 * delete an empty directory
 * @path: directory need to be delete
 * RETURN: success returns 0, otherwise return errno in err.h
 * */
MUGGLE_CC_EXPORT
int muggle_os_rmdir(const char *path);

/*
 * rename the file or directory src to dst
 * RETURN: success returns 0, otherwise return errno in err.h
 * */
MUGGLE_CC_EXPORT
int muggle_os_rename(const char *src, const char *dst);

EXTERN_C_END

#endif