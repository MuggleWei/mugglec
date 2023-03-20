/******************************************************************************
 *  @file         sleep.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-15
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec sleep functions
 *****************************************************************************/

#include "sleep.h"
#include "muggle/c/base/err.h"

#if MUGGLE_PLATFORM_WINDOWS
	#include <windows.h>
#else
	#include <unistd.h>
	#include <time.h>
	#include <errno.h>
#endif

int muggle_msleep(unsigned long ms)
{
#if MUGGLE_PLATFORM_WINDOWS
	Sleep(ms);
	return MUGGLE_OK;
#else

#if _POSIX_C_SOURCE >= 199309L
	struct timespec ts;

	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;

	int res = 0;
	do {
		res = nanosleep(&ts, &ts);
	} while (res && errno == EINTR);

	return 0;
#else
	return usleep(ms * 1000) == 0 ? MUGGLE_OK : MUGGLE_ERR_INTERRUPT;
#endif /* _POSIX_C_SOURCE >= 199309L */

#endif
}
