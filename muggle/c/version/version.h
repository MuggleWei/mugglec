/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_VERSION_H_
#define MUGGLE_C_VERSION_H_

#include "muggle/c/base/macro.h"

EXTERN_C_BEGIN

MUGGLE_C_EXPORT
const char* mugglec_version();

MUGGLE_C_EXPORT
const char* mugglec_compile_time();

EXTERN_C_END

#endif
