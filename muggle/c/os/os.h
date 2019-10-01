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

MUGGLE_CC_EXPORT
int muggle_os_process_path(char *path, unsigned int size);

MUGGLE_CC_EXPORT
int muggle_os_curdir(char *path, unsigned int size);

MUGGLE_CC_EXPORT
int muggle_os_chdir(const char *path);

EXTERN_C_END

#endif
