#ifndef EXAMPLE_TCP_SERVER_H_
#define EXAMPLE_TCP_SERVER_H_

#include "args.h"

void tcp_server_run(sys_args_t *args, void *user_data);

// event callback
void tcp_server_on_read(muggle_event_loop_t *evloop, muggle_event_context_t *ctx);
void tcp_server_on_close(muggle_event_loop_t *evloop, muggle_event_context_t *ctx);
void tcp_server_on_wake(muggle_event_loop_t *evloop);
void tcp_server_on_timer(muggle_event_loop_t *evloop);
void tcp_server_on_clear(muggle_event_loop_t *evloop, muggle_event_context_t *ctx);

// tcp callback
muggle_socket_t tcp_server_on_accept(
	muggle_event_loop_t *evloop, muggle_socket_context_t *listen_ctx);
void tcp_server_on_connect(muggle_event_loop_t *evloop, muggle_socket_t fd);
void tcp_server_on_message(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);

#endif /* ifndef EXAMPLE_TCP_SERVER_H_ */
