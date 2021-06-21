/******************************************************************************
 *  @file         socket_event_utils.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-21
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec socket event utils
 *****************************************************************************/

#ifndef MUGGLE_C_NET_SOCKET_EVENT_UTILS_H_
#define MUGGLE_C_NET_SOCKET_EVENT_UTILS_H_

#include <time.h>
#include "muggle/c/net/socket_event.h"
#include "muggle/c/memory/memory_pool.h"

EXTERN_C_BEGIN

/**
 * @brief on socket event message
 *
 * @param ev    socket event
 * @param peer  socket peer
 */
void muggle_socket_event_on_message(muggle_socket_event_t *ev, muggle_socket_peer_t *peer);

/**
 * @brief socket event timer handle
 *
 * @param ev  socket event
 * @param t1  last timestamp
 * @param t2  current timestamp
 */
void muggle_socket_event_timer_handle(muggle_socket_event_t *ev, struct timespec *t1, struct timespec *t2);

/**
 * @brief event listen peer accept
 *
 * @param listen_peer  listen socket
 * @param peer         accepted socket
 */
void muggle_socket_event_accept(muggle_socket_peer_t *listen_peer, muggle_socket_peer_t *peer);

/**
 * @brief refuse new connection
 *
 * @param listen_peer  listen socket
 */
void muggle_socket_event_refuse_accept(muggle_socket_peer_t *listen_peer);

EXTERN_C_END

#endif
