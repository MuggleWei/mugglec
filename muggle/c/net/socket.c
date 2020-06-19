/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "socket.h"
#include <string.h>

int muggle_socket_lib_init()
{
#if MUGGLE_PLATFORM_WINDOWS
	WSADATA wsaData;
	return WSAStartup(MAKEWORD(2, 2), &wsaData);
#else
	// ignore PIPE
	signal(SIGPIPE, SIG_IGN);
	return 0;
#endif
}

muggle_socket_t muggle_socket_create(int family, int type, int protocol)
{
	return socket(family, type, protocol);
}

int muggle_socket_close(muggle_socket_t fd)
{
#if MUGGLE_PLATFORM_WINDOWS
	return closesocket(fd);
#else
	return close(fd);
#endif
}

int muggle_socket_shutdown(muggle_socket_t fd, int how)
{
	return shutdown(fd, how);
}

int muggle_socket_strerror(int errnum, char *buf, size_t bufsize)
{
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

int muggle_socket_set_nonblock(muggle_socket_t socket, int on)
{
#if MUGGLE_PLATFORM_WINDOWS
	u_long iMode = (u_long)on;

	// If iMode = 0, blocking is enabled;
	// If iMode != 0, non-blocking mode is enabled.
	return ioctlsocket(socket, FIONBIO, &iMode);
#else
	int flags = 0;
	flags = fcntl(socket, F_GETFL, 0);
	if (on)
	{
		flags |= O_NONBLOCK;
	}
	else
	{
		flags &= ~O_NONBLOCK;
	}
	return fcntl(socket, F_SETFL, flags);
#endif
}
