#include "time_client_handle.h"

#define MAX_MSG_SIZE 65536

muggle_thread_ret_t conn_thread_routine(void *p_args)
{
	conn_thread_args_t *args = (conn_thread_args_t*)p_args;

	// get socket context type
	int socket_ctx_type = MUGGLE_SOCKET_CTX_TYPE_NULL;
	if (strcmp(args->sock_type, "tcp") == 0)
	{
		socket_ctx_type = MUGGLE_SOCKET_CTX_TYPE_TCP_CLIENT;
	}
	else
	{
		socket_ctx_type = MUGGLE_SOCKET_CTX_TYPE_UDP;
	}

	while (1)
	{
		// create socket fd
		muggle_socket_t fd = MUGGLE_INVALID_SOCKET;
		if (strcmp(args->sock_type, "tcp") == 0)
		{
			fd = muggle_tcp_connect(args->host, args->serv, 3);
		}
		else if (strcmp(args->sock_type, "udp") == 0)
		{
			fd = muggle_udp_bind(args->host, args->serv);
		}
		else if (strcmp(args->sock_type, "mcast") == 0)
		{
			fd = muggle_mcast_join(args->host, args->serv, NULL, NULL);
		}
		else
		{
			LOG_ERROR("invalid socket type: %s", args->sock_type);
			exit(EXIT_FAILURE);
		}

		if (fd == MUGGLE_INVALID_SOCKET)
		{
			LOG_ERROR("failed create socket: %s %s", args->host, args->serv);
			muggle_msleep(3000);
			continue;
		}

		// create socket context
		muggle_socket_context_t *ctx = (muggle_socket_context_t*)malloc(sizeof(muggle_socket_context_t));
		muggle_bytes_buffer_t *bytes_buf = (muggle_bytes_buffer_t*)malloc(sizeof(muggle_bytes_buffer_t));
		muggle_bytes_buffer_init(bytes_buf, 8 * 1024 * 1024);
		muggle_socket_ctx_init(ctx, fd, bytes_buf, socket_ctx_type);

		// add context to event loop
		muggle_socket_evloop_add_ctx(args->evloop, ctx);

		// exit connect thread
		break;
	}

	return 0;
}

static void on_timestr(const char *msg)
{
	LOG_INFO("recv timestamp string: %s", msg);
}

static void on_udp_msg(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	while (1)
	{
		char buf[MAX_MSG_SIZE];
		int n = muggle_socket_ctx_read(ctx, buf, sizeof(buf));
		if (n > 0)
		{
			if (n < 4)
			{
				LOG_ERROR("invalid message with number of bytes < 4");
				muggle_socket_ctx_shutdown(ctx);
				break;
			}

			uint32_t len = ntohl(*(uint32_t*)buf);
			if (len > MAX_MSG_SIZE - 4)
			{
				LOG_ERROR("length field in message too long");
				muggle_socket_ctx_shutdown(ctx);
				break;
			}

			on_timestr((char*)buf + 4);
		}
		else
		{
			break;
		}
	}
}

static void on_tcp_msg(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	muggle_bytes_buffer_t *bytes_buf = (muggle_bytes_buffer_t*)muggle_socket_ctx_get_data(ctx);
	while (1)
	{
		// find contiguous memory
		char *ptr = (char*)muggle_bytes_buffer_writer_fc(bytes_buf, MAX_MSG_SIZE);
		if (ptr == NULL)
		{
			LOG_ERROR("bytes buffer full");
			break;
		}

		int n = muggle_socket_ctx_read(ctx, ptr, MAX_MSG_SIZE);
		if (n > 0)
		{
			if (!muggle_bytes_buffer_writer_move_n(bytes_buf, ptr, n))
			{
				MUGGLE_ASSERT(0);
				MUGGLE_LOG_ERROR("bytes buffer inner error!");
				muggle_socket_ctx_shutdown(ctx);
				return;
			}

			if (n < MAX_MSG_SIZE)
			{
				break;
			}
		}
		else if (n <= 0 &&
			MUGGLE_SOCKET_LAST_ERRNO != MUGGLE_SYS_ERRNO_INTR &&
			MUGGLE_SOCKET_LAST_ERRNO != MUGGLE_SYS_ERRNO_WOULDBLOCK)
		{
			return;
		}
	}

	while (1)
	{
		uint32_t len;
		if (!muggle_bytes_buffer_fetch(bytes_buf, 4, &len))
		{
			// failed get length information
			break;
		}

		// in this example, time string is yyyy-mm-dd hh:MM:ss, 65536 is enough
		len = ntohl(len);
		if (len > MAX_MSG_SIZE - 4)
		{
			MUGGLE_LOG_ERROR("time string too long, must something wrong");
			muggle_socket_ctx_shutdown(ctx);
			return;
		}

		// get readable number of bytes
		int pkg_size = len + 4;
		int readable = muggle_bytes_buffer_readable(bytes_buf);
		if (readable < pkg_size)
		{
			// wait for more bytes from network
			break;
		}

		// parse package
		char *ptr = muggle_bytes_buffer_reader_fc(bytes_buf, pkg_size);
		if (ptr)
		{
			on_timestr(ptr + 4);
			if (!muggle_bytes_buffer_reader_move(bytes_buf, pkg_size))
			{
				MUGGLE_ASSERT(0);
				LOG_ERROR("failed buffer reader move");
				muggle_socket_ctx_shutdown(ctx);
				return;
			}
		}
		else
		{
			// there have no contiguous for this package
			char buf[MAX_MSG_SIZE];
			if (!muggle_bytes_buffer_read(bytes_buf, pkg_size, buf))
			{
				MUGGLE_ASSERT(0);
				MUGGLE_LOG_ERROR("failed buffer read");
				muggle_socket_ctx_shutdown(ctx);
				return;
			}
			on_timestr(buf + 4);
		}
	}
}

void run_conn_thread(conn_thread_args_t *args)
{
	muggle_thread_t th;
	muggle_thread_create(&th, conn_thread_routine, args);
	muggle_thread_detach(&th);
}

void on_add_ctx(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	LOG_INFO("socket context ready");
}

void on_message(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	int socket_ctx_type = muggle_socket_ctx_type(ctx);
	switch (socket_ctx_type)
	{
		case MUGGLE_SOCKET_CTX_TYPE_UDP:
		{
			on_udp_msg(evloop, ctx);
		}break;
		case MUGGLE_SOCKET_CTX_TYPE_TCP_CLIENT:
		{
			on_tcp_msg(evloop, ctx);
		}break;
	}
}

void on_close(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	LOG_INFO("socket context closed, try to reconnect");
	conn_thread_args_t *args = (conn_thread_args_t*)muggle_evloop_get_data(evloop);
	run_conn_thread(args);
}

void on_release(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	muggle_bytes_buffer_t *bytes_buf = (muggle_bytes_buffer_t*)muggle_socket_ctx_get_data(ctx);
	muggle_bytes_buffer_destroy(bytes_buf);
	free(bytes_buf);
}
