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
#include <sys/socket.h>
#include <fcntl.h>
#endif

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
