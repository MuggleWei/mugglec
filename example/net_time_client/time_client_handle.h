#ifndef EXAMPLE_TIME_CLIENT_HANDLE_H_
#define EXAMPLE_TIME_CLIENT_HANDLE_H_

#include "muggle/c/muggle_c.h"

int on_error(struct muggle_socket_event *ev, struct muggle_socket_peer *peer);

int on_message(struct muggle_socket_event *ev, struct muggle_socket_peer *peer);

#endif