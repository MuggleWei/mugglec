/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "socket_event_poll.h"
#include <stdlib.h>
#include <string.h>
#include "muggle/c/log/log.h"
#include "socket_event_utils.h"
#include "socket_event_memmgr.h"

static void muggle_socket_event_poll_listen(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *listen_peer,
	muggle_socket_event_memmgr_t *mem_mgr,
	struct pollfd *fds,
	muggle_socket_peer_list_node_t **p_nodes,
	int capacity, int *cnt_fd)
{
	while (1)
	{
		// reach at up limit
		if (*cnt_fd == capacity)
		{
			muggle_socket_event_refuse_accept(listen_peer);
			break;
		}

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

		// add new connection socket into nodes
		p_nodes[*cnt_fd] = node;
		memset(&fds[*cnt_fd], 0, sizeof(struct pollfd));
		fds[*cnt_fd].fd = node->peer.fd;
		fds[*cnt_fd].events = POLLIN;
		++(*cnt_fd);

		// notify user
		node->peer.ev = ev;
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

void muggle_socket_event_poll(muggle_socket_event_t *ev)
{
	MUGGLE_LOG_TRACE("socket event poll run...");

	// get memory manager
	muggle_socket_event_memmgr_t *p_mem_mgr = (muggle_socket_event_memmgr_t*)ev->mem_mgr;

	struct pollfd *fds = (struct pollfd*)malloc(ev->capacity * sizeof(struct pollfd));
	muggle_socket_peer_list_node_t **p_nodes =
		(muggle_socket_peer_list_node_t**)malloc(ev->capacity * sizeof(muggle_socket_peer_list_node_t*));
	if (fds == NULL || p_nodes == NULL)
	{
		if (fds)
		{
			free(fds);
		}

		if (p_nodes)
		{
			free(p_nodes);
		}

		muggle_socket_event_memmgr_destroy(p_mem_mgr);
		return;
	}

	for (int i = 0; i < ev->capacity; ++i)
	{
		p_nodes[i] = NULL;
		memset(&fds[i], 0, sizeof(struct pollfd));
	}

	int cnt_fd = 0;
	muggle_socket_peer_list_node_t *node = muggle_socket_event_memmgr_get_node(p_mem_mgr);
	while (node)
	{
		p_nodes[cnt_fd] = node;
		fds[cnt_fd].fd = node->peer.fd;
		fds[cnt_fd].events = POLLIN;
		cnt_fd++;

		node = node->next;
	}

	// set timeout
	int timeout = ev->timeout_ms;

	struct timespec t1, t2;
	if (ev->timeout_ms > 0)
	{
		timespec_get(&t1, TIME_UTC);
	}

	while (1)
	{
#if MUGGLE_PLATFORM_WINDOWS
		int n = WSAPoll(fds, cnt_fd, timeout);
#else
		int n = poll(fds, cnt_fd, timeout);
#endif
		if (n > 0)
		{
			for (int i = cnt_fd - 1; i >= 0; --i)
			{
				muggle_socket_peer_t *peer = &p_nodes[i]->peer;
				if (fds[i].revents & POLLIN)
				{
					switch (peer->peer_type)
					{
					case MUGGLE_SOCKET_PEER_TYPE_TCP_LISTEN:
						{
							muggle_socket_event_poll_listen(ev, peer, p_mem_mgr, fds, p_nodes, ev->capacity, &cnt_fd);
						}break;
					case MUGGLE_SOCKET_PEER_TYPE_TCP_PEER:
					case MUGGLE_SOCKET_PEER_TYPE_UDP_PEER:
						{
							muggle_socket_event_on_message(ev, peer);
						}break;
					default:
						{
							MUGGLE_LOG_ERROR("invalid peer type: %d", peer->peer_type);
						}break;
					}

					--n;
				}
				else if (fds[i].revents & (POLLHUP | POLLERR))
				{
					muggle_socket_peer_close(peer);

					--n;
				}

				if (peer->status == MUGGLE_SOCKET_PEER_STATUS_CLOSED)
				{
					if (ev->on_error)
					{
						ev->on_error(ev, peer);
					}

					if (i != cnt_fd - 1)
					{
						muggle_socket_peer_list_node_t *p_tmp;
						p_tmp = p_nodes[i];
						p_nodes[i] = p_nodes[cnt_fd - 1];
						p_nodes[cnt_fd - 1] = p_tmp;

						memcpy(&fds[i], &fds[cnt_fd - 1], sizeof(struct pollfd));
					}

					muggle_socket_event_memmgr_recycle(p_mem_mgr, p_nodes[cnt_fd - 1]);
					p_nodes[cnt_fd - 1] = NULL;

					--cnt_fd;
				}

				if (n <= 0)
				{
					break;
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
			MUGGLE_LOG_ERROR("failed poll loop - %s", err_msg);

			ev->to_exit = 1;
		}

		// exit loop
		if (ev->to_exit)
		{
			MUGGLE_LOG_INFO("exit event loop");
			break;
		}

		// recycle node
		muggle_socket_event_memmgr_clear(p_mem_mgr);
	}

	// free memory
	free(fds);
	free(p_nodes);
}
