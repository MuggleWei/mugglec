#ifndef EXAMPLE_ECHO_SERV_HANDLE_H_
#define EXAMPLE_ECHO_SERV_HANDLE_H_

#include "muggle/c/muggle_c.h"

int on_connect(
	struct muggle_socket_event *ev, struct muggle_socket_peer *listen_peer, struct muggle_socket_peer *peer);

int on_error(struct muggle_socket_event *ev, struct muggle_socket_peer *peer);

int on_message(struct muggle_socket_event *ev, struct muggle_socket_peer *peer);

void on_timer(struct muggle_socket_event *ev);

#endif
