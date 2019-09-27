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

MUGGLE_CC_EXPORT 
void muggle_path_abspath(const char *path, char *ret, unsigned int size);

MUGGLE_CC_EXPORT 
int muggle_path_dirname(const char *path, char *ret, unsigned int size);

MUGGLE_CC_EXPORT 
int muggle_path_isabs(const char *path);

// TODO:

EXTERN_C_END

#endif
