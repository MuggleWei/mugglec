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
	int n, num_bytes;
	char buf[4096];
	while (1)
	{
		if (peer->peer_type == MUGGLE_SOCKET_PEER_TYPE_UDP_PEER)
		{
			struct sockaddr_storage addr;
			muggle_socklen_t addrlen;
			n = muggle_socket_peer_recvfrom(
				peer, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &addrlen);
			if (n <= 0)
			{
				break;
			}

			num_bytes = muggle_socket_peer_sendto(peer, buf, n, 0, (struct sockaddr*)&addr, addrlen);
		}
		else if (peer->peer_type == MUGGLE_SOCKET_PEER_TYPE_TCP_PEER)
		{
			n = muggle_socket_peer_recv(peer, buf, sizeof(buf), 0);
			if (n <= 0)
			{
				break;
			}

			num_bytes = muggle_socket_peer_send(peer, buf, n, 0);
		}

		if (n != num_bytes)
		{
			MUGGLE_LOG_WARNING("partially written");
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

	muggle_socket_peer_t peers[2];

	// create tcp listen socket
	if (muggle_tcp_listen(host, serv, 512, &peers[0]) == MUGGLE_INVALID_SOCKET)
	{
		MUGGLE_LOG_ERROR("failed create tcp listen for %s:%s", host, serv);
		exit(EXIT_FAILURE);
	}

	// create udp bind socket
	if (muggle_udp_bind(host, serv, &peers[1]) == MUGGLE_INVALID_SOCKET)
	{
		MUGGLE_LOG_ERROR("failed create udp bind for %s:%s", host, serv);
		exit(EXIT_FAILURE);
	}

	// fill up event loop input arguments
	muggle_socket_event_init_arg_t ev_init_arg;
	memset(&ev_init_arg, 0, sizeof(ev_init_arg));
	ev_init_arg.cnt_peer = 2;
	ev_init_arg.peers = peers;
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
