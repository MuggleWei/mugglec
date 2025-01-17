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

int muggle_nsleep(uint64_t ns)
{
#if MUGGLE_PLATFORM_WINDOWS
	HANDLE timer;
	LARGE_INTEGER ft;

	LONGLONG hundred_ns = (LONGLONG)(ns / 100);
	if (hundred_ns == 0) {
		hundred_ns = 1;
	}

	// NOTE:
	//   * negative value represent relative
	//   * time interval unit is 100 ns
	ft.QuadPart = -hundred_ns;

	timer = CreateWaitableTimer(NULL, TRUE, NULL);
	SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);

	return 0;
#else
	#if _POSIX_C_SOURCE >= 199309L || MUGGLE_PLATFORM_APPLE || \
		MUGGLE_PLATFORM_ANDROID
	struct timespec ts = {
		.tv_sec = ns / 1000000000ll,
		.tv_nsec = ns % 1000000000ll
	};

	int res = 0;
	do {
		res = nanosleep(&ts, &ts);
	} while (res && errno == EINTR);

	return 0;
	#else
	static_assert(0, "_POSIX_C_SOURCE >= 199309L is required");
	return MUGGLE_ERR_SYS_CALL;
	#endif /* _POSIX_C_SOURCE >= 199309L */
#endif
}
