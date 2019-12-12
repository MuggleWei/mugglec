/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_GMTIME_H_
#define MUGGLE_C_GMTIME_H_

#include "muggle/c/base/macro.h"
#include <time.h>

EXTERN_C_BEGIN

#if MUGGLE_PLATFORM_WINDOWS

MUGGLE_CC_EXPORT
struct tm* gmtime_r(const time_t *timep, struct tm *result);

#endif

EXTERN_C_END


#endif