/******************************************************************************
 *  @file         socket_evloop_handle.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-08-10
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec socket event loop handle
 *****************************************************************************/

#ifndef MUGGLE_C_SOCKET_EVLOOP_HANDLE_H_
#define MUGGLE_C_SOCKET_EVLOOP_HANDLE_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/dsaa/queue.h"
#include "muggle/c/event/event_loop.h"
#include "muggle/c/net/socket_context.h"
#include "muggle/c/sync/mutex.h"

EXTERN_C_BEGIN

// callback prototypes
typedef void (*fn_muggle_socket_evloop_cb1)(
	muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);
typedef void (*fn_muggle_socket_evloop_cb2)(muggle_event_loop_t *evloop);
typedef muggle_socket_context_t* (*fn_muggle_socket_evloop_alloc)(void *pool);
typedef void (*fn_muggle_socket_evloop_free)(void *pool, muggle_socket_context_t *data);

/**
 * @brief socket event loop handle
 *
 * @note
 *     - The difference between cb_close and cb_release:
 *       When cb_close be called, it's mean ctx will remove from event loop soon.
 *       When cb_release be called, it's mean safe to release user data in context.
 *       If context ref_retain and move to other thread, cb_release maybe not be
 *       called by event loop. user need to invoke ref_release in other thread, if
 *       return value is 0, user need manual free user data, close and free context
 */
typedef struct muggle_socket_evloop_handle
{
	muggle_event_loop_t *evloop;

	muggle_queue_t *ctx_queue; //!< new context queue
	muggle_mutex_t *mtx;       //!< mutex for new_ctx_queue

	int timeout;  //!< timer interval

	fn_muggle_socket_evloop_cb1 cb_conn;    //!< TCP connection callback
	fn_muggle_socket_evloop_cb1 cb_msg;     //!< on socket message callback
	fn_muggle_socket_evloop_cb1 cb_close;   //!< on close socket callback
	fn_muggle_socket_evloop_cb1 cb_release; //!< on safe to release context resource

	void *mempool;  //!< memory pool
	fn_muggle_socket_evloop_alloc cb_alloc; //!< alloc callback
	fn_muggle_socket_evloop_free  cb_free;  //!< free callback

	fn_muggle_socket_evloop_cb1 cb_add_ctx; //!< on add context from outside

	fn_muggle_socket_evloop_cb2 cb_wake;  //!< on event loop wakeup callback
	fn_muggle_socket_evloop_cb2 cb_timer; //!< on event loop timer callback
} muggle_socket_evloop_handle_t;

/**
 * @brief socket event loop handle init
 *
 * @param handle  socket event loop handle
 *
 * @return 
 *     0 - success
 *     otherwise - failed
 */
MUGGLE_C_EXPORT
int muggle_socket_evloop_handle_init(muggle_socket_evloop_handle_t *handle);

/**
 * @brief destroy socket event loop handle
 *
 * @param handle  socket event loop handle
 */
MUGGLE_C_EXPORT
void muggle_socket_evloop_handle_destroy(muggle_socket_evloop_handle_t *handle);

/**
 * @brief attach handle to event loop
 *
 * @param handle  socket event loop handle
 * @param evloop  event loop handle
 */
MUGGLE_C_EXPORT
void muggle_socket_evloop_handle_attach(
	muggle_socket_evloop_handle_t *handle,
	muggle_event_loop_t *evloop);

/**
 * @brief add socket context into event loop
 *
 * @param handle  socket event loop handle
 * @param ctx     socket context
 *
 * @note
 * By default, the context is in heap, which is returned by malloc. Because
 * when release context, socket_evloop_handle will use 'free()' to destroy 
 * context. If user wanna add context that in stack or allocate by memory pool,
 * please set callback by muggle_socket_evloop_handle_set_alloc_free
 */
MUGGLE_C_EXPORT
void muggle_socket_evloop_add_ctx(
	muggle_event_loop_t *evloop,
	muggle_socket_context_t *ctx);

/**
 * @brief set event loop timer interval
 *
 * @param evloop   socket event loop handle
 * @param timeout  timer interval in milliseconds
 */
MUGGLE_C_EXPORT
void muggle_socket_evloop_handle_set_timer_interval(
	muggle_socket_evloop_handle_t *handle,
	int timeout);

/**
 * @brief set TCP connection callback
 *
 * @param handle  socket event loop handle
 * @param cb      callback function
 */
MUGGLE_C_EXPORT
void muggle_socket_evloop_handle_set_cb_conn(
	muggle_socket_evloop_handle_t *handle,
	fn_muggle_socket_evloop_cb1 cb);

/**
 * @brief set socket on message callback
 *
 * @param handle  socket event loop handle
 * @param cb      callback function
 */
MUGGLE_C_EXPORT
void muggle_socket_evloop_handle_set_cb_msg(
	muggle_socket_evloop_handle_t *handle,
	fn_muggle_socket_evloop_cb1 cb);

/**
 * @brief set on close socket callback
 *
 * @param handle  socket event loop handle
 * @param cb      callback function
 */
MUGGLE_C_EXPORT
void muggle_socket_evloop_handle_set_cb_close(
	muggle_socket_evloop_handle_t *handle,
	fn_muggle_socket_evloop_cb1 cb);

/**
 * @brief set on release callback
 *
 * @param handle  socket event loop handle
 * @param cb      callback function
 */
MUGGLE_C_EXPORT
void muggle_socket_evloop_handle_set_cb_release(
	muggle_socket_evloop_handle_t *handle,
	fn_muggle_socket_evloop_cb1 cb);

/**
 * @brief set context alloc and free function
 *
 * @param handle    socket event loop handle
 * @param pool      memory pool
 * @param cb_alloc  allocate callback function
 * @param cb_free   free callback function
 */
MUGGLE_C_EXPORT
void muggle_socket_evloop_handle_set_alloc_free(
	muggle_socket_evloop_handle_t *handle,
	void *mempool,
	fn_muggle_socket_evloop_alloc cb_alloc,
	fn_muggle_socket_evloop_free cb_free);

/**
 * @brief set on add context from outside callback
 *
 * @param handle  socket event loop handle
 * @param cb      callback function
 */
MUGGLE_C_EXPORT
void muggle_socket_evloop_handle_set_cb_add_ctx(
	muggle_socket_evloop_handle_t *handle,
	fn_muggle_socket_evloop_cb1 cb);

/**
 * @brief set event loop wakeup callback
 *
 * @param handle  socket event loop handle
 * @param cb      callback function
 */
MUGGLE_C_EXPORT
void muggle_socket_evloop_handle_set_cb_wake(
	muggle_socket_evloop_handle_t *handle,
	fn_muggle_socket_evloop_cb2 cb);

/**
 * @brief set event loop timer callback
 *
 * @param handle  socket event loop handle
 * @param cb      callback function
 *
 * @NOTE
 * when user need busy loop and timer in the same time, need to set timeout 0 
 * and cusomize timer callback that calculate interval by self
 */
MUGGLE_C_EXPORT
void muggle_socket_evloop_handle_set_cb_timer(
	muggle_socket_evloop_handle_t *handle,
	fn_muggle_socket_evloop_cb2 cb);

EXTERN_C_END

#endif /* ifndef MUGGLE_C_SOCKET_EVLOOP_HANDLE_H_ */
