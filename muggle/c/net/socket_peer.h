/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

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

/*
 * socket peer
 * usage tip:
 *   1. if only use in event thread, just muggle_socket_peer_close(peer) at anywhere u wanna close
 *   2. if send peer to other thread, need invoke muggle_socket_peer_retain(peer) before send, and 
 *      invoke muggle_socket_peer_release(peer) after use in other thread, if u wanna close peer in
 *      other thread, in addition to invoke muggle_socket_peer_close(peer), muggle_socket_peer_release
 *      is still need to invoked
 * */
typedef struct muggle_socket_peer
{
	muggle_atomic_int       ref_cnt;
	muggle_socket_t         fd;
	int                     peer_type; // MUGGLE_SOCKET_PEER_TYPE_*
	int                     status;    // MUGGLE_SOCKET_PEER_STATUS_*
	struct sockaddr_storage addr;
	muggle_socklen_t        addr_len;
	void                    *data;
}muggle_socket_peer_t;

/*
 * init socket peer
 * */
MUGGLE_CC_EXPORT
void muggle_socket_peer_init(
	muggle_socket_peer_t *peer, muggle_socket_t fd,
	int peer_type, const struct sockaddr *addr, muggle_socklen_t addr_len);

/*
 * increases the reference count of peer
 * RETURN: refence count of peer after this call
 * */
MUGGLE_CC_EXPORT
int muggle_socket_peer_retain(muggle_socket_peer_t *peer);

/*
 * decreases the reference count by 1 and peer deallocates if the reference count reaches
 * at 0
 * RETURN: refence count of peer after this call
 * */
MUGGLE_CC_EXPORT
int muggle_socket_peer_release(muggle_socket_peer_t *peer);

/*
 * try to close socket peer
 * */
MUGGLE_CC_EXPORT
void muggle_socket_peer_close(muggle_socket_peer_t *peer);

/*
 * receive messages from a socket peer
 * */
MUGGLE_CC_EXPORT
int muggle_socket_peer_recvfrom(
	muggle_socket_peer_t *peer, void *buf, size_t len, int flags,
	struct sockaddr *addr, muggle_socklen_t *addrlen);

/*
 * receive messages from a socket peer
 * */
MUGGLE_CC_EXPORT
int muggle_socket_peer_recv(muggle_socket_peer_t *peer, void *buf, size_t len, int flags);

/*
 * transmit a message to another socket
 * */
MUGGLE_CC_EXPORT
int muggle_socket_peer_sendto(muggle_socket_peer_t *peer, const void *buf, size_t len, int flags,
	const struct sockaddr *dest_addr, socklen_t addrlen);

/*
 * transmit a message to another socket
 * */
MUGGLE_CC_EXPORT
int muggle_socket_peer_send(muggle_socket_peer_t *peer, const void *buf, size_t len, int flags);

EXTERN_C_END

#endif