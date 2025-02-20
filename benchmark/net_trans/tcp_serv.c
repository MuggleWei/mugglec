#include "tcp_serv.h"

struct tcp_serv_user_data {
	muggle_benchmark_handle_t *handle;
	muggle_benchmark_config_t *config;
};

static void tcp_recv_message(muggle_socket_context_t *ctx,
							 muggle_bytes_buffer_t *bytes_buf)
{
	int read_bytes = 4096;
	while (1) {
		void *p = muggle_bytes_buffer_writer_fc(bytes_buf, read_bytes);
		if (p == NULL) {
			MUGGLE_LOG_ERROR("bytes buffer full");
			exit(EXIT_FAILURE);
		}

		int n = muggle_socket_ctx_recv(ctx, p, read_bytes, 0);
		if (n > 0) {
			muggle_bytes_buffer_writer_move_n(bytes_buf, p, n);
		}

		if (n < read_bytes) {
			break;
		}
	}
}

bool parse_message_uncontiguous(muggle_socket_context_t *ctx,
								muggle_bytes_buffer_t *bytes_buf, int readable,
								struct tcp_serv_user_data *user_data)
{
	struct pkg msg;
	if (!muggle_bytes_buffer_fetch(bytes_buf, sizeof(struct pkg_header),
								   &msg.header)) {
		return false;
	}

	if (readable >= (int)(msg.header.data_len + sizeof(struct pkg_header))) {
		bool ret = muggle_bytes_buffer_read(
			bytes_buf, msg.header.data_len + sizeof(struct pkg_header), &msg);
		MUGGLE_ASSERT(ret == true);
		if (!ret) {
			return false;
		}

		if (onRecvPkg(ctx, &msg, user_data->handle, user_data->config) != 0) {
			muggle_socket_ctx_shutdown(ctx);
			MUGGLE_LOG_INFO("shutdown socket");
		}

		return true;
	}

	return false;
}

static int parse_message(muggle_socket_context_t *ctx,
						 muggle_bytes_buffer_t *bytes_buf,
						 struct tcp_serv_user_data *user_data)
{
	int is_closed = 0;

	int readable = 0;
	while (1) {
		readable = muggle_bytes_buffer_readable(bytes_buf);
		if (readable < (int)sizeof(struct pkg_header)) {
			break;
		}

		struct pkg_header *header =
			muggle_bytes_buffer_reader_fc(bytes_buf, sizeof(struct pkg_header));
		if (header != NULL) {
			int len_msg =
				(int)(header->data_len + (uint32_t)sizeof(struct pkg_header));
			if (len_msg > (int)sizeof(struct pkg)) {
				MUGGLE_LOG_ERROR(
					"size of data was wrong! msg_len=%d, data_len=%d", len_msg,
					header->data_len);
				exit(EXIT_FAILURE);
			}

			if (readable < len_msg) {
				break;
			}

			struct pkg *msg = muggle_bytes_buffer_reader_fc(bytes_buf, len_msg);
			if (msg != NULL) {
				if (onRecvPkg(ctx, msg, user_data->handle, user_data->config) !=
					0) {
					muggle_socket_ctx_shutdown(ctx);
					MUGGLE_LOG_INFO("shutdown socket");
					is_closed = 1;
				}

				muggle_bytes_buffer_reader_move(bytes_buf, len_msg);
			} else {
				if (!parse_message_uncontiguous(ctx, bytes_buf, readable,
												user_data)) {
					break;
				}
			}
		} else {
			if (!parse_message_uncontiguous(ctx, bytes_buf, readable,
											user_data)) {
				break;
			}
		}
	}

	return is_closed;
}

static void on_tcp_connect(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx)
{
	MUGGLE_UNUSED(evloop);

	muggle_bytes_buffer_t *bytes_buf =
		(muggle_bytes_buffer_t *)malloc(sizeof(muggle_bytes_buffer_t));
	muggle_bytes_buffer_init(bytes_buf, 1024 * 1024 * 4);
	muggle_socket_ctx_set_data(ctx, bytes_buf);
}

static void on_tcp_message(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx)
{
	muggle_bytes_buffer_t *bytes_buf = muggle_socket_ctx_get_data(ctx);

	// read message into bytes buffer
	tcp_recv_message(ctx, bytes_buf);

	// parse message
	parse_message(ctx, bytes_buf,
				  (struct tcp_serv_user_data *)muggle_evloop_get_data(evloop));
}

static void on_tcp_release(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx)
{
	muggle_bytes_buffer_t *bytes_buf = muggle_socket_ctx_get_data(ctx);
	if (bytes_buf) {
		muggle_bytes_buffer_destroy(bytes_buf);
		free(bytes_buf);
		muggle_socket_ctx_set_data(ctx, NULL);
	}

	muggle_evloop_exit(evloop);
}

void run_tcp_serv_multiplexing(const char *host, const char *port, int is_busy,
							   muggle_benchmark_handle_t *handle,
							   muggle_benchmark_config_t *config)
{
	// user data
	struct tcp_serv_user_data user_data;
	memset(&user_data, 0, sizeof(user_data));
	user_data.handle = handle;
	user_data.config = config;

	// create tcp listen socket
	muggle_socket_t fd = muggle_tcp_listen(host, port, 512);
	if (fd == MUGGLE_INVALID_SOCKET) {
		LOG_ERROR("failed create tcp listen for %s %s", host, port);
		exit(EXIT_FAILURE);
	}

	// set TCP_NODELAY
	int enable = 1;
	muggle_setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&enable,
					  sizeof(enable));

	// create socket context
	muggle_socket_context_t *ctx =
		(muggle_socket_context_t *)malloc(sizeof(muggle_socket_context_t));
	muggle_socket_ctx_init(ctx, fd, NULL, MUGGLE_SOCKET_CTX_TYPE_TCP_LISTEN);

	// new event loop
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
	muggle_evloop_set_data(evloop, &user_data);

	// bind socket event loop handle
	muggle_socket_evloop_handle_t evloop_handle;
	muggle_socket_evloop_handle_init(&evloop_handle);
	muggle_socket_evloop_handle_set_cb_conn(&evloop_handle, on_tcp_connect);
	muggle_socket_evloop_handle_set_cb_msg(&evloop_handle, on_tcp_message);
	muggle_socket_evloop_handle_set_cb_release(&evloop_handle, on_tcp_release);
	if (is_busy) {
		muggle_socket_evloop_handle_set_timer_interval(&evloop_handle, 0);
	}
	muggle_socket_evloop_handle_attach(&evloop_handle, evloop);
	LOG_INFO("socket handle attached to event loop");

	// add socket context into evloop
	muggle_socket_evloop_add_ctx(evloop, ctx);

	// run
	muggle_evloop_run(evloop);

	// cleanup
	muggle_socket_evloop_handle_destroy(&evloop_handle);
	muggle_evloop_delete(evloop);
}

static muggle_socket_t tcp_server_do_accept(muggle_event_fd listen_fd)
{
	muggle_socket_t fd = MUGGLE_INVALID_SOCKET;
	do {
		fd = accept(listen_fd, NULL, NULL);
		if (fd == MUGGLE_INVALID_SOCKET) {
			if (MUGGLE_SOCKET_LAST_ERRNO == MUGGLE_SYS_ERRNO_INTR) {
				continue;
			} else if (MUGGLE_SOCKET_LAST_ERRNO ==
					   MUGGLE_SYS_ERRNO_WOULDBLOCK) {
				continue;
			} else {
				LOG_SYS_ERR(LOG_LEVEL_ERROR, "failed accept");
				break;
			}
		}
	} while (fd == MUGGLE_INVALID_SOCKET);

	return fd;
}

void run_tcp_serv_single(const char *host, const char *port, int is_busy,
						 muggle_benchmark_handle_t *handle,
						 muggle_benchmark_config_t *config)
{
	// create tcp listen socket
	muggle_socket_t listen_fd = muggle_tcp_listen(host, port, 512);
	if (listen_fd == MUGGLE_INVALID_SOCKET) {
		LOG_ERROR("failed create tcp listen for %s %s", host, port);
		exit(EXIT_FAILURE);
	}

	if (is_busy) {
		muggle_socket_set_nonblock(listen_fd, 1);
	}

	int enable = 1;
	muggle_setsockopt(listen_fd, IPPROTO_TCP, TCP_NODELAY, (void *)&enable,
					  sizeof(enable));

	muggle_socket_t fd = tcp_server_do_accept(listen_fd);
	if (fd == MUGGLE_INVALID_SOCKET) {
		LOG_ERROR("failed accept");
		exit(EXIT_FAILURE);
	}
	muggle_socket_close(listen_fd);

	// set TCP_NODELAY
	// NOTE: TCP single always busy read
	if (is_busy) {
		muggle_socket_set_nonblock(fd, 1);
	}

	enable = 1;
	muggle_setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&enable,
					  sizeof(enable));

	// prepare bytes buffer
	muggle_bytes_buffer_t bytes_buf;
	muggle_bytes_buffer_init(&bytes_buf, 1024 * 1024 * 4);

	// prepare socket context
	muggle_socket_context_t ctx;
	muggle_socket_ctx_init(&ctx, fd, NULL, MUGGLE_SOCKET_CTX_TYPE_TCP_CLIENT);
	muggle_socket_ctx_set_data(&ctx, &bytes_buf);

	struct tcp_serv_user_data user_data;
	user_data.handle = handle;
	user_data.config = config;

	while (1) {
		// read message into bytes buffer
		tcp_recv_message(&ctx, &bytes_buf);

		// parse message
		int is_closed = parse_message(&ctx, &bytes_buf, &user_data);
		if (is_closed) {
			break;
		}
	}

	// cleanup
	muggle_bytes_buffer_destroy(&bytes_buf);
}

void run_tcp_serv(const char *host, const char *port, int is_multiplexing,
				  int is_busy, muggle_benchmark_handle_t *handle,
				  muggle_benchmark_config_t *config)
{
	if (is_multiplexing) {
		run_tcp_serv_multiplexing(host, port, is_busy, handle, config);
	} else {
		run_tcp_serv_single(host, port, is_busy, handle, config);
	}
}
