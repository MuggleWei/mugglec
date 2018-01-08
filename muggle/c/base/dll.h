/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_DLL_H_
#define MUGGLE_C_DLL_H_

#include "muggle/c/base/macro.h"
#include <stdbool.h>

EXTERN_C_BEGIN

MUGGLE_CC_EXPORT void* MuggleDllLoad(const char* name);
MUGGLE_CC_EXPORT void* MuggleDllQueryFunc(void* dll, const char* func_name);
MUGGLE_CC_EXPORT bool MuggleDllFree(void* dll);

EXTERN_C_END

#endif