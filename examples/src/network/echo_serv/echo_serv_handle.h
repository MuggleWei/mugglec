#ifndef EXAMPLE_ECHO_SERV_HANDLE_H_
#define EXAMPLE_ECHO_SERV_HANDLE_H_

#include "muggle/c/muggle_c.h"

void on_message(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);

#endif
