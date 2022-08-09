/******************************************************************************
 *  @file         socket_context.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-08-08
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec socket context
 *****************************************************************************/

#ifndef MUGGLE_C_SOCKET_EV_CONTEXT_H_
#define MUGGLE_C_SOCKET_EV_CONTEXT_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/net/socket.h"
#include "muggle/c/event/event_context.h"

EXTERN_C_BEGIN

enum
{
	MUGGLE_SOCKET_CTX_TYPE_NULL = 0,
	MUGGLE_SOCKET_CTX_TYPE_TCP_LISTEN,
	MUGGLE_SOCKET_CTX_TYPE_TCP_CLIENT,
	MUGGLE_SOCKET_CTX_TYPE_UDP,
	MUGGLE_SOCKET_CTX_TYPE_MAX,
};

/**
 * @brief muggle socket context
 */
typedef struct muggle_socket_context
{
	muggle_event_context_t base;      //!< event context
	int                    sock_type; //!< socket context type, see MUGGLE_SOCKET_CTX_TYPE_*
} muggle_socket_context_t;

/**
 * @brief initialize socket context
 *
 * @param ctx        socket context
 * @param fd         socket fd
 * @param data       user data
 * @param sock_type  socket context type
 *
 * @return 
 */
MUGGLE_C_EXPORT
int muggle_socket_ctx_init(
	muggle_socket_context_t *ctx,
	muggle_socket_t fd,
	void *data, int sock_type);

#define muggle_socket_ctx_fd(ctx) \
	muggle_ev_ctx_fd((muggle_event_context_t*)ctx)

#define muggle_socket_ctx_data(ctx) \
	muggle_ev_ctx_data((muggle_event_context_t*)ctx)

#define muggle_socket_ctx_set_flag(ctx, flag) \
	muggle_ev_ctx_set_flag((muggle_event_context_t*)ctx, flag)

#define muggle_socket_ctx_ref_retain(ctx) \
	muggle_ev_ctx_ref_retain((muggle_event_context_t*)ctx)

#define muggle_socket_ctx_ref_release(ctx) \
	muggle_ev_ctx_ref_release((muggle_event_context_t*)ctx)

#define muggle_socket_ctx_shutdown(ctx) \
	muggle_ev_ctx_shutdown((muggle_event_context_t*)ctx)

#define muggle_socket_ctx_close(ctx) \
	muggle_ev_ctx_close((muggle_event_context_t*)ctx)

#define muggle_socket_ctx_read(ctx, buf, len) \
	muggle_ev_ctx_read((muggle_event_context_t*)ctx, buf, len)

#define muggle_socket_ctx_write(ctx, buf, len) \
	muggle_ev_ctx_write((muggle_event_context_t*)ctx, buf, len)

/**
 * @brief get socket context type
 *
 * @param ctx socket context
 *
 * @return socket context type
 */
MUGGLE_C_EXPORT
int muggle_socket_ctx_type(muggle_socket_context_t *ctx);

/**
 * @brief read bytes from socket event context
 *
 * @param ctx    socket context
 * @param buf    buffer store received bytes
 * @param len    buffer size
 * @param flags  recv flags
 *
 * @return 
 *     - on success, thre number of bytes read is returned, 0 indicates end of event context
 *     - on error, MUGGLE_EVENT_ERROR is returned and MUGGLE_EVENT_LAST_ERRNO is set
 */
MUGGLE_C_EXPORT
int muggle_socket_ctx_recv(muggle_socket_context_t *ctx, void *buf, size_t len, int flags);

/**
 * @brief read bytes from socket event context
 *
 * @param ctx    socket context
 * @param buf    pointer to the data need to send
 * @param len    buffer size
 * @param flags  send flags
 *
 * @return 
 *     - on success, thre number of bytes sent is returned, 0 indicates end of event context
 *     - on error, MUGGLE_EVENT_ERROR is returned and MUGGLE_EVENT_LAST_ERRNO is set
 */
MUGGLE_C_EXPORT
int muggle_socket_ctx_send(muggle_socket_context_t *ctx, void *buf, size_t len, int flags);

/**
 * @brief receive messages from a socket
 *
 * @param ctx      socket context
 * @param buf      buffer store received bytes
 * @param len      buffer size
 * @param flags    send flags
 * @param addr     socket address
 * @param addrlen  socket address length
 *
 * @return 
 *     - return positive value, the number of bytes received
 *     - return 0, the connction has been closed
 *     - return MUGGLE_SOCKET_ERROR, an error occurred, MUGGLE_SOCKET_LAST_ERRNO is set.
 */
MUGGLE_C_EXPORT
int muggle_socket_ctx_recvfrom(
	muggle_socket_context_t *ctx, void *buf, size_t len, int flags,
	struct sockaddr *addr, muggle_socklen_t *addrlen);

/**
 * @brief socket send message
 *
 * @param ctx    socket context
 * @param buf    pointer to the data need to send
 * @param len    buffer size
 * @param flags  send flags
 * @param dest_addr  dest socket address
 * @param addrlen    dest socket address length
 *
 * @return 
 *     - on success, return the number of bytes sent
 *     - on error, MUGGLE_SOCKET_ERROR is returned and MUGGLE_SOCKET_LAST_ERRNO is set
 */
MUGGLE_C_EXPORT
int muggle_socket_ctx_sendto(
	muggle_socket_context_t *ctx, void *buf, size_t len, int flags,
	const struct sockaddr *dest_addr, socklen_t addrlen);

EXTERN_C_END

#endif /* ifndef MUGGLE_C_SOCKET_CONTEXT_H_ */
