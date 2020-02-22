#ifndef EXAMPLE_TIME_SERV_HANDLE_H_
#define EXAMPLE_TIME_SERV_HANDLE_H_

#include "muggle/c/muggle_c.h"

struct peer_container
{
	int max_peer;
	int cnt_peer;
	muggle_socket_peer_t *udp_peer;
	muggle_socket_peer_t **peers;
};

int on_connect(
	struct muggle_socket_event *ev, struct muggle_socket_peer *listen_peer, struct muggle_socket_peer *peer);

int on_error(struct muggle_socket_event *ev, struct muggle_socket_peer *peer);

void on_timer(struct muggle_socket_event *ev);

#endif