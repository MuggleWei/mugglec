/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "tcp_serv.h"

static void tcp_serv_on_connect(
	struct muggle_socket_event *ev, struct muggle_socket_peer *listen_peer, struct muggle_socket_peer *peer)
{
	sendPkgs(peer);
}

static void tcp_serv_on_error(struct muggle_socket_event *ev, struct muggle_socket_peer *peer)
{
	muggle_socket_event_loop_exit(ev);
}

void run_tcp_serv(const char *host, const char *port)
{
	muggle_socket_peer_t tcp_peer;

	// create tcp listen socket
	if (muggle_tcp_listen(host, port, 512, &tcp_peer) == MUGGLE_INVALID_SOCKET)
	{
		MUGGLE_LOG_ERROR("failed create tcp listen for %s:%s", host, port);
		exit(EXIT_FAILURE);
	}

	// set TCP_NODELAY
	int enable = 1;
	setsockopt(tcp_peer.fd, IPPROTO_TCP, TCP_NODELAY, (char*)&enable, sizeof(enable));

	// fill up event loop input arguments
	muggle_socket_event_init_arg_t ev_init_arg;
	memset(&ev_init_arg, 0, sizeof(ev_init_arg));
	ev_init_arg.ev_loop_type = MUGGLE_SOCKET_EVENT_LOOP_TYPE_NULL;
	ev_init_arg.cnt_peer = 1;
	ev_init_arg.peers = &tcp_peer;
	ev_init_arg.timeout_ms = -1;
	ev_init_arg.on_connect = tcp_serv_on_connect;
	ev_init_arg.on_error = tcp_serv_on_error;

	// event loop
	muggle_socket_event_t ev;
	if (muggle_socket_event_init(&ev_init_arg, &ev) != 0)
	{
		MUGGLE_LOG_ERROR("failed init socket event");
		exit(EXIT_FAILURE);
	}
	muggle_socket_event_loop(&ev);
}
