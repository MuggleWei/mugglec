#include "tcp_serv.h"

struct tcp_serv_user_data
{
	int flags;
	muggle_benchmark_handle_t *handle;
	muggle_benchmark_config_t *config;
};

static void tcp_serv_on_connect(
	struct muggle_socket_event *ev, struct muggle_socket_peer *listen_peer, struct muggle_socket_peer *peer)
{
	struct tcp_serv_user_data *data = (struct tcp_serv_user_data*)ev->datas;
	sendPkgs(peer, data->flags, data->handle, data->config);
}

static void tcp_serv_on_error(struct muggle_socket_event *ev, struct muggle_socket_peer *peer)
{
	muggle_socket_event_loop_exit(ev);
}

void run_tcp_serv(
	const char *host, const char *port,
	int flags,
	muggle_benchmark_handle_t *handle,
	muggle_benchmark_config_t *config)
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
	muggle_setsockopt(tcp_peer.fd, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));

	// user data
	struct tcp_serv_user_data user_data;
	memset(&user_data, 0, sizeof(user_data));
	user_data.flags = flags;
	user_data.handle = handle;
	user_data.config = config;

	// fill up event loop input arguments
	muggle_socket_event_init_arg_t ev_init_arg;
	memset(&ev_init_arg, 0, sizeof(ev_init_arg));
	ev_init_arg.ev_loop_type = MUGGLE_SOCKET_EVENT_LOOP_TYPE_NULL;
	ev_init_arg.cnt_peer = 1;
	ev_init_arg.peers = &tcp_peer;
	ev_init_arg.timeout_ms = -1;
	ev_init_arg.on_connect = tcp_serv_on_connect;
	ev_init_arg.on_error = tcp_serv_on_error;
	ev_init_arg.datas = &user_data;

	// event loop
	muggle_socket_event_t ev;
	if (muggle_socket_event_init(&ev_init_arg, &ev) != 0)
	{
		MUGGLE_LOG_ERROR("failed init socket event");
		exit(EXIT_FAILURE);
	}
	muggle_socket_event_loop(&ev);
}
