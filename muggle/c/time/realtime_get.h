/******************************************************************************
 *  @file         realtime_get.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2023-12-05
 *  @copyright    Copyright 2023 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec realtime_get
 *****************************************************************************/

#ifndef MUGGLE_C_REALTIME_GET_H_
#define MUGGLE_C_REALTIME_GET_H_

#include "muggle/c/base/macro.h"
#include <time.h>

EXTERN_C_BEGIN

#if MUGGLE_PLATFORM_ANDROID 
	#define muggle_realtime_get(ts) clock_gettime(CLOCK_REALTIME, &ts)
#else
	#define muggle_realtime_get(ts) timespec_get(&ts, TIME_UTC);
#endif

EXTERN_C_END

#endif // !MUGGLE_C_REALTIME_GET_H_
