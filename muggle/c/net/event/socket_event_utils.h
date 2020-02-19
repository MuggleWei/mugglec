#ifndef MUGGLE_C_NET_SOCKET_EVENT_UTILS_H_
#define MUGGLE_C_NET_SOCKET_EVENT_UTILS_H_

#include "muggle/c/net/socket_event.h"
#include <time.h>

EXTERN_C_BEGIN

int muggle_socket_event_on_message(muggle_socket_event_t *ev, muggle_socket_peer_t *peer);

void muggle_socket_event_timer_handle(muggle_socket_event_t *ev, struct timespec *t1, struct timespec *t2);

EXTERN_C_END

#endif
