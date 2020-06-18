#include "socket_event_select.h"
#include <string.h>
#include "muggle/c/log/log.h"
#include "muggle/c/memory/memory_pool.h"
#include "socket_event_utils.h"

struct muggle_peer_list_node
{
	struct muggle_peer_list_node *next;
	muggle_socket_peer_t         peer;
};

#if MUGGLE_ENABLE_TRACE

static void muggle_ev_select_debug_print(
	char *buf, int offset, int bufsize,
	struct muggle_peer_list_node *head, int nfds)
{
    offset += snprintf(buf + offset, bufsize - offset, "current sockets: [head] -> ");
    struct muggle_peer_list_node *node = head->next;
	int cnt = 0;
    while (node)
    {
		if (cnt >= 5)
		{
			offset += snprintf(buf + offset, bufsize - offset, "... ->");
			break;
		}

		char straddr[MUGGLE_SOCKET_ADDR_STRLEN];
		if (node->peer.addr_len == 0 ||
			muggle_socket_ntop((struct sockaddr*)&node->peer.addr, straddr, MUGGLE_SOCKET_ADDR_STRLEN, 0) == NULL)
		{
			snprintf(straddr, MUGGLE_SOCKET_ADDR_STRLEN, "?:?");
		}

#if MUGGLE_PLATFORM_WINDOWS
        offset += snprintf(buf + offset, bufsize - offset, "[%s] -> ", straddr);
#else
        offset += snprintf(buf + offset, bufsize - offset, "%d[%s] -> ", node->peer.fd, straddr);
#endif
        node = node->next;

		++cnt;
    }

    offset += snprintf(buf + offset, bufsize - offset, "NULL | nfds: %d", nfds);
    MUGGLE_DEBUG_INFO(buf);
}

#endif

static void muggle_socket_event_select_listen(
	muggle_socket_event_t *ev,
	struct muggle_peer_list_node *node,
	struct muggle_peer_list_node *head,
	muggle_memory_pool_t *pool,
	fd_set *allset, int *nfds)
{
	while (1)
	{
		// get new peer
		struct muggle_peer_list_node tmp_node;
		struct muggle_peer_list_node *new_node = (struct muggle_peer_list_node*)muggle_memory_pool_alloc(pool);
		if (new_node == NULL)
		{
			new_node = &tmp_node;
		}
		memset(new_node, 0, sizeof(struct muggle_peer_list_node));

		// accept new connection
		int ret = muggle_socket_event_accept(ev, &node->peer, &new_node->peer);
		if (ret != MUGGLE_SOCKET_EVENT_ACCEPT_RET_PEER)
		{
			if (new_node != &tmp_node)
			{
				muggle_memory_pool_free(pool, new_node);
			}

			if (ret == MUGGLE_SOCKET_EVENT_ACCEPT_RET_INTR)
			{
				continue;
			}
			else if (ret == MUGGLE_SOCKET_EVENT_ACCEPT_RET_WBLOCK ||
				ret == MUGGLE_SOCKET_EVENT_ACCEPT_RET_CLOSED)
			{
				return;
			}
		}

		if (new_node == &tmp_node)
		{
			// close socket if number reached the upper limit
			char straddr[MUGGLE_SOCKET_ADDR_STRLEN];
			if (muggle_socket_ntop((struct sockaddr*)&new_node->peer.addr, straddr, sizeof(straddr), 0) == NULL)
			{
				snprintf(straddr, sizeof(straddr), "unknown:unknown");
			}

			MUGGLE_LOG_WARNING("refuse connection %s - number of connection reached the upper limit", straddr);
			muggle_socket_close(new_node->peer.fd);
		}
		else
		{
			new_node->peer.peer_type = MUGGLE_SOCKET_PEER_TYPE_TCP_PEER;

			// set socket nonblock
			muggle_socket_set_nonblock(new_node->peer.fd, 1);

			// add new connection socket into read fds
			FD_SET(new_node->peer.fd, allset);
#if !MUGGLE_PLATFORM_WINDOWS
			if (new_node->peer.fd > *nfds)
			{
				*nfds = new_node->peer.fd;
			}
#endif

			// add node into list
			new_node->next = head->next;
			head->next = new_node;

			// notify user
			if (ev->on_connect)
			{
				ev->on_connect(ev, &node->peer, &new_node->peer);
			}

#if MUGGLE_ENABLE_TRACE
			char debug_buf[4096];
			int debug_offset = 0;
			debug_offset = snprintf(debug_buf, sizeof(debug_buf) - debug_offset, "new connection |");
			muggle_ev_select_debug_print(
				debug_buf, debug_offset, sizeof(debug_buf), head, *nfds);
#endif
		}
	}
}

void muggle_socket_event_select(muggle_socket_event_t *ev, muggle_socket_ev_arg_t *ev_arg)
{
	MUGGLE_LOG_TRACE("socket event select run...");

	// set timeout
	struct timeval timeout, save_timeout;
	struct timeval *p_timeout = &timeout;
	memset(&timeout, 0, sizeof(timeout));
	if (ev->timeout_ms < 0)
	{
		p_timeout = NULL;
	}
	else if (ev->timeout_ms > 0)
	{
		timeout.tv_sec = ev->timeout_ms / 1000;
		timeout.tv_usec = (ev->timeout_ms % 1000) * 1000;
		memcpy(&save_timeout, &timeout, sizeof(struct timeval));
	}

	// fixed size pool for peers
	muggle_memory_pool_t pool;
	if (!muggle_memory_pool_init(&pool, ev->capacity, sizeof(struct muggle_peer_list_node)))
	{
		MUGGLE_LOG_ERROR("failed init memory pool for capacity: %d, unit size: %d",
			ev->capacity, sizeof(struct muggle_peer_list_node));
		for (int i = 0; i < ev_arg->cnt_peer; ++i)
		{
			muggle_socket_close(ev_arg[i].peers->fd);
		}
		return;
	}
	muggle_memory_pool_set_flag(&pool, MUGGLE_MEMORY_POOL_CONSTANT_SIZE);

	// peer alive node list and recycle node list
	struct muggle_peer_list_node head, recycle_head;
	head.next = NULL;
	recycle_head.next = NULL;

	// convert sockets into peer
	for (int i = 0; i < ev_arg->cnt_peer; i++)
	{
		struct muggle_peer_list_node *node = (struct muggle_peer_list_node*)muggle_memory_pool_alloc(&pool);
		memset(node, 0, sizeof(struct muggle_peer_list_node));
		node->next = head.next;
		head.next = node;

		memcpy(&node->peer, &ev_arg->peers[i], sizeof(muggle_socket_peer_t));
		muggle_socket_set_nonblock(node->peer.fd, 1);
	}

	// select fds
	int nfds = 0;
	fd_set rset, allset;
	FD_ZERO(&allset);
	struct muggle_peer_list_node *node = NULL, *prev_node = NULL;

	node = head.next;
	while (node)
	{
#if !MUGGLE_PLATFORM_WINDOWS
		if (node->peer.fd > nfds)
		{
			nfds = node->peer.fd;
		}
#endif
		FD_SET(node->peer.fd, &allset);
		node = node->next;
	}

	struct timespec t1, t2;
	if (ev->timeout_ms > 0)
	{
		timespec_get(&t1, TIME_UTC);
	}

	while (1)
	{
		// select need reset timeout
		if (p_timeout)
		{
			memcpy(&timeout, &save_timeout, sizeof(struct timeval));
		}

		// select loop
		rset = allset;
		int n = select(nfds + 1, &rset, NULL, NULL, p_timeout);
		if (n > 0)
		{
			node = head.next;
			prev_node = &head;
			while (node)
			{
				if (FD_ISSET(node->peer.fd, &rset))
				{
					switch (node->peer.peer_type)
					{
					case MUGGLE_SOCKET_PEER_TYPE_TCP_LISTEN:
						{
							muggle_socket_event_select_listen(ev, node, &head, &pool, &allset, &nfds);
						}break;
					case MUGGLE_SOCKET_PEER_TYPE_TCP_PEER:
					case MUGGLE_SOCKET_PEER_TYPE_UDP_PEER:
						{
							muggle_socket_event_on_message(ev, &node->peer);
						}break;
					default:
						{
							MUGGLE_LOG_ERROR("invalid peer type: %d", node->peer.peer_type);
						}break;
					}
					
					if (node->peer.status == MUGGLE_SOCKET_PEER_STATUS_CLOSED)
					{
						if (ev->on_error)
						{
							ev->on_error(ev, &node->peer);
						}

						FD_CLR(node->peer.fd, &allset);

						prev_node->next = node->next;
						int ref_cnt = muggle_socket_peer_release(&node->peer);
						if (ref_cnt == 0)
						{
							muggle_memory_pool_free(&pool, node);
						}
						else
						{
							node->next = recycle_head.next;
							recycle_head.next = node;
						}
						node = prev_node->next;

#if MUGGLE_ENABLE_TRACE
						char debug_buf[4096];
						int debug_offset = 0;
						debug_offset = snprintf(debug_buf, sizeof(debug_buf) - debug_offset, "disconnection |");
						muggle_ev_select_debug_print(
							debug_buf, debug_offset, sizeof(debug_buf), &head, nfds);
#endif
					}
					else
					{
						prev_node = node;
						node = node->next;
					}

					if (--n <= 0)
					{
						break;
					}
				}
				else
				{
					prev_node = node;
					node = node->next;
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
			if (MUGGLE_SOCKET_LAST_ERRNO == MUGGLE_SYS_ERRNO_INTR)
			{
				continue;
			}

			char err_msg[1024];
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_LOG_ERROR("failed select loop - %s", err_msg);

			ev->to_exit = 1;
		}

		// exit loop
		if (ev->to_exit)
		{
			MUGGLE_LOG_INFO("exit event loop");
			node = head.next;
			while (node)
			{
				muggle_socket_close(node->peer.fd);
				node = node->next;
			}
			break;
		}

		// recycle node
		struct muggle_peer_list_node *recycle_node = recycle_head.next, *recycle_next = NULL, *recycle_prev = &recycle_head;
		while (recycle_node)
		{
			if (recycle_node->peer.ref_cnt == 0)
			{
				recycle_prev->next = recycle_node->next;
				muggle_memory_pool_free(&pool, recycle_node);
				recycle_node = recycle_prev->next;
			}
			else
			{
				recycle_prev = recycle_node;
				recycle_node = recycle_node->next;
			}
		}
	}

	// free memory pool
    muggle_memory_pool_destroy(&pool);
}
