/******************************************************************************
 *  @file         event.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-08-01
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec event
 *****************************************************************************/

#include "event.h"

#include <string.h>
#if MUGGLE_PLATFORM_WINDOWS
#else
#include <signal.h>
#endif

int muggle_event_lib_init()
{
	static int inited = 0;
	if (inited)
	{
		return 0;
	}

	inited = 1;
#if MUGGLE_PLATFORM_WINDOWS
	WSADATA wsaData;
	return WSAStartup(MAKEWORD(2, 2), &wsaData);
#else
	// ignore PIPE
	signal(SIGPIPE, SIG_IGN);
	return 0;
#endif
}

int muggle_event_lasterror()
{
#if MUGGLE_PLATFORM_WINDOWS
	return WSAGetLastError();
#else
	return errno;
#endif
}

int muggle_event_strerror(int errnum, char *buf, size_t bufsize)
{
	memset(buf, 0, bufsize);
#if MUGGLE_PLATFORM_WINDOWS
	DWORD ret = FormatMessageA(
			FORMAT_MESSAGE_FROM_SYSTEM, NULL, errnum,
			MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
			buf, (DWORD)bufsize, 0);

	return ret > 0 ? 0 : -1;
#else
	// return strerror_r(errnum, buf, bufsize);
	strerror_r(errnum, buf, bufsize);
	return 0;
#endif
}
