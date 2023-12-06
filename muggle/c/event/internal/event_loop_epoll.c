/******************************************************************************
 *  @file         event_loop_epoll.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-08-09
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec event loop epoll
 *****************************************************************************/

#include "event_loop_epoll.h"
#include <stdlib.h>
#include <string.h>

void muggle_evloop_handle_timer(muggle_event_loop_t *evloop);

#if MUGGLE_PLATFORM_LINUX || MUGGLE_PLATFORM_ANDROID

static void muggle_evloop_epoll_handle_wakeup(muggle_event_loop_t *evloop, struct epoll_event *event)
{
	if (event->events & EPOLLIN)
	{
		muggle_ev_signal_clearup(evloop->ev_signal);
		if (evloop->cb_wake)
		{
			evloop->cb_wake(evloop);
		}
	}
}

int muggle_evloop_init_epoll(muggle_event_loop_t *evloop, muggle_event_loop_init_args_t *args)
{
	muggle_event_loop_epoll_t *evloop_epoll = (muggle_event_loop_epoll_t*)evloop;
	evloop_epoll->epfd = MUGGLE_INVALID_EVENT_FD;

	int capacity = args->hints_max_fd;
	if (capacity < 1)
	{
		capacity = 8;
	}
	capacity += 1; // for fd in event_signal

	evloop_epoll->capacity = capacity;
	evloop_epoll->epfd = epoll_create(capacity);
	if (evloop_epoll->epfd == MUGGLE_INVALID_EVENT_FD)
	{
		goto evloop_init_epoll_except;
	}

	evloop_epoll->events = (struct epoll_event*)malloc(capacity * sizeof(struct epoll_event));
	if (evloop_epoll->events == NULL)
	{
		goto evloop_init_epoll_except;
	}

	return 0;

evloop_init_epoll_except:
	muggle_evloop_destroy_epoll(evloop);
	return -1;
}

void muggle_evloop_destroy_epoll(muggle_event_loop_t *evloop)
{
	muggle_event_loop_epoll_t *evloop_epoll = (muggle_event_loop_epoll_t*)evloop;
	if (evloop_epoll->events)
	{
		free(evloop_epoll->events);
		evloop_epoll->events = NULL;
	}

	if (evloop_epoll->epfd != MUGGLE_INVALID_EVENT_FD)
	{
		muggle_ev_fd_close(evloop_epoll->epfd);
		evloop_epoll->epfd = MUGGLE_INVALID_EVENT_FD;
	}
}

void muggle_evloop_run_epoll(muggle_event_loop_t *evloop)
{
	muggle_event_loop_epoll_t *evloop_epoll = (muggle_event_loop_epoll_t*)evloop;

	// add event signal into epoll
	muggle_event_context_t signal_ctx;
	muggle_ev_ctx_init(&signal_ctx, muggle_ev_signal_rfd(evloop->ev_signal), NULL);
	muggle_linked_list_node_t signal_node;
	signal_node.data = &signal_ctx;
	muggle_evloop_add_ctx_epoll(evloop, &signal_ctx, &signal_node);

	muggle_event_fd epfd = evloop_epoll->epfd;
	struct epoll_event *events = evloop_epoll->events;
	int capacity = evloop_epoll->capacity;
	while (1)
	{
		int nfds = epoll_wait(epfd, events, capacity, evloop->timeout);
		for (int i = 0; i < nfds; i++)
		{
			muggle_linked_list_node_t *node = (muggle_linked_list_node_t*)events[i].data.ptr;
			muggle_event_context_t *ctx = (muggle_event_context_t*)node->data;
			if (ctx == &signal_ctx)
			{
				muggle_evloop_epoll_handle_wakeup(evloop, &events[i]);
			}
			else
			{
				if (events[i].events & EPOLLIN)
				{
					if (evloop->cb_read)
					{
						evloop->cb_read(evloop, ctx);
					}
				}
				else if (events[i].events & (EPOLLERR | EPOLLHUP))
				{
					muggle_ev_ctx_set_flag(ctx, MUGGLE_EV_CTX_FLAG_CLOSED);
				}

				if (ctx->flags & MUGGLE_EV_CTX_FLAG_CLOSED)
				{
					epoll_ctl(epfd, EPOLL_CTL_DEL, ctx->fd, &events[i]);
					if (evloop->cb_close)
					{
						evloop->cb_close(evloop, ctx);
					}
					muggle_linked_list_remove(evloop->ctx_list, node, NULL, NULL);
				}
			}
		}

		// when loop is busy, timeout will not trigger, use
		// customize timer handle avoid that
		if (evloop->timeout >= 0 && nfds >= 0)
		{
			muggle_evloop_handle_timer(evloop);
		}

		if (nfds < 0)
		{
			if (MUGGLE_EVENT_LAST_ERRNO != MUGGLE_SYS_ERRNO_INTR) {
				muggle_evloop_exit(evloop);
			}
		}

		if (evloop->to_exit)
		{
			break;
		}
	}
}

int muggle_evloop_add_ctx_epoll(muggle_event_loop_t *evloop, muggle_event_context_t *ctx, void *node)
{
	struct epoll_event event;
	memset(&event, 0, sizeof(event));
	event.data.ptr = node;
	event.events = EPOLLIN | EPOLLET;

	muggle_event_loop_epoll_t *evloop_epoll = (muggle_event_loop_epoll_t*)evloop;
	if (epoll_ctl(evloop_epoll->epfd, EPOLL_CTL_ADD, ctx->fd, &event) != 0)
	{
		return -1;
	}

	return 0;
}

#endif
