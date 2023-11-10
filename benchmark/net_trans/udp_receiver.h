#ifndef UDP_RECEIVER_H_
#define UDP_RECEIVER_H_

#include "trans_message.h"

void run_udp_receiver(const char *host, const char *port, int is_multiplexing,
					  int is_busy, muggle_benchmark_handle_t *handle,
					  muggle_benchmark_config_t *config);

#endif
