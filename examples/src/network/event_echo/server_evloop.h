#ifndef EXAMPLE_SERVER_EVLOOP_H_
#define EXAMPLE_SERVER_EVLOOP_H_

#include "struct.h"

void run_echo_server_evloop(const char *host, const char *port);

// event callback
void on_read(muggle_event_loop_t *evloop, muggle_event_context_t *ctx);
void on_close(muggle_event_loop_t *evloop, muggle_event_context_t *ctx);
void on_exit(muggle_event_loop_t *evloop);

#endif