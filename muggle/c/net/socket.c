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
	return muggle_event_lib_init();
}

muggle_socket_t muggle_socket_create(int family, int type, int protocol)
{
	return socket(family, type, protocol);
}

int muggle_socket_close(muggle_socket_t fd)
{
	return muggle_event_close(fd);
}

int muggle_socket_shutdown(muggle_socket_t fd, int how)
{
	return shutdown(fd, how);
}

int muggle_socket_lasterror()
{
	return muggle_event_lasterror();
}

int muggle_socket_strerror(int errnum, char *buf, size_t bufsize)
{
	return muggle_event_strerror(errnum, buf, bufsize);
}

int muggle_setsockopt(
	muggle_socket_t socket, int level, int optname,
	const void *optval, muggle_socklen_t optlen)
{
#if MUGGLE_PLATFORM_WINDOWS
	return setsockopt(socket, level, optname, (const char*)optval, optlen);
#else
	return setsockopt(socket, level, optname, optval, optlen);
#endif
}

int muggle_getsockopt(
	muggle_socket_t socket, int level, int optname,
	void *optval, muggle_socklen_t *optlen)
{
#if MUGGLE_PLATFORM_WINDOWS
	return getsockopt(socket, level, optname, (const char*)optval, optlen);
#else
	return getsockopt(socket, level, optname, optval, optlen);
#endif
}

int muggle_socket_set_nonblock(muggle_socket_t socket, int on)
{
	return muggle_event_set_nonblock(socket, on);
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
