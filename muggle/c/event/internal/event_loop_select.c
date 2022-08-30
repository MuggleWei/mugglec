/******************************************************************************
 *  @file         event_loop_select.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-08-09
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec event loop select
 *****************************************************************************/

#include "event_loop_select.h"
#include <string.h>

void muggle_evloop_handle_timer(muggle_event_loop_t *evloop);

static void muggle_evloop_select_handle_wakeup(muggle_event_loop_select_t *evloop_select, fd_set *rset)
{
	muggle_event_loop_t *evloop = (muggle_event_loop_t*)evloop_select;
	muggle_event_signal_t *ev_signal = evloop->ev_signal;
	muggle_event_fd evfd = muggle_ev_signal_rfd(ev_signal);
	if (FD_ISSET(evfd, rset))
	{
		muggle_ev_signal_clearup(ev_signal);
		if (evloop->cb_wake)
		{
			evloop->cb_wake(evloop);
		}
	}
	FD_SET(evfd, &evloop_select->allset);
	evloop_select->nfds = evfd;
}

static void muggle_evloop_select_set_fd(muggle_event_loop_select_t *evloop_select, muggle_event_fd fd)
{
	FD_SET(fd, &evloop_select->allset);
	if (fd > evloop_select->nfds)
	{
		evloop_select->nfds = fd;
	}
}

int muggle_evloop_init_select(muggle_event_loop_t *evloop, muggle_event_loop_init_args_t *args)
{
	muggle_event_loop_select_t *evloop_select = (muggle_event_loop_select_t*)evloop;
	FD_ZERO(&evloop_select->allset);

	muggle_event_signal_t *ev_signal = evloop->ev_signal;
	muggle_event_fd evfd = muggle_ev_signal_rfd(ev_signal);
	FD_SET(evfd, &evloop_select->allset);
	evloop_select->nfds = evfd;

	return 0;
}

void muggle_evloop_destroy_select(muggle_event_loop_t *evloop)
{
	muggle_event_loop_select_t *evloop_select = (muggle_event_loop_select_t*)evloop;
	FD_ZERO(&evloop_select->allset);
}

void muggle_evloop_run_select(muggle_event_loop_t *evloop)
{
	// prepare timer
	struct timeval timeout, save_timeout;
	struct timeval *p_timeout = &timeout;
	memset(&timeout, 0, sizeof(timeout));
	if (evloop->timeout < 0)
	{
		p_timeout = NULL;
	}
	else
	{
		timeout.tv_sec = evloop->timeout / 1000;
		timeout.tv_usec = (evloop->timeout % 1000) * 1000;
		memcpy(&save_timeout, &timeout, sizeof(struct timeval));
	}

	// set fds
	fd_set rset;
	muggle_linked_list_t *linked_list = evloop->ctx_list;
	muggle_linked_list_node_t *node = NULL;

	// run loop
	muggle_event_loop_select_t *evloop_select = (muggle_event_loop_select_t*)evloop;
	while (1)
	{
		// reset timeout
		if (p_timeout)
		{
			memcpy(&timeout, &save_timeout, sizeof(struct timeval));
		}

		// select loop
		rset = evloop_select->allset;
		int n = select(evloop_select->nfds + 1, &rset, NULL, NULL, p_timeout);
		if (n > 0)
		{
			// reset fd_set and nfds
			evloop_select->nfds = 0;
			FD_ZERO(&evloop_select->allset);

			// handle wakeup
			muggle_evloop_select_handle_wakeup(evloop_select, &rset);

			// handle read and error
			for (node = muggle_linked_list_first(linked_list); node;)
			{
				muggle_event_context_t *ctx = (muggle_event_context_t*)node->data;
				if (FD_ISSET(ctx->fd, &rset) && evloop->cb_read)
				{
					evloop->cb_read(evloop, ctx);
				}

				if (ctx->flags & MUGGLE_EV_CTX_FLAG_CLOSED)
				{
					if (evloop->cb_close)
					{
						evloop->cb_close(evloop, ctx);
					}
					node = muggle_linked_list_remove(linked_list, node, NULL, NULL);
				}
				else
				{
					muggle_evloop_select_set_fd(evloop_select, ctx->fd);
					node = muggle_linked_list_next(evloop->ctx_list, node);
				}
			}
		}
		else if (n < 0)
		{
			muggle_evloop_exit(evloop);
		}

		// n == 0: timer trigger
		// n > 0: when loop is busy, timeout will not trigger, use customize timer handle avoid that
		if (n >= 0 && evloop->timeout >= 0)
		{
			muggle_evloop_handle_timer(evloop);
		}

		if (evloop->to_exit)
		{
			break;
		}
	}
}

int muggle_evloop_add_ctx_select(muggle_event_loop_t *evloop, muggle_event_context_t *ctx, void *node)
{
	muggle_event_loop_select_t *evloop_select = (muggle_event_loop_select_t*)evloop;
	muggle_evloop_select_set_fd(evloop_select, ctx->fd);
	return 0;
}
