#include "async_serv_handle.h"

static muggle_atomic_int s_task_id = 1;

struct async_args
{
	muggle_socket_context_t *ctx;
	int task_id;
};

typedef struct user_data
{
	char str_addr[MUGGLE_SOCKET_ADDR_STRLEN];
} user_data_t;

static void ctx_release(muggle_socket_context_t *ctx)
{
	user_data_t *user_data = muggle_socket_ctx_data(ctx);
	int ref_cnt = muggle_socket_ctx_ref_release(ctx);
	LOG_INFO("context[%s] release, reference count=%d", user_data->str_addr, ref_cnt);
	if (ref_cnt == 0)
	{
		user_data_t *user_data = muggle_socket_ctx_data(ctx);
		free(user_data);
		muggle_socket_ctx_close(ctx);
		free(ctx);
	}
}

muggle_thread_ret_t async_worker(void *arg)
{
	struct async_args *p = (struct async_args*)arg;
	muggle_socket_context_t *ctx = p->ctx;
	int task_id = (int)p->task_id;
	free(arg);

	user_data_t *user_data = muggle_socket_ctx_data(ctx);
	MUGGLE_LOG_INFO("context[%s] task[%d] start", user_data->str_addr, task_id);

	if (task_id % 3 == 0)
	{
		muggle_socket_ctx_shutdown(ctx);
		MUGGLE_LOG_INFO("close context[%s]", user_data->str_addr);
	}
	else
	{
		// do somthing
		muggle_msleep(3 * 1000);
		LOG_INFO("context[%s] task[%d] completed", user_data->str_addr, task_id);

		char buf[128];
		snprintf(buf, sizeof(buf), "task[%d] completed", task_id);
		muggle_socket_ctx_write(ctx, buf, strlen(buf));
	}

	// release ref count
	ctx_release(ctx);

	return 0;
}

static void tcp_on_accept(muggle_event_loop_t *evloop, muggle_socket_context_t *listen_ctx)
{
	do {
		struct sockaddr_storage addr;
		memset(&addr, 0, sizeof(addr));
		muggle_socklen_t addrlen = sizeof(addr);

		muggle_socket_t fd = accept(listen_ctx->base.fd, (struct sockaddr*)&addr, &addrlen);
		if (fd == MUGGLE_INVALID_SOCKET)
		{
			if (MUGGLE_SOCKET_LAST_ERRNO != MUGGLE_SYS_ERRNO_INTR &&
				MUGGLE_SOCKET_LAST_ERRNO != MUGGLE_SYS_ERRNO_WOULDBLOCK)
			{
				LOG_SYS_ERR(LOG_LEVEL_ERROR, "failed accept");
				muggle_evloop_exit(evloop);
			}
			break;
		}

		user_data_t *user_data = (user_data_t*)malloc(sizeof(user_data_t));
		memset(user_data, 0, sizeof(*user_data));
		muggle_socket_ntop((struct sockaddr*)&addr, user_data->str_addr, sizeof(user_data->str_addr), 0);

		LOG_INFO("on connection: %s", user_data->str_addr);

		muggle_socket_context_t *ctx =
			(muggle_socket_context_t*)malloc(sizeof(muggle_socket_context_t));
		muggle_socket_ctx_init(ctx, fd, user_data, MUGGLE_SOCKET_CTX_TYPE_TCP_CLIENT);

		int ret = muggle_evloop_add_ctx(evloop, (muggle_event_context_t*)ctx);
		if (ret != 0)
		{
			LOG_ERROR("failed add context: %s", user_data->str_addr);
			free(user_data);
			free(ctx);
			muggle_socket_close(fd);
		}
	} while(1);
}

static void tcp_on_message(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	user_data_t *user_data = muggle_socket_ctx_data(ctx);

	char buf[4096];
	while (muggle_socket_ctx_read(ctx, buf, sizeof(buf)) > 0)
	{
		// retain socket peer
		int ref_cnt = muggle_socket_ctx_ref_retain(ctx);
		if (ref_cnt < 0)
		{
			LOG_WARNING("failed retain peer");
			break;
		}
		LOG_INFO("context[%s] retain, reference count=%d", user_data->str_addr, ref_cnt);

		// run async thread
		struct async_args *args = (struct async_args*)malloc(sizeof(struct async_args));
		args->ctx = ctx;
		args->task_id = muggle_atomic_fetch_add(&s_task_id, 1, muggle_memory_order_relaxed);

		LOG_INFO("context[%s] task[%d] launch", user_data->str_addr, args->task_id);

		muggle_thread_t th;
		muggle_thread_create(&th, async_worker, (void*)args);
		muggle_thread_detach(&th);
	}
}

void on_read(muggle_event_loop_t *evloop, muggle_event_context_t *ctx)
{
	if (ctx == NULL)
	{
		LOG_ERROR("failed get socket context");
		return;
	}

	muggle_socket_context_t *socket_ctx = (muggle_socket_context_t*)ctx;
	switch (socket_ctx->sock_type)
	{
		case MUGGLE_SOCKET_CTX_TYPE_TCP_LISTEN:
		{
			tcp_on_accept(evloop, socket_ctx);
		}break;
		case MUGGLE_SOCKET_CTX_TYPE_TCP_CLIENT:
		{
			tcp_on_message(evloop, socket_ctx);
		}break;
	}
}

void on_close(muggle_event_loop_t *evloop, muggle_event_context_t *ctx)
{
	LOG_INFO("on disconnection");
	ctx_release((muggle_socket_context_t*)ctx);
}
