/******************************************************************************
 *  @file         socket.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-17
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec socket
 *****************************************************************************/
 
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

int muggle_socket_send(muggle_socket_t fd, const void *buf, size_t len, int flags)
{
#if MUGGLE_PLATFORM_WINDOWS
	return send(fd, (const char*)buf, (int)len, flags);
#else
	return (int)send(fd, buf, len, flags);
#endif
}

int muggle_socket_sendto(muggle_socket_t fd, const void *buf, size_t len, int flags,
	const struct sockaddr *dest_addr, muggle_socklen_t addrlen)
{
#if MUGGLE_PLATFORM_WINDOWS
	return sendto(fd, (const char*)buf, (int)len, flags, dest_addr, addrlen);
#else
	return (int)sendto(fd, buf, len, flags, dest_addr, addrlen);
#endif
}

int muggle_socket_recv(muggle_socket_t fd, void *buf, size_t len, int flags)
{
#if MUGGLE_PLATFORM_WINDOWS
	return recv(fd, (char*)buf, (int)len, flags);
#else
	return (int)recv(fd, buf, len, flags);
#endif
}

int muggle_socket_recvfrom(muggle_socket_t fd, void *buf, size_t len, int flags,
	struct sockaddr *addr, muggle_socklen_t *addrlen)
{
#if MUGGLE_PLATFORM_WINDOWS
	return recvfrom(fd, (char*)buf, (int)len, flags, addr, addrlen);
#else
	return (int)recvfrom(fd, buf, len, flags, addr, addrlen);
#endif
}
