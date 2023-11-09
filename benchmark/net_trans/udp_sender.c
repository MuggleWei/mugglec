#include "udp_sender.h"

void run_udp_sender(const char *host, const char *port, int is_busy,
					muggle_benchmark_handle_t *handle,
					muggle_benchmark_config_t *config)
{
	muggle_socket_t fd = muggle_udp_connect(host, port);
	if (fd == MUGGLE_INVALID_SOCKET) {
		LOG_ERROR("failed connect udp sender target");
		exit(EXIT_FAILURE);
	}

	if (is_busy) {
		muggle_socket_set_nonblock(fd, 1);
	}

	muggle_socket_context_t ctx;
	muggle_socket_ctx_init(&ctx, fd, NULL, MUGGLE_SOCKET_CTX_TYPE_UDP);

	sendPkgs(&ctx, handle, config);

	muggle_socket_ctx_close(&ctx);
}
