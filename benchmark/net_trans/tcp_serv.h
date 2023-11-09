#ifndef TCP_SERV_H_
#define TCP_SERV_H_

#include "trans_message.h"

void run_tcp_serv(const char *host, const char *port, int is_multiplexing,
				  int is_busy, muggle_benchmark_handle_t *handle,
				  muggle_benchmark_config_t *config);

#endif
