#include "echo_serv_handle.h"

void on_message(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	int n = 0;
	char buf[4096];
	if (ctx->sock_type == MUGGLE_SOCKET_CTX_TYPE_TCP_CLIENT)
	{
		while ((n = muggle_socket_ctx_read(ctx, buf, sizeof(buf))) > 0)
		{
			muggle_socket_ctx_write(ctx, buf, n);
		}
	}
	else if (ctx->sock_type == MUGGLE_SOCKET_CTX_TYPE_UDP)
	{
		struct sockaddr_storage saddr;
		muggle_socklen_t len = sizeof(saddr);
		while ((n = muggle_socket_ctx_recvfrom(
			ctx, buf, sizeof(buf), 0, 
			(struct sockaddr*)&saddr, &len)) > 0)
		{
			muggle_socket_ctx_sendto(ctx, buf, n, 0, (struct sockaddr*)&saddr, len);
		}
	}
}
