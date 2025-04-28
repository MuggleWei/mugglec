#include "tcp_block_serv_handle.h"

typedef struct {
	const char *host;
	const char *serv;
} client_th_args_t;

muggle_socket_context_t *tcp_listen(const char *host, const char *serv)
{
	// tcp listen
	muggle_socket_t listen_fd = MUGGLE_INVALID_SOCKET;
	do {
		listen_fd = muggle_tcp_listen(host, serv, 512);
		if (listen_fd == MUGGLE_INVALID_SOCKET) {
			LOG_ERROR("failed create tcp listen for %s:%s", host, serv);
			muggle_msleep(3000);
		}
	} while (listen_fd == MUGGLE_INVALID_SOCKET);
	LOG_INFO("success listen %s %s", host, serv);

	// new context
	muggle_socket_context_t *listen_ctx =
		(muggle_socket_context_t *)malloc(sizeof(muggle_socket_context_t));
	if (muggle_socket_ctx_init(listen_ctx, listen_fd, NULL,
							   MUGGLE_SOCKET_CTX_TYPE_TCP_LISTEN) != 0) {
		muggle_socket_close(listen_fd);
		LOG_ERROR("failed create socket context");
		return NULL;
	}

	return listen_ctx;
}

void run_server(const char *host, const char *serv)
{
	// init event loop
	muggle_event_loop_init_args_t ev_init_args;
	memset(&ev_init_args, 0, sizeof(ev_init_args));
	ev_init_args.evloop_type = MUGGLE_EVLOOP_TYPE_NULL;
	ev_init_args.hints_max_fd = 32;
	ev_init_args.use_mem_pool = 0;

	muggle_event_loop_t *evloop = muggle_evloop_new(&ev_init_args);
	if (evloop == NULL) {
		LOG_ERROR("failed new event loop");
		exit(EXIT_FAILURE);
	}
	LOG_INFO("success new event loop");

	// init socket event loop handle
	muggle_socket_evloop_handle_t handle;
	muggle_socket_evloop_handle_init(&handle);
	muggle_socket_evloop_handle_set_cb_msg(&handle, serv_on_message);
	muggle_socket_evloop_handle_attach(&handle, evloop);
	LOG_INFO("socket handle attached event loop");

	// create tcp listen socket
	muggle_socket_context_t *tcp_listen_ctx = tcp_listen(host, serv);
	muggle_socket_evloop_add_ctx(evloop, tcp_listen_ctx);

	// run event loop
	LOG_INFO("run event loop");
	muggle_evloop_run(evloop);

	// destroy socket event loop handle
	muggle_socket_evloop_handle_destroy(&handle);

	// delete event loop
	muggle_evloop_delete(evloop);
}

muggle_thread_ret_t run_client(void *args)
{
	client_th_args_t *p_args = (client_th_args_t *)args;

	muggle_socket_t fd = MUGGLE_INVALID_SOCKET;
	do {
		fd = muggle_tcp_connect(p_args->host, p_args->serv, 3);
		if (fd != MUGGLE_INVALID_SOCKET) {
			break;
		}

		muggle_msleep(3);
	} while (true);

	muggle_socket_set_nonblock(fd, 1);

	char buf[9];
	memset(buf, 0, sizeof(buf));

	const int write_n = 8;
	uint32_t idx = 0;
	while (true) {
		snprintf(buf, sizeof(buf), "%08u", ++idx);

		// non-block write
		// int n = muggle_socket_write(fd, buf, write_n);

		// block write
		// int n = muggle_socket_block_write(fd, buf, write_n, 400);

		// blocking write
		muggle_time_counter_t tc;
		muggle_time_counter_init(&tc);
		muggle_time_counter_start(&tc);

		unsigned long timeout_ms = 3000;
		int n = muggle_socket_blocking_write(fd, buf, write_n, 400, timeout_ms);

		muggle_time_counter_end(&tc);

		if (n != write_n) {
			LOG_ERROR("socket write return %d, write elapsed: %lld ms", n,
					  (long long)muggle_time_counter_interval_ms(&tc));
			LOG_ERROR("write elapsed should approximately equal to %lu ms",
					  timeout_ms);
			break;
		}
	}

	muggle_socket_close(fd);

	return 0;
}

int main(int argc, char *argv[])
{
	if (muggle_log_complicated_init(LOG_LEVEL_INFO, LOG_LEVEL_INFO,
									"logs/tcp_block_write.log") != 0) {
		LOG_ERROR("failed initalize log");
		exit(EXIT_FAILURE);
	}

	if (muggle_socket_lib_init() != 0) {
		LOG_ERROR("failed initalize socket library");
		exit(EXIT_FAILURE);
	}

	if (argc < 2) {
		LOG_ERROR("usage: %s <IP> <Port>", argv[0]);
		exit(EXIT_FAILURE);
	}

	const char *host = argv[1];
	const char *serv = argv[2];

	client_th_args_t th_args;
	th_args.host = host;
	th_args.serv = serv;

	muggle_thread_t th;
	muggle_thread_create(&th, run_client, &th_args);

	run_server(host, serv);

	muggle_thread_join(&th);

	return 0;
}
