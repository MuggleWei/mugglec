#include "server_evloop.h"
#include "tcp_server.h"

static muggle_event_loop_t* new_echo_server_evloop()
{
	muggle_event_loop_init_args_t evloop_arg;
	memset(&evloop_arg, 0, sizeof(evloop_arg));
	evloop_arg.evloop_type = MUGGLE_EVLOOP_TYPE_NULL;
	evloop_arg.hints_max_fd = 32;
	evloop_arg.use_mem_pool = 1;

	evloop_data_t *data = (evloop_data_t*)malloc(sizeof(evloop_data_t));
	strncpy(data->msg, "Echo Server", sizeof(data->msg) - 1);

	muggle_event_loop_t *evloop = muggle_evloop_new(&evloop_arg);
	muggle_evloop_set_timer_interval(evloop, 3000);
	muggle_evloop_set_cb_read(evloop, on_read);
	muggle_evloop_set_cb_close(evloop, on_close);
	muggle_evloop_set_cb_exit(evloop, on_exit);
	muggle_evloop_set_data(evloop, data);

	return evloop;
}

void run_echo_server_evloop(const char *host, const char *port)
{
	// event loop
	muggle_event_loop_t *evloop = new_echo_server_evloop();

	// TCP
	muggle_socket_t tcp_listen_fd = muggle_tcp_listen(host, port, 512);
	if (tcp_listen_fd == MUGGLE_INVALID_SOCKET)
	{
		LOG_ERROR("failed TCP listen %s %s", host, port);
		exit(EXIT_FAILURE);
	}
	LOG_INFO("success TCP listen: %s %s", host, port);

	ev_data_t tcp_ev_data;
	memset(&tcp_ev_data, 0, sizeof(tcp_ev_data));
	tcp_ev_data.cb_read = tcp_server_on_accept;

	muggle_event_context_t tcp_ev_ctx;
	muggle_ev_ctx_init(&tcp_ev_ctx, tcp_listen_fd, &tcp_ev_data);
	if (muggle_evloop_add_ctx(evloop, &tcp_ev_ctx) != 0)
	{
		LOG_ERROR("failed add listen socket context into event loop");
		exit(EXIT_FAILURE);
	}
	LOG_INFO("success insert listen socket into event loop");

	// run event loop
	LOG_INFO("event loop run");
	muggle_evloop_run(evloop);

	// delete tcp listen context
	muggle_ev_ctx_close(&tcp_ev_ctx);

	// delete event loop
	muggle_evloop_delete(evloop);
}

void on_read(muggle_event_loop_t *evloop, muggle_event_context_t *ctx)
{
	ev_data_t *data = muggle_ev_ctx_data(ctx);
	if (data && data->cb_read)
	{
		data->cb_read(evloop, ctx);
	}
}
void on_close(muggle_event_loop_t *evloop, muggle_event_context_t *ctx)
{
	ev_data_t *data = muggle_ev_ctx_data(ctx);
	if (data && data->cb_close)
	{
		data->cb_close(evloop, ctx);
	}
}

void on_exit(muggle_event_loop_t *evloop)
{
	evloop_data_t *data = muggle_evloop_get_data(evloop);
	if (data)
	{
		free(data);
		muggle_evloop_set_data(evloop, NULL);
	}
}