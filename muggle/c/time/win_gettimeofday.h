/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_GETTIMEOFDAY_H_
#define MUGGLE_C_GETTIMEOFDAY_H_

#include "muggle/c/base/macro.h"
#include <time.h>

EXTERN_C_BEGIN

#if MUGGLE_PLATFORM_WINDOWS

struct timezone
{
	int  tz_minuteswest; /* minutes W of Greenwich */
	int  tz_dsttime;     /* type of dst correction */
};

MUGGLE_CC_EXPORT
int gettimeofday(struct timeval *tv, struct timezone *tz);

#endif

EXTERN_C_END

#endif
