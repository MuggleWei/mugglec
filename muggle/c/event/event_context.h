/******************************************************************************
 *  @file         event_context.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-08-06
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec event context
 *****************************************************************************/

#ifndef MUGGLE_C_EVENT_CONTEXT_H_
#define MUGGLE_C_EVENT_CONTEXT_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/base/atomic.h"
#include "muggle/c/event/event.h"
#include "muggle/c/event/event_fd.h"

EXTERN_C_BEGIN

enum
{
	MUGGLE_EV_CTX_FLAG_CLOSED = 0x01,  //!< event context closed
};

/**
 * @brief muggle event context
 */
typedef struct muggle_event_context
{
	muggle_event_fd   fd;      //!< event file descriptor
	int               flags;   //!< event flags, see MUGGLE_EV_CTX_FLAG_*
	muggle_atomic_int ref_cnt; //!< reference count of this context
	void              *data;   //!< user data
} muggle_event_context_t;

/**
 * @brief initialize event context
 *
 * @param ctx   event context
 * @param fd    event fd
 * @param data  user context data
 *
 * @return 
 *     0 - success
 *     otherwise - failed
 */
int muggle_ev_ctx_init(muggle_event_context_t *ctx, muggle_event_fd fd, void *data);

/**
 * @brief
 * increases the reference count of context by 1
 *
 * @param ctx  event context
 *
 * @return
 *     on success, return reference count of context after this call
 *     on failed, return -1.
 *     when reference count already 0, then try to retain will return -1
 *
 * @note
 * When using event context in multithreading, user can easily use muggle_ev_ctx_ref_retain
 * and muggle_ev_ctx_ref_release control reference count of event context, when the reference
 * count reaches 0, use can close fd and release context resources
 */
int muggle_ev_ctx_ref_retain(muggle_event_context_t *ctx);

/**
 * @brief 
 * decreases the reference count by 1
 *
 * @param ctx  event context
 *
 * @return
 *     on success, return reference count of context after this call
 *     on failed, return -1.
 *     when reference count already 0, then try to release will return -1
 *
 * @note
 * When using event context in multithreading, user can easily use muggle_ev_ctx_ref_retain
 * and muggle_ev_ctx_ref_release control reference count of event context, when the reference
 * count reaches 0, use can close fd and release context resources
 */
int muggle_ev_ctx_ref_release(muggle_event_context_t *ctx);

/**
 * @brief shutdown fd in event context and set context closed
 *
 * @param ctx  event context
 *
 * @return 
 *     - returns 0 on success
 *     - on error, MUGGLE_EVENT_ERROR is returned and MUGGLE_EVENT_LAST_ERRNO is set
 */
int muggle_ev_ctx_shutdown(muggle_event_context_t *ctx);

/**
 * @brief close fd in event context and set context closed
 *
 * @param ctx  event context
 *
 * @return 
 *     - returns 0 on success
 *     - on error, MUGGLE_EVENT_ERROR is returned and MUGGLE_EVENT_LAST_ERRNO is set
 */
int muggle_ev_ctx_close(muggle_event_context_t *ctx);

EXTERN_C_END

#endif /* ifndef MUGGLE_C_EVENT_CONTEXT_H_ */
