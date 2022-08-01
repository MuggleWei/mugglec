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
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
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

int muggle_event_close(muggle_event_fd fd)
{
#if MUGGLE_PLATFORM_WINDOWS
	return closesocket(fd);
#else
	return close(fd);
#endif
}

int muggle_event_shutdown(muggle_event_fd fd, int how)
{
	return shutdown(fd, how);
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
	return strerror_r(errnum, buf, bufsize);
#endif
}

int muggle_event_set_nonblock(muggle_event_fd fd, int on)
{
#if MUGGLE_PLATFORM_WINDOWS
	u_long iMode = (u_long)on;

	// If iMode = 0, blocking is enabled;
	// If iMode != 0, non-blocking mode is enabled.
	return ioctlsocket(fd, FIONBIO, &iMode);
#else
	int flags = 0;
	flags = fcntl(fd, F_GETFL, 0);
	if (on)
	{
		flags |= O_NONBLOCK;
	}
	else
	{
		flags &= ~O_NONBLOCK;
	}
	return fcntl(fd, F_SETFL, flags);
#endif
}
