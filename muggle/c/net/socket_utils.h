/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_SOCKET_UTILS_H_
#define MUGGLE_C_SOCKET_UTILS_H_

#include "muggle/c/net/socket.h"

EXTERN_C_BEGIN

enum
{
	MUGGLE_SOCKET_PEER_TYPE_NULL = 0,
	MUGGLE_SOCKET_PEER_TYPE_TCP_LISTEN,
	MUGGLE_SOCKET_PEER_TYPE_TCP_PEER,
	MUGGLE_SOCKET_PEER_TYPE_UDP_PEER,
	MUGGLE_SOCKET_PEER_TYPE_MAX,
};

// socket peer
typedef struct muggle_socket_peer
{
	muggle_socket_t         fd;
	int                     peer_type; // MUGGLE_SOCKET_PEER_TYPE_*
	struct sockaddr_storage addr;
	muggle_socklen_t        addr_len;
	void                    *data;
}muggle_socket_peer_t;

/*
 * convert sockaddr to presentation string
 * @sa: pointer to sockaddr
 * @buf: buffer that store presentation string
 * @bufsize: size of buffer
 * @host_only: if 1, only show ip address without port
 * RETURN: return presentation string if success, otherwise return NULL
 * */
MUGGLE_CC_EXPORT
const char* muggle_socket_ntop(const struct sockaddr *sa, void *buf, size_t bufsize, int host_only);

/*
 * tcp listen
 * @host: internet host
 * @serv: internet service or port
 * @backlog: maximum length to which the queue of pending connections
 * RETURN: on success, listen socket description is returned, otherwise return MUGGLE_INVALID_SOCKET
 * */
MUGGLE_CC_EXPORT
muggle_socket_t muggle_tcp_listen(const char *host, const char *serv, int backlog);

/*
 * tcp connect
 * @host: target host
 * @serv: target service or port
 * @timeout_sec: max seconds for wait connect complete
 * @peer: socket peer store the connection information, if not care about connection info, set NULL
 * RETURN: on success, connected socket description is returned, otherwise return MUGGLE_INVALID_SOCKET
 * */
MUGGLE_CC_EXPORT
muggle_socket_t muggle_tcp_connect(const char *host, const char *serv, int timeout_sec, muggle_socket_peer_t *peer);

EXTERN_C_END

#endif
