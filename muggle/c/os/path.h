/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_PATH_H_
#define MUGGLE_C_PATH_H_

#include "muggle/c/base/macro.h"

EXTERN_C_BEGIN

/*
 * return a normalized absolutized version of the path
 * @path: the original path
 * @ret: normalized absolutized version of path
 * @size: the max size of returned path (include '\0')
 * RETURN: on success returns 0, otherwise return errno in err.h
 * */
MUGGLE_CC_EXPORT 
int muggle_path_abspath(const char *path, char *ret, unsigned int size);

/*
 * return the base name of pathname path
 * @path: the original path
 * @ret: the base name
 * @size: the max size of returned basename (include '\0')
 * RETURN: on success returns 0, otherwise return errno in err.h
 * */
MUGGLE_CC_EXPORT 
int muggle_path_basename(const char *path, char *ret, unsigned int size);

/*
 * return the dirname of pathname path
 * @path: the original path
 * @ret: the dirname
 * @size: the max size of returned dirname (include '\0')
 * RETURN: on success returns 0, otherwise return errno in err.h
 * */
MUGGLE_CC_EXPORT 
int muggle_path_dirname(const char *path, char *ret, unsigned int size);

/*
 * check whether the path is absolutized version of path
 * RETURN: on success returns nonzero, otherwise returns zero
 * */
MUGGLE_CC_EXPORT 
int muggle_path_isabs(const char *path);

/*
 * check whether the path exists
 * RETURN: on success returns nonzero, otherwise returns zero
 * */
MUGGLE_CC_EXPORT
int muggle_path_exists(const char *path);

/*
 * join two path
 * @path1: path 1
 * @path2: path 2
 * @ret: the joined path
 * @size: the max size of returned path (include '\0')
 * RETURN: on success returns 0, otherwise return errno in err.h
 * */
MUGGLE_CC_EXPORT
int muggle_path_join(const char *path1, const char *path2, char *ret, unsigned int size);

/*
 * normalize a pathname
 * @path: original path
 * @ret: normalized path
 * @size: the max size of returned path (include '\0')
 * RETURN: on success returns 0, otherwise return errno in err.h
 * */
MUGGLE_CC_EXPORT
int muggle_path_normpath(const char *path, char *ret, unsigned int size);

EXTERN_C_END

#endif
