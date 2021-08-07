/******************************************************************************
 *  @file         socket_event_epoll.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-21
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec socket event - epoll
 *****************************************************************************/

#include "socket_event_epoll.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "muggle/c/log/log.h"
#include "socket_event_utils.h"
#include "socket_event_memmgr.h"

#if MUGGLE_PLATFORM_LINUX

static void muggle_socket_event_epoll_listen(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *listen_peer,
	muggle_socket_event_memmgr_t *mem_mgr,
	muggle_socket_t *epfd,
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

		// add new connection socket into epoll
		struct epoll_event epev;
		memset(&epev, 0, sizeof(epev));
		epev.data.ptr = node;
		epev.events = EPOLLIN | EPOLLET;
		if (epoll_ctl(*epfd, EPOLL_CTL_ADD, node->peer.fd, &epev) == MUGGLE_INVALID_SOCKET)
		{
			char err_msg[1024] = {0};
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_LOG_ERROR("failed epoll_ctl EPOLL_CTL_ADD - %s", err_msg);

			muggle_socket_event_memmgr_recycle(mem_mgr, node);
			continue;
		}
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

void muggle_socket_event_epoll(muggle_socket_event_t *ev)
{
	MUGGLE_LOG_TRACE("socket event epoll run...");

	// get memory manager
	muggle_socket_event_memmgr_t *p_mem_mgr = (muggle_socket_event_memmgr_t*)ev->mem_mgr;

	struct epoll_event *ret_epevs = (struct epoll_event*)malloc(ev->capacity * sizeof(struct epoll_event));
	if (ret_epevs == NULL)
	{
		muggle_socket_event_memmgr_destroy(p_mem_mgr);
		return;
	}

	for (int i = 0; i < ev->capacity; ++i)
	{
		memset(&ret_epevs[i], 0, sizeof(struct epoll_event));
	}

	muggle_socket_t epfd = epoll_create(ev->capacity);
	if (epfd == MUGGLE_INVALID_SOCKET)
	{
        char err_msg[1024] = {0};
        muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
        MUGGLE_LOG_ERROR("failed epoll_create - %s", err_msg);

		free(ret_epevs);
		muggle_socket_event_memmgr_destroy(p_mem_mgr);
		return;
	}

	int cnt_fd;
	struct epoll_event epev;
	muggle_socket_peer_list_node_t *node = muggle_socket_event_memmgr_get_node(p_mem_mgr);
	while (node)
	{
		memset(&epev, 0, sizeof(epev));
		epev.data.ptr = node;
		epev.events = EPOLLIN | EPOLLET;

		if (epoll_ctl(epfd, EPOLL_CTL_ADD, node->peer.fd, &epev) == MUGGLE_INVALID_SOCKET)
		{
			char err_msg[1024] = {0};
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_LOG_ERROR("failed epoll_ctl EPOLL_CTL_ADD - %s", err_msg);

			muggle_socket_peer_list_node_t *next_node = node->next;
			muggle_socket_event_memmgr_recycle(p_mem_mgr, node);
			node = next_node;
			continue;
		}

		node = node->next;
		++cnt_fd;
	}

	// timer
	int timeout = ev->timeout_ms;

	struct timespec t1, t2;
	if (ev->timeout_ms > 0)
	{
		timespec_get(&t1, TIME_UTC);
	}

	while (1)
	{
		int n = epoll_wait(epfd, ret_epevs, ev->capacity, timeout); 
		if (n > 0)
		{
			for (int i = 0; i < n; ++i)
			{
				muggle_socket_peer_list_node_t *node = (muggle_socket_peer_list_node_t*)ret_epevs[i].data.ptr;
				muggle_socket_peer_t *peer = &node->peer;

				if (ret_epevs[i].events & EPOLLIN)
				{
					switch (peer->peer_type)
					{
					case MUGGLE_SOCKET_PEER_TYPE_TCP_LISTEN:
						{
							muggle_socket_event_epoll_listen(ev, peer, p_mem_mgr, &epfd, ev->capacity, &cnt_fd);
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
				}
				else if (ret_epevs[i].events & (EPOLLERR | EPOLLHUP))
				{
					muggle_socket_peer_close(peer);
				}

				if (peer->status == MUGGLE_SOCKET_PEER_STATUS_CLOSED)
				{
					if (ev->on_error)
					{
						ev->on_error(ev, peer);
					}

					epoll_ctl(epfd, EPOLL_CTL_DEL, peer->fd, &ret_epevs[i]);

					muggle_socket_event_memmgr_recycle(p_mem_mgr, node);
					--cnt_fd;
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
			MUGGLE_LOG_ERROR("failed epoll loop - %s", err_msg);

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
	free(ret_epevs);
}

#endif
