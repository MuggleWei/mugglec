/******************************************************************************
 *  @file         event_fd.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-08-06
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec event fd
 *****************************************************************************/

#include "event_fd.h"

#if MUGGLE_PLATFORM_WINDOWS
#else
#include <unistd.h>
#include <fcntl.h>
#endif

int muggle_ev_fd_close(muggle_event_fd fd)
{
#if MUGGLE_PLATFORM_WINDOWS
	return closesocket(fd);
#else
	return close(fd);
#endif
}

int muggle_ev_fd_shutdown(muggle_event_fd fd, int how)
{
	return shutdown(fd, how);
}

int muggle_ev_fd_set_nonblock(muggle_event_fd fd, int on)
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

int muggle_ev_fd_recv(muggle_event_fd fd, void *buf, size_t len, int flags)
{
#if MUGGLE_PLATFORM_WINDOWS
	return recv(fd, (char*)buf, (int)len, flags);
#else
	return (int)recv(fd, buf, len, flags);
#endif
}

int muggle_ev_fd_send(muggle_event_fd fd, const void *buf, size_t len, int flags)
{
#if MUGGLE_PLATFORM_WINDOWS
	return send(fd, (const char*)buf, (int)len, flags);
#else
	return (int)send(fd, buf, len, flags);
#endif
}
