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
#include <stdio.h>

EXTERN_C_BEGIN

enum {
	MUGGLE_FILE_TYPE_NULL = 0,
	MUGGLE_FILE_TYPE_REGULAR, //!< regular file
	MUGGLE_FILE_TYPE_DIR, //!< directory
	MUGGLE_FILE_TYPE_LINK, //!< symbol link
};

/**
 * @brief file node
 */
typedef struct {
	void *next;
	char *filepath;
} muggle_file_list_node_t;

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

/**
 * @brief open file, if dir is not exists, create dir automatically
 *
 * @param filepath  file path
 * @param mode      open file mode
 *
 * @return
 *     - on success, return FILE pointer
 *     - on failed, NULL is returned
 */
MUGGLE_C_EXPORT
FILE *muggle_os_fopen(const char *filepath, const char *mode);

/**
 * @brief list file in directory
 *
 * @param dirpath  directory path
 * @param ftype    file type filter; MUGGLE_FILE_TYPE_*
 *                 0 represent without filter
 *
 * @return file list nodes
 */
MUGGLE_C_EXPORT
muggle_file_list_node_t *muggle_os_listdir(const char *dirpath, int ftype);

/**
 * @brief free file nodes
 *
 * @param nodes  file nodes pointer
 */
MUGGLE_C_EXPORT
void muggle_os_free_file_nodes(muggle_file_list_node_t *nodes);

EXTERN_C_END

#endif
