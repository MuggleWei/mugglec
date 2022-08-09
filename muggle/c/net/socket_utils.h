/******************************************************************************
 *  @file         socket_utils.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-21
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec socket utils
 *****************************************************************************/

#ifndef MUGGLE_C_SOCKET_UTILS_H_
#define MUGGLE_C_SOCKET_UTILS_H_

#include "muggle/c/net/socket.h"

EXTERN_C_BEGIN

/**
 * @brief convert sockaddr to presentation string
 *
 * @param sa         pointer to sockaddr
 * @param buf        buffer that store presentation string
 * @param bufsize    size of buffer
 * @param host_only  if 1, only show ip address without port
 *
 * @return return presentation string if success, otherwise return NULL
 */
MUGGLE_C_EXPORT
const char* muggle_socket_ntop(const struct sockaddr *sa, void *buf, size_t bufsize, int host_only);

/**
 * @brief get socket address info
 *
 * @param host     internet host
 * @param serv     internet service or port
 * @param hints    specifies criteria for selecting the socket address structures
 * @param addrinfo the first matching addrinfo returned from getaddrinfo
 * @param addr     the first matching sockaddr in addrinfo
 *
 * @return returns 0 if it succeeds, otherwise return -1
 */
MUGGLE_C_EXPORT
int muggle_socket_getaddrinfo(const char *host, const char *serv, struct addrinfo *hints, struct addrinfo *addrinfo, struct sockaddr *addr);

/**
 * @brief tcp listen
 *
 * @param host    internet host
 * @param serv    internet service or port
 * @param backlog maximum length to which the queue of pending connections
 *
 * @return 
 * on success, listen socket description is returned, otherwise return MUGGLE_INVALID_SOCKET
 */
MUGGLE_C_EXPORT
muggle_socket_t muggle_tcp_listen(const char *host, const char *serv, int backlog);

/**
 * @brief tcp connect
 *
 * @param host        target host
 * @param serv        target service or port
 * @param timeout_sec max seconds for wait connect complete
 *
 * @return on success, connected socket description is returned, otherwise return MUGGLE_INVALID_SOCKET
 */
MUGGLE_C_EXPORT
muggle_socket_t muggle_tcp_connect(const char *host, const char *serv, int timeout_sec);

/**
 * @brief udp bind
 *
 * @param host  bind host
 * @param serv  bind service or port
 *
 * @return 
 * on success, binded socket description is returned, otherwise return MUGGLE_INVALID_SOCKET
 */
MUGGLE_C_EXPORT
muggle_socket_t muggle_udp_bind(const char *host, const char *serv);

/**
 * @brief udp connect
 *
 * @param host target host
 * @param serv target service or port
 *
 * @return on success, connected socket description is returned, otherwise return MUGGLE_INVALID_SOCKET
 */
MUGGLE_C_EXPORT
muggle_socket_t muggle_udp_connect(const char *host, const char *serv);

/**
 * @brief udp multicast join
 *
 * @param host  multicast host
 * @param serv  multicast service or port
 * @param iface
 *   in *nix: net interface name, see 'ifconfig -s', if NULL, let kernel select interface
 *   in windows: this argument represent ip host, and port as serv
 * @param src_grp multicast source group, if NULL, not source-specific
 *
 * @return on success, mcast joined socket description is returned, otherwise return MUGGLE_INVALID_SOCKET
 */
MUGGLE_C_EXPORT
muggle_socket_t muggle_mcast_join(
	const char *host,
	const char *serv,
	const char *iface,
	const char *src_grp);

/**
 * @brief udp multicast leave
 *
 * @param host   multicast host
 * @param serv   multicast service or port
 * @param iface
 *   in *nix: net interface name, see 'ifconfig -s', if NULL, let kernel select interface
 *   in windows: this argument represent ip host, and port as serv
 * @param src_grp multicast source group, if NULL, not source-specific
 *
 * @return on success return 0, otherwise return -1
 */
MUGGLE_C_EXPORT
int muggle_mcast_leave(
	muggle_socket_t fd,
	const char *host,
	const char *serv,
	const char *iface,
	const char *src_grp);

EXTERN_C_END

#endif
