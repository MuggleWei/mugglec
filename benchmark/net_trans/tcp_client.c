#include "tcp_client.h"

void run_tcp_client(const char *host, const char *port, int is_busy,
					muggle_benchmark_handle_t *handle,
					muggle_benchmark_config_t *config)
{
	// create tcp connect socket
	muggle_socket_t fd = muggle_tcp_connect(host, port, 3);
	if (fd == MUGGLE_INVALID_SOCKET)
	{
		LOG_ERROR("failed connect %s %s", host, port);
		exit(EXIT_FAILURE);
	}

	if (is_busy) {
		muggle_socket_set_nonblock(fd, 1);
	}

	// set TCP_NODELAY
	int enable = 1;
	muggle_setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));

	muggle_socket_context_t ctx;
	muggle_socket_ctx_init(&ctx, fd, NULL, MUGGLE_SOCKET_CTX_TYPE_TCP_CLIENT);

	// sleep for a while, gurantee server on connect completed
	muggle_msleep(100);

	sendPkgs(&ctx, handle, config);

	muggle_socket_ctx_close(&ctx);
}
