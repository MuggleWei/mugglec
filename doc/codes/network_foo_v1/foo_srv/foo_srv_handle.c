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
	if (peer->data == NULL)
	{
		return;
	}
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
	MUGGLE_LOG_DEBUG(
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
