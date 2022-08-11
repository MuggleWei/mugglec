#include "udp_receiver.h"

void run_udp_receiver(
	const char *host, const char *port,
	int flags,
	muggle_benchmark_handle_t *handle,
	muggle_benchmark_config_t *config)
{
	// bind address
	muggle_socket_t fd = muggle_udp_bind(host, port);
	if (fd == MUGGLE_INVALID_SOCKET)
	{
		MUGGLE_LOG_ERROR("failed create udp bind for %s:%s", host, port);
		exit(EXIT_FAILURE);
	}

	muggle_socket_context_t ctx;
	muggle_socket_ctx_init(&ctx, fd, NULL, MUGGLE_SOCKET_CTX_TYPE_UDP);

	char buf[65536];
	while (1)
	{
		int n = muggle_socket_recv(fd, buf, sizeof(buf), flags);
		if (n > 0)
		{
			if (onRecvPkg(&ctx, (struct pkg*)buf, handle, config) != 0)
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

	muggle_socket_ctx_close(&ctx);
}
