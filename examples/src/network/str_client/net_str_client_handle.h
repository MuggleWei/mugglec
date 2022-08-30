#ifndef EXAMPLE_NET_STR_CLIENT_HANDLE_H_
#define EXAMPLE_NET_STR_CLIENT_HANDLE_H_

#include "net_str_client_msg.h"

struct client_thread_arg
{
	const char *host;
	const char *serv;
	const char *socket_type;
	muggle_ring_buffer_t *ring;
};

muggle_thread_ret_t thread_socket_event(void *arg);

#endif