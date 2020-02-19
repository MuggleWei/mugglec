/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_SOCKET_EVENT_H_
#define MUGGLE_C_SOCKET_EVENT_H_

#include "muggle/c/net/socket.h"
#include "muggle/c/net/socket_utils.h"

EXTERN_C_BEGIN

enum
{
	MUGGLE_SOCKET_EVENT_LOOP_TYPE_NULL = 0,
	MUGGLE_SOCKET_EVENT_LOOP_TYPE_MULTHREAD,
	MUGGLE_SOCKET_EVENT_LOOP_TYPE_SELECT,
	MUGGLE_SOCKET_EVENT_LOOP_TYPE_POLL,
	MUGGLE_SOCKET_EVENT_LOOP_TYPE_EPOLL,
	MUGGLE_SOCKET_EVENT_LOOP_TYPE_KQUEUE,
	MUGGLE_SOCKET_EVENT_LOOP_TYPE_IOCP,
	MUGGLE_SOCKET_EVENT_LOOP_TYPE_MAX,
};

struct muggle_socket_peer;
struct muggle_socket_event;

/*
 * return 0, everything is ok
 * return -1, muggle_socket_event will close socket and free peer
 * return 1, user will close socket and muggle_socket_event will free peer
 * */
typedef int (*muggle_socket_event_connect)(
	struct muggle_socket_event *ev, struct muggle_socket_peer *listen_peer, struct muggle_socket_peer *peer);

/*
 * when return 0, then muggle_socket_event will close socket, otherwise user is responsible
 * for close socket. No matter muggle_socket_event or user close socket, the peer
 * will free by muggle_socket_event, if user wanna transfer peer to other thread for close, need
 * malloc and copy peer for that
 * */
typedef int (*muggle_socket_event_error)(struct muggle_socket_event *ev, struct muggle_socket_peer *peer);

/*
 * return 0, everything is ok
 * return -1, muggle_socket_event will close socket and free peer
 * return 1, user will close socket and muggle_socket_event will free peer
 * */
typedef int (*muggle_socket_event_message)(struct muggle_socket_event *ev, struct muggle_socket_peer *peer);

typedef void (*muggle_socket_event_timer)(struct muggle_socket_event *ev);

// socket event loop handle
typedef struct muggle_socket_event
{
	int  ev_loop_type;
	int  timeout_ms;
	void *datas;

	muggle_socket_event_connect on_connect;
	muggle_socket_event_error   on_error;
	muggle_socket_event_message on_message;
	muggle_socket_event_timer   on_timer;
}muggle_socket_event_t;

// socket event loop input arguments
typedef struct muggle_socket_ev_arg
{
	int                  ev_loop_type;   // event loop type, MUGGLE_SOCKET_EVENT_LOOP_TYPE_*
	int                  hints_max_peer; // hints max peer in the event loop
	int                  cnt_peer;       // the number of socket descriptor in this arguments
	muggle_socket_peer_t *peers;         // socket peer array, number is cnt_peer
	int                  timeout_ms;     // event loop timer in millisec, -1, 0 or any positive number, the same as epoll timeout
	void                 *datas;         // user custom data

	// event callbacks
	muggle_socket_event_connect on_connect;
	muggle_socket_event_error   on_error;
	muggle_socket_event_message on_message;
	muggle_socket_event_timer   on_timer;
}muggle_socket_ev_arg_t;

/*
 * monitor multiple socket descriptiors, waiting util one or
 * more of the descriptors become "ready"
 * */
MUGGLE_CC_EXPORT
void muggle_socket_event_loop(muggle_socket_ev_arg_t *ev_arg);

EXTERN_C_END

#endif
