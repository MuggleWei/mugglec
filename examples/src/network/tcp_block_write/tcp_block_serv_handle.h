#ifndef EXAMPLE_TCP_BLOCK_SERV_HANDLE_H_
#define EXAMPLE_TCP_BLOCK_SERV_HANDLE_H_

#include "muggle/c/muggle_c.h"

void serv_on_message(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);

#endif // !EXAMPLE_TCP_BLOCK_SERV_HANDLE_H_
