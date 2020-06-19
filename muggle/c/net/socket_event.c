/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "socket_event.h"
#include <string.h>
#include "muggle/c/log/log.h"
#include "event/socket_event_select.h"
#include "event/socket_event_poll.h"
#include "event/socket_event_epoll.h"

static int muggle_get_event_loop_type(int event_loop_type)
{
#if !MUGGLE_PLATFORM_LINUX
	if (event_loop_type == MUGGLE_SOCKET_EVENT_LOOP_TYPE_EPOLL)
	{
		event_loop_type = MUGGLE_SOCKET_EVENT_LOOP_TYPE_NULL;
	}
#endif

#if !MUGGLE_PLATFORM_WINDOWS
	if (event_loop_type == MUGGLE_SOCKET_EVENT_LOOP_TYPE_IOCP)
	{
		event_loop_type = MUGGLE_SOCKET_EVENT_LOOP_TYPE_NULL;
	}
#endif

#if !MUGGLE_PLATFORM_FREEBSD
	if (event_loop_type == MUGGLE_SOCKET_EVENT_LOOP_TYPE_KQUEUE)
	{
		event_loop_type = MUGGLE_SOCKET_EVENT_LOOP_TYPE_NULL;
	}
#endif

	if (event_loop_type <= MUGGLE_SOCKET_EVENT_LOOP_TYPE_NULL ||
		event_loop_type >= MUGGLE_SOCKET_EVENT_LOOP_TYPE_MAX)
	{
#if MUGGLE_PLATFORM_LINUX
		event_loop_type = MUGGLE_SOCKET_EVENT_LOOP_TYPE_EPOLL;
#elif MUGGLE_PLATFORM_WINDOWS
		// event_loop_type = MUGGLE_SOCKET_EVENT_LOOP_TYPE_IOCP;
		event_loop_type = MUGGLE_SOCKET_EVENT_LOOP_TYPE_SELECT;
#elif MUGGLE_PLATFORM_FREEBSD
		// event_loop_type = MUGGLE_SOCKET_EVENT_LOOP_TYPE_KQUEUE;
		event_loop_type = MUGGLE_SOCKET_EVENT_LOOP_TYPE_SELECT;
#else
		event_loop_type = MUGGLE_SOCKET_EVENT_LOOP_TYPE_SELECT;
#endif
	}

	return event_loop_type;
}

/*
 * init socket event input arguments
 * RETURN: 0 - success, otherwise failed
 * */
static int muggle_socket_ev_arg_init(muggle_socket_event_t *ev, muggle_socket_event_init_arg_t *ev_init_arg)
{
	memset(ev, 0, sizeof(ev));

	// set event loop type
	ev->ev_loop_type = muggle_get_event_loop_type(ev_init_arg->ev_loop_type);

	// set capacity
	int capacity = ev_init_arg->hints_max_peer;
	if (ev->ev_loop_type == MUGGLE_SOCKET_EVENT_LOOP_TYPE_SELECT)
	{
		if (capacity <= 0 || capacity > FD_SETSIZE)
		{
			capacity = FD_SETSIZE;
		}
	}
	else
	{
		if (capacity <= 0)
		{
			capacity = 1024;
		}
	}
	if (capacity < ev_init_arg->cnt_peer)
	{
		MUGGLE_LOG_ERROR("capacity space not enough for all peers");
		for (int i = 0; i < ev_init_arg->cnt_peer; ++i)
		{
			muggle_socket_close(ev_init_arg[i].peers->fd);
		}
		return -1;
	}
	ev->capacity = capacity;

	// set timeout
	if (ev_init_arg->timeout_ms < 0)
	{
		ev->timeout_ms = -1;
	}
	else
	{
		ev->timeout_ms = ev_init_arg->timeout_ms;
	}

	ev->to_exit = 0;
	ev->datas = ev_init_arg->datas;

	// set callbacks
	ev->on_connect = ev_init_arg->on_connect;
	ev->on_error = ev_init_arg->on_error;
	ev->on_message = ev_init_arg->on_message;
	ev->on_timer = ev_init_arg->on_timer;

	return 0;
}

/*
 * event loop run
 * */
static int muggle_socket_event_loop_run(muggle_socket_event_t *ev, muggle_socket_event_init_arg_t *ev_init_arg)
{
	int ret = 0;
	switch (ev->ev_loop_type)
	{
	case MUGGLE_SOCKET_EVENT_LOOP_TYPE_MULTHREAD:
	{
		MUGGLE_LOG_ERROR("unimplemented event loop type: multhread, to be continued...");
		ret = -1;
	}break;
	case MUGGLE_SOCKET_EVENT_LOOP_TYPE_SELECT:
	{
		muggle_socket_event_select(ev, ev_init_arg);
	}break;
	case MUGGLE_SOCKET_EVENT_LOOP_TYPE_POLL:
	{
		muggle_socket_event_poll(ev, ev_init_arg);
	}break;
	case MUGGLE_SOCKET_EVENT_LOOP_TYPE_EPOLL:
	{
#if MUGGLE_PLATFORM_LINUX
		muggle_socket_event_epoll(ev, ev_init_arg);
#else
		MUGGLE_LOG_ERROR("epoll event loop support linux only");
		ret = -1;
#endif
	}break;
	case MUGGLE_SOCKET_EVENT_LOOP_TYPE_IOCP:
	{
#if MUGGLE_PLATFORM_WINDOWS
		MUGGLE_LOG_ERROR("unimplemented event loop type: iocp, to be continued...");
		ret = -1;
#else
		MUGGLE_LOG_ERROR("iocp event loop support windows only");
		ret = -1;
#endif
	}break;
	case MUGGLE_SOCKET_EVENT_LOOP_TYPE_KQUEUE:
	{
#if MUGGLE_PLATFORM_FREEBSD
		MUGGLE_LOG_ERROR("unimplemented event loop type: kqueue, to be continued...");
		ret = -1;
#else
		MUGGLE_LOG_ERROR("kqueue event loop support FreeBSD only");
		ret = -1;
#endif
	}break;
	default:
	{
		MUGGLE_LOG_ERROR("unsupport event loop type: %d", ev->ev_loop_type);
		ret = -1;
	}break;
	}

	return ret;
}

int muggle_socket_event_loop(muggle_socket_event_init_arg_t *ev_init_arg)
{
	muggle_socket_event_t ev;
	int ret = muggle_socket_ev_arg_init(&ev, ev_init_arg);
	if (ret != 0)
	{
		MUGGLE_LOG_ERROR("failed init socket event loop");
		for (int i = 0; i < ev_init_arg->cnt_peer; ++i)
		{
			muggle_socket_close(ev_init_arg->peers[i].fd);
		}
		return ret;
	}

	return muggle_socket_event_loop_run(&ev, ev_init_arg);
}

void muggle_socket_event_loop_exit(muggle_socket_event_t *ev)
{
	ev->to_exit = 1;
}
