/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_NET_SOCKET_EVENT_UTILS_H_
#define MUGGLE_C_NET_SOCKET_EVENT_UTILS_H_

#include <time.h>
#include "muggle/c/net/socket_event.h"
#include "muggle/c/memory/memory_pool.h"

EXTERN_C_BEGIN

/*
 * on socket event message
 * @ev: socket event
 * @peer: socket peer
 * */
void muggle_socket_event_on_message(muggle_socket_event_t *ev, muggle_socket_peer_t *peer);

/*
 * socket event timer handle
 * */
void muggle_socket_event_timer_handle(muggle_socket_event_t *ev, struct timespec *t1, struct timespec *t2);

/*
 * event listen peer accept
 * RETURN: MUGGLE_SOCKET_EVENT_ACCEPT_RET_*
 * */
void muggle_socket_event_accept(muggle_socket_peer_t *listen_peer, muggle_socket_peer_t *peer);

/* 
 * refuse new connection
 * */
void muggle_socket_event_refuse_accept(muggle_socket_peer_t *listen_peer);

EXTERN_C_END

#endif
