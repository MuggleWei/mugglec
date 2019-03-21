#ifndef LATENCY_COMMON_WIN_GETTIMEOFDAY_H_
#define LATENCY_COMMON_WIN_GETTIMEOFDAY_H_

#include "muggle/c/base/macro.h"
#include <time.h>

#if MUGGLE_PLATFORM_WINDOWS

EXTERN_C_BEGIN

int gettimeofday(struct timeval *tv, struct timezone *tz);


EXTERN_C_END

#endif

#endif