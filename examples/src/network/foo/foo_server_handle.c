#include "foo_server_handle.h"
#include "foo/foo_handle.h"
#include "foo/foo_dispatcher.h"
#include "foo/codec/codec_bytes.h"

#define SERV_TIMER_INTERVAL_SEC 5

typedef struct foo_server_thread_args
{
	muggle_event_loop_t *evloop;
	sys_args_t *args;
} foo_server_thread_args_t;

static muggle_thread_ret_t tcp_server_listen(void *p_args)
{
	foo_server_thread_args_t *args = (foo_server_thread_args_t*)p_args;

	// tcp listen
	muggle_socket_t fd = MUGGLE_INVALID_SOCKET;
	do {
		fd = muggle_tcp_listen(args->args->host, args->args->port, 512);
		if (fd == MUGGLE_INVALID_SOCKET)
		{
			LOG_ERROR("failed tcp listen %s %s", args->args->host, args->args->port);
			muggle_msleep(3000);
		}
	} while (fd == MUGGLE_INVALID_SOCKET);

	// new context
	muggle_socket_context_t *ctx =
		(muggle_socket_context_t*)malloc(sizeof(muggle_socket_context_t));
	muggle_socket_ctx_init(ctx, fd, NULL, MUGGLE_SOCKET_CTX_TYPE_TCP_LISTEN);

	muggle_socket_evloop_add_ctx(args->evloop, ctx);

	// free arguments
	free(args);

	return 0;
}

void init_tcp_server_handle(
	muggle_event_loop_t *evloop,
	muggle_socket_evloop_handle_t *handle,
	sys_args_t *args)
{
	// init socket handle
	muggle_socket_evloop_handle_init(handle);
	muggle_socket_evloop_handle_set_cb_conn(handle, tcp_serv_on_connect);
	muggle_socket_evloop_handle_set_cb_msg(handle, tcp_serv_on_message);
	muggle_socket_evloop_handle_set_cb_close(handle, tcp_serv_on_close);
	muggle_socket_evloop_handle_set_cb_release(handle, tcp_serv_on_release);
	muggle_socket_evloop_handle_set_cb_timer(handle, tcp_serv_on_timer);
	muggle_socket_evloop_handle_set_timer_interval(handle, SERV_TIMER_INTERVAL_SEC * 1000);
	muggle_socket_evloop_handle_attach(handle, evloop);
	LOG_INFO("socket handle attached event loop");

	// set event loop data
	foo_handle_init(evloop, NULL);

	// run connect thread
	foo_server_thread_args_t *th_args = (foo_server_thread_args_t*)malloc(sizeof(foo_server_thread_args_t));
	memset(th_args, 0, sizeof(*th_args));
	th_args->evloop = evloop;
	th_args->args = args;

	muggle_thread_t th;
	muggle_thread_create(&th, tcp_server_listen, th_args);
	muggle_thread_detach(&th);
}

void tcp_serv_on_connect(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	// limit connection number
	muggle_linked_list_t *conn_list = foo_handle_conn_list(evloop);
	if (muggle_linked_list_size(conn_list) > CONN_NUM_LIMIT)
	{
		LOG_WARNING("Connection reach the limit, reject new connection");
		muggle_socket_ctx_shutdown(ctx);
		return;
	}

	// on connect
	foo_on_connect(evloop, ctx);

	foo_socket_ctx_data_t *ctx_data = (foo_socket_ctx_data_t*)muggle_socket_ctx_get_data(ctx);
	LOG_INFO("TCP connection: %s", ctx_data->straddr);
}

void tcp_serv_on_message(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	foo_on_message(evloop, ctx);
}

void tcp_serv_on_close(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	foo_socket_ctx_data_t *ctx_data =
		(foo_socket_ctx_data_t*)muggle_socket_ctx_get_data(ctx);
	LOG_INFO("TCP Disconnection: %s", ctx_data->straddr);
}

void tcp_serv_on_release(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	// release socket context data
	foo_on_release(evloop, ctx);
}

void tcp_serv_on_timer(muggle_event_loop_t *evloop)
{
	foo_dispatcher_t *dispatcher = foo_handle_dipatcher(evloop);
	muggle_linked_list_t *conn_list = foo_handle_conn_list(evloop);

	uint64_t cur_sec = (uint64_t)time(NULL);

	muggle_linked_list_node_t *node = muggle_linked_list_first(conn_list);
	for (; node; node = muggle_linked_list_next(conn_list, node))
	{
		muggle_socket_context_t *ctx = (muggle_socket_context_t*)node->data;
		foo_socket_ctx_data_t *user_data = (foo_socket_ctx_data_t*)muggle_socket_ctx_get_data(ctx);
		uint64_t elapsed = cur_sec - user_data->last_sec;
		if (elapsed > SERV_TIMER_INTERVAL_SEC * 3)
		{
			LOG_WARNING("heartbeat timeout: addr=%s", user_data->straddr);
			muggle_socket_ctx_shutdown(ctx);
			continue;
		}

		foo_msg_ping_t ping;
		memset(&ping, 0, sizeof(ping));
		ping.header.msg_type = FOO_MSG_TYPE_PING;
		ping.sec = cur_sec;
		foo_dispatcher_send(dispatcher, evloop, ctx, &ping, sizeof(ping));

		LOG_DEBUG("send ping message: addr=%s", user_data->straddr);
	}
}

void tcp_server_on_req_login(
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx,
	void *msg)
{
	foo_msg_req_login_t *req = (foo_msg_req_login_t*)msg;

	foo_socket_ctx_data_t *ctx_data = (foo_socket_ctx_data_t*)muggle_socket_ctx_get_data(ctx);
	LOG_INFO(
		"recv request login message: "
		"addr=%s, user_id=%lu, passwd=******",
		ctx_data->straddr, (unsigned long)req->user_id);

	// do something check user password
	
	foo_msg_rsp_login_t rsp;
	memset(&rsp, 0, sizeof(rsp));
	rsp.header.msg_type = FOO_MSG_TYPE_RSP_LOGIN;
	rsp.login_result = 1;

	foo_dispatcher_t *dispatcher = foo_handle_dipatcher(evloop);
	foo_dispatcher_send(dispatcher, evloop, ctx, &rsp, sizeof(rsp));
}

void tcp_server_on_msg_pong(
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx,
	void *msg)
{
	// update alive ts
	foo_socket_ctx_data_t *ctx_data = (foo_socket_ctx_data_t*)muggle_socket_ctx_get_data(ctx);
	ctx_data->last_sec = (uint64_t)time(NULL);

	foo_socket_ctx_data_t *user_data = (foo_socket_ctx_data_t*)muggle_socket_ctx_get_data(ctx);
	LOG_DEBUG("recv pong message: addr=%s", user_data->straddr);
}

void tcp_server_on_req_sum(
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx,
	void *msg)
{
	foo_socket_ctx_data_t *ctx_data = (foo_socket_ctx_data_t*)muggle_socket_ctx_get_data(ctx);
	foo_msg_req_sum_t *req = (foo_msg_req_sum_t*)msg;

	LOG_INFO(
		"recv request sum message: "
		"addr=%s, req_id=%lu",
		ctx_data->straddr, (unsigned long)req->req_id);

	int32_t sum = 0;
	int32_t *arr = (int32_t*)(req + 1);
	for (uint32_t i = 0; i < req->arr_len; i++)
	{
		sum += arr[i];
	}

	foo_msg_rsp_sum_t rsp;
	memset(&rsp, 0, sizeof(rsp));
	rsp.header.msg_type = FOO_MSG_TYPE_RSP_SUM;
	rsp.req_id = req->req_id;
	rsp.sum = sum;

	foo_dispatcher_t *dispatcher = foo_handle_dipatcher(evloop);
	foo_dispatcher_send(dispatcher, evloop, ctx, &rsp, sizeof(rsp));
}
