/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_CC_VERSION_H_
#define MUGGLE_CC_VERSION_H_

#include "muggle/c/base/macro.h"

EXTERN_C_BEGIN

MUGGLE_CC_EXPORT
void mugglecc_version(char *buf, unsigned int bufsize);

MUGGLE_CC_EXPORT
void mugglecc_compile_time(char *buf, unsigned int bufsize);

EXTERN_C_END

#endif
