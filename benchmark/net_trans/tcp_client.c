#include "tcp_client.h"

struct tcp_client_user_data
{
	int flags;
	muggle_benchmark_handle_t *handle;
	muggle_benchmark_config_t *config;
	muggle_bytes_buffer_t *bytes_buf;
};

void recv_message(muggle_socket_context_t *ctx, muggle_bytes_buffer_t *bytes_buf)
{
	int read_bytes = 4096;
	while (1)
	{
		void *p = muggle_bytes_buffer_writer_fc(bytes_buf, read_bytes);
		if (p == NULL)
		{
			MUGGLE_LOG_ERROR("bytes buffer full");
			exit(EXIT_FAILURE);
		}

		int n = muggle_socket_ctx_recv(ctx, p, read_bytes, 0);
		if (n > 0)
		{
			muggle_bytes_buffer_writer_move(bytes_buf, n);
		}

		if (n < read_bytes)
		{
			break;
		}
	}
}

bool parse_message_uncontiguous(
	muggle_socket_context_t *ctx, muggle_bytes_buffer_t *bytes_buf, int readable, struct tcp_client_user_data *user_data)
{
	struct pkg msg;
	if (!muggle_bytes_buffer_fetch(bytes_buf, sizeof(struct pkg_header), &msg.header))
	{
		return false;
	}

	if (readable >= (int)(msg.header.data_len + sizeof(struct pkg_header)))
	{
		bool ret = muggle_bytes_buffer_read(bytes_buf, msg.header.data_len + sizeof(struct pkg_header), &msg);
		MUGGLE_ASSERT(ret == true);
		if (!ret)
		{
			return false;
		}

		if (onRecvPkg(ctx, &msg, user_data->handle, user_data->config) != 0)
		{
			muggle_socket_ctx_shutdown(ctx);
			MUGGLE_LOG_INFO("shutdown socket");
		}

		return true;
	}

	return false;
}

void parse_message(muggle_socket_context_t *ctx, struct tcp_client_user_data *user_data)
{
	muggle_bytes_buffer_t *bytes_buf = user_data->bytes_buf;

	int readable = 0;
	while (1)
	{
		readable = muggle_bytes_buffer_readable(bytes_buf);
		if (readable < (int)sizeof(struct pkg_header))
		{
			break;
		}

		struct pkg_header *header = muggle_bytes_buffer_reader_fc(bytes_buf, sizeof(struct pkg_header));
		if (header != NULL)
		{
			int len_msg = (int)(header->data_len + (uint32_t)sizeof(struct pkg_header));
			if (len_msg > (int)sizeof(struct pkg))
			{
				MUGGLE_LOG_ERROR("size of data was wrong! msg_len=%d, data_len=%d", len_msg, header->data_len);
				exit(EXIT_FAILURE);
			}

			if (readable < len_msg)
			{
				break;
			}

			struct pkg *msg = muggle_bytes_buffer_reader_fc(bytes_buf, len_msg);
			if (msg != NULL)
			{
				if (onRecvPkg(ctx, msg, user_data->handle, user_data->config) != 0)
				{
					muggle_socket_ctx_shutdown(ctx);
					MUGGLE_LOG_INFO("shutdown socket");
				}

				muggle_bytes_buffer_reader_move(bytes_buf, len_msg);
			}
			else
			{
				if (!parse_message_uncontiguous(ctx, bytes_buf, readable, user_data))
				{
					break;
				}
			}
		}
		else
		{
			if (!parse_message_uncontiguous(ctx, bytes_buf, readable, user_data))
			{
				break;
			}
		}
	}
}

static void tcp_client_on_message(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	struct tcp_client_user_data *user_data = muggle_socket_ctx_get_data(ctx);
	muggle_bytes_buffer_t *bytes_buf = user_data->bytes_buf;

	// read message into bytes buffer
	recv_message(ctx, bytes_buf);

	// parse message
	parse_message(ctx, user_data);
}

static void tcp_client_on_close(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	muggle_evloop_exit(evloop);
}

void run_tcp_client(
	const char *host, const char *port,
	int flags,
	muggle_benchmark_handle_t *handle,
	muggle_benchmark_config_t *config)
{
	// create tcp connect socket
	muggle_socket_t fd = muggle_tcp_connect(host, port, 3);
	if (fd == MUGGLE_INVALID_SOCKET)
	{
		LOG_ERROR("failed connect %s %s", host, port);
		exit(EXIT_FAILURE);
	}

	// set TCP_NODELAY
	int enable = 1;
	muggle_setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));
	int timeout_ms = -1;
#if ! defined(MUGGLE_PLATFORM_WINDOWS)
	if (flags & MSG_DONTWAIT)
	{
		timeout_ms = 0;
	}
#endif

	// init bytes buffer
	muggle_bytes_buffer_t bytes_buf;
	if (!muggle_bytes_buffer_init(&bytes_buf, 1024 * 1024 * 16))
	{
		MUGGLE_LOG_ERROR("failed init bytes buf");
		exit(EXIT_FAILURE);
	}

	// user data
	struct tcp_client_user_data user_data;
	memset(&user_data, 0, sizeof(user_data));
	user_data.flags = flags;
	user_data.handle = handle;
	user_data.config = config;
	user_data.bytes_buf = &bytes_buf;

	// create socket context
	muggle_socket_context_t *ctx =
		(muggle_socket_context_t*)malloc(sizeof(muggle_socket_context_t));
	muggle_socket_ctx_init(ctx, fd, &user_data, MUGGLE_SOCKET_CTX_TYPE_TCP_CLIENT);

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

	// bind socket event loop handle
	muggle_socket_evloop_handle_t evloop_handle;
	muggle_socket_evloop_handle_init(&evloop_handle);
	muggle_socket_evloop_handle_set_timer_interval(&evloop_handle, timeout_ms);
	muggle_socket_evloop_handle_set_cb_msg(&evloop_handle, tcp_client_on_message);
	muggle_socket_evloop_handle_set_cb_close(&evloop_handle, tcp_client_on_close);
	muggle_socket_evloop_handle_attach(&evloop_handle, evloop);
	LOG_INFO("socket handle attached to event loop");

	// add tcp client into event loop
	muggle_socket_evloop_add_ctx(evloop, ctx);

	// run
	muggle_evloop_run(evloop);

	// clear
	muggle_evloop_delete(evloop);

	// destroy bytes buffer
	muggle_bytes_buffer_destroy(&bytes_buf);
}
