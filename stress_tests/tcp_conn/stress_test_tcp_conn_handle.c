#include "stress_test_tcp_conn_handle.h"

#define MAX_MSG_SIZE 65536

muggle_thread_ret_t conn_thread_routine(void *p_args)
{
	stress_test_tcp_conn_args_t *args = (stress_test_tcp_conn_args_t *)p_args;
	for (int i = 0; i < args->total_conns; ++i) {
		muggle_socket_t fd = MUGGLE_INVALID_SOCKET;
		if (1) {
			fd = muggle_tcp_connect(args->host, args->serv, 3);
		} else {
			fd = muggle_tcp_bind_connect("127.0.0.1", NULL, args->host,
										 args->serv, 3);
		}

		if (fd == MUGGLE_INVALID_SOCKET) {
			LOG_ERROR("failed create socket: %s %s", args->host, args->serv);
			muggle_msleep(3000);
			continue;
		}

		// create socket context
		muggle_socket_context_t *ctx =
			(muggle_socket_context_t *)malloc(sizeof(muggle_socket_context_t));
		muggle_bytes_buffer_t *bytes_buf =
			(muggle_bytes_buffer_t *)malloc(sizeof(muggle_bytes_buffer_t));
		muggle_bytes_buffer_init(bytes_buf, 4 * 1024 * 1024);
		muggle_socket_ctx_init(ctx, fd, bytes_buf,
							   MUGGLE_SOCKET_CTX_TYPE_TCP_CLIENT);

		// set socket context index
		muggle_socket_ctx_set_data(ctx, (void *)(intptr_t)i);

		// add context to event loop
		muggle_socket_evloop_add_ctx(args->evloop, ctx);
	}

	return 0;
}

void run_conn_thread(stress_test_tcp_conn_args_t *args)
{
	muggle_thread_t th;
	muggle_thread_create(&th, conn_thread_routine, args);
	muggle_thread_detach(&th);
}

void on_add_ctx(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	MUGGLE_UNUSED(evloop);

	char ip[128];
	int port;
	muggle_socket_local_ip_port(ctx->base.fd, ip, sizeof(ip), &port);

	LOG_INFO("socket context ready, idx=%d, local_addr=%s:%d",
			 (int)(intptr_t)muggle_socket_ctx_get_data(ctx), ip, port);
}

void on_message(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	MUGGLE_UNUSED(evloop);
	MUGGLE_UNUSED(ctx);
}

void on_close(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	MUGGLE_UNUSED(evloop);
	LOG_INFO("socket context closed, idx=%d",
			 (int)(intptr_t)muggle_socket_ctx_get_data(ctx));
}

void on_release(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	MUGGLE_UNUSED(evloop);
	LOG_INFO("socket context release, idx=%d",
			 (int)(intptr_t)muggle_socket_ctx_get_data(ctx));

	muggle_bytes_buffer_t *bytes_buf =
		(muggle_bytes_buffer_t *)muggle_socket_ctx_get_data(ctx);
	muggle_bytes_buffer_destroy(bytes_buf);
	free(bytes_buf);
}
