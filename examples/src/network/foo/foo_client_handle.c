#include "foo_client_handle.h"
#include "foo/foo_handle.h"
#include "foo/foo_dispatcher.h"
#include "foo/codec/codec_bytes.h"
#include "foo/foo_msg.h"

typedef struct foo_client_thread_args
{
	muggle_event_loop_t *evloop;
	sys_args_t *args;
} foo_client_thread_args_t;

static muggle_thread_ret_t tcp_client_connect(void *p_args)
{
	foo_client_thread_args_t *args = (foo_client_thread_args_t*)p_args;

	// tcp connect
	muggle_socket_t fd = MUGGLE_INVALID_SOCKET;
	do {
		fd = muggle_tcp_connect(args->args->host, args->args->port, 3);
		if (fd == MUGGLE_INVALID_SOCKET)
		{
			LOG_ERROR("failed tcp connect %s %s", args->args->host, args->args->port);
			muggle_msleep(3000);
		}
	} while (fd == MUGGLE_INVALID_SOCKET);

	// new context
	muggle_socket_context_t *ctx =
		(muggle_socket_context_t*)malloc(sizeof(muggle_socket_context_t));
	muggle_socket_ctx_init(ctx, fd, NULL, MUGGLE_SOCKET_CTX_TYPE_TCP_CLIENT);

	muggle_socket_evloop_add_ctx(args->evloop, ctx);

	// free arguments
	free(args);

	return 0;
};

void init_tcp_client_handle(
	muggle_event_loop_t *evloop,
	muggle_socket_evloop_handle_t *handle,
	sys_args_t *args)
{
	// init socket handle
	muggle_socket_evloop_handle_init(handle);
	muggle_socket_evloop_handle_set_cb_add_ctx(handle, tcp_client_on_add_ctx);
	muggle_socket_evloop_handle_set_cb_msg(handle, tcp_client_on_message);
	muggle_socket_evloop_handle_set_cb_close(handle, tcp_client_on_close);
	muggle_socket_evloop_handle_set_cb_release(handle, tcp_client_on_release);
	muggle_socket_evloop_handle_set_cb_timer(handle, tcp_client_on_timer);
	muggle_socket_evloop_handle_set_timer_interval(handle, 1 * 1000);
	muggle_socket_evloop_handle_attach(handle, evloop);
	LOG_INFO("socket handle attached event loop");

	// set event loop data
	foo_handle_init(evloop, args);

	// run connect thread
	foo_client_thread_args_t *th_args = (foo_client_thread_args_t*)malloc(sizeof(foo_client_thread_args_t));
	memset(th_args, 0, sizeof(*th_args));
	th_args->evloop = evloop;
	th_args->args = args;

	muggle_thread_t th;
	muggle_thread_create(&th, tcp_client_connect, th_args);
	muggle_thread_detach(&th);

	// rand seed
	srand((unsigned int)time(NULL));
}

void tcp_client_on_add_ctx(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	// on connect
	foo_on_connect(evloop, ctx);

	LOG_INFO("success connect to server");

	// request login
	foo_msg_req_login_t req;
	memset(&req, 0, sizeof(req));
	req.header.msg_type = FOO_MSG_TYPE_REQ_LOGIN;
	req.user_id = 5;
	strncpy(req.password, "123456", sizeof(req.password)-1);

	foo_dispatcher_t *dispatcher = foo_handle_dipatcher(evloop);
	foo_dispatcher_send(dispatcher, evloop, ctx, &req, sizeof(req));
}

void tcp_client_on_message(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	foo_on_message(evloop, ctx);
}

void tcp_client_on_close(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	LOG_INFO("Disconnection");

	foo_evloop_data_t *evloop_data = (foo_evloop_data_t*)muggle_evloop_get_data(evloop);

	foo_client_thread_args_t *th_args = (foo_client_thread_args_t*)malloc(sizeof(foo_client_thread_args_t));
	memset(th_args, 0, sizeof(*th_args));
	th_args->evloop = evloop;
	th_args->args = (sys_args_t*)evloop_data->user_data;

	muggle_thread_t th;
	muggle_thread_create(&th, tcp_client_connect, th_args);
	muggle_thread_detach(&th);
}

void tcp_client_on_release(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	LOG_INFO("on_release");
	foo_on_release(evloop, ctx);
}

void tcp_client_on_timer(muggle_event_loop_t *evloop)
{
	static uint32_t s_req_id = 0;

	muggle_linked_list_t *conn_list = foo_handle_conn_list(evloop);
	muggle_linked_list_node_t *node = muggle_linked_list_first(conn_list);
	if (node)
	{
		muggle_socket_context_t *ctx = (muggle_socket_context_t*)node->data;

		uint32_t cnt = ((uint32_t)rand()) % MAX_MSG_REQ_SUM_LEN + 1;
		uint32_t datalen = sizeof(foo_msg_req_sum_t) + sizeof(uint32_t) * cnt;

		char str[MAX_MSG_REQ_SUM_LEN * 5 + 1];
		memset(str, 0, sizeof(str));
		char *p = str;

		foo_msg_req_sum_t *req = (foo_msg_req_sum_t*)malloc(datalen);
		memset(req, 0, sizeof(*req));

		uint32_t *arr = (uint32_t*)(req + 1);
		for (uint32_t i = 0; i < cnt; i++)
		{
			// int value belong to [-100, 100]
			arr[i] = (int32_t)((uint32_t)rand() % 201) - 100;
			int offset = snprintf(p, 5, "%d ", arr[i]);
			p += offset;
		}

		req->header.msg_type = FOO_MSG_TYPE_REQ_SUM;
		req->req_id = s_req_id++;
		req->arr_len = cnt;

		LOG_INFO(
			"send request sum message: "
			"req_id=%lu, req_array=[%s]",
			(unsigned long)req->req_id, str);

		foo_dispatcher_t *dispatcher = foo_handle_dipatcher(evloop);
		foo_dispatcher_send(dispatcher, evloop, ctx, req, datalen);

		free(req);
	}
}

void tcp_client_on_rsp_login(
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx,
	void *msg)
{
	foo_msg_rsp_login_t *rsp = (foo_msg_rsp_login_t*)msg;

	LOG_INFO(
		"recv response login message: "
		"result=%d",
		(int)rsp->login_result);
}

void tcp_client_on_msg_ping(
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx,
	void *msg)
{
	foo_msg_ping_t *req = (foo_msg_ping_t*)msg;

	LOG_INFO("recv ping message: sec=%llu", (unsigned long long)req->sec);

	foo_msg_pong_t rsp;
	memset(&rsp, 0, sizeof(rsp));
	rsp.header.msg_type = FOO_MSG_TYPE_PONG;
	rsp.sec = req->sec;

	foo_dispatcher_t *dispatcher = foo_handle_dipatcher(evloop);
	foo_dispatcher_send(dispatcher, evloop, ctx, &rsp, sizeof(rsp));
}

void tcp_server_on_rsp_sum(
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx,
	void *msg)
{
	foo_msg_rsp_sum_t *rsp = (foo_msg_rsp_sum_t*)msg;

	LOG_INFO(
		"recv response sum message: "
		"req_id=%lu, sum=%ld",
		(unsigned long)rsp->req_id, (long)rsp->sum);
}
