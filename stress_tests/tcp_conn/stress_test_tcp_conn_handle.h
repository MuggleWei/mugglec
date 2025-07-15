#ifndef EXAMPLE_TCP_STRESS_TEST_HANDLE_H_
#define EXAMPLE_TCP_STRESS_TEST_HANDLE_H_

#include "muggle/c/muggle_c.h"

typedef struct {
	muggle_event_loop_t *evloop;
	const char *host;
	const char *serv;
	int total_conns;
} stress_test_tcp_conn_args_t;

void run_conn_thread(stress_test_tcp_conn_args_t *args);

void on_add_ctx(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);

void on_message(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);

void on_close(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);

void on_release(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);

#endif // !EXAMPLE_TCP_STRESS_TEST_HANDLE_H_
