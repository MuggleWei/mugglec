#ifndef FOO_SRV_HANDLE_H_
#define FOO_SRV_HANDLE_H_

#include "foo/foo_handle.h"

void on_connect(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *listen_peer,
	muggle_socket_peer_t *peer);

void on_error(muggle_socket_event_t *ev, muggle_socket_peer_t *peer);

void on_close(muggle_socket_event_t *ev, muggle_socket_peer_t *peer);

void on_message(muggle_socket_event_t *ev, muggle_socket_peer_t *peer);

void on_timer(muggle_socket_event_t *ev);

///////////// message callback /////////////

void on_msg_req_login(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer,
	void *msg);

void on_msg_req_sum(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer,
	void *msg);

///////////// worker thread /////////////

#define NUM_WORKER (8)

typedef struct worker_thread_msg
{
	muggle_socket_event_t *ev;
	muggle_socket_peer_t  *peer;
	foo_msg_req_sum_t     req;
}worker_thread_msg_t;

typedef struct worker_thread_args
{
	int              idx;
	muggle_channel_t *chan;
}worker_thread_args_t;

muggle_thread_ret_t worker_thread(void *args);

#endif /* ifndef FOO_SRV_HANDLE_H_ */
