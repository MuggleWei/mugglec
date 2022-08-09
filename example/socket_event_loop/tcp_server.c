#include "tcp_server.h"

void tcp_server_run(sys_args_t *args, void *user_data)
{
	LOG_INFO("run tcp server in %s:%s", args->host, args->port);

	// init event loop
	LOG_INFO("initialize event loop");

	muggle_event_loop_init_args_t evloop_arg;
	memset(&evloop_arg, 0, sizeof(evloop_arg));
	evloop_arg.evloop_type = MUGGLE_EVLOOP_TYPE_NULL;
	evloop_arg.hints_max_fd = 32;
	evloop_arg.use_mem_pool = 1;

	muggle_event_loop_t *evloop = muggle_evloop_new(&evloop_arg);
	muggle_evloop_set_timer_interval(evloop, 3000);
	muggle_evloop_set_cb_read(evloop, tcp_server_on_read);
	muggle_evloop_set_cb_close(evloop, tcp_server_on_close);
	muggle_evloop_set_cb_wake(evloop, tcp_server_on_wake);
	muggle_evloop_set_cb_timer(evloop, tcp_server_on_timer);
	muggle_evloop_set_cb_clear(evloop, tcp_server_on_clear);
	muggle_evloop_set_data(evloop, user_data);

	// set user data evloop
	((evloop_user_data_t*)user_data)->evloop = evloop;

	// create linsten socket and add into event loop
	muggle_socket_t listenfd = muggle_tcp_listen(args->host, args->port, 512, NULL);
	if (listenfd == MUGGLE_INVALID_SOCKET)
	{
		LOG_ERROR("failed listen %s:%s", args->host, args->port);
		exit(EXIT_FAILURE);
	}
	LOG_INFO("create TCP socket listen %s:%s", args->host, args->port);

	muggle_socket_context_t ctx;
	muggle_socket_ctx_init(&ctx, listenfd, NULL, MUGGLE_SOCKET_CTX_TYPE_TCP_CLIENT);
	if (muggle_evloop_add_ctx(evloop, (muggle_event_context_t*)&ctx) != 0)
	{
		LOG_ERROR("failed add listen socket context into event loop");
		exit(EXIT_FAILURE);
	}
	LOG_INFO("success insert listen socket into event loop");

	// run event loop
	LOG_INFO("event loop run");
	muggle_evloop_run(evloop);

	// free listen context
	muggle_socket_ctx_close((muggle_event_context_t*)&ctx);

	// delete event loop
	muggle_evloop_delete(evloop);
}

void tcp_server_on_read(muggle_event_loop_t *evloop, muggle_event_context_t *ctx)
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
			do {
				muggle_socket_t fd = tcp_server_on_accept(evloop, socket_ctx);
				if (fd == MUGGLE_INVALID_SOCKET)
				{
					break;
				}
				tcp_server_on_connect(evloop, fd);
			} while(1);
		}break;
		case MUGGLE_SOCKET_CTX_TYPE_TCP_CLIENT:
		{
			tcp_server_on_message(evloop, socket_ctx);
		}break;
	}
}
void tcp_server_on_close(muggle_event_loop_t *evloop, muggle_event_context_t *ctx)
{
	LOG_INFO("remove socket ctx");
	if (muggle_socket_ctx_ref_release(ctx) == 0)
	{
		muggle_socket_ctx_close(ctx);
		free(ctx);
	}
}
void tcp_server_on_wake(muggle_event_loop_t *evloop)
{
	evloop_user_data_t *user_data = (evloop_user_data_t*)muggle_evloop_get_data(evloop);
	LOG_INFO("on wake: %s", user_data->msg);
}
void tcp_server_on_timer(muggle_event_loop_t *evloop)
{
	LOG_INFO("on timer");
}
void tcp_server_on_clear(muggle_event_loop_t *evloop, muggle_event_context_t *ctx)
{
	LOG_INFO("on event loop exit clear");
	muggle_socket_context_t *socket_ctx = (muggle_socket_context_t*)ctx;
	if (socket_ctx->sock_type == MUGGLE_SOCKET_CTX_TYPE_TCP_CLIENT)
	{
		if (muggle_ev_ctx_ref_release(ctx) == 0)
		{
			muggle_ev_ctx_close(ctx);
			free(ctx);
		}
	}
}

muggle_socket_t tcp_server_on_accept(
	muggle_event_loop_t *evloop, muggle_socket_context_t *listen_ctx)
{
	muggle_socket_t fd = MUGGLE_INVALID_SOCKET;
	do {
		fd = accept(listen_ctx->base.fd, NULL, NULL);
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
				LOG_SYS_ERR(LOG_LEVEL_ERROR, "failed accept");
				muggle_evloop_exit(evloop);
				break;
			}
		}
	} while(fd == MUGGLE_INVALID_SOCKET);

	return fd;
}

void tcp_server_on_connect(muggle_event_loop_t *evloop, muggle_socket_t fd)
{
	LOG_INFO("on connection");

	muggle_socket_context_t *ctx =
		(muggle_socket_context_t*)malloc(sizeof(muggle_socket_context_t));
	muggle_socket_ctx_init(ctx, fd, NULL, MUGGLE_SOCKET_CTX_TYPE_TCP_CLIENT);

	int ret = muggle_evloop_add_ctx(evloop, (muggle_event_context_t*)ctx);
	if (ret != 0)
	{
		free(ctx);
		muggle_socket_close(fd);
	}
}

void tcp_server_on_message(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	char buf[1024];
	do {
		int n = muggle_socket_ctx_read(ctx, buf, sizeof(buf));
		if (n <= 0)
		{
			break;
		}

		if (muggle_socket_ctx_write(ctx, buf, n) != n)
		{
			muggle_socket_ctx_shutdown(ctx);
			break;
		}
	} while(1);
}
