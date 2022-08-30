#ifndef EXAMPLE_TCP_SERVER_H_
#define EXAMPLE_TCP_SERVER_H_

#include "args.h"

// TCP listen socket callbacks
void tcp_server_on_accept(muggle_event_loop_t *evloop, muggle_event_context_t *ctx);

muggle_socket_t tcp_server_do_accept(
	muggle_event_loop_t *evloop, muggle_event_context_t *listen_ctx);
void tcp_server_on_connect(muggle_event_loop_t *evloop, muggle_socket_t fd);

// TCP client socket callback
void tcp_on_message(muggle_event_loop_t *evloop, muggle_event_context_t *ctx);
void tcp_on_close(muggle_event_loop_t *evloop, muggle_event_context_t *ctx);

#endif /* ifndef EXAMPLE_TCP_SERVER_H_ */
