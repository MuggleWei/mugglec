#ifndef FOO_CLI_HANDLE_H_
#define FOO_CLI_HANDLE_H_

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

void on_msg_rsp_login(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer,
	void *msg);

void on_msg_rsp_sum(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer,
	void *msg);

#endif /* ifndef FOO_CLI_HANDLE_H_ */
