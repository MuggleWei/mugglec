#include "socket_event_utils.h"
#include "muggle/c/log/log.h"

static inline void muggle_socket_event_check_error(muggle_socket_event_t *ev, muggle_socket_peer_t *peer)
{
	if (peer->ref_cnt == 0)
	{
		if (ev->on_error)
		{
			ev->on_error(ev, peer);
		}
		peer->status = MUGGLE_SOCKET_PEER_STATUS_CLOSED;
	}
}

void muggle_socket_event_on_message(muggle_socket_event_t *ev, muggle_socket_peer_t *peer, int handle_error)
{
	if (ev->on_message)
	{
		ev->on_message(ev, peer);

		if (handle_error)
		{
			muggle_socket_event_check_error(ev, peer);
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

				muggle_socket_peer_close(peer);
				if (handle_error)
				{
					muggle_socket_event_check_error(ev, peer);
				}
			}
			else
			{
				muggle_socket_peer_close(peer);
				if (handle_error)
				{
					muggle_socket_event_check_error(ev, peer);
				}
			}
		}
	}
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

int muggle_socket_event_accept(muggle_socket_event_t *ev, muggle_socket_peer_t *listen_peer, muggle_socket_peer_t *peer)
{
	peer->ref_cnt = 1;
	peer->addr_len = sizeof(peer->addr);
	peer->fd = accept(listen_peer->fd, (struct sockaddr*)&peer->addr, &peer->addr_len);
	if (peer->fd == MUGGLE_INVALID_SOCKET)
	{
		if (MUGGLE_SOCKET_LAST_ERRNO == MUGGLE_SYS_ERRNO_INTR)
		{
			return MUGGLE_SOCKET_EVENT_ACCEPT_RET_INTR;
		}
		else if (MUGGLE_SOCKET_LAST_ERRNO == MUGGLE_SYS_ERRNO_WOULDBLOCK)
		{
			return MUGGLE_SOCKET_EVENT_ACCEPT_RET_WBLOCK;
		}

		char err_msg[1024];
		muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
		MUGGLE_LOG_TRACE("failed accept - %s", err_msg);

		// close listen socket
		muggle_socket_peer_close(listen_peer);
		if (ev->on_error != NULL)
		{
			ev->on_error(ev, listen_peer);
		}
		listen_peer->status = MUGGLE_SOCKET_PEER_STATUS_CLOSED;

		return MUGGLE_SOCKET_EVENT_ACCEPT_RET_CLOSED;
	}

	return MUGGLE_SOCKET_EVENT_ACCEPT_RET_PEER;
}