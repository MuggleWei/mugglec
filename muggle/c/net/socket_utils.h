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

/*
 * convert sockaddr to presentation string
 * @sa: pointer to sockaddr
 * @buf: buffer that store presentation string
 * @bufsize: size of buffer
 * @host_only: if 1, only show ip address without port
 * RETURN: return presentation string if success, otherwise return NULL
 * */
const char* muggle_socket_ntop(const struct sockaddr *sa, void *buf, size_t bufsize, int host_only);

/*
 * tcp listen
 * @host: internet host
 * @serv: internet service or port
 * @backlog: maximum length to which the queue of pending connections
 * RETURN: on success, listen socket description is returned, otherwise return MUGGLE_INVALID_SOCKET
 * */
muggle_socket_t muggle_tcp_listen(const char *host, const char *serv, int backlog);

/*
 * tcp connect
 * @host: target host
 * @serv: target service or port
 * @timeout_sec: max seconds for wait connect complete
 * RETURN: on success, connected socket description is returned, otherwise return MUGGLE_INVALID_SOCKET
 * */
muggle_socket_t muggle_tcp_connect(const char *host, const char *serv, int timeout_sec);

EXTERN_C_END

#endif
