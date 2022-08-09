#include "event_loop.h"
#include <stdlib.h>
#include <string.h>
#include "muggle/c/event/internal/event_loop_select.h"
#include "muggle/c/event/internal/event_loop_poll.h"
#include "muggle/c/event/internal/event_loop_epoll.h"

struct muggle_evloop_fn
{
	fn_muggle_evloop_init fn_init;
	fn_muggle_evloop_destroy fn_destroy;
	fn_muggle_evloop_run fn_run;
	fn_muggle_evloop_add_ctx fn_add_ctx;
};
static struct muggle_evloop_fn s_evloop_fn[] = {
	{NULL, NULL, NULL, NULL},
	// select
	{
		muggle_evloop_init_select,
		muggle_evloop_destroy_select,
		muggle_evloop_run_select,
		muggle_evloop_add_ctx_select
	},
	// poll
	{
		muggle_evloop_init_poll,
		muggle_evloop_destroy_poll,
		muggle_evloop_run_poll,
		muggle_evloop_add_ctx_poll
	},
	// epoll
#if MUGGLE_PLATFORM_LINUX
	{
		muggle_evloop_init_epoll,
		muggle_evloop_destroy_epoll,
		muggle_evloop_run_epoll,
		muggle_evloop_add_ctx_epoll
	}
#else
	{NULL, NULL, NULL, NULL}
#endif
};

void muggle_evloop_handle_timer(muggle_event_loop_t *evloop)
{
	if (evloop->cb_timer == NULL)
	{
		return;
	}

	struct timespec curr_ts;
	timespec_get(&curr_ts, TIME_UTC);
	int interval_ms = 
		(int)((curr_ts.tv_sec - evloop->last_ts.tv_sec) * 1000 +
		(curr_ts.tv_nsec - evloop->last_ts.tv_nsec) / 1000000);
	if (interval_ms >= evloop->timeout)
	{
		evloop->cb_timer(evloop);
		evloop->last_ts.tv_sec = curr_ts.tv_sec;
		evloop->last_ts.tv_nsec = curr_ts.tv_nsec;
	}
}

/**
 * @brief initialize event loop
 *
 * @param evloop  event loop
 * @param args    event loop initialize arguments
 *
 * @return 
 *     0 - success
 *     otherwise - failed init eventloop
 */
static int muggle_evloop_init(muggle_event_loop_t *evloop, muggle_event_loop_init_args_t *args);

/**
 * @brief destroy event loop
 *
 * @param evloop  event loop
 */
static void muggle_evloop_destroy(muggle_event_loop_t *evloop);

static int muggle_evloop_init(muggle_event_loop_t *evloop, muggle_event_loop_init_args_t *args)
{
	// don't memset here, cause muggle_evloop_new already set
	// memset(evloop, 0, sizeof(*evloop));

	if (args->hints_max_fd < 1)
	{
		args->hints_max_fd = 8;
	}

	// initialize linked list
	size_t capacity = 0;
	if (args->use_mem_pool)
	{
		capacity = args->hints_max_fd;
	}

	evloop->ctx_list = (muggle_linked_list_t*)malloc(sizeof(muggle_linked_list_t));
	if (evloop->ctx_list == NULL)
	{
		goto muggle_evloop_init_except;
	}

	if (!muggle_linked_list_init(evloop->ctx_list, capacity))
	{
		free(evloop->ctx_list);
		evloop->ctx_list = NULL;
		goto muggle_evloop_init_except;
	}

	// initialize event signal
	evloop->ev_signal = (muggle_event_signal_t*)malloc(sizeof(muggle_event_signal_t));
	if (evloop->ev_signal == NULL)
	{
		goto muggle_evloop_init_except;
	}

	if (muggle_ev_signal_init(evloop->ev_signal) != 0)
	{
		free(evloop->ev_signal);
		evloop->ev_signal = NULL;
		goto muggle_evloop_init_except;
	}

	// before run, temporarily set thread
	evloop->tid = muggle_thread_current_id();

	// without timeout by default
	evloop->timeout = -1;

	return 0;

muggle_evloop_init_except:
	muggle_evloop_destroy(evloop);
	return -1;
}

static void muggle_evloop_destroy(muggle_event_loop_t *evloop)
{
	if (evloop->ev_signal)
	{
		muggle_ev_signal_destroy(evloop->ev_signal);
		free(evloop->ev_signal);
		evloop->ev_signal = NULL;
	}

	if (evloop->ctx_list)
	{
		muggle_linked_list_destroy(evloop->ctx_list, NULL, NULL);
		free(evloop->ctx_list);
		evloop->ctx_list= NULL;
	}
}

static int muggle_evloop_get_type(muggle_event_loop_init_args_t *args)
{
	int evloop_type = args->evloop_type;
#if !MUGGLE_PLATFORM_LINUX
	if (evloop_type == MUGGLE_EVLOOP_TYPE_EPOLL)
	{
		evloop_type = MUGGLE_EVLOOP_TYPE_NULL;
	}
#endif

#if !MUGGLE_PLATFORM_FREEBSD
	if (evloop_type == MUGGLE_EVLOOP_TYPE_KQUEUE)
	{
		evloop_type = MUGGLE_EVLOOP_TYPE_NULL;
	}
#endif

	if (evloop_type <= MUGGLE_EVLOOP_TYPE_NULL ||
		evloop_type >= MUGGLE_MAX_EVLOOP_TYPE)
	{
#if MUGGLE_PLATFORM_LINUX
		evloop_type = MUGGLE_EVLOOP_TYPE_EPOLL;
#elif MUGGLE_PLATFORM_WINDOWS
		evloop_type = MUGGLE_EVLOOP_TYPE_POLL;
#elif MUGGLE_PLATFORM_FREEBSD
		// evloop_type = MUGGLE_EVLOOP_TYPE_KQUEUE;
		evloop_type = MUGGLE_EVLOOP_TYPE_SELECT;
#else
		evloop_type = MUGGLE_EVLOOP_TYPE_SELECT;
#endif
	}

	return evloop_type;
}

muggle_event_loop_t* muggle_evloop_new(muggle_event_loop_init_args_t *args)
{
	args->evloop_type = muggle_evloop_get_type(args);

	muggle_event_loop_t *evloop = NULL;
	switch (args->evloop_type)
	{
		case MUGGLE_EVLOOP_TYPE_SELECT:
		{
			evloop = (muggle_event_loop_t*)malloc(sizeof(muggle_event_loop_select_t));
			if (evloop == NULL)
			{
				return NULL;
			}
			memset(evloop, 0, sizeof(muggle_event_loop_select_t));
		}break;
		case MUGGLE_EVLOOP_TYPE_POLL:
		{
			evloop = (muggle_event_loop_t*)malloc(sizeof(muggle_event_loop_poll_t));
			if (evloop == NULL)
			{
				return NULL;
			}
			memset(evloop, 0, sizeof(muggle_event_loop_poll_t));
		}break;
		case MUGGLE_EVLOOP_TYPE_EPOLL:
		{
#if MUGGLE_PLATFORM_LINUX
			evloop = (muggle_event_loop_t*)malloc(sizeof(muggle_event_loop_epoll_t));
			if (evloop == NULL)
			{
				return NULL;
			}
			memset(evloop, 0, sizeof(muggle_event_loop_epoll_t));
#else
			return NULL;
#endif
		}break;
		default:
		{
			return NULL;
		}break;
	}
	evloop->evloop_type = args->evloop_type;

	int ret = muggle_evloop_init(evloop, args);
	if (ret != 0)
	{
		return NULL;
	}

	if (s_evloop_fn[evloop->evloop_type].fn_init(evloop, args) != 0)
	{
		muggle_evloop_destroy(evloop);
		free(evloop);
		return NULL;
	}

	return evloop;
}

void muggle_evloop_delete(muggle_event_loop_t *evloop)
{
	if (evloop)
	{
		s_evloop_fn[evloop->evloop_type].fn_destroy(evloop);
		muggle_evloop_destroy(evloop);
		free(evloop);
	}
}

void muggle_evloop_set_timer_interval(muggle_event_loop_t *evloop, int timeout)
{
	evloop->timeout = timeout;
}

void muggle_evloop_set_cb_read(muggle_event_loop_t *evloop, fn_muggle_evloop_cb1 cb)
{
	evloop->cb_read = cb;
}

void muggle_evloop_set_cb_close(muggle_event_loop_t *evloop, fn_muggle_evloop_cb1 cb)
{
	evloop->cb_close = cb;
}

void muggle_evloop_set_cb_wake(muggle_event_loop_t *evloop, fn_muggle_evloop_cb2 cb)
{
	evloop->cb_wake = cb;
}

void muggle_evloop_set_cb_timer(muggle_event_loop_t *evloop, fn_muggle_evloop_cb2 cb)
{
	evloop->cb_timer = cb;
}

void muggle_evloop_set_cb_clear(muggle_event_loop_t *evloop, fn_muggle_evloop_cb1 cb)
{
	evloop->cb_clear = cb;
}

void muggle_evloop_set_data(muggle_event_loop_t *evloop, void *data)
{
	evloop->user_data = data;
}

void* muggle_evloop_get_data(muggle_event_loop_t *evloop)
{
	return evloop->user_data;
}

int muggle_evloop_wakeup(muggle_event_loop_t *evloop)
{
	return muggle_ev_signal_wakeup(evloop->ev_signal);
}

void muggle_evloop_exit(muggle_event_loop_t *evloop)
{
	evloop->to_exit = 1;
	if (!muggle_thread_equal(evloop->tid, muggle_thread_current_id()))
	{
		muggle_evloop_wakeup(evloop);
	}
}

int muggle_evloop_add_ctx(muggle_event_loop_t *evloop, muggle_event_context_t *ctx)
{
	// only support add context in the same thread of event loop run
	if (!muggle_thread_equal(evloop->tid, muggle_thread_current_id()))
	{
		return -1;
	}

	// all fd in event loop need set non-blocking
	if (muggle_ev_fd_set_nonblock(ctx->fd, 1) != 0)
	{
		return -1;
	}

	muggle_linked_list_node_t *node = muggle_linked_list_append(evloop->ctx_list, NULL, ctx);
	if (node == NULL)
	{
		return -1;
	}

	int ret = s_evloop_fn[evloop->evloop_type].fn_add_ctx(evloop, ctx, node);
	if (ret != 0)
	{
		muggle_linked_list_remove(evloop->ctx_list, node, NULL, NULL);
		return -1;
	}

	return 0;
}

void muggle_evloop_run(muggle_event_loop_t *evloop)
{
	// get thread id
	evloop->tid = muggle_thread_current_id();

	// reset timer last tick
	timespec_get(&evloop->last_ts, TIME_UTC);

	// run
	s_evloop_fn[evloop->evloop_type].fn_run(evloop);

	// clear
	if (evloop->cb_clear)
	{
		muggle_linked_list_t *linked_list = evloop->ctx_list;
		muggle_linked_list_node_t *node = muggle_linked_list_first(linked_list);
		for (; node; node = muggle_linked_list_next(linked_list, node))
		{
			muggle_event_context_t *ctx = (muggle_event_context_t*)node->data;
			evloop->cb_clear(evloop, ctx);
		}
	}
}
