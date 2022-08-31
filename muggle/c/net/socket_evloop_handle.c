#include "socket_evloop_handle.h"
#include <stdlib.h>
#include <string.h>
#include "muggle/c/log/log.h"
#include "muggle/c/os/sys.h"

//--------------------------------------------------
// default socket event loop handle callbacks
//--------------------------------------------------
static muggle_socket_t muggle_socket_evloop_on_accept(muggle_socket_context_t *ctx)
{
	muggle_socket_t fd = MUGGLE_INVALID_SOCKET;
	do {
		fd = accept(ctx->base.fd, NULL, NULL);
		if (fd == MUGGLE_INVALID_SOCKET)
		{
			if (MUGGLE_SOCKET_LAST_ERRNO == MUGGLE_SYS_ERRNO_INTR)
			{
				continue;
			}
			else if (MUGGLE_SOCKET_LAST_ERRNO == MUGGLE_SYS_ERRNO_WOULDBLOCK)
			{
				break;
			}
			else
			{
				MUGGLE_LOG_SYS_ERR(MUGGLE_LOG_LEVEL_ERROR, "failed accept");
				muggle_socket_ctx_set_flag(ctx, MUGGLE_EV_CTX_FLAG_CLOSED);
				break;
			}
		}
		else
		{
			break;
		}
	} while(1);

	return fd;
}

muggle_socket_context_t* muggle_socket_evloop_handle_alloc(void *pool)
{
	return (muggle_socket_context_t*)malloc(sizeof(muggle_socket_context_t));
}
void muggle_socket_evloop_handle_free(void *pool, muggle_socket_context_t *data)
{
	free(data);
}

static void muggle_socket_evloop_release_ctx(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	muggle_socket_evloop_handle_t *handle = (muggle_socket_evloop_handle_t*)evloop->sys_data;
	if (muggle_socket_ctx_ref_release(ctx) == 0)
	{
		if (handle->cb_release)
		{
			handle->cb_release(evloop, ctx);
		}

		muggle_socket_ctx_close(ctx);

		handle->cb_free(handle->mempool, ctx);
	}
}

//--------------------------------------------------
// event loop callbacks
//--------------------------------------------------
static void muggle_socket_evloop_on_read(muggle_event_loop_t *evloop, muggle_event_context_t *ctx)
{
	if (ctx == NULL)
	{
		return;
	}

	muggle_socket_evloop_handle_t *handle = (muggle_socket_evloop_handle_t*)evloop->sys_data;
	muggle_socket_context_t *socket_ctx = (muggle_socket_context_t*)ctx;
	switch (socket_ctx->sock_type)
	{
		case MUGGLE_SOCKET_CTX_TYPE_TCP_LISTEN:
		{
			muggle_socket_t fd = muggle_socket_evloop_on_accept(socket_ctx);
			if (fd != MUGGLE_INVALID_SOCKET)
			{
				muggle_socket_context_t *new_ctx = handle->cb_alloc(handle->mempool);
				if (new_ctx == NULL)
				{
					muggle_socket_close(fd);
					return;
				}
				muggle_socket_ctx_init(new_ctx, fd, NULL, MUGGLE_SOCKET_CTX_TYPE_TCP_CLIENT);

				int ret = muggle_evloop_add_ctx(evloop, (muggle_event_context_t*)new_ctx);
				if (ret != 0)
				{
					handle->cb_free(handle->mempool, new_ctx);
					muggle_socket_close(fd);
					return;
				}

				if (handle->cb_conn)
				{
					handle->cb_conn(evloop, new_ctx);
				}
			}
		}break;
		default:
		{
			if (handle->cb_msg)
			{
				handle->cb_msg(evloop, socket_ctx);
			}
			else
			{
				char buf[1024];
				while (muggle_socket_ctx_read(ctx, buf, sizeof(buf)) > 0);
			}
		}break;
	}
}

static void muggle_socket_evloop_on_close(muggle_event_loop_t *evloop, muggle_event_context_t *ctx)
{
	muggle_socket_evloop_handle_t *handle = (muggle_socket_evloop_handle_t*)evloop->sys_data;
	if (handle->cb_close)
	{
		handle->cb_close(evloop, (muggle_socket_context_t*)ctx);
	}
	muggle_socket_evloop_release_ctx(evloop, (muggle_socket_context_t*)ctx);
}

static void muggle_socket_evloop_on_wake(muggle_event_loop_t *evloop)
{
	muggle_socket_evloop_handle_t *handle = (muggle_socket_evloop_handle_t*)evloop->sys_data;

	// handle async add ctx
	muggle_mutex_lock(handle->mtx);

	while (muggle_queue_size(handle->ctx_queue) > 0)
	{
		muggle_queue_node_t *node = muggle_queue_front(handle->ctx_queue);

		muggle_socket_context_t *ctx = (muggle_socket_context_t*)node->data;

		muggle_evloop_add_ctx(evloop, (muggle_event_context_t*)ctx);
		if (handle->cb_add_ctx)
		{
			handle->cb_add_ctx(evloop, ctx);
		}

		muggle_queue_dequeue(handle->ctx_queue, NULL, NULL);
	}

	muggle_mutex_unlock(handle->mtx);

	if (handle->cb_wake)
	{
		handle->cb_wake(evloop);
	}
}

static void muggle_socket_evloop_on_timer(muggle_event_loop_t *evloop)
{
	muggle_socket_evloop_handle_t *handle = (muggle_socket_evloop_handle_t*)evloop->sys_data;
	if (handle->cb_timer)
	{
		handle->cb_timer(evloop);
	}
}

static void muggle_socket_evloop_on_clear(muggle_event_loop_t *evloop, muggle_event_context_t *ctx)
{
	muggle_socket_evloop_release_ctx(evloop, (muggle_socket_context_t*)ctx);
}

static void muggle_socket_evloop_on_exit(muggle_event_loop_t *evloop)
{
	muggle_socket_evloop_handle_t *handle = (muggle_socket_evloop_handle_t*)evloop->sys_data;

	muggle_mutex_lock(handle->mtx);

	while (muggle_queue_size(handle->ctx_queue) > 0)
	{
		muggle_queue_node_t *node = muggle_queue_front(handle->ctx_queue);
		muggle_socket_context_t *ctx = (muggle_socket_context_t*)node->data;
		muggle_socket_evloop_release_ctx(evloop, ctx);
		muggle_queue_dequeue(handle->ctx_queue, NULL, NULL);
	}

	muggle_mutex_unlock(handle->mtx);
}

int muggle_socket_evloop_handle_init(muggle_socket_evloop_handle_t *handle)
{
	memset(handle, 0, sizeof(*handle));

	// timeout
	handle->timeout = -1;

	// init context queue
	handle->ctx_queue = (muggle_queue_t*)malloc(sizeof(muggle_queue_t));
	if (handle->ctx_queue == NULL)
	{
		goto muggle_socket_evloop_handle_init_except;
	}

	if (!muggle_queue_init(handle->ctx_queue, 0))
	{
		free(handle->ctx_queue);
		handle->ctx_queue = NULL;
		goto muggle_socket_evloop_handle_init_except;
	}

	// init mutex
	handle->mtx = (muggle_mutex_t*)malloc(sizeof(muggle_mutex_t));
	if (handle->mtx == NULL)
	{
		goto muggle_socket_evloop_handle_init_except;
	}

	if (muggle_mutex_init(handle->mtx) != 0)
	{
		free(handle->mtx);
		handle->mtx = NULL;
		goto muggle_socket_evloop_handle_init_except;
	}

	// set default alloc and free
	handle->cb_alloc = muggle_socket_evloop_handle_alloc;
	handle->cb_free = muggle_socket_evloop_handle_free;

	return 0;

muggle_socket_evloop_handle_init_except:
	muggle_socket_evloop_handle_destroy(handle);
	return -1;
}

void muggle_socket_evloop_handle_destroy(muggle_socket_evloop_handle_t *handle)
{
	if (handle->mtx)
	{
		muggle_mutex_destroy(handle->mtx);
		free(handle->mtx);
		handle->mtx = NULL;
	}

	if (handle->ctx_queue)
	{
		muggle_queue_destroy(handle->ctx_queue, NULL, NULL);
		free(handle->ctx_queue);
		handle->ctx_queue = NULL;
	}
}

void muggle_socket_evloop_handle_attach(
	muggle_socket_evloop_handle_t *handle,
	muggle_event_loop_t *evloop)
{
	evloop->sys_data = (void*)handle;
	muggle_evloop_set_timer_interval(evloop, handle->timeout);
	muggle_evloop_set_cb_read(evloop, muggle_socket_evloop_on_read);
	muggle_evloop_set_cb_close(evloop, muggle_socket_evloop_on_close);
	muggle_evloop_set_cb_wake(evloop, muggle_socket_evloop_on_wake);
	muggle_evloop_set_cb_timer(evloop, muggle_socket_evloop_on_timer);
	muggle_evloop_set_cb_clear(evloop, muggle_socket_evloop_on_clear);
	muggle_evloop_set_cb_exit(evloop, muggle_socket_evloop_on_exit);
}

void muggle_socket_evloop_add_ctx(
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx)
{
	muggle_socket_evloop_handle_t *handle = (muggle_socket_evloop_handle_t*)evloop->sys_data;
	muggle_mutex_lock(handle->mtx);
	muggle_queue_enqueue(handle->ctx_queue, ctx);
	muggle_mutex_unlock(handle->mtx);

	muggle_evloop_wakeup(evloop);
}

void muggle_socket_evloop_handle_set_timer_interval(
	muggle_socket_evloop_handle_t *handle,
	int timeout)
{
	handle->timeout = timeout;
}

void muggle_socket_evloop_handle_set_cb_conn(
	muggle_socket_evloop_handle_t *handle,
	fn_muggle_socket_evloop_cb1 cb)
{
	handle->cb_conn = cb;
}

void muggle_socket_evloop_handle_set_cb_msg(
	muggle_socket_evloop_handle_t *handle,
	fn_muggle_socket_evloop_cb1 cb)
{
	handle->cb_msg = cb;
}

void muggle_socket_evloop_handle_set_cb_close(
	muggle_socket_evloop_handle_t *handle,
	fn_muggle_socket_evloop_cb1 cb)
{
	handle->cb_close = cb;
}

void muggle_socket_evloop_handle_set_cb_release(
	muggle_socket_evloop_handle_t *handle,
	fn_muggle_socket_evloop_cb1 cb)
{
	handle->cb_release = cb;
}

void muggle_socket_evloop_handle_set_alloc_free(
	muggle_socket_evloop_handle_t *handle,
	void *mempool,
	fn_muggle_socket_evloop_alloc cb_alloc,
	fn_muggle_socket_evloop_free cb_free)
{
	handle->mempool = mempool;
	handle->cb_alloc = cb_alloc;
	handle->cb_free = cb_free;
}

void muggle_socket_evloop_handle_set_cb_add_ctx(
	muggle_socket_evloop_handle_t *handle,
	fn_muggle_socket_evloop_cb1 cb)
{
	handle->cb_add_ctx = cb;
}

void muggle_socket_evloop_handle_set_cb_wake(
	muggle_socket_evloop_handle_t *handle,
	fn_muggle_socket_evloop_cb2 cb)
{
	handle->cb_wake = cb;
}

void muggle_socket_evloop_handle_set_cb_timer(
	muggle_socket_evloop_handle_t *handle,
	fn_muggle_socket_evloop_cb2 cb)
{
	handle->cb_timer = cb;
}
