#ifndef EXAMPLE_TIME_CLIENT_HANDLE_H_
#define EXAMPLE_TIME_CLIENT_HANDLE_H_

#include "muggle/c/muggle_c.h"

typedef struct conn_thread_args
{
	muggle_event_loop_t *evloop;
	const char *host;
	const char *serv;
	const char *sock_type;
} conn_thread_args_t;

void run_conn_thread(conn_thread_args_t *args);

void on_add_ctx(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);

void on_message(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);

void on_close(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);

void on_release(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);

#endif
