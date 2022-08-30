#include "udp_receiver.h"

void run_udp_receiver(
	const char *host, const char *port,
	int busy_mode,
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

	if (busy_mode)
	{
		muggle_socket_set_nonblock(fd, 1);
	}

	muggle_socket_context_t ctx;
	muggle_socket_ctx_init(&ctx, fd, NULL, MUGGLE_SOCKET_CTX_TYPE_UDP);

	char buf[65536];
	while (1)
	{
		int n = muggle_socket_read(fd, buf, sizeof(buf));
		if (n > 0)
		{
			if (onRecvPkg(&ctx, (struct pkg*)buf, handle, config) != 0)
			{
				break;
			}
		}
		else
		{
			int errnum = MUGGLE_SOCKET_LAST_ERRNO;
			if (busy_mode && errnum == MUGGLE_SYS_ERRNO_WOULDBLOCK)
			{
				continue;
			}
			else
			{
				muggle_event_strerror(errnum, buf, sizeof(buf));
				LOG_ERROR("failed socket read: %s", buf);
			}
			break;
		}
	}

	muggle_socket_ctx_close(&ctx);
}
