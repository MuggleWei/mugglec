#include "echo_serv_handle.h"

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

		free(peer->data);
		peer->data = NULL;
	}
}

void on_message(struct muggle_socket_event *ev, struct muggle_socket_peer *peer)
{
	char buf[4096];
	int n, num_bytes;
	int ret = 0;
	struct sockaddr_storage addr;
	muggle_socklen_t addrlen;
	while (1)
	{
		if (peer->peer_type == MUGGLE_SOCKET_PEER_TYPE_UDP_PEER)
		{
			addrlen = sizeof(addr);
			n = muggle_socket_peer_recvfrom(peer, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &addrlen);
			if (n > 0)
			{
				num_bytes = muggle_socket_peer_sendto(peer, buf, n, 0, (struct sockaddr*)&addr, addrlen);
			}
			else
			{
				break;
			}
		}
		else if (peer->peer_type == MUGGLE_SOCKET_PEER_TYPE_TCP_PEER)
		{
			n = muggle_socket_peer_recv(peer, buf, sizeof(buf), 0);
			if (n > 0)
			{
				num_bytes = muggle_socket_peer_send(peer, buf, n, 0);
			}
			else
			{
				break;
			}
		}
		else
		{
			MUGGLE_LOG_ERROR("something wrong");
			exit(EXIT_FAILURE);
		}

		if (n != num_bytes)
		{
			break;
		}
	}
}

void on_timer(struct muggle_socket_event *ev)
{
	MUGGLE_LOG_INFO("timer with interval(%dms) output: %s", ev->timeout_ms, (const char*)ev->datas);
}
