/******************************************************************************
 *  @file         event_loop_poll.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-08-09
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec event loop poll
 *****************************************************************************/

#include "event_loop_poll.h"
#include <stdlib.h>
#include <string.h>

void muggle_evloop_handle_timer(muggle_event_loop_t *evloop);

static void muggle_evloop_poll_handle_wakeup(muggle_event_loop_poll_t *evloop_poll)
{
	muggle_event_loop_t *evloop = (muggle_event_loop_t*)evloop_poll;
	if (evloop_poll->fds[0].revents & POLLIN)
	{
		muggle_ev_signal_clearup(evloop->ev_signal);
		if (evloop->cb_wake)
		{
			evloop->cb_wake(evloop);
		}
	}
}

int muggle_evloop_init_poll(muggle_event_loop_t *evloop, muggle_event_loop_init_args_t *args)
{
	int capacity = args->hints_max_fd;
	if (capacity < 1)
	{
		capacity = 8;
	}
	capacity += 1; // for fd in event_signal

	muggle_event_loop_poll_t *evloop_poll = (muggle_event_loop_poll_t*)evloop;
	evloop_poll->capcity = capacity;
	evloop_poll->fds = (struct pollfd*)malloc(capacity * sizeof(struct pollfd));
	if (evloop_poll->fds == NULL)
	{
		goto muggle_evloop_init_poll_except;
	}
	evloop_poll->nodes = (void**)malloc(capacity * sizeof(void*));
	if (evloop_poll->nodes == NULL)
	{
		goto muggle_evloop_init_poll_except;
	}

	for (int i = 0; i < capacity; i++)
	{
		memset(&evloop_poll->fds[i], 0, sizeof(struct pollfd));
		evloop_poll->nodes[i] = NULL;
	}

	// add event_signal fd into fds
	muggle_event_fd evfd = muggle_ev_signal_rfd(evloop->ev_signal);
	evloop_poll->fds[0].fd = evfd;
	evloop_poll->fds[0].events = POLLIN;
	evloop_poll->nfd = 1;

	return 0;

muggle_evloop_init_poll_except:
	muggle_evloop_destroy_poll(evloop);
	return -1;
}

void muggle_evloop_destroy_poll(muggle_event_loop_t *evloop)
{
	muggle_event_loop_poll_t *evloop_poll = (muggle_event_loop_poll_t*)evloop;
	if (evloop_poll->fds)
	{
		free(evloop_poll->fds);
		evloop_poll->fds = NULL;
	}
	if (evloop_poll->nodes)
	{
		free(evloop_poll->nodes);
		evloop_poll->nodes = NULL;
	}
}

void muggle_evloop_run_poll(muggle_event_loop_t *evloop)
{
	muggle_event_loop_poll_t *evloop_poll = (muggle_event_loop_poll_t*)evloop;
	struct pollfd *fds = evloop_poll->fds;
	muggle_linked_list_node_t **nodes = (muggle_linked_list_node_t**)evloop_poll->nodes;
	while (1)
	{
#if MUGGLE_PLATFORM_WINDOWS
		int n = WSAPoll(evloop_poll->fds, evloop_poll->nfd, evloop->timeout);
#else
		int n = poll(evloop_poll->fds, evloop_poll->nfd, evloop->timeout);
#endif
		if (n > 0)
		{
			for (int i = evloop_poll->nfd - 1; i >= 0; --i)
			{
				if (i == 0)
				{
					muggle_evloop_poll_handle_wakeup(evloop_poll);
				}
				else
				{
					muggle_event_context_t *ctx = (muggle_event_context_t*)nodes[i]->data;
					if (fds[i].revents & POLLIN)
					{
						if (evloop->cb_read)
						{
							evloop->cb_read(evloop, (muggle_event_context_t*)nodes[i]->data);
						}
						--n;
					}
					if (fds[i].revents & (POLLHUP | POLLERR))
					{
						muggle_ev_ctx_set_flag(ctx, MUGGLE_EV_CTX_FLAG_CLOSED);
						--n;
					}

					if (ctx->flags & MUGGLE_EV_CTX_FLAG_CLOSED)
					{
						if (evloop->cb_close)
						{
							evloop->cb_close(evloop, ctx);
						}

						if (i != evloop_poll->nfd - 1)
						{
							muggle_linked_list_node_t *p_tmp = nodes[i];
							nodes[i] = nodes[evloop_poll->nfd - 1];
							nodes[evloop_poll->nfd - 1] = p_tmp;
							memcpy(&fds[i], &fds[evloop_poll->nfd-1], sizeof(struct pollfd));
						}
						nodes[evloop_poll->nfd-1] = NULL;
						--evloop_poll->nfd;
					}
				}

				if (n <= 0)
				{
					break;
				}
			}

			// when loop is busy, timeout will not trigger, use customize timer handle avoid that
			if (evloop->timeout >= 0)
			{
				muggle_evloop_handle_timer(evloop);
			}
		}
		else if (n == 0)
		{
			muggle_evloop_handle_timer(evloop);
		}
		else
		{
			muggle_evloop_exit(evloop);
		}

		if (evloop->to_exit)
		{
			break;
		}
	}
}

int muggle_evloop_add_ctx_poll(muggle_event_loop_t *evloop, muggle_event_context_t *ctx, void *node)
{
	muggle_event_loop_poll_t *evloop_poll = (muggle_event_loop_poll_t*)evloop;
	if (evloop_poll->nfd == evloop_poll->capcity)
	{
		return -1;
	}

	int idx = evloop_poll->nfd++;
	evloop_poll->fds[idx].fd = ctx->fd;
	evloop_poll->fds[idx].events = POLLIN;
	evloop_poll->nodes[idx] = node;

	return 0;
}
