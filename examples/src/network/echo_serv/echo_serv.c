#include "echo_serv_handle.h"

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
		LOG_ERROR("invalid socket event loop type: %s", str);
		exit(EXIT_FAILURE);
	}

	return evloop_type;
}

muggle_socket_context_t* tcp_listen(const char *host, const char *serv)
{
	// tcp listen
	muggle_socket_t listen_fd = MUGGLE_INVALID_SOCKET;
	do {
		listen_fd = muggle_tcp_listen(host, serv, 512);
		if (listen_fd == MUGGLE_INVALID_SOCKET)
		{
			LOG_ERROR("failed create tcp listen for %s:%s", host, serv);
			muggle_msleep(3000);
		}
	} while(listen_fd == MUGGLE_INVALID_SOCKET);
	LOG_INFO("success listen %s %s", host, serv);

	// new context
	muggle_socket_context_t *listen_ctx =
		(muggle_socket_context_t*)malloc(sizeof(muggle_socket_context_t));
	if (muggle_socket_ctx_init(listen_ctx, listen_fd, NULL, MUGGLE_SOCKET_CTX_TYPE_TCP_LISTEN) != 0)
	{
		muggle_socket_close(listen_fd);
		LOG_ERROR("failed create socket context");
		return NULL;
	}

	return listen_ctx;
}

muggle_socket_context_t* udp_bind(const char *host, const char *serv)
{
	// udp bind
	muggle_socket_t bind_fd = MUGGLE_INVALID_SOCKET;
	do {
		bind_fd = muggle_udp_bind(host, serv);
		if (bind_fd == MUGGLE_INVALID_SOCKET)
		{
			LOG_ERROR("failed create udp bind for %s:%s", host, serv);
			muggle_msleep(3000);
		}
	} while(bind_fd == MUGGLE_INVALID_SOCKET);

	// new context
	muggle_socket_context_t *bind_ctx =
		(muggle_socket_context_t*)malloc(sizeof(muggle_socket_context_t));
	if (muggle_socket_ctx_init(bind_ctx, bind_fd, NULL, MUGGLE_SOCKET_CTX_TYPE_UDP) != 0)
	{
		muggle_socket_close(bind_fd);
		LOG_ERROR("failed create socket context");
		return NULL;
	}

	return bind_ctx;
}

int main(int argc, char *argv[])
{
	if (muggle_log_simple_init(LOG_LEVEL_INFO, LOG_LEVEL_INFO) != 0)
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
	LOG_INFO("success new event loop");

	// init socket event loop handle
	muggle_socket_evloop_handle_t handle;
	muggle_socket_evloop_handle_init(&handle);
	muggle_socket_evloop_handle_set_cb_conn(&handle, on_connect);
	muggle_socket_evloop_handle_set_cb_close(&handle, on_close);
	muggle_socket_evloop_handle_set_cb_release(&handle, on_release);
	muggle_socket_evloop_handle_set_cb_msg(&handle, on_message);
	muggle_socket_evloop_handle_attach(&handle, evloop);
	LOG_INFO("socket handle attached event loop");

	// create tcp listen socket
	muggle_socket_context_t *tcp_listen_ctx = tcp_listen(host, serv);
	muggle_socket_evloop_add_ctx(evloop, tcp_listen_ctx);

	// create udp bind socket
	muggle_socket_context_t *udp_bind_ctx = udp_bind(host, serv);
	muggle_socket_evloop_add_ctx(evloop, udp_bind_ctx);

	// run event loop
	LOG_INFO("run event loop");
	muggle_evloop_run(evloop);

	// destroy socket event loop handle
	muggle_socket_evloop_handle_destroy(&handle);

	// delete event loop
	muggle_evloop_delete(evloop);

	return 0;
}
