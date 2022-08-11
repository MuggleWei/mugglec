#include "tcp_serv.h"

struct tcp_serv_user_data
{
	int flags;
	muggle_benchmark_handle_t *handle;
	muggle_benchmark_config_t *config;
};

static void tcp_serv_on_connect(
	muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	struct tcp_serv_user_data *data =
		(struct tcp_serv_user_data*)muggle_evloop_get_data(evloop);
	sendPkgs(ctx, data->flags, data->handle, data->config);
}

static void tcp_serv_on_close(
	muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	muggle_evloop_exit(evloop);
}

void run_tcp_serv(
	const char *host, const char *port,
	int flags,
	muggle_benchmark_handle_t *handle,
	muggle_benchmark_config_t *config)
{
	// create tcp listen socket
	muggle_socket_t fd = muggle_tcp_listen(host, port, 512);
	if (fd == MUGGLE_INVALID_SOCKET)
	{
		LOG_ERROR("failed create tcp listen for %s %s", host, port);
		exit(EXIT_FAILURE);
	}

	// set TCP_NODELAY
	int enable = 1;
	muggle_setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));

	// user data
	struct tcp_serv_user_data user_data;
	memset(&user_data, 0, sizeof(user_data));
	user_data.flags = flags;
	user_data.handle = handle;
	user_data.config = config;

	// create socket context
	muggle_socket_context_t *ctx =
		(muggle_socket_context_t*)malloc(sizeof(muggle_socket_context_t));
	muggle_socket_ctx_init(ctx, fd, NULL, MUGGLE_SOCKET_CTX_TYPE_TCP_LISTEN);

	// new event loop
	muggle_event_loop_init_args_t ev_init_args;
	memset(&ev_init_args, 0, sizeof(ev_init_args));
	ev_init_args.evloop_type = MUGGLE_EVLOOP_TYPE_NULL;
	ev_init_args.hints_max_fd = 32;
	ev_init_args.use_mem_pool = 0;

	muggle_event_loop_t *evloop = muggle_evloop_new(&ev_init_args);
	if (evloop == NULL)
	{
		LOG_ERROR("failed new event loop");
		exit(EXIT_FAILURE);
	}
	LOG_INFO("success new event loop");
	muggle_evloop_set_data(evloop, &user_data);

	// bind socket event loop handle
	muggle_socket_evloop_handle_t evloop_handle;
	muggle_socket_evloop_handle_init(&evloop_handle);
	muggle_socket_evloop_handle_set_cb_conn(&evloop_handle, tcp_serv_on_connect);
	muggle_socket_evloop_handle_set_cb_close(&evloop_handle, tcp_serv_on_close);
	muggle_socket_evloop_handle_attach(&evloop_handle, evloop);
	LOG_INFO("socket handle attached to event loop");

	// add tcp client into event loop
	muggle_socket_evloop_add_ctx(evloop, ctx);

	// run
	muggle_evloop_run(evloop);

	// clear
	muggle_socket_evloop_handle_destroy(&evloop_handle);
	muggle_evloop_delete(evloop);
}
