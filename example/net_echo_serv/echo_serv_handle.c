#include "echo_serv_handle.h"

void on_message(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	int n = 0;
	char buf[4096];
	while ((n = muggle_socket_ctx_read(ctx, buf, sizeof(buf))) > 0)
	{
		muggle_socket_ctx_write(ctx, buf, n);
	}
}
