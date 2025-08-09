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
	/*
	 * NOTE:
	 *   On GCC compiler, this function may do nothing, cause error message not 
	 *   fillup the buf param, it return from strerror_r.
	 *
	 *   The return type of strerror_r may be different on different platform, 
	 *   although it can be determined by `(_POSIX_C_SOURCE >= 200112L) && !_GNU_SOURCE`
	 *   on gcc, it's still difficult to determine the return type when 
	 *   considering all different system.
	 *
	 *   So here neigher the GNU-specific nor the XSI-compliant handle is used, 
	 *   but return value of strerror_r is ignored here.
	 *
	 * # handle GNU-specific
	 * const char *s = strerror_r(errnum, buf, bufsize);
	 * if ((buf[0] == '\0') && s) {
	 *     strncpy(buf, s, bufsize - 1);
	 * }
	 * return buf[0] != '\0' ? 0 : -1;
	 *
	 * # handle XSI-compliant
	 * return strerror_r(errnum, buf, bufsize);
	 * */
	strerror_r(errnum, buf, bufsize);
	return 0;
#endif
}
