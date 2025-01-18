#include "time_serv_handle.h"

static muggle_thread_ret_t mcast_conn_routine(void *p_args)
{
	evloop_data_t *args = (evloop_data_t*)p_args;

	muggle_socket_t fd = MUGGLE_INVALID_SOCKET;
	do {
		LOG_INFO("multicast connect: %s %s", args->multicast_host, args->multicast_serv);
		fd = muggle_udp_connect(args->multicast_host, args->multicast_serv);
		if (fd == MUGGLE_INVALID_SOCKET)
		{
			LOG_SYS_ERR(LOG_LEVEL_ERROR, "failed connect multicast addr");
			muggle_msleep(3000);
			continue;
		}
		break;
	} while(1);

	LOG_INFO("success multicast connect");

	muggle_socket_context_t *ctx =
		(muggle_socket_context_t*)malloc(sizeof(muggle_socket_context_t));
	muggle_socket_ctx_init(ctx, fd, NULL, MUGGLE_SOCKET_CTX_TYPE_UDP);

	muggle_socket_evloop_add_ctx(args->evloop, ctx);

	return 0;
}

static muggle_thread_ret_t tcp_listen_routine(void *p_args)
{
	evloop_data_t *args = (evloop_data_t*)p_args;

	muggle_socket_t fd = MUGGLE_INVALID_SOCKET;
	do {
		LOG_INFO("tcp listen: %s %s", args->host, args->serv);
		fd = muggle_tcp_listen(args->host, args->serv, 512);
		if (fd == MUGGLE_INVALID_SOCKET)
		{
			LOG_SYS_ERR(LOG_LEVEL_ERROR, "failed listen");
			muggle_msleep(3000);
			continue;
		}
		break;
	} while(1);

	LOG_INFO("success tcp listen");

	muggle_socket_context_t *ctx =
		(muggle_socket_context_t*)malloc(sizeof(muggle_socket_context_t));
	muggle_socket_ctx_init(ctx, fd, NULL, MUGGLE_SOCKET_CTX_TYPE_TCP_LISTEN);

	muggle_socket_evloop_add_ctx(args->evloop, ctx);

	return 0;
}

void run_tcp_listen(evloop_data_t *args)
{
	if (args->host && args->serv)
	{
		muggle_thread_t th;
		muggle_thread_create(&th, tcp_listen_routine, args);
		muggle_thread_detach(&th);
	}
}

void run_mcast_conn(evloop_data_t *args)
{
	if (args->multicast_host && args->multicast_serv)
	{
		muggle_thread_t th;
		muggle_thread_create(&th, mcast_conn_routine, args);
		muggle_thread_detach(&th);
	}
}

void on_add_ctx(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	evloop_data_t *data = muggle_evloop_get_data(evloop);

	switch (ctx->sock_type)
	{
		case MUGGLE_SOCKET_CTX_TYPE_TCP_LISTEN:
		{
			LOG_INFO("on add socket context: listen");
			data->listen_ctx = ctx;
		} break;
		case MUGGLE_SOCKET_CTX_TYPE_UDP:
		{
			LOG_INFO("on add socket context: mcast");
			data->mcast_ctx = ctx;
		} break;
		default:
		{
			LOG_ERROR("something wrong");
		}break;
	}
}

void on_connect(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	evloop_data_t *data = muggle_evloop_get_data(evloop);

	char addr[MUGGLE_SOCKET_ADDR_STRLEN];
	muggle_socket_remote_addr(ctx->base.fd, addr, sizeof(addr), 0);
	LOG_INFO("new TCP connection: %s", addr);

	muggle_linked_list_node_t *node = muggle_linked_list_append(&data->conn_list, NULL, ctx);
	muggle_socket_ctx_set_data(ctx, node);
}

void on_timer(muggle_event_loop_t *evloop)
{
	evloop_data_t *data = muggle_evloop_get_data(evloop);

	// get timestamp
	char buf[65536];
	time_t ts = time(NULL);
	struct tm t;
	gmtime_r(&ts, &t);
	uint32_t n = (uint32_t)snprintf(&buf[4], sizeof(buf) - 4 - 1, "%d-%02d-%02d %02d:%02d:%02d",
		t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
		t.tm_hour, t.tm_min, t.tm_sec);
	buf[4 + n] = '\0';
	n += 1;
	*(uint32_t*)buf = htonl(n);

	size_t len = 4 + n;

	// send to mcast
	if (data->mcast_ctx)
	{
		muggle_socket_ctx_write(data->mcast_ctx, buf, len);
	}

	// send to TCP client
	muggle_linked_list_node_t *node = muggle_linked_list_first(&data->conn_list);
	for (; node; node = muggle_linked_list_next(&data->conn_list, node))
	{
		muggle_socket_context_t *ctx = (muggle_socket_context_t*)node->data;
		muggle_socket_ctx_write(ctx, buf, len);
	}
}

void on_close(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	evloop_data_t *data = muggle_evloop_get_data(evloop);

	if (ctx == data->mcast_ctx)
	{
		LOG_INFO("mcast closed");
		data->mcast_ctx = NULL;
		run_mcast_conn(data);
	}
	else if (ctx == data->listen_ctx)
	{
		LOG_INFO("listen closed");
		data->listen_ctx = NULL;
		run_tcp_listen(data);
	}
	else
	{
		char addr[MUGGLE_SOCKET_ADDR_STRLEN];
		if (muggle_socket_remote_addr(ctx->base.fd, addr, sizeof(addr), 0)) {
			LOG_INFO("TCP disconnection: %s", addr);
		} else {
			LOG_INFO("TCP disconnection: (failed get addr)");
		}
	}
}

void on_release(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	evloop_data_t *data = muggle_evloop_get_data(evloop);

	if (ctx == data->mcast_ctx || ctx == data->listen_ctx)
	{
		return;
	}

	char addr[MUGGLE_SOCKET_ADDR_STRLEN];
	if (muggle_socket_remote_addr(ctx->base.fd, addr, sizeof(addr), 0)) {
		LOG_INFO("TCP context release user data: %s", addr);
	} else {
		LOG_INFO("TCP context release user data: (failed get addr)");
	}

	muggle_linked_list_node_t *node = (muggle_linked_list_node_t*)muggle_socket_ctx_get_data(ctx);
	muggle_linked_list_remove(&data->conn_list, node, NULL, NULL);
}
