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

#if MUGGLE_PLATFORM_LINUX

#include <unistd.h>
#include <sys/eventfd.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>

int muggle_event_signal_init(muggle_event_signal_t *ev_signal)
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

void muggle_event_signal_destroy(muggle_event_signal_t *ev_signal)
{
	if (ev_signal->evfd != -1)
	{
		muggle_event_close(ev_signal->evfd);
		ev_signal->evfd = -1;
	}
}

int muggle_event_signal_wakeup(muggle_event_signal_t *ev_signal)
{
	uint64_t v = 1;
	if (write(ev_signal->evfd, &v, sizeof(v)) != sizeof(v))
	{
		return MUGGLE_EVENT_ERROR;
	}
	return 0;
}

int muggle_event_signal_clearup(muggle_event_signal_t *ev_signal)
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

#elif MUGGLE_PLATFORM_WINDOWS

int muggle_event_signal_init(muggle_event_signal_t *ev_signal)
{
	// TODO:
}

void muggle_event_signal_destroy(muggle_event_signal_t *ev_signal)
{
	// TODO:
}

int muggle_event_signal_wakeup(muggle_event_signal_t *ev_signal)
{
	// TODO:
}

int muggle_event_signal_clearup(muggle_event_signal_t *ev_signal)
{
	// TODO:
}

#else

static int muggle_event_signal_writen(muggle_event_fd fd, void *ptr, size_t numbytes)
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
				return -1;
			}
		}

		nleft -= n;
		p += n;
	} while (nleft > 0);

	return numbytes;
}

static int muggle_event_signal_readall(muggle_event_fd fd)
{
	uint64_t buf[128];
	int n = 0;
	int cnt = 0;
	do {
		n = read(fd, buf, sizeof(buf));
		if (n < 0)
		{
			int errnum = MUGGLE_EVENT_LAST_ERRNO;
			if (errnum == EWOULDBLOCK || errnum == EAGAIN)
			{
				break;
			}
			else if (errnum == EINTR)
			{
				continue;
			}
			else
			{
				return -1;
			}
		}
		else if (n == 0)
		{
			// EOF
			return -1;
		}

		cnt += n / sizeof(uint64_t);
		if (n < sizeof(buf))
		{
			break;
		}
	} while (1);

	return cnt;
}

int muggle_event_signal_init(muggle_event_signal_t *ev_signal)
{
	// TODO:
}

void muggle_event_signal_destroy(muggle_event_signal_t *ev_signal)
{
	// TODO:
}

int muggle_event_signal_wakeup(muggle_event_signal_t *ev_signal)
{
	// TODO:
}

int muggle_event_signal_clearup(muggle_event_signal_t *ev_signal)
{
	// TODO:
}

#endif
