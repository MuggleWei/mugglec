#ifndef MUGGLE_C_NET_SOCKET_EVENT_UTILS_H_
#define MUGGLE_C_NET_SOCKET_EVENT_UTILS_H_

#include "muggle/c/net/socket_event.h"
#include <time.h>

EXTERN_C_BEGIN

enum
{
	MUGGLE_SOCKET_EVENT_ACCEPT_RET_PEER = 0, // success accept socket
	MUGGLE_SOCKET_EVENT_ACCEPT_RET_INTR,     // failed accept socket cause MUGGLE_SYS_ERRNO_INTR
	MUGGLE_SOCKET_EVENT_ACCEPT_RET_WBLOCK,   // failed accept socket cause MUGGLE_SYS_ERRNO_WOULDBLOCK
	MUGGLE_SOCKET_EVENT_ACCEPT_RET_CLOSED,   // listen socket already closed
};

/*
 * on socket event message
 * @ev: socket event
 * @peer: socket peer
 * @handle_error:
 *     some event loop(like select), don't distinguish message and error happend, need handle error
 *     in the same function of on message;
 *     other event loop(like poll, epoll), don't need handle error in on message
 *     0 - do not handle on error
 *     1 - handle on error
 * */
void muggle_socket_event_on_message(muggle_socket_event_t *ev, muggle_socket_peer_t *peer, int handle_error);

void muggle_socket_event_timer_handle(muggle_socket_event_t *ev, struct timespec *t1, struct timespec *t2);

/*
 * event listen peer accept
 * RETURN: MUGGLE_SOCKET_EVENT_ACCEPT_RET_*
 * */
int muggle_socket_event_accept(muggle_socket_event_t *ev, muggle_socket_peer_t *listen_peer, muggle_socket_peer_t *peer);

EXTERN_C_END

#endif
