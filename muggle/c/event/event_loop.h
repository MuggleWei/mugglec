/******************************************************************************
 *  @file         event_loop.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-08-09
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec event loop
 *****************************************************************************/

#ifndef MUGGLE_C_EVENT_LOOP_H_
#define MUGGLE_C_EVENT_LOOP_H_

#include "muggle/c/base/macro.h"
#include <time.h>
#include "muggle/c/base/thread.h"
#include "muggle/c/dsaa/linked_list.h"
#include "muggle/c/event/event.h"
#include "muggle/c/event/event_context.h"
#include "muggle/c/event/event_signal.h"

EXTERN_C_BEGIN

enum
{
	MUGGLE_EVLOOP_TYPE_NULL = 0,
	MUGGLE_EVLOOP_TYPE_SELECT,
	MUGGLE_EVLOOP_TYPE_POLL,
	MUGGLE_EVLOOP_TYPE_EPOLL,
	MUGGLE_EVLOOP_TYPE_KQUEUE,
	MUGGLE_MAX_EVLOOP_TYPE,
};

struct muggle_event_loop_init_args;
struct muggle_event_loop;

// event loop implement functions
typedef int (*fn_muggle_evloop_init)(
	struct muggle_event_loop *evloop,
	struct muggle_event_loop_init_args *args);
typedef void (*fn_muggle_evloop_destroy)(struct muggle_event_loop *evloop);
typedef void (*fn_muggle_evloop_run)(struct muggle_event_loop *evloop);
typedef int (*fn_muggle_evloop_add_ctx)(
	struct muggle_event_loop *evloop,
	muggle_event_context_t *ctx,
	void *node);

#define MUGGLE_EV_LOOP_IMPL_DECLARE(impl) \
int muggle_evloop_init_##impl(muggle_event_loop_t *evloop, muggle_event_loop_init_args_t *args); \
void muggle_evloop_destroy_##impl(muggle_event_loop_t *evloop); \
void muggle_evloop_run_##impl(muggle_event_loop_t *evloop); \
int muggle_evloop_add_ctx_##impl(muggle_event_loop_t *evloop, muggle_event_context_t *ctx, void *node);

/**
 * @brief event loop callback prototypes
 *
 * @param evloop  event loop
 * @param ctx     event context
 *
 * @return 
 */
typedef void (*fn_muggle_evloop_cb1)(struct muggle_event_loop *evloop, muggle_event_context_t *ctx);
typedef void (*fn_muggle_evloop_cb2)(struct muggle_event_loop *evloop);

/**
 * @brief event loop initialize arguments
 */
typedef struct muggle_event_loop_init_args
{
	int evloop_type;  //!< event loop type, see enum MUGGLE_EVLOOP_TYPE_*
	int hints_max_fd; //!< hints max event fd count
	int use_mem_pool; //!< is use memory pool
} muggle_event_loop_init_args_t;

/**
 * @brief event loop
 */
typedef struct muggle_event_loop
{
	int evloop_type; //!< event loop type

	muggle_linked_list_t  *ctx_list;  //!< linked list store event context
	muggle_event_signal_t *ev_signal; //!< event signal
	muggle_thread_id      tid;        //!< event loop run thread id
	int                   to_exit;    //!< to exit flags
	int                   timeout;    //!< timeout expires in milliseconds

	fn_muggle_evloop_cb1 cb_read;  //!< on event context read callback
	fn_muggle_evloop_cb1 cb_close; //!< on event context close callback

	fn_muggle_evloop_cb2 cb_wake;  //!< on event loop wakeup callback
	fn_muggle_evloop_cb2 cb_timer; //!< on event loop timer callback
	fn_muggle_evloop_cb1 cb_clear; //!< on event loop exit soon, foreach clear context callback
	fn_muggle_evloop_cb2 cb_exit;  //!< on event loop exit

	void *sys_data;   //!< middleware data
	void *user_data;  //!< user data
} muggle_event_loop_t;

/**
 * @brief new event loop
 *
 * @param args  event loop initialize arguments
 *
 * @return 
 *     - on success, return event loop pointer
 *     - on failed, return NULL
 *
 * @note
 *     - if evloop_type == 0, then auto select a recommend event loop type
 *     - if hints_max_fd < 1, the hints_max_fd auto be set a positive value
 *     - if use_mem_pool is true, then the hints_max_fd use a memory pool size
 *       and the max number of event fd was limit to hints_max_fd
 */
MUGGLE_C_EXPORT
muggle_event_loop_t* muggle_evloop_new(muggle_event_loop_init_args_t *args);

/**
 * @brief delete event loop
 *
 * @param evloop  event loop
 */
MUGGLE_C_EXPORT
void muggle_evloop_delete(muggle_event_loop_t *evloop);

/**
 * @brief set event loop timer interval
 *
 * @param evloop   event loop
 * @param timeout  timer interval in milliseconds
 */
MUGGLE_C_EXPORT
void muggle_evloop_set_timer_interval(muggle_event_loop_t *evloop, int timeout);

/**
 * @brief set event loop context read callback
 *
 * @param evloop  event loop
 * @param cb      read callback
 */
MUGGLE_C_EXPORT
void muggle_evloop_set_cb_read(muggle_event_loop_t *evloop, fn_muggle_evloop_cb1 cb);

/**
 * @brief set event loop context close callback
 *
 * @param evloop  event loop
 * @param cb      close callback
 */
MUGGLE_C_EXPORT
void muggle_evloop_set_cb_close(muggle_event_loop_t *evloop, fn_muggle_evloop_cb1 cb);

/**
 * @brief set event loop wake callback
 *
 * @param evloop  event loop
 * @param cb      wake callback
 */
MUGGLE_C_EXPORT
void muggle_evloop_set_cb_wake(muggle_event_loop_t *evloop, fn_muggle_evloop_cb2 cb);

/**
 * @brief set event loop timer callback
 *
 * @param evloop  event loop
 * @param cb      timer callback
 */
MUGGLE_C_EXPORT
void muggle_evloop_set_cb_timer(muggle_event_loop_t *evloop, fn_muggle_evloop_cb2 cb);

/**
 * @brief set event loop clear callback
 *
 * @param evloop  event loop
 * @param cb      clear callback
 */
MUGGLE_C_EXPORT
void muggle_evloop_set_cb_clear(muggle_event_loop_t *evloop, fn_muggle_evloop_cb1 cb);

/**
 * @brief set event loop exit callback
 *
 * @param evloop  event loop
 * @param cb      clear callback
 */
MUGGLE_C_EXPORT
void muggle_evloop_set_cb_exit(muggle_event_loop_t *evloop, fn_muggle_evloop_cb2 cb);

/**
 * @brief set user data
 *
 * @param evloop  event loop
 * @param data    user data
 */
MUGGLE_C_EXPORT
void muggle_evloop_set_data(muggle_event_loop_t *evloop, void *data);

/**
 * @brief get user data
 *
 * @param evloop  event loop
 *
 * @return user data
 */
MUGGLE_C_EXPORT
void* muggle_evloop_get_data(muggle_event_loop_t *evloop);

/**
 * @brief wakeup event loop
 *
 * @param evloop  event loop
 *
 * @return 
 *     0 - success
 *     otherwise - return MUGGLE_EVENT_ERROR and MUGGLE_EVENT_LAST_ERRNO is set
 */
MUGGLE_C_EXPORT
int muggle_evloop_wakeup(muggle_event_loop_t *evloop);

/**
 * @brief set event loop exit flag true
 *
 * @param evloop  event loop
 */
MUGGLE_C_EXPORT
void muggle_evloop_exit(muggle_event_loop_t *evloop);

/**
 * @brief add event context into event loop
 *
 * @param evloop  event loop
 * @param ctx     event context
 *
 * @return 
 *     0 - success add new event context
 *     otherwise - failed add
 *
 * @note
 * only support add context in the same thread of event loop run.
 * if in another thread, need put new context in somethere, wakeup eventloop and
 * add context in wake callback
 */
MUGGLE_C_EXPORT
int muggle_evloop_add_ctx(muggle_event_loop_t *evloop, muggle_event_context_t *ctx);

/**
 * @brief event loop run
 *
 * @param evloop  event loop
 */
MUGGLE_C_EXPORT
void muggle_evloop_run(muggle_event_loop_t *evloop);

EXTERN_C_END

#endif /* ifndef MUGGLE_C_EVENT_LOOP_H_ */
