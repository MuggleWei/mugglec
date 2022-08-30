#ifndef UDP_SENDER_H_
#define UDP_SENDER_H_

#include "trans_message.h"

void run_udp_sender(
	const char *host, const char *port,
	int busy_mode,
	muggle_benchmark_handle_t *handle,
	muggle_benchmark_config_t *config);

#endif

