#ifndef EXAMPLE_ASYNC_TCP_SERV_HANDLE_H_
#define EXAMPLE_ASYNC_TCP_SERV_HANDLE_H_

#include "muggle/c/muggle_c.h"

void on_add_ctx(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);
void on_connect(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);
void on_message(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);
void on_close(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);
void on_release(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);

#endif
