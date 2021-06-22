/******************************************************************************
 *  @file         win_gettimeofday.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-22
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec gettimeofday in windows
 *****************************************************************************/

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

MUGGLE_C_EXPORT
int gettimeofday(struct timeval *tv, struct timezone *tz);

#endif

EXTERN_C_END

#endif
