#include "socket_event_utils.h"

static inline void muggle_socket_event_on_message_error(muggle_socket_event_t *ev, muggle_socket_peer_t *peer)
{
	if (ev->on_error)
	{
		if (ev->on_error(ev, peer) == 0)
		{
			muggle_socket_close(peer->fd);
		}
	}
	else
	{
		muggle_socket_close(peer->fd);
	}
}

int muggle_socket_event_on_message(muggle_socket_event_t *ev, muggle_socket_peer_t *peer)
{
	if (ev->on_message)
	{
		int ret = ev->on_message(ev, peer);
		if (ret == 0)
		{
			return 0;
		}
		else
		{
			if (ret == -1)
			{
				muggle_socket_event_on_message_error(ev, peer);
			}
			return 1;
		}
	}
	else
	{
		char buf[4096];
		int n;
		while (1)
		{
			n = recv(peer->fd, buf, sizeof(buf), 0);
			if (n > 0)
			{
				continue;
			}
			else if (n < 0)
			{
				if (MUGGLE_SOCKET_LAST_ERRNO == MUGGLE_SYS_ERRNO_INTR)
				{
					continue;
				}
				else if (MUGGLE_SOCKET_LAST_ERRNO == MUGGLE_SYS_ERRNO_WOULDBLOCK)
				{
					break;
				}

				muggle_socket_event_on_message_error(ev, peer);
				return 1;
			}
			else
			{
				muggle_socket_event_on_message_error(ev, peer);
				return 1;
			}
		}
	}

	return 0;
}

void muggle_socket_event_timer_handle(muggle_socket_event_t *ev, struct timespec *t1, struct timespec *t2)
{
	timespec_get(t2, TIME_UTC);
	int interval_ms =
		(int)((t2->tv_sec - t1->tv_sec) * 1000 +
		(t2->tv_nsec - t1->tv_nsec) / 1000000);
	if (interval_ms >= ev->timeout_ms)
	{
		if (ev->on_timer)
		{
			ev->on_timer(ev);
		}
		t1->tv_sec = t2->tv_sec;
		t1->tv_nsec = t2->tv_nsec;
	}
}
