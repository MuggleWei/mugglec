#include "tcp_block_serv_handle.h"

static uint64_t s_total_read_bytes = 0;

void serv_on_message(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	MUGGLE_UNUSED(evloop);

	int n = 0;
	char buf[8];
	while ((n = muggle_socket_ctx_read(ctx, buf, sizeof(buf))) > 0) {
		s_total_read_bytes += n;
		LOG_INFO("read data: %*s", n, buf);
		LOG_INFO("read bytes: %d, total read bytes: %llu", n,
				 (unsigned long long)s_total_read_bytes);
		LOG_INFO("sleep 1s");
		muggle_msleep(1000);
	}
}
