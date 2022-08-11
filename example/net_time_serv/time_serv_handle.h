#ifndef EXAMPLE_TIME_SERV_HANDLE_H_
#define EXAMPLE_TIME_SERV_HANDLE_H_

#include "muggle/c/muggle_c.h"

typedef struct evloop_data
{
	const char          *host;
	const char          *serv;
	const char          *multicast_host;
	const char          *multicast_serv;

	muggle_socket_context_t *listen_ctx;
	muggle_socket_context_t *mcast_ctx;

	muggle_linked_list_t conn_list;

	muggle_event_loop_t *evloop;
} evloop_data_t;

typedef struct user_data
{
	muggle_linked_list_node_t *node;
} user_data_t;

void run_tcp_listen(evloop_data_t *args);
void run_mcast_conn(evloop_data_t *args);

void on_add_ctx(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);
void on_connect(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);
void on_timer(muggle_event_loop_t *evloop);
void on_close(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);
void on_release(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);

#endif
