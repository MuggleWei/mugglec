/******************************************************************************
 *  @file         win_gmtime.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-22
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec gmtime_r and timegm in windows
 *****************************************************************************/

#ifndef MUGGLE_C_GMTIME_H_
#define MUGGLE_C_GMTIME_H_

#include "muggle/c/base/macro.h"
#include <time.h>

EXTERN_C_BEGIN

#if MUGGLE_PLATFORM_WINDOWS

MUGGLE_C_EXPORT
struct tm* gmtime_r(const time_t *timep, struct tm *result);

MUGGLE_C_EXPORT
struct tm* localtime_r(const time_t *timep, struct tm *result);

MUGGLE_C_EXPORT
time_t timegm(struct tm *p_tm);

#endif

EXTERN_C_END


#endif
