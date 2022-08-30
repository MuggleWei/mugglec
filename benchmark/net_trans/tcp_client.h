#ifndef TCP_CLIENT_H_
#define TCP_CLIENT_H_

#include "trans_message.h"

void run_tcp_client(
	const char *host, const char *port,
	int busy_mode,
	muggle_benchmark_handle_t *handle,
	muggle_benchmark_config_t *config);

#endif
