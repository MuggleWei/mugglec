#include "tcp_server.h"
#include "struct.h"

void tcp_server_on_accept(muggle_event_loop_t *evloop, muggle_event_context_t *ctx)
{
	do {
		muggle_socket_t fd = tcp_server_do_accept(evloop, ctx);
		if (fd == MUGGLE_INVALID_SOCKET)
		{
			break;
		}
		tcp_server_on_connect(evloop, fd);
	} while(1);
}

muggle_socket_t tcp_server_do_accept(
	muggle_event_loop_t *evloop, muggle_event_context_t *listen_ctx)
{
	muggle_socket_t fd = MUGGLE_INVALID_SOCKET;
	do {
		fd = accept(muggle_ev_ctx_fd(listen_ctx), NULL, NULL);
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
	LOG_INFO("on TCP connection");

	muggle_event_context_t *ctx =
		(muggle_event_context_t*)malloc(sizeof(muggle_event_context_t));

	ev_data_t *ev_data = (ev_data_t*)malloc(sizeof(ev_data_t));
	memset(ev_data, 0, sizeof(*ev_data));
	ev_data->cb_read = tcp_on_message;
	ev_data->cb_close = tcp_on_close;
	
	muggle_ev_ctx_init(ctx, fd, (void*)ev_data);
	int ret = muggle_evloop_add_ctx(evloop, (muggle_event_context_t*)ctx);
	if (ret != 0)
	{
		free(ev_data);
		free(ctx);
		muggle_socket_close(fd);
	}
}

void tcp_on_message(muggle_event_loop_t *evloop, muggle_event_context_t *ctx)
{
	muggle_event_fd fd = muggle_ev_ctx_fd(ctx);

	char buf[1024];
	do {
		int n = muggle_ev_fd_read(fd, buf, sizeof(buf));
		if (n <= 0)
		{
			break;
		}

		if (muggle_ev_fd_write(fd, buf, n) != n)
		{
			muggle_ev_ctx_shutdown(ctx);
			break;
		}
	} while(1);
}

void tcp_on_close(muggle_event_loop_t *evloop, muggle_event_context_t *ctx)
{
	LOG_INFO("on disconnection");
	ev_data_t *ev_data = (ev_data_t*)muggle_ev_ctx_data(ctx);
	if (ev_data)
	{
		free(ev_data);
	}

	muggle_ev_ctx_close(ctx);
	free(ctx);
}