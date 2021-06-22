/******************************************************************************
 *  @file         socket_event.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-17
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec socket event
 *****************************************************************************/
 
#ifndef MUGGLE_C_SOCKET_EVENT_H_
#define MUGGLE_C_SOCKET_EVENT_H_

#include "muggle/c/net/socket.h"
#include "muggle/c/net/socket_peer.h"
#include "muggle/c/net/socket_utils.h"

EXTERN_C_BEGIN

enum
{
	MUGGLE_SOCKET_EVENT_LOOP_TYPE_NULL = 0,
	MUGGLE_SOCKET_EVENT_LOOP_TYPE_MULTHREAD,  //!< multiple thread
	MUGGLE_SOCKET_EVENT_LOOP_TYPE_SELECT,     //!< select
	MUGGLE_SOCKET_EVENT_LOOP_TYPE_POLL,       //!< poll
	MUGGLE_SOCKET_EVENT_LOOP_TYPE_EPOLL,      //!< epoll
	MUGGLE_SOCKET_EVENT_LOOP_TYPE_KQUEUE,     //!< kqueue
	MUGGLE_SOCKET_EVENT_LOOP_TYPE_IOCP,       //!< iocp
	MUGGLE_SOCKET_EVENT_LOOP_TYPE_MAX,
};

struct muggle_socket_event;

/**
 * @brief prototype of socket event callback - on connect
 *
 * @param ev           socket event pointer
 * @param listen_peer  listen socket
 * @param peer         socket that new connected
 */
typedef void (*muggle_socket_event_connect)(struct muggle_socket_event *ev, struct muggle_socket_peer *listen_peer, struct muggle_socket_peer *peer);

/**
 * @brief prototype of socket event callback - on error
 *
 * @param ev           socket event pointer
 * @param peer         socket that error happend
 */
typedef void (*muggle_socket_event_error)(struct muggle_socket_event *ev, struct muggle_socket_peer *peer);

/**
 * @brief prototype of socket event callback - on message
 *
 * @param ev           socket event pointer
 * @param peer         socket that receiced message
 */
typedef void (*muggle_socket_event_message)(struct muggle_socket_event *ev, struct muggle_socket_peer *peer);

/**
 * @brief prototype of socket event callback - on timer
 *
 * @param ev           socket event pointer
 */
typedef void (*muggle_socket_event_timer)(struct muggle_socket_event *ev);

/**
 * @brief prototype of socket event callback - on close
 *
 * @param ev           socket event pointer
 * @param peer         socket that closed
 */
typedef void (*muggle_socket_event_close)(struct muggle_socket_event *ev, struct muggle_socket_peer *peer);

/**
 * @brief socket event loop handle
 */
typedef struct muggle_socket_event
{
	int  ev_loop_type;
	int  capacity;
	int  timeout_ms;
	int  to_exit;
	void *mem_mgr;
	void *datas;

	muggle_socket_event_connect on_connect;
	muggle_socket_event_error   on_error;
	muggle_socket_event_close   on_close;
	muggle_socket_event_message on_message;
	muggle_socket_event_timer   on_timer;
}muggle_socket_event_t;

/**
 * @brief socket event loop input arguments
 */
typedef struct muggle_socket_event_init_arg
{
	int                  ev_loop_type;   //!< event loop type, MUGGLE_SOCKET_EVENT_LOOP_TYPE_*
	int                  hints_max_peer; //!< hints max peer in the event loop
	int                  cnt_peer;       //!< the number of socket descriptor in this arguments
	muggle_socket_peer_t *peers;         //!< socket peer array, size is cnt_peer
	muggle_socket_peer_t **p_peers;      //!< return peers holds by ev, if wanna use it in other thread, remember call retain function
	int                  timeout_ms;     //!< event loop timer in millisec, -1, 0 or any positive number, the same as epoll timeout
	void                 *datas;         //!< user custom data

	// event callbacks
	muggle_socket_event_connect on_connect; //!< callback for socket connect
	muggle_socket_event_error   on_error;   //!< callback for socket error/disconnect
	muggle_socket_event_close   on_close;   //!< callback for socket close, free peer soon, safe to free peer->data
	muggle_socket_event_message on_message; //!< callback for socket on message
	muggle_socket_event_timer   on_timer;   //!< callback for socket on timer
}muggle_socket_event_init_arg_t;

/**
 * @brief init socket event
 *
 * @param ev_init_arg  event initlaize arguments
 * @param ev           socket event
 *
 * @return 0 - success, otherwise failed init event
 */
MUGGLE_C_EXPORT
int muggle_socket_event_init(muggle_socket_event_init_arg_t *ev_init_arg, muggle_socket_event_t *ev);

/**
 * @brief 
 * monitor multiple socket descriptiors, waiting util one or more of 
 * the descriptors become "ready"
 *
 * @param ev   socket event
 *
 * @return 0 - exit normally, otherwise failed init
 */
MUGGLE_C_EXPORT
int muggle_socket_event_loop(muggle_socket_event_t *ev);

/**
 * @brief exit event loop 
 *
 * @param ev   socket event
 */
MUGGLE_C_EXPORT
void muggle_socket_event_loop_exit(muggle_socket_event_t *ev);

EXTERN_C_END

#endif
