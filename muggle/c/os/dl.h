/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_DL_H_
#define MUGGLE_C_DL_H_

#include "muggle/c/base/macro.h"

EXTERN_C_BEGIN

MUGGLE_CC_EXPORT
void* muggle_dl_load(const char* name);

MUGGLE_CC_EXPORT
void* muggle_dl_sym(void *handle, const char *symbol);

MUGGLE_CC_EXPORT
int muggle_dl_close(void *handle);

EXTERN_C_END

#endif
