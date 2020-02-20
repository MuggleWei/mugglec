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
 * getaddrinfo
 * @host: internet host
 * @serv: internet service or port
 * @hints: specifies criteria for selecting the socket address structures
 * @addrinfo: the first matching addrinfo returned from getaddrinfo
 * @addr: the first matching sockaddr in addrinfo
 * RETURN: returns 0 if it succeeds, otherwise return -1
 * */
MUGGLE_CC_EXPORT
int muggle_socket_getaddrinfo(const char *host, const char *serv, struct addrinfo *hints, struct addrinfo *addrinfo, struct sockaddr *addr);

/*
 * tcp listen
 * @host: internet host
 * @serv: internet service or port
 * @backlog: maximum length to which the queue of pending connections
 * @peer: store listen peer information, if not care about info, set NULL
 * RETURN: on success, listen socket description is returned, otherwise return MUGGLE_INVALID_SOCKET
 * */
MUGGLE_CC_EXPORT
muggle_socket_t muggle_tcp_listen(const char *host, const char *serv, int backlog, muggle_socket_peer_t *peer);

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

/*
 * udp bind
 * @host: bind host
 * @serv: bind service or port
 * @peer: store bind peer information, if not care about info, set NULL
 * RETURN: on success, binded socket description is returned, otherwise return MUGGLE_INVALID_SOCKET
 * */
MUGGLE_CC_EXPORT
muggle_socket_t muggle_udp_bind(const char *host, const char *serv, muggle_socket_peer_t *peer);

/*
 * udp connect
 * @host: target host
 * @serv: target service or port
 * @peer: socket peer store the connection information, if not care about connection info, set NULL
 * RETURN: on success, connected socket description is returned, otherwise return MUGGLE_INVALID_SOCKET
 * */
MUGGLE_CC_EXPORT
muggle_socket_t muggle_udp_connect(const char *host, const char *serv, muggle_socket_peer_t *peer);

/*
 * udp multicast join
 * @host: multicast host
 * @serv: multicast service or port
 * @iface:
 *   in *nix: net interface name, see 'ifconfig -s', if NULL, let kernel select interface
 *   in windows: this argument represent ip host, and port as serv
 * @src_grp: multicast source group, if NULL, not source-specific
 * @peer: socket peer store the connection information, if not care about connection info, set NULL
 * RETURN: on success, mcast joined socket description is returned, otherwise return MUGGLE_INVALID_SOCKET
 * */
MUGGLE_CC_EXPORT
muggle_socket_t muggle_mcast_join(
	const char *host,
	const char *serv,
	const char *iface,
	const char *src_grp,
	muggle_socket_peer_t *peer);

/*
* udp multicast leave
* @host: multicast host
* @serv: multicast service or port
* @iface:
*   in *nix: net interface name, see 'ifconfig -s', if NULL, let kernel select interface
*   in windows: this argument represent ip host, and port as serv
* @src_grp: multicast source group, if NULL, not source-specific
* RETURN: on success return 0, otherwise return -1
* */
MUGGLE_CC_EXPORT
int muggle_mcast_leave(
	muggle_socket_t fd,
	const char *host,
	const char *serv,
	const char *iface,
	const char *src_grp);

EXTERN_C_END

#endif
