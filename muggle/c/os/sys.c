/******************************************************************************
 *  @file         sys.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-06-30
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec sys
 *****************************************************************************/

#include "err.h"
#include <errno.h>
#include <string.h>

#if MUGGLE_PLATFORM_WINDOWS
#include <windows.h>
#endif

int muggle_sys_lasterror()
{
#if MUGGLE_PLATFORM_WINDOWS
	return GetLastError();
#else
	return errno;
#endif
}

int muggle_sys_strerror(int errnum, char *buf, size_t bufsize)
{
	memset(buf, 0, bufsize);
#if MUGGLE_PLATFORM_WINDOWS
	return strerror_s(buf, bufsize, errnum);
#else
	return strerror_r(errnum, buf, bufsize);
#endif
}
