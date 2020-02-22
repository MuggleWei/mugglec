#include "socket_event_epoll.h"
#include <stdlib.h>
#include <string.h>
#include "muggle/c/log/log.h"
#include "socket_event_utils.h"

#if MUGGLE_PLATFORM_LINUX

typedef struct muggle_epoll_peer
{
	int                  ptr_idx;
#if MUGGLE_ENABLE_TRACE
	int                  array_idx;
#endif
	struct epoll_event   epev;
	muggle_socket_peer_t peer;
} muggle_epoll_peer_t;

#if MUGGLE_ENABLE_TRACE

static void muggle_ev_epoll_debug_print(
        char *buf, int offset, int bufsize,
        muggle_epoll_peer_t **p_epeers, int cnt_fd)
{
    offset += snprintf(buf + offset, bufsize - offset, "current sockets: ");
	int cnt = 0;
	for (int i = 0; i < cnt_fd; i++)
	{
		if (cnt >= 5)
		{
			offset += snprintf(buf + offset, bufsize - offset, "... | ");
			break;
		}

		char straddr[MUGGLE_SOCKET_ADDR_STRLEN];
		if (p_epeers[i]->peer.addr_len == 0 ||
			muggle_socket_ntop((struct sockaddr*)&p_epeers[i]->peer.addr, straddr, MUGGLE_SOCKET_ADDR_STRLEN, 0) == NULL)
		{
			snprintf(straddr, MUGGLE_SOCKET_ADDR_STRLEN, "?:?");
		}

#if MUGGLE_PLATFORM_WINDOWS
		offset += snprintf(buf + offset, bufsize - offset, "[%s](%d<->%d) | ",
			straddr, p_epeers[i]->ptr_idx, p_epeers[i]->array_idx);
#else
		offset += snprintf(buf + offset, bufsize - offset, "%d[%s](%d<->%d) | ",
			p_epeers[i]->peer.fd, straddr, p_epeers[i]->ptr_idx, p_epeers[i]->array_idx);
#endif

		++cnt;
	}

    offset += snprintf(buf + offset, bufsize - offset, "cnt_fd: %d", cnt_fd);
    MUGGLE_DEBUG_INFO(buf);
}

#endif

static int muggle_socket_event_epoll_listen(
	muggle_socket_event_t *ev,
	muggle_socket_t *epfd,
	muggle_epoll_peer_t *listen_epeer,
	muggle_epoll_peer_t *epeers,
	muggle_epoll_peer_t **p_epeers,
	int capacity, int *cnt_fd)
{
	while (1)
	{
		muggle_epoll_peer_t tmp_epeer;
		muggle_epoll_peer_t *epeer;
		if (*cnt_fd == capacity)
		{
			epeer = &tmp_epeer;
		}
		else
		{
			epeer = p_epeers[*cnt_fd];
		}
		memset(&epeer->peer, 0, sizeof(muggle_socket_peer_t));

		muggle_socket_peer_t *peer = &epeer->peer;
		peer->addr_len = sizeof(peer->addr);
		peer->fd = accept(listen_epeer->peer.fd, (struct sockaddr*)&peer->addr, &peer->addr_len);
		if (peer->fd == MUGGLE_INVALID_SOCKET)
		{
			if (MUGGLE_SOCKET_LAST_ERRNO == MUGGLE_SYS_ERRNO_INTR)
			{
				continue;
			}
			else if (MUGGLE_SOCKET_LAST_ERRNO == MUGGLE_SYS_ERRNO_WOULDBLOCK)
			{
				return 0;
			}

			char err_msg[1024];
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_ERROR("failed accept - %s", err_msg);

			// close listen socket
			if (ev->on_error != NULL)
			{
				if (ev->on_error(ev, &listen_epeer->peer) == 0)
				{
					muggle_socket_close(listen_epeer->peer.fd);
				}
			}
			else
			{
				muggle_socket_close(listen_epeer->peer.fd);
			}

			return 1;
		}

		if (epeer == &tmp_epeer)
		{
			// close socket if number reached the upper limit
			char straddr[MUGGLE_SOCKET_ADDR_STRLEN];
			if (muggle_socket_ntop((struct sockaddr*)&peer->addr, straddr, sizeof(straddr), 0) == NULL)
			{
				snprintf(straddr, sizeof(straddr), "unknown:unknown");
			}

			MUGGLE_WARNING("refuse connection %s - number of connection reached the upper limit", straddr);
			muggle_socket_close(peer->fd);
		}
		else
		{
			peer->peer_type = MUGGLE_SOCKET_PEER_TYPE_TCP_PEER;

			// set socket nonblock
			muggle_socket_set_nonblock(peer->fd, 1);

			int ret = 0;
			if (ev->on_connect)
			{
				ret = ev->on_connect(ev, &listen_epeer->peer, peer);
			}

			if (ret == 0)
			{
				// add new connection socket peer
				epeer->epev.events = EPOLLIN | EPOLLET;
				if (epoll_ctl(*epfd, EPOLL_CTL_ADD, epeer->peer.fd, &epeer->epev) == MUGGLE_INVALID_SOCKET)
				{
					char err_msg[1024] = {0};
					muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
					MUGGLE_ERROR("failed epoll_ctl EPOLL_CTL_ADD - %s", err_msg);

					muggle_socket_close(epeer->peer.fd);
					continue;
				}

				++(*cnt_fd);

#if MUGGLE_ENABLE_TRACE
				char debug_buf[4096];
				int debug_offset = 0;
				debug_offset = snprintf(debug_buf, sizeof(debug_buf) - debug_offset, "new connection |");
				muggle_ev_epoll_debug_print(
					debug_buf, debug_offset, sizeof(debug_buf), p_epeers, *cnt_fd);
#endif
			}
			else
			{
				if (ret == -1)
				{
					muggle_socket_close(peer->fd);
				}
			}
		}
	}
}

void muggle_socket_event_epoll(muggle_socket_event_t *ev, muggle_socket_ev_arg_t *ev_arg)
{
	MUGGLE_TRACE("socket event epoll run...");

	// set fd capacity
	int capacity = ev_arg->hints_max_peer;
	if (capacity <= 0)
	{
		capacity = 1024;
	}

	if (capacity < ev_arg->cnt_peer)
	{
		MUGGLE_ERROR("capacity space not enough for all peers");
		for (int i = 0; i < ev_arg->cnt_peer; ++i)
		{
			muggle_socket_close(ev_arg[i].peers->fd);
		}
		return;
	}

	// timer
	int timeout = ev_arg->timeout_ms;

	muggle_epoll_peer_t *epeers = (muggle_epoll_peer_t*)malloc(capacity * sizeof(muggle_epoll_peer_t));
	muggle_epoll_peer_t **p_epeers = (muggle_epoll_peer_t**)malloc(capacity * sizeof(muggle_epoll_peer_t*));
	struct epoll_event *ret_epev = (struct epoll_event*)malloc(capacity * sizeof(struct epoll_event));
	for (int i = 0; i < capacity; ++i)
	{
		memset(&epeers[i], 0, sizeof(muggle_epoll_peer_t));
		p_epeers[i] = &epeers[i];
		epeers[i].ptr_idx = i;
#if MUGGLE_ENABLE_TRACE
		epeers[i].array_idx = i;
#endif
		epeers[i].epev.data.ptr = &epeers[i];
	}

	// create epoll file description
	muggle_socket_t epfd = epoll_create(capacity);
	if (epfd == MUGGLE_INVALID_SOCKET)
	{
        char err_msg[1024] = {0};
        muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
        MUGGLE_ERROR("failed epoll_create - %s", err_msg);

		for (int i = 0; i < ev_arg->cnt_peer; ++i)
		{
			muggle_socket_close(ev_arg[i].peers->fd);
		}
		return;
	}

	int cnt_fd = 0;
	for (int i = 0; i < ev_arg->cnt_peer; ++i)
	{
		muggle_epoll_peer_t *epeer = p_epeers[i];
		memcpy(&epeer->peer, &ev_arg->peers[i], sizeof(muggle_socket_peer_t));
		epeer->epev.events = EPOLLIN | EPOLLET;

		muggle_socket_set_nonblock(epeer->peer.fd, 1);

		if (epoll_ctl(epfd, EPOLL_CTL_ADD, epeer->peer.fd, &epeer->epev) == MUGGLE_INVALID_SOCKET)
		{
			char err_msg[1024] = {0};
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_ERROR("failed epoll_ctl EPOLL_CTL_ADD - %s", err_msg);

			muggle_socket_close(epeer->peer.fd);

			--i;
			continue;
		}

		++cnt_fd;
	}

	struct timespec t1, t2;
	if (ev->timeout_ms > 0)
	{
		timespec_get(&t1, TIME_UTC);
	}
	while (1)
	{
		int n = epoll_wait(epfd, ret_epev, capacity, timeout); 
		if (n > 0)
		{
			for (int i = 0; i < n; ++i)
			{
				muggle_epoll_peer_t *epeer = (muggle_epoll_peer_t*)ret_epev[i].data.ptr;
				int need_close = 0;
				if (ret_epev[i].events & EPOLLIN)
				{
					muggle_socket_peer_t *peer = &epeer->peer;
					switch (peer->peer_type)
					{
					case MUGGLE_SOCKET_PEER_TYPE_TCP_LISTEN:
						{
							need_close = muggle_socket_event_epoll_listen(ev, &epfd, epeer, epeers, p_epeers, capacity, &cnt_fd);
						}break;
					case MUGGLE_SOCKET_PEER_TYPE_TCP_PEER:
					case MUGGLE_SOCKET_PEER_TYPE_UDP_PEER:
						{
							need_close = muggle_socket_event_on_message(ev, peer);
						}break;
					default:
						{
							MUGGLE_ERROR("invalid peer type: %d", peer->peer_type);
						}break;
					}
				}
				else if (ret_epev[i].events & (EPOLLERR | EPOLLHUP))
				{
					if (ev->on_error != NULL)
					{
						if (ev->on_error(ev, &epeer->peer) == 0)
						{
							muggle_socket_close(epeer->peer.fd);
						}
					}
					need_close = 1;
				}

				if (need_close)
				{
					if (epeer->ptr_idx != cnt_fd - 1)
					{
						epoll_ctl(epfd, EPOLL_CTL_DEL, epeer->peer.fd, &epeer->epev);

						int ptr_idx = epeer->ptr_idx;
						muggle_epoll_peer_t *p_tmp = p_epeers[ptr_idx];
						p_epeers[ptr_idx] = p_epeers[cnt_fd - 1];
						p_epeers[cnt_fd - 1] = p_tmp;

						p_epeers[cnt_fd - 1]->ptr_idx = cnt_fd - 1;
						p_epeers[ptr_idx]->ptr_idx = ptr_idx;
					}
					--cnt_fd;

#if MUGGLE_ENABLE_TRACE
				char debug_buf[4096];
				int debug_offset = 0;
				debug_offset = snprintf(debug_buf, sizeof(debug_buf) - debug_offset, "disconnection |");
				muggle_ev_epoll_debug_print(
					debug_buf, debug_offset, sizeof(debug_buf), p_epeers, cnt_fd);
#endif
				}
			}

			// when loop is busy, timeout will not trigger, use
			// customize timer handle avoid that
			if (ev->timeout_ms > 0)
			{
				muggle_socket_event_timer_handle(ev, &t1, &t2);
			}
		}
		else if (n == 0)
		{
			muggle_socket_event_timer_handle(ev, &t1, &t2);
		}
		else
		{
			char err_msg[1024];
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_ERROR("failed epoll loop - %s", err_msg);

			ev->to_exit = 1;
		}

		if (ev->to_exit)
		{
			MUGGLE_INFO("exit event loop");
			for (int i = 0; i < cnt_fd; ++i)
			{
				muggle_socket_close(p_epeers[i]->peer.fd);
				epoll_ctl(epfd, EPOLL_CTL_DEL, p_epeers[i]->peer.fd, &p_epeers[i]->epev);
			}
			muggle_socket_close(epfd);
			break;
		}
	}

	// free memory
	free(epeers);
	free(p_epeers);
	free(ret_epev);
}

#endif
