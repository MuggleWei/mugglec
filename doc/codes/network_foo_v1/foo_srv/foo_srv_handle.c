#include "foo_srv_handle.h"

void on_connect(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *listen_peer,
	muggle_socket_peer_t *peer)
{
	foo_on_connect(ev, listen_peer, peer);
}

void on_error(muggle_socket_event_t *ev, muggle_socket_peer_t *peer)
{
	foo_on_error(ev, peer);
}

void on_close(muggle_socket_event_t *ev, muggle_socket_peer_t *peer)
{
	foo_on_close(ev, peer);
}

void on_message(muggle_socket_event_t *ev, muggle_socket_peer_t *peer)
{
	foo_on_message(ev, peer);
}

void on_timer(muggle_socket_event_t *ev)
{
}

void on_msg_req_login(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer,
	void *msg)
{
	foo_msg_req_login_t *req = (foo_msg_req_login_t*)msg;
	MUGGLE_LOG_INFO(
		"recv request login message:"
		"user_id=%lu, passwd=%s",
		(unsigned long)req->user_id, req->password);

	// do something check use password
	
	foo_msg_rsp_login_t rsp;
	memset(&rsp, 0, sizeof(rsp));
	rsp.header.msg_type = FOO_MSG_TYPE_RSP_LOGIN;
	rsp.login_result = 1;

	foo_send(ev, peer, (void*)&rsp, sizeof(rsp));
}

void on_msg_req_sum(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer,
	void *msg)
{
	foo_socket_peer_data_t *data = (foo_socket_peer_data_t*)peer->data;

	foo_msg_req_sum_t *req = (foo_msg_req_sum_t*)msg;
	MUGGLE_LOG_INFO(
		"recv request sum message: addr=%s",
		data->straddr);

	// check array length
	uint32_t expect_arr_len = (req->header.body_len - sizeof(req->arr_len) - sizeof(req->arr)) / sizeof(int32_t);
	if (expect_arr_len != req->arr_len)
	{
		MUGGLE_LOG_ERROR("invalid sum message array length: addr=%s", data->straddr);
		muggle_socket_peer_close(peer);
		return;
	}

	// pointer the array
	int32_t *arr = (int32_t*)((intptr_t)msg + (intptr_t)req->arr);

	int32_t sum = 0;
	for (uint32_t i = 0; i < expect_arr_len; i++)
	{
		sum += arr[i];
	}

	foo_msg_rsp_sum_t rsp;
	memset(&rsp, 0, sizeof(rsp));
	rsp.header.msg_type = FOO_MSG_TYPE_RSP_SUM;
	rsp.sum = sum;

	foo_send(ev, peer, (void*)&rsp, sizeof(rsp));
}
