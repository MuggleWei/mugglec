/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "socket_event_select.h"
#include <string.h>
#include "muggle/c/log/log.h"
#include "muggle/c/memory/memory_pool.h"
#include "socket_event_utils.h"
#include "socket_event_memmgr.h"

static void muggle_socket_event_select_listen(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *listen_peer,
	muggle_socket_event_memmgr_t *mem_mgr,
	fd_set *allset, int *nfds)
{
	while (1)
	{
		// get new peer
		muggle_socket_peer_list_node_t *node = muggle_socket_event_memmgr_allocate(mem_mgr);
		if (node == NULL)
		{
			muggle_socket_event_refuse_accept(listen_peer);
			break;
		}

		// accept new connection
		muggle_socket_event_accept(listen_peer, &node->peer);
		if (node->peer.fd == MUGGLE_INVALID_SOCKET)
		{
			muggle_socket_event_memmgr_free(mem_mgr, node);
			break;
		}

		// add new connection socket into read fds
		FD_SET(node->peer.fd, allset);
#if !MUGGLE_PLATFORM_WINDOWS
		if (node->peer.fd > *nfds)
		{
			*nfds = new_node->peer.fd;
		}
#endif

		// notify user
		if (ev->on_connect)
		{
			ev->on_connect(ev, listen_peer, &node->peer);
		}


#if MUGGLE_ENABLE_TRACE
		char debug_buf[4096];
		int debug_offset = 0;
		debug_offset = snprintf(debug_buf, sizeof(debug_buf) - debug_offset, "active list | ");
		muggle_socket_event_memmgr_debug_print(mem_mgr->active_head.next, debug_buf, debug_offset, sizeof(debug_buf));

		debug_offset = 0;
		debug_offset = snprintf(debug_buf, sizeof(debug_buf) - debug_offset, "recycle list | ");
		muggle_socket_event_memmgr_debug_print(mem_mgr->recycle_head.next, debug_buf, debug_offset, sizeof(debug_buf));
#endif
	}
}

void muggle_socket_event_select(muggle_socket_event_t *ev, muggle_socket_ev_arg_t *ev_arg)
{
	MUGGLE_LOG_TRACE("socket event select run...");

	// init memory manager
	muggle_socket_event_memmgr_t mem_mgr;
	if (muggle_socket_event_memmgr_init(ev, ev_arg, &mem_mgr) != 0)
	{
		return;
	}

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

	struct timespec t1, t2;
	if (ev->timeout_ms > 0)
	{
		timespec_get(&t1, TIME_UTC);
	}

	// set fds
	int nfds = 0;
	fd_set rset, allset;
	FD_ZERO(&allset);
	muggle_socket_peer_list_node_t *node = NULL;

	node = muggle_socket_event_memmgr_get_node(&mem_mgr);
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
		// reset timeout
		if (p_timeout)
		{
			memcpy(&timeout, &save_timeout, sizeof(struct timeval));
		}

		// select loop
		rset = allset;
		int n = select(nfds + 1, &rset, NULL, NULL, p_timeout);
		if (n > 0)
		{
			node = muggle_socket_event_memmgr_get_node(&mem_mgr);
			while (node)
			{
				if (FD_ISSET(node->peer.fd, &rset))
				{
					switch (node->peer.peer_type)
					{
					case MUGGLE_SOCKET_PEER_TYPE_TCP_LISTEN:
						{
							muggle_socket_event_select_listen(ev, &node->peer, &mem_mgr, &allset, &nfds);
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

						muggle_socket_peer_list_node_t *next_node = node->next;
						muggle_socket_event_memmgr_recycle(&mem_mgr, node);
						node = next_node;
					}
					else
					{
						node = node->next;
					}

					if (--n <= 0)
					{
						break;
					}
				}
				else
				{
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
			break;
		}

		// recycle node
		muggle_socket_event_memmgr_clear(&mem_mgr);
	}

	// destroy memory manager
	muggle_socket_event_memmgr_destroy(&mem_mgr);
}
