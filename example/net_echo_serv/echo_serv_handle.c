#include "echo_serv_handle.h"

int on_connect(
	struct muggle_socket_event *ev, struct muggle_socket_peer *listen_peer, struct muggle_socket_peer *peer)
{
	char *straddr = (char*)malloc(MUGGLE_SOCKET_ADDR_STRLEN);
	if (muggle_socket_ntop((struct sockaddr*)&peer->addr, straddr, MUGGLE_SOCKET_ADDR_STRLEN, 0) == NULL)
	{
		snprintf(straddr, MUGGLE_SOCKET_ADDR_STRLEN, "unknown:unknown");
	}
	peer->data = (void*)straddr;

	MUGGLE_LOG_INFO("connect - %s", (char*)peer->data);

	return 0;
}

int on_error(struct muggle_socket_event *ev, struct muggle_socket_peer *peer)
{
	if (peer->data)
	{
		MUGGLE_LOG_INFO("disconnect - %s", (char*)peer->data);

		free(peer->data);
		peer->data = NULL;
	}

	return 0;
}

int on_message(struct muggle_socket_event *ev, struct muggle_socket_peer *peer)
{
	char buf[4096];
	int n;
	int ret = 0;
	struct sockaddr_storage addr;
	muggle_socklen_t addrlen;
	while (1)
	{
		if (peer->peer_type == MUGGLE_SOCKET_PEER_TYPE_UDP_PEER)
		{
			addrlen = sizeof(addr);
			n = recvfrom(peer->fd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &addrlen);
		}
		else if (peer->peer_type == MUGGLE_SOCKET_PEER_TYPE_TCP_PEER)
		{
			n = recv(peer->fd, buf, sizeof(buf), 0);
		}
		else
		{
			MUGGLE_LOG_ERROR("something wrong");
			exit(EXIT_FAILURE);
		}

		if (n > 0)
		{
			int num_bytes = 0;
			if (peer->peer_type == MUGGLE_SOCKET_PEER_TYPE_UDP_PEER)
			{
				num_bytes = sendto(peer->fd, buf, n, 0, (struct sockaddr*)&addr, addrlen);
			}
			else
			{
				num_bytes = send(peer->fd, buf, n, 0);
			}

			if (num_bytes != n)
			{
				if (num_bytes == MUGGLE_SOCKET_ERROR)
				{
					char err_msg[1024] = {0};
					muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
					MUGGLE_LOG_WARNING("failed send msg - %s", err_msg);
				}
				else
				{
					MUGGLE_LOG_WARNING("send buffer full");
				}
				
				muggle_socket_peer_close(peer);
				break;
			}
		}
		else if (n < 0)
		{
			if (MUGGLE_SOCKET_LAST_ERRNO == MUGGLE_SYS_ERRNO_INTR)
			{
				continue;
			}
			else if (MUGGLE_SOCKET_LAST_ERRNO == MUGGLE_SYS_ERRNO_WOULDBLOCK)
			{
				break;
			}

			if (peer->data)
			{
				char err_msg[1024];
				muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
				MUGGLE_LOG_INFO("failed recv from %s - %s", (char*)peer->data, (char*)peer->data);
			}
			
			muggle_socket_peer_close(peer);
			break;
		}
		else
		{
			muggle_socket_peer_close(peer);
			break;
		}
	}

	return ret;
}

void on_timer(struct muggle_socket_event *ev)
{
	MUGGLE_LOG_INFO("timer with interval(%dms) output: %s", ev->timeout_ms, (const char*)ev->datas);
}
