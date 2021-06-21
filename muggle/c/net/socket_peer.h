/******************************************************************************
 *  @file         socket_peer.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-17
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec socket peer
 *****************************************************************************/
 
#ifndef MUGGLE_C_SOCKET_PEER_H_
#define MUGGLE_C_SOCKET_PEER_H_

#include "muggle/c/net/socket.h"
#include "muggle/c/base/atomic.h"

EXTERN_C_BEGIN

enum
{
	MUGGLE_SOCKET_PEER_TYPE_NULL = 0,
	MUGGLE_SOCKET_PEER_TYPE_TCP_LISTEN,
	MUGGLE_SOCKET_PEER_TYPE_TCP_PEER,
	MUGGLE_SOCKET_PEER_TYPE_UDP_PEER,
	MUGGLE_SOCKET_PEER_TYPE_MAX,
};

enum
{
	MUGGLE_SOCKET_PEER_STATUS_ACTIVE = 0,
	MUGGLE_SOCKET_PEER_STATUS_CLOSED = 1,
};

struct muggle_socket_event;

/**
 * @brief socket peer
 *
 * usage tip:
 *
 *   1. if only use in event thread, just muggle_socket_peer_close(peer) at anywhere u wanna close
 *
 *   2. if send peer to other thread, need invoke muggle_socket_peer_retain(peer) before send, and 
 *      invoke muggle_socket_peer_release(peer) after use in other thread, if u wanna close peer in
 *      other thread, in addition to invoke muggle_socket_peer_close(peer), muggle_socket_peer_release
 *      is still need to invoked
 */
typedef struct muggle_socket_peer
{
	muggle_atomic_int       ref_cnt;
	muggle_socket_t         fd;
	int                     peer_type; // MUGGLE_SOCKET_PEER_TYPE_*
	int                     status;    // MUGGLE_SOCKET_PEER_STATUS_*
	struct sockaddr_storage addr;
	muggle_socklen_t        addr_len;
	void                    *data;
	struct muggle_socket_event *ev;
}muggle_socket_peer_t;

/**
 * @brief init socket peer
 *
 * @param peer      socket peer pointer
 * @param fd        socket file descriptor
 * @param peer_type socket peer type
 * @param addr      socket address
 * @param addr_len  socket address length
 */
MUGGLE_C_EXPORT
void muggle_socket_peer_init(
	muggle_socket_peer_t *peer, muggle_socket_t fd,
	int peer_type, const struct sockaddr *addr, muggle_socklen_t addr_len);

/**
 * @brief increases the reference count of peer
 *
 * @param peer socket peer pointer
 *
 * @return refence count of peer after this call
 */
MUGGLE_C_EXPORT
int muggle_socket_peer_retain(muggle_socket_peer_t *peer);

/**
 * @brief 
 *   decreases the reference count by 1 and peer deallocates if the reference count
 *   reaches at 0
 *
 * @param peer  socket peer pointer
 *
 * @return refence count of peer after this call
 */
MUGGLE_C_EXPORT
int muggle_socket_peer_release(muggle_socket_peer_t *peer);

/**
 * @brief try to close socket peer
 *
 * @param peer  socket peer pointer
 */
MUGGLE_C_EXPORT
void muggle_socket_peer_close(muggle_socket_peer_t *peer);

/**
 * @brief receive messages from a socket peer
 *
 * @param peer     socket peer pointer
 * @param buf      buffer store received bytes
 * @param len      buffer size
 * @param flags    flag
 * @param addr     socket address
 * @param addrlen  socket address length
 *
 * @return 
 * return the number of bytes received, if error occurred, will try to close socket
 */
MUGGLE_C_EXPORT
int muggle_socket_peer_recvfrom(
	muggle_socket_peer_t *peer, void *buf, size_t len, int flags,
	struct sockaddr *addr, muggle_socklen_t *addrlen);

/**
 * @brief receive messages from a socket peer
 *
 * @param peer  socket peer pointer
 * @param buf   buffer store received bytes
 * @param len   buffer size
 * @param flags flag
 *
 * @return 
 */
MUGGLE_C_EXPORT
int muggle_socket_peer_recv(muggle_socket_peer_t *peer, void *buf, size_t len, int flags);

/**
 * @brief socket send message
 *
 * @param peer       socket peer pointer
 * @param buf        buffer store the bytes that need to sent
 * @param len        buffer size
 * @param flags      flag
 * @param dest_addr  dest socket address
 * @param addrlen    dest socket address length
 *
 * @return 
 */
MUGGLE_C_EXPORT
int muggle_socket_peer_sendto(muggle_socket_peer_t *peer, const void *buf, size_t len, int flags,
	const struct sockaddr *dest_addr, socklen_t addrlen);

/**
 * @brief 
 *
 * @param peer       socket peer pointer
 * @param buf        buffer store the bytes that need to sent
 * @param len        buffer size
 * @param flags      flag
 *
 * @return 
 */
MUGGLE_C_EXPORT
int muggle_socket_peer_send(muggle_socket_peer_t *peer, const void *buf, size_t len, int flags);

EXTERN_C_END

#endif
