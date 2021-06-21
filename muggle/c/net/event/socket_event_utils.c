/******************************************************************************
 *  @file         socket_event_utils.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-21
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec socket event utils
 *****************************************************************************/

#include "socket_event_utils.h"
#include "muggle/c/log/log.h"

void muggle_socket_event_on_message(muggle_socket_event_t *ev, muggle_socket_peer_t *peer)
{
	if (ev->on_message)
	{
		ev->on_message(ev, peer);
	}
	else
	{
		char buf[4096];
		int n;
		while (1)
		{
			n = muggle_socket_peer_recv(peer, buf, sizeof(buf), 0);
			if (n <= 0)
			{
				break;
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

void muggle_socket_event_accept(muggle_socket_peer_t *listen_peer, muggle_socket_peer_t *peer)
{
	while (1)
	{
		peer->addr_len = sizeof(peer->addr);
		peer->fd = accept(listen_peer->fd, (struct sockaddr*)&peer->addr, &peer->addr_len);
		if (peer->fd == MUGGLE_INVALID_SOCKET)
		{
			if (MUGGLE_SOCKET_LAST_ERRNO == MUGGLE_SYS_ERRNO_INTR)
			{
				continue;
			}
			else if (MUGGLE_SOCKET_LAST_ERRNO == MUGGLE_SYS_ERRNO_WOULDBLOCK)
			{
				break;
			}
			else
			{
				char err_msg[1024];
				muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
				MUGGLE_LOG_TRACE("failed accept - %s", err_msg);

				// close listen socket
				muggle_socket_peer_close(listen_peer);
				break;
			}
		}

		peer->ref_cnt = 1;
		peer->peer_type = MUGGLE_SOCKET_PEER_TYPE_TCP_PEER;
		peer->status = MUGGLE_SOCKET_PEER_STATUS_ACTIVE;

		// set socket nonblock
		muggle_socket_set_nonblock(peer->fd, 1);

		break;
	}
}

void muggle_socket_event_refuse_accept(muggle_socket_peer_t *listen_peer)
{
	struct sockaddr_storage addr;
	muggle_socklen_t addr_len;
	while (1)
	{
		addr_len = sizeof(addr);
		muggle_socket_t fd = accept(listen_peer->fd, (struct sockaddr*)&addr, &addr_len);
		if (fd == MUGGLE_INVALID_SOCKET)
		{
			if (MUGGLE_SOCKET_LAST_ERRNO == MUGGLE_SYS_ERRNO_INTR)
			{
				continue;
			}
			else if (MUGGLE_SOCKET_LAST_ERRNO == MUGGLE_SYS_ERRNO_WOULDBLOCK)
			{
				return;
			}

			char err_msg[1024];
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_LOG_TRACE("failed accept - %s", err_msg);

			muggle_socket_peer_close(listen_peer);
		}
		else
		{
			char straddr[MUGGLE_SOCKET_ADDR_STRLEN];
			if (muggle_socket_ntop((struct sockaddr*)&addr, straddr, sizeof(straddr), 0) == NULL)
			{
				snprintf(straddr, sizeof(straddr), "unknown:unknown");
			}
			MUGGLE_LOG_WARNING("refuse connection %s - number of connection reached the upper limit", straddr);
			muggle_socket_close(fd);
		}
	}
}
