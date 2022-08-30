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
	user_data_t *user_data = muggle_socket_ctx_get_data(ctx);
	int ref_cnt = muggle_socket_ctx_ref_release(ctx);
	LOG_INFO("context[%s] release, ref_cnt=%d", user_data->str_addr, ref_cnt);
	if (ref_cnt == 0)
	{
		LOG_INFO("context[%s] release", user_data->str_addr);
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

	user_data_t *user_data = muggle_socket_ctx_get_data(ctx);
	MUGGLE_LOG_INFO("context[%s] task start, task_id=%d", user_data->str_addr, task_id);

	if (task_id % 3 == 0)
	{
		muggle_socket_ctx_shutdown(ctx);
		MUGGLE_LOG_INFO("context[%s] shutdown", user_data->str_addr);
	}
	else
	{
		// do somthing
		muggle_msleep(3 * 1000);
		LOG_INFO("context[%s] task completed, task_id=%d", user_data->str_addr, task_id);

		char buf[128];
		snprintf(buf, sizeof(buf), "task[%d] completed", task_id);
		muggle_socket_ctx_write(ctx, buf, strlen(buf));
	}

	// release ref count
	ctx_release(ctx);

	return 0;
}

void on_add_ctx(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	if (ctx->sock_type == MUGGLE_SOCKET_CTX_TYPE_TCP_LISTEN)
	{
		LOG_INFO("on add context[listen]");
	}
}

void on_connect(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	// output connection remote address
	user_data_t *user_data = (user_data_t*)malloc(sizeof(user_data_t));
	memset(user_data, 0, sizeof(*user_data));
	muggle_socket_ctx_get_fd(ctx);
	muggle_socket_remote_addr(
		muggle_socket_ctx_get_fd(ctx),
		user_data->str_addr, sizeof(user_data->str_addr), 0);
	LOG_INFO("context[%s] on connection", user_data->str_addr);

	// set user data
	muggle_socket_ctx_set_data(ctx, user_data);
}
void on_message(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	user_data_t *user_data = muggle_socket_ctx_get_data(ctx);

	char buf[4096];
	while (muggle_socket_ctx_read(ctx, buf, sizeof(buf)) > 0)
	{
		if (strncmp(buf, "exit", strlen("exit")) == 0)
		{
			LOG_INFO("context[%s] close server event loop", user_data->str_addr);
			muggle_evloop_exit(evloop);
			break;
		}

		// retain socket peer
		int ref_cnt = muggle_socket_ctx_ref_retain(ctx);
		if (ref_cnt < 0)
		{
			LOG_WARNING("failed retain peer");
			break;
		}
		LOG_INFO("context[%s] retain, ref_cnt=%d", user_data->str_addr, ref_cnt);

		// run async thread
		struct async_args *args = (struct async_args*)malloc(sizeof(struct async_args));
		args->ctx = ctx;
		args->task_id = muggle_atomic_fetch_add(&s_task_id, 1, muggle_memory_order_relaxed);

		LOG_INFO("context[%s] launch task, task_id=%d", user_data->str_addr, args->task_id);
		muggle_thread_t th;
		muggle_thread_create(&th, async_worker, (void*)args);
		muggle_thread_detach(&th);
	}
}
void on_close(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	user_data_t *user_data = muggle_socket_ctx_get_data(ctx);
	LOG_INFO("context[%s] disconnection", user_data->str_addr);
}
void on_release(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	if (ctx->sock_type == MUGGLE_SOCKET_CTX_TYPE_TCP_CLIENT)
	{
		user_data_t *user_data = muggle_socket_ctx_get_data(ctx);
		LOG_INFO("context[%s] release", user_data->str_addr);
		free(user_data);
	}
	else if (ctx->sock_type == MUGGLE_SOCKET_CTX_TYPE_TCP_LISTEN)
	{
		LOG_INFO("context[listen] release");
	}
}
