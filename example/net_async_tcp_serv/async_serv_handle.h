#ifndef EXAMPLE_ASYNC_TCP_SERV_HANDLE_H_
#define EXAMPLE_ASYNC_TCP_SERV_HANDLE_H_

#include "muggle/c/muggle_c.h"

void on_read(muggle_event_loop_t *evloop, muggle_event_context_t *ctx);

void on_close(muggle_event_loop_t *evloop, muggle_event_context_t *ctx);

#endif
