#include "muggle/c/muggle_c.h"

void on_connect(
	struct muggle_socket_event *ev, struct muggle_socket_peer *listen_peer, struct muggle_socket_peer *peer)
{
	char *straddr = (char*)malloc(MUGGLE_SOCKET_ADDR_STRLEN);
	if (muggle_socket_ntop((struct sockaddr*)&peer->addr, straddr, MUGGLE_SOCKET_ADDR_STRLEN, 0) == NULL)
	{
		snprintf(straddr, MUGGLE_SOCKET_ADDR_STRLEN, "unknown:unknown");
	}
	peer->data = (void*)straddr;

	MUGGLE_LOG_INFO("connect - %s", (char*)peer->data);
}

void on_error(struct muggle_socket_event *ev, struct muggle_socket_peer *peer)
{
	if (peer->data)
	{
		MUGGLE_LOG_INFO("disconnect - %s", (char*)peer->data);
	}
}

void on_close(struct muggle_socket_event *ev, struct muggle_socket_peer *peer)
{
	if (peer->data)
	{
		MUGGLE_LOG_INFO("close - %s", (char*)peer->data);

		free(peer->data);
		peer->data = NULL;
	}
}

void on_message(struct muggle_socket_event *ev, struct muggle_socket_peer *peer)
{
	char buf[4096];
	while (1)
	{
		int n = muggle_socket_peer_recv(peer, buf, sizeof(buf), 0);
		if (n > 0)
		{
			muggle_socket_peer_send(peer, buf, n, 0);
		}
		else
		{
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	// init log and socket library
	if (muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO) != 0)
	{
		MUGGLE_LOG_ERROR("failed initalize log");
		exit(EXIT_FAILURE);
	}

	if (muggle_socket_lib_init() != 0)
	{
		MUGGLE_LOG_ERROR("failed initalize socket library");
		exit(EXIT_FAILURE);
	}

	// create tcp listen socket peer
	if (argc < 3)
	{
		MUGGLE_LOG_ERROR("usage: %s <IP> <Port|Service>", argv[0]);
		exit(EXIT_FAILURE);
	}
	const char *host = argv[1];
	const char *serv = argv[2];

	muggle_socket_peer_t peer;
	if (muggle_tcp_listen(host, serv, 512, &peer) == MUGGLE_INVALID_SOCKET)
	{
		MUGGLE_LOG_ERROR("failed create tcp listen for %s:%s", host, serv);
		exit(EXIT_FAILURE);
	}

	// fill up event loop input arguments
	muggle_socket_event_init_arg_t ev_init_arg;
	memset(&ev_init_arg, 0, sizeof(ev_init_arg));
	ev_init_arg.cnt_peer = 1;
	ev_init_arg.peers = &peer;
	ev_init_arg.on_connect = on_connect;
	ev_init_arg.on_error = on_error;
	ev_init_arg.on_close = on_close;
	ev_init_arg.on_message = on_message;

	// event loop
	muggle_socket_event_t ev;
	if (muggle_socket_event_init(&ev_init_arg, &ev) != 0)
	{
		MUGGLE_LOG_ERROR("failed init socket event");
		exit(EXIT_FAILURE);
	}
	muggle_socket_event_loop(&ev);
	
	return 0;
}
