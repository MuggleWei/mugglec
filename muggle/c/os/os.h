/******************************************************************************
 *  @file         os.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-21
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec os utils
 *****************************************************************************/

#ifndef MUGGLE_C_OS_H_
#define MUGGLE_C_OS_H_

#include "muggle/c/base/macro.h"

EXTERN_C_BEGIN

/**
 * @brief get current process file path
 *
 * @param path  the returned path
 * @param size  the max size of path (include '\0')
 *
 * @return on success returns 0, otherwise return errno in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_os_process_path(char *path, unsigned int size);

/**
 * @brief get current working directory
 *
 * @param path  the returned path
 * @param size  the max size of path (include '\0')
 *
 * @return on success returns 0, otherwise return errno in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_os_curdir(char *path, unsigned int size);

/**
 * @brief change working directory
 *
 * @param path  target working directory
 *
 * @return on success returns 0, otherwise return errno in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_os_chdir(const char *path);

/**
 * @brief recursive create directory named path
 *
 * @param path: path need to create
 *
 * @return success returns 0, otherwise return errno in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_os_mkdir(const char *path);

/**
 * @brief remove the file path
 *
 * @param path  the file need to be remove
 *
 * @return success returns 0, otherwise return errno in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_os_remove(const char *path);

/**
 * @brief delete an empty directory
 *
 * @param path  directory need to be delete
 *
 * @return success returns 0, otherwise return errno in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_os_rmdir(const char *path);

/**
 * @brief rename the file or directory src to dst
 *
 * @param src  source file path
 * @param dst  destination fiel path
 *
 * @return success returns 0, otherwise return errno in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_os_rename(const char *src, const char *dst);

EXTERN_C_END

#endif
