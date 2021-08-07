#include "foo_cli_handle.h"

void on_connect(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *listen_peer,
	muggle_socket_peer_t *peer)
{
	foo_on_connect(ev, listen_peer, peer);

	foo_msg_req_login_t req;
	memset(&req, 0, sizeof(req));

	req.header.msg_type = FOO_MSG_TYPE_REQ_LOGIN;
	req.user_id = 10000;
	strncpy(req.password, "123456", sizeof(req.password) - 1);

	foo_send(ev, peer, &req, sizeof(req));
}

void on_error(muggle_socket_event_t *ev, muggle_socket_peer_t *peer)
{
	foo_on_error(ev, peer);
}

void on_close(muggle_socket_event_t *ev, muggle_socket_peer_t *peer)
{
	foo_on_close(ev, peer);

	foo_ev_data_t *ev_data = (foo_ev_data_t*)ev->datas;
	ev_data->user_data = NULL;

	// exit event loop
	muggle_socket_event_loop_exit(ev);
}

void on_message(muggle_socket_event_t *ev, muggle_socket_peer_t *peer)
{
	foo_on_message(ev, peer);
}

#define MAX_ARRAY_LEN 16

#pragma pack(push)
#pragma pack(1)

typedef struct foo_msg_req_sum_ext
{
	foo_msg_req_sum_t req;
	int32_t           arr[MAX_ARRAY_LEN];
}foo_msg_req_sum_ext_t;

#pragma pack(pop)

void on_timer(muggle_socket_event_t *ev)
{
	foo_ev_data_t *ev_data = (foo_ev_data_t*)ev->datas;
	muggle_socket_peer_t *peer = (muggle_socket_peer_t*)ev_data->user_data;
	if (peer == NULL)
	{
		return;
	}

	foo_msg_req_sum_ext_t ext_req;
	memset(&ext_req, 0, sizeof(ext_req));

	char str[MAX_ARRAY_LEN * 5 + 1];
	memset(str, 0, sizeof(str));
	char *p = str;

	uint32_t cnt = ((uint32_t)rand()) % MAX_ARRAY_LEN;
	for (uint32_t i = 0; i < cnt; i++)
	{
		// int value belong to [-100, 100]
		ext_req.arr[i] = (uint32_t)rand() % 201 - 100;
		int offset = snprintf(p, 5, "%d ", ext_req.arr[i]);
		p += offset;
	}

	MUGGLE_LOG_INFO("send request sum message: %s", str);

	ext_req.req.header.msg_type = FOO_MSG_TYPE_REQ_SUM;
	ext_req.req.arr_len = cnt;
	ext_req.req.arr = (int32_t*)(intptr_t)sizeof(foo_msg_req_sum_t);

	size_t num_bytes =
		sizeof(foo_msg_req_sum_t) + 
		cnt * sizeof(int32_t);
	foo_send(ev, peer, &ext_req, num_bytes);
}

///////////// message callback /////////////

void on_msg_rsp_login(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer,
	void *msg)
{
	foo_msg_rsp_login_t *rsp = (foo_msg_rsp_login_t*)msg;
	MUGGLE_LOG_INFO("login result: %s", rsp->login_result ? "success" : "failed");
}

void on_msg_rsp_sum(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer,
	void *msg)
{
	foo_msg_rsp_sum_t *rsp = (foo_msg_rsp_sum_t*)msg;
	MUGGLE_LOG_INFO("recv response sum message: %d", (int)rsp->sum);
}
