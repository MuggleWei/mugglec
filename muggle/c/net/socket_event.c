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
#else
		event_loop_type = MUGGLE_SOCKET_EVENT_LOOP_TYPE_SELECT;
#endif
	}

	// TODO: Only support linux and windows for the time being
	if (event_loop_type == MUGGLE_SOCKET_EVENT_LOOP_TYPE_KQUEUE)
	{
		event_loop_type = MUGGLE_SOCKET_EVENT_LOOP_TYPE_SELECT;
	}

	return event_loop_type;
}

void muggle_socket_event_loop(muggle_socket_ev_arg_t *ev_arg)
{
	muggle_socket_event_t ev;
	memset(&ev, 0, sizeof(ev));
	ev.ev_loop_type = muggle_get_event_loop_type(ev_arg->ev_loop_type);
	if (ev_arg->timeout_ms < 0)
	{
		ev.timeout_ms = -1;
	}
	else
	{
		ev.timeout_ms = ev_arg->timeout_ms;
	}
	ev.datas = ev_arg->datas;

	ev.on_connect = ev_arg->on_connect;
	ev.on_error = ev_arg->on_error;
	ev.on_message = ev_arg->on_message;
	ev.on_timer = ev_arg->on_timer;

	switch (ev.ev_loop_type)
	{
	case MUGGLE_SOCKET_EVENT_LOOP_TYPE_MULTHREAD:
		{
			// TODO:
			MUGGLE_ERROR("to be continued...");
		}break;
	case MUGGLE_SOCKET_EVENT_LOOP_TYPE_SELECT:
		{
			muggle_socket_event_select(&ev, ev_arg);
		}break;
	case MUGGLE_SOCKET_EVENT_LOOP_TYPE_POLL:
		{
			muggle_socket_event_poll(&ev, ev_arg);
		}break;
	case MUGGLE_SOCKET_EVENT_LOOP_TYPE_EPOLL:
		{
			muggle_socket_event_epoll(&ev, ev_arg);
		}break;
	case MUGGLE_SOCKET_EVENT_LOOP_TYPE_KQUEUE:
		{
			MUGGLE_ERROR("unsupport event loop type: kqueue");
		}break;
	default:
		{
			MUGGLE_ERROR("unsupport event loop type: %d", ev.ev_loop_type);
		}break;
	}
}
