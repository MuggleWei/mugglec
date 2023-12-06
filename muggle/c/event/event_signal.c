/******************************************************************************
 *  @file         event_signal.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-08-01
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec event signal
 *****************************************************************************/

#include "event_signal.h"

#if (MUGGLE_PLATFORM_LINUX || MUGGLE_PLATFORM_ANDROID) && MUGGLE_EVENT_SIGNAL_LINUX_USE_EVENTFD 

#include <unistd.h>
#include <sys/eventfd.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>

int muggle_ev_signal_init(muggle_event_signal_t *ev_signal)
{
	memset(ev_signal, 0, sizeof(*ev_signal));
	ev_signal->evfd = -1;

	ev_signal->evfd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
	if (ev_signal->evfd == -1)
	{
		return MUGGLE_EVENT_ERROR;
	}

	return 0;
}

void muggle_ev_signal_destroy(muggle_event_signal_t *ev_signal)
{
	if (ev_signal->evfd != -1)
	{
		muggle_ev_fd_close(ev_signal->evfd);
		ev_signal->evfd = -1;
	}
}

int muggle_ev_signal_wakeup(muggle_event_signal_t *ev_signal)
{
	uint64_t v = 1;
	if (write(ev_signal->evfd, &v, sizeof(v)) != sizeof(v))
	{
		return MUGGLE_EVENT_ERROR;
	}
	return 0;
}

int muggle_ev_signal_clearup(muggle_event_signal_t *ev_signal)
{
	uint64_t v = 0;
	ssize_t n = read(ev_signal->evfd, &v, sizeof(v));
	if (n != sizeof(v))
	{
		if (MUGGLE_EVENT_LAST_ERRNO == MUGGLE_SYS_ERRNO_WOULDBLOCK ||
			MUGGLE_EVENT_LAST_ERRNO == MUGGLE_SYS_ERROR_AGAIN)
		{
			return 0;
		}
		else
		{
			return MUGGLE_EVENT_ERROR;
		}
	}
	return (int)v;
}

muggle_event_fd muggle_ev_signal_rfd(muggle_event_signal_t *ev_signal)
{
	return ev_signal->evfd;
}

#elif MUGGLE_PLATFORM_WINDOWS

#include <stdint.h>
#include <stdlib.h>

#define MUGGLE_EVENT_SIGNAL_SOCK_READ_END 0
#define MUGGLE_EVENT_SIGNAL_SOCK_WRITE_END 1

int muggle_ev_signal_init(muggle_event_signal_t *ev_signal)
{
	memset(ev_signal, 0, sizeof(*ev_signal));
	ev_signal->socket_fds[0] = MUGGLE_INVALID_EVENT_FD;
	ev_signal->socket_fds[1] = MUGGLE_INVALID_EVENT_FD;

	// create udp read end socket
	ev_signal->socket_fds[MUGGLE_EVENT_SIGNAL_SOCK_READ_END] = socket(AF_INET, SOCK_DGRAM, 0);
	if (ev_signal->socket_fds[MUGGLE_EVENT_SIGNAL_SOCK_READ_END] == MUGGLE_INVALID_EVENT_FD)
	{
		goto event_signal_init_except;
	}
	SOCKET read_end = ev_signal->socket_fds[MUGGLE_EVENT_SIGNAL_SOCK_READ_END];

	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &sin.sin_addr);
	sin.sin_port = 0;

	int ret = bind(read_end, (struct sockaddr*)&sin, sizeof(sin));
	if (ret == SOCKET_ERROR)
	{
		goto event_signal_init_except;
	}

	if (muggle_ev_fd_set_nonblock(read_end, 1) != 0)
	{
		goto event_signal_init_except;
	}

	// create udp write end socket
	ev_signal->socket_fds[MUGGLE_EVENT_SIGNAL_SOCK_WRITE_END] = socket(AF_INET, SOCK_DGRAM, 0);
	if (ev_signal->socket_fds[MUGGLE_EVENT_SIGNAL_SOCK_WRITE_END] == MUGGLE_INVALID_EVENT_FD)
	{
		goto event_signal_init_except;
	}
	SOCKET write_end = ev_signal->socket_fds[MUGGLE_EVENT_SIGNAL_SOCK_WRITE_END];

	memset(&sin, 0, sizeof(sin));
	int addrlen = sizeof(sin);
	ret = getsockname(read_end, (struct sockaddr *)&sin, &addrlen);
	if (ret != 0)
	{
		goto event_signal_init_except;
	}

	ret = connect(write_end, (struct sockaddr*)&sin, sizeof(sin));
	if (ret == SOCKET_ERROR)
	{
		goto event_signal_init_except;
	}

	if (muggle_ev_fd_set_nonblock(write_end, 1) != 0)
	{
		goto event_signal_init_except;
	}

	// init mutex
	ev_signal->mtx = (muggle_mutex_t*)malloc(sizeof(muggle_mutex_t));
	if (ev_signal->mtx == NULL)
	{
		goto event_signal_init_except;
	}

	if (muggle_mutex_init(ev_signal->mtx) != 0)
	{
		free(ev_signal->mtx);
		ev_signal->mtx = NULL;
		goto event_signal_init_except;
	}

	return 0;

event_signal_init_except:
	muggle_ev_signal_destroy(ev_signal);
	return MUGGLE_EVENT_ERROR;
}

void muggle_ev_signal_destroy(muggle_event_signal_t *ev_signal)
{
	if (ev_signal->mtx)
	{
		muggle_mutex_destroy(ev_signal->mtx);
		free(ev_signal->mtx);
		ev_signal->mtx = NULL;
	}

	if (ev_signal->socket_fds[0] != INVALID_SOCKET)
	{
		closesocket(ev_signal->socket_fds[0]);
		ev_signal->socket_fds[0] = INVALID_SOCKET;
	}

	if (ev_signal->socket_fds[1] != INVALID_SOCKET)
	{
		closesocket(ev_signal->socket_fds[1]);
		ev_signal->socket_fds[1] = INVALID_SOCKET;
	}
}

int muggle_ev_signal_wakeup(muggle_event_signal_t *ev_signal)
{
	uint64_t v = 0;
	muggle_mutex_lock(ev_signal->mtx);
	int n = send(ev_signal->socket_fds[MUGGLE_EVENT_SIGNAL_SOCK_WRITE_END], (const char*)&v, sizeof(v), 0);
	muggle_mutex_unlock(ev_signal->mtx);

	return n == sizeof(v) ? 0 : MUGGLE_EVENT_ERROR;
}

int muggle_ev_signal_clearup(muggle_event_signal_t *ev_signal)
{
	// NOTE: cause use UDP, unlike pipe, this use a uint64_t variable replace uint64_t array
	uint64_t v;
	int n = 0;
	int cnt = 0;
	do {
		n = recv(ev_signal->socket_fds[MUGGLE_EVENT_SIGNAL_SOCK_READ_END], (char*)&v, sizeof(v), 0);
		if (n == SOCKET_ERROR)
		{
			int errnum = MUGGLE_EVENT_LAST_ERRNO;
			if (MUGGLE_EVENT_LAST_ERRNO == MUGGLE_SYS_ERRNO_WOULDBLOCK ||
				MUGGLE_EVENT_LAST_ERRNO == MUGGLE_SYS_ERROR_AGAIN)
			{
				break;
			}
			else if (MUGGLE_EVENT_LAST_ERRNO == MUGGLE_SYS_ERRNO_INTR)
			{
				continue;
			}
			else
			{
				return MUGGLE_EVENT_ERROR;
			}
		}

		cnt += 1;
		if (n < (int)sizeof(v))
		{
			break;
		}
	} while(1);

	return cnt;
}

muggle_event_fd muggle_ev_signal_rfd(muggle_event_signal_t *ev_signal)
{
	return ev_signal->socket_fds[MUGGLE_EVENT_SIGNAL_SOCK_READ_END];
}

#else

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>

#define MUGGLE_EVENT_SIGNAL_PIPE_READ_END 0
#define MUGGLE_EVENT_SIGNAL_PIPE_WRITE_END 1

static int muggle_ev_signal_writen(muggle_event_fd fd, void *ptr, size_t numbytes)
{
	size_t nleft = numbytes;
	const char *p = (const char*)ptr;
	long n = 0;
	do {
		n = write(fd, p, nleft);
		if (n < 0)
		{
			if (n < 0 && MUGGLE_EVENT_LAST_ERRNO == MUGGLE_SYS_ERRNO_INTR)
			{
				continue;
			}
			else
			{
				return MUGGLE_EVENT_ERROR;
			}
		}

		nleft -= n;
		p += n;
	} while (nleft > 0);

	return numbytes;
}

static int muggle_ev_signal_readall(muggle_event_fd fd)
{
	uint64_t buf[128];
	int n = 0;
	int cnt = 0;
	do {
		n = read(fd, buf, sizeof(buf));
		if (n < 0)
		{
			if (MUGGLE_EVENT_LAST_ERRNO == MUGGLE_SYS_ERRNO_WOULDBLOCK ||
				MUGGLE_EVENT_LAST_ERRNO == MUGGLE_SYS_ERROR_AGAIN)
			{
				break;
			}
			else if (MUGGLE_EVENT_LAST_ERRNO == MUGGLE_SYS_ERRNO_INTR)
			{
				continue;
			}
			else
			{
				return MUGGLE_EVENT_ERROR;
			}
		}
		else if (n == 0)
		{
			// EOF
			return MUGGLE_EVENT_ERROR;
		}

		cnt += n;
		if (n < (int)sizeof(buf))
		{
			break;
		}
	} while (1);

	return (int)(cnt / sizeof(uint64_t));
}

int muggle_ev_signal_init(muggle_event_signal_t *ev_signal)
{
	memset(ev_signal, 0, sizeof(*ev_signal));
	ev_signal->pipe_fds[0] = -1;
	ev_signal->pipe_fds[1] = -1;

	if (pipe(ev_signal->pipe_fds) == -1)
	{
		goto event_signal_init_except;
	}

	if (muggle_ev_fd_set_nonblock(ev_signal->pipe_fds[0], 1) != 0)
	{
		goto event_signal_init_except;
	}

	if (muggle_ev_fd_set_nonblock(ev_signal->pipe_fds[1], 1) != 0)
	{
		goto event_signal_init_except;
	}

	ev_signal->mtx = (muggle_mutex_t*)malloc(sizeof(muggle_mutex_t));
	if (ev_signal->mtx == NULL)
	{
		goto event_signal_init_except;
	}

	if (muggle_mutex_init(ev_signal->mtx) != 0)
	{
		free(ev_signal->mtx);
		ev_signal->mtx = NULL;
		goto event_signal_init_except;
	}

	return 0;

event_signal_init_except:
	muggle_ev_signal_destroy(ev_signal);
	return MUGGLE_EVENT_ERROR;
}

void muggle_ev_signal_destroy(muggle_event_signal_t *ev_signal)
{
	if (ev_signal->mtx)
	{
		muggle_mutex_destroy(ev_signal->mtx);
		free(ev_signal->mtx);
		ev_signal->mtx = NULL;
	}

	if (ev_signal->pipe_fds[0] != -1)
	{
		close(ev_signal->pipe_fds[0]);
		ev_signal->pipe_fds[0] = -1;
	}

	if (ev_signal->pipe_fds[1] != -1)
	{
		close(ev_signal->pipe_fds[1]);
		ev_signal->pipe_fds[1] = -1;
	}
}

int muggle_ev_signal_wakeup(muggle_event_signal_t *ev_signal)
{
	uint64_t v = 0;
	muggle_mutex_lock(ev_signal->mtx);
	int n = muggle_ev_signal_writen(
		ev_signal->pipe_fds[MUGGLE_EVENT_SIGNAL_PIPE_WRITE_END],
		&v, sizeof(v));
	muggle_mutex_unlock(ev_signal->mtx);

	return n == sizeof(v) ? 0 : MUGGLE_EVENT_ERROR;
}

int muggle_ev_signal_clearup(muggle_event_signal_t *ev_signal)
{
	return muggle_ev_signal_readall(ev_signal->pipe_fds[MUGGLE_EVENT_SIGNAL_PIPE_READ_END]);
}

muggle_event_fd muggle_ev_signal_rfd(muggle_event_signal_t *ev_signal)
{
	return ev_signal->pipe_fds[MUGGLE_EVENT_SIGNAL_PIPE_READ_END];
}

#endif
