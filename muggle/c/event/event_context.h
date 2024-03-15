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
#include "muggle/c/sync/ref_cnt.h"

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
	muggle_ref_cnt_t  ref_cnt; //!< reference count of this context
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
MUGGLE_C_EXPORT
int muggle_ev_ctx_init(muggle_event_context_t *ctx, muggle_event_fd fd, void *data);

/**
 * @brief get event fd in context
 *
 * @param ctx  event context
 *
 * @return event fd
 */
MUGGLE_C_EXPORT
muggle_event_fd muggle_ev_ctx_fd(muggle_event_context_t *ctx);

/**
 * @brief get user data in context
 *
 * @param ctx  event context
 *
 * @return user data
 */
MUGGLE_C_EXPORT
void* muggle_ev_ctx_data(muggle_event_context_t *ctx);

/**
 * @brief event context flags bitwise or flag
 *
 * @param ctx   event context
 * @param flag  flag, see MUGGLE_EV_CTX_FLAG_*
 */
MUGGLE_C_EXPORT
void muggle_ev_ctx_set_flag(muggle_event_context_t *ctx, int flag);

/**
 * @brief get the reference count of context
 *
 * @param ctx  event context
 * @param memorder  memory order; see: muggle_memory_order_*
 *
 * @return  reference count of context
 */
MUGGLE_C_EXPORT
int muggle_ev_ctx_ref_num(muggle_event_context_t *ctx, int memorder);

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
MUGGLE_C_EXPORT
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
MUGGLE_C_EXPORT
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
MUGGLE_C_EXPORT
int muggle_ev_ctx_shutdown(muggle_event_context_t *ctx);

/**
 * @brief close fd in event context and set context closed
 *
 * @param ctx  event context
 *
 * @return 
 *     - returns 0 on success
 *     - on error, MUGGLE_EVENT_ERROR is returned and MUGGLE_EVENT_LAST_ERRNO is set
 *
 * @note
 * don't close ctx twice
 */
MUGGLE_C_EXPORT
int muggle_ev_ctx_close(muggle_event_context_t *ctx);

/**
 * @brief read bytes from event context
 *
 * @param ctx  event context
 * @param buf  buffer store received bytes
 * @param len  buffer size
 *
 * @return 
 *     - on success, thre number of bytes read is returned, 0 indicates end of event context
 *     - on error, MUGGLE_EVENT_ERROR is returned and MUGGLE_EVENT_LAST_ERRNO is set
 */
MUGGLE_C_EXPORT
int muggle_ev_ctx_read(muggle_event_context_t *ctx, void *buf, size_t len);

/**
 * @brief write len bytes from buffer to event context
 *
 * @param ctx  event context
 * @param buf  buffer store the bytes that need to send
 * @param len  number of bytes be written
 *
 * @return 
 *     - on success, thre number of bytes sent is returned, 0 indicates end of event context
 *     - on error, MUGGLE_EVENT_ERROR is returned and MUGGLE_EVENT_LAST_ERRNO is set
 */
MUGGLE_C_EXPORT
int muggle_ev_ctx_write(muggle_event_context_t *ctx, void *buf, size_t len);

EXTERN_C_END

#endif /* ifndef MUGGLE_C_EVENT_CONTEXT_H_ */
