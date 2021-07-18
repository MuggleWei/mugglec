/******************************************************************************
 *  @file         win_gmtime.cpp
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-22
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec gmtime_r and timegm in windows
 *****************************************************************************/

#include "win_gmtime.h"

#if MUGGLE_PLATFORM_WINDOWS

struct tm* gmtime_r(const time_t *timep, struct tm *result)
{
	errno_t err = gmtime_s(result, timep);
	if (err == 0)
	{
		return result;
	}

	return nullptr;
}

struct tm* localtime_r(const time_t *timep, struct tm *result)
{
	errno_t err = localtime_s(result, timep);
	if (err == 0)
	{
		return result;
	}

	return nullptr;
}

time_t timegm(struct tm *p_tm)
{
	return _mkgmtime(p_tm);
}

#endif
