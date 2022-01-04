#include "udp_receiver.h"

void run_udp_receiver(
	const char *host, const char *port,
	int flags,
	muggle_benchmark_handle_t *handle,
	muggle_benchmark_config_t *config)
{
	// bind address
	muggle_socket_peer_t udp_peer;
	udp_peer.fd = muggle_udp_bind(host, port, &udp_peer);
	if (udp_peer.fd == MUGGLE_INVALID_SOCKET)
	{
		MUGGLE_LOG_ERROR("failed create udp bind for %s:%s", host, port);
		exit(EXIT_FAILURE);
	}

	muggle_benchmark_record_t record;
	char buf[65536];
	while (1)
	{
		int n = recv(udp_peer.fd, buf, sizeof(buf), flags);
		if (n > 0)
		{
			if (onRecvPkg(&udp_peer, (struct pkg*)buf, handle, config) != 0)
			{
				break;
			}
		}
		else
		{
#if ! defined(MUGGLE_PLATFORM_WINDOWS)
			if ((flags & MSG_DONTWAIT) && MUGGLE_SOCKET_LAST_ERRNO == EWOULDBLOCK)
			{
				continue;
			}
#endif
			break;
		}
	}
}
