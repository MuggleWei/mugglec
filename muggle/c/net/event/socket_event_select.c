#include "socket_event_select.h"
#include <string.h>
#include "muggle/c/log/log.h"
#include "muggle/c/memory/memory_pool.h"

struct muggle_peer_list_node
{
	struct muggle_peer_list_node *next;
	muggle_socket_peer_t    peer;
};

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

		int write_bytes = 0;
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

static int muggle_socket_event_select_listen(
	muggle_socket_event_t *ev,
	struct muggle_peer_list_node *node,
	struct muggle_peer_list_node *head,
	muggle_memory_pool_t *pool,
	fd_set *allset, int *nfds)
{
	while (1)
	{
		// get new connection
		struct muggle_peer_list_node tmp_node;
		struct muggle_peer_list_node *new_node = (struct muggle_peer_list_node*)muggle_memory_pool_alloc(pool);
		if (new_node == NULL)
		{
			new_node = &tmp_node;
		}
		memset(new_node, 0, sizeof(struct muggle_peer_list_node));

		new_node->peer.addr_len = sizeof(new_node->peer.addr);
		new_node->peer.fd = accept(node->peer.fd, (struct sockaddr*)&new_node->peer.addr, &new_node->peer.addr_len);
		if (new_node->peer.fd == MUGGLE_INVALID_SOCKET)
		{
			if (new_node != &tmp_node)
			{
				muggle_memory_pool_free(pool, new_node);
			}

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
				if (ev->on_error(ev, &node->peer) == 0)
				{
					muggle_socket_close(node->peer.fd);
				}
			}
			else
			{
				muggle_socket_close(node->peer.fd);
			}

			return 1;
		}

		if (new_node == &tmp_node)
		{
			// close socket if number reached the upper limit
			char straddr[MUGGLE_SOCKET_ADDR_STRLEN];
			if (muggle_socket_ntop((struct sockaddr*)&new_node->peer.addr, straddr, sizeof(straddr), 0) == NULL)
			{
				snprintf(straddr, sizeof(straddr), "unknown:unknown");
			}

			MUGGLE_WARNING("refuse connection %s - number of connection reached the upper limit", straddr);
			muggle_socket_close(new_node->peer.fd);
		}
		else
		{
			new_node->peer.peer_type = MUGGLE_SOCKET_PEER_TYPE_TCP_PEER;

			// set socket nonblock
			muggle_socket_set_nonblock(new_node->peer.fd, 1);

			int ret = 0;
			if (ev->on_connect)
			{
				ret = ev->on_connect(ev, &node->peer, &new_node->peer);
			}

			if (ret == 0)
			{
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

#if MUGGLE_DEBUG
				char debug_buf[4096];
				int debug_offset = 0;
				debug_offset = snprintf(debug_buf, sizeof(debug_buf) - debug_offset, "new connection |");
				muggle_ev_select_debug_print(
					debug_buf, debug_offset, sizeof(debug_buf), head, *nfds);
#endif
			}
			else
			{
				if (ret == -1)
				{
					muggle_socket_close(new_node->peer.fd);
				}
				muggle_memory_pool_free(pool, new_node);
			}
		}
	}

	return 0;
}

static int muggle_socket_event_select_peer(
	muggle_socket_event_t *ev,
	struct muggle_peer_list_node *node)
{
	if (ev->on_message)
	{
		int ret = ev->on_message(ev, &node->peer);
		if (ret == 0)
		{
			return 0;
		}
		else
		{
			if (ret == -1)
			{
				muggle_socket_close(node->peer.fd);
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
			n = recv(node->peer.fd, buf, sizeof(buf), 0);
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

				muggle_socket_close(node->peer.fd);
				return 1;
			}
			else
			{
				muggle_socket_close(node->peer.fd);
				return 1;
			}
		}
	}

	return 0;
}

void muggle_socket_event_select(muggle_socket_event_t *ev, muggle_socket_ev_arg_t *ev_arg)
{
	MUGGLE_TRACE("socket event select run...");

	// set fd capacity
	int capacity = ev_arg->hints_max_peer;
	if (capacity <= 0 || capacity > FD_SETSIZE)
	{
		capacity = FD_SETSIZE;
	}

	if (capacity < ev_arg->cnt_peer)
	{
		MUGGLE_WARNING("capacity space not enough for all peers");
		return;
	}

	// set timeout
	struct timeval timeout;
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
	}

	// fixed size pool for peers
	muggle_memory_pool_t pool;
	if (!muggle_memory_pool_init(&pool, capacity, sizeof(struct muggle_peer_list_node)))
	{
		MUGGLE_ERROR("failed init memory pool for capacity: %d, unit size: %d",
			capacity, sizeof(struct muggle_peer_list_node));
		return;
	}
	muggle_memory_pool_set_flag(&pool, MUGGLE_MEMORY_POOL_CONSTANT_SIZE);
	struct muggle_peer_list_node head;
	head.next = NULL;

	// convert sockets into peer
	for (int i = 0; i < ev_arg->cnt_peer; i++)
	{
		struct muggle_peer_list_node *node = (struct muggle_peer_list_node*)muggle_memory_pool_alloc(&pool);
		node->next = head.next;
		head.next = node;
		node->peer.peer_type = ev_arg->peers[i].peer_type;
		node->peer.fd = ev_arg->peers[i].fd;
		node->peer.data = ev_arg->peers[i].data;
		if (node->peer.peer_type == MUGGLE_SOCKET_PEER_TYPE_TCP_LISTEN ||
			node->peer.peer_type == MUGGLE_SOCKET_PEER_TYPE_TCP_PEER)
		{
			muggle_socket_set_nonblock(node->peer.fd, 1);
		}
	}

	// select
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
	while (1)
	{
		rset = allset;
		int n = select(nfds + 1, &rset, NULL, NULL, p_timeout);
		if (n > 0)
		{
			node = head.next;
			prev_node = &head;
			while (node)
			{
				int need_close = 0;
				if (FD_ISSET(node->peer.fd, &rset))
				{
					switch (node->peer.peer_type)
					{
					case MUGGLE_SOCKET_PEER_TYPE_TCP_LISTEN:
						{
							need_close = muggle_socket_event_select_listen(ev, node, &head, &pool, &allset, &nfds);
						}break;
					case MUGGLE_SOCKET_PEER_TYPE_TCP_PEER:
					case MUGGLE_SOCKET_PEER_TYPE_UDP_PEER:
						{
							need_close = muggle_socket_event_select_peer(ev, node);
						}break;
					default:
						{
							MUGGLE_ERROR("invalid peer type: %d", node->peer.peer_type);
						}break;
					}

					if (need_close)
					{
						FD_CLR(node->peer.fd, &allset);

						prev_node->next = node->next;
						muggle_memory_pool_free(&pool, node);
						node = prev_node->next;

#if MUGGLE_DEBUG
						char debug_buf[4096];
						int debug_offset = 0;
						debug_offset = snprintf(debug_buf, sizeof(debug_buf) - debug_offset, "new connection |");
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
		}
		else if (n == 0)
		{
			ev->on_timer(ev);
		}
		if (n == MUGGLE_SOCKET_ERROR)
		{
			char err_msg[1024];
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_ERROR("failed select loop - %s", err_msg);

			node = head.next;
			while (node)
			{
				muggle_socket_close(node->peer.fd);
				node = node->next;
			}
			break;
		}
	}

	// free memory pool
    muggle_memory_pool_destroy(&pool);
}
