#include "async_serv_handle.h"

int get_event_loop_type(const char *str)
{
	// get socket event loop type
	int evloop_type = MUGGLE_EVLOOP_TYPE_NULL;
	if (strcmp(str, "select") == 0)
	{
		evloop_type = MUGGLE_EVLOOP_TYPE_SELECT;
	}
	else if (strcmp(str, "poll") == 0)
	{
		evloop_type = MUGGLE_EVLOOP_TYPE_POLL;
	}
	else if (strcmp(str, "epoll") == 0)
	{
		evloop_type = MUGGLE_EVLOOP_TYPE_EPOLL;
	}
	else if (strcmp(str, "kqueue") == 0)
	{
		evloop_type = MUGGLE_EVLOOP_TYPE_KQUEUE;
	}
	else
	{
		MUGGLE_LOG_ERROR("invalid socket event loop type: %s", str);
		exit(EXIT_FAILURE);
	}

	return evloop_type;
}

int main(int argc, char *argv[])
{
	if (muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO) != 0)
	{
		LOG_ERROR("failed initalize log");
		exit(EXIT_FAILURE);
	}

	if (muggle_socket_lib_init() != 0)
	{
		LOG_ERROR("failed initalize socket library");
		exit(EXIT_FAILURE);
	}

	if (argc < 3)
	{
		LOG_ERROR("usage: %s <IP> <Port> [select|poll|epoll|kqueue]", argv[0]);
		exit(EXIT_FAILURE);
	}

	const char *host = argv[1];
	const char *serv = argv[2];

	// get event loop type
	int evloop_type = MUGGLE_EVLOOP_TYPE_NULL;
	if (argc > 3)
	{
		evloop_type = get_event_loop_type(argv[3]);
	}

	// init event loop
	muggle_event_loop_init_args_t ev_init_args;
	memset(&ev_init_args, 0, sizeof(ev_init_args));
	ev_init_args.evloop_type = evloop_type;
	ev_init_args.hints_max_fd = 32;
	ev_init_args.use_mem_pool = 0;

	muggle_event_loop_t *evloop = muggle_evloop_new(&ev_init_args);
	if (evloop == NULL)
	{
		LOG_ERROR("failed new event loop");
		exit(EXIT_FAILURE);
	}

	// set event loop callback
	muggle_evloop_set_cb_read(evloop, on_read);
	muggle_evloop_set_cb_close(evloop, on_close);

	// create tcp listen socket
	muggle_socket_t listen_fd = muggle_tcp_listen(host, serv, 512);
	if (listen_fd == MUGGLE_INVALID_SOCKET)
	{
		LOG_ERROR("failed create tcp listen for %s:%s", host, serv);
		exit(EXIT_FAILURE);
	}
	muggle_socket_context_t listen_ctx;
	if (muggle_socket_ctx_init(&listen_ctx, listen_fd, NULL, MUGGLE_SOCKET_CTX_TYPE_TCP_LISTEN) != 0)
	{
		LOG_ERROR("failed create socket context");
		exit(EXIT_FAILURE);
	}
	muggle_evloop_add_ctx(evloop, (muggle_event_context_t*)&listen_ctx);

	// run event loop
	muggle_evloop_run(evloop);

	// delete event loop
	muggle_evloop_delete(evloop);

	return 0;
}
