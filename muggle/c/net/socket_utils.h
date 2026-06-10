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
 * @brief tcp create callback function
 *
 * @param fd         socket fd
 * @param user_data  user data pointer
 */
typedef void (*fn_muggle_tcp_create_callback)(muggle_socket_t fd, void *user_data);

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
 * @brief convert sockaddr to presentation ip and port
 *
 * @param sa         pointer to sockaddr
 * @param buf        buffer that store host string
 * @param bufsize    size of buffer
 * @param port       port pointer
 *
 * @return return 0 if it succeeds, otherwise return -1
 */
MUGGLE_C_EXPORT
int muggle_socket_nto_ip_port(const struct sockaddr *sa, void *buf, size_t bufsize, int *port);

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
 * @brief get socket local address info in presentation string
 *
 * @param fd          socket fd
 * @param buf         buffer that store presentation string
 * @param bufsize     size of buffer
 * @param host_only   if 1, only show ip address without port
 *
 * @return return presentation string if success, otherwise return NULL
 */
MUGGLE_C_EXPORT
const char* muggle_socket_local_addr(muggle_socket_t fd, char *buf, size_t bufsize, int host_only);

/**
 * @brief get socket local host string and port
 *
 * @param fd        socket fd
 * @param buf       buffer that store presentation host string
 * @param bufsize   size of buffer
 * @param port      port pointer
 *
 * @return return 0 if it succeeds, otherwise return -1
 */
MUGGLE_C_EXPORT
int muggle_socket_local_ip_port(muggle_socket_t fd, char *buf, size_t bufsize, int *port);

/**
 * @brief get socket remote address info in presentation string
 *
 * @param fd          socket fd
 * @param buf         buffer that store presentation string
 * @param bufsize     size of buffer
 * @param host_only   if 1, only show ip address without port
 *
 * @return return presentation string if success, otherwise return NULL
 */
MUGGLE_C_EXPORT
const char* muggle_socket_remote_addr(muggle_socket_t fd, char *buf, size_t bufsize, int host_only);

/**
 * @brief get socket remote host string and port
 *
 * @param fd        socket fd
 * @param buf       buffer that store presentation host string
 * @param bufsize   size of buffer
 * @param port      port pointer
 *
 * @return return 0 if it succeeds, otherwise return -1
 */
MUGGLE_C_EXPORT
int muggle_socket_remote_ip_port(muggle_socket_t fd, char *buf, size_t bufsize, int *port);

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
 * @brief tcp listen with callback
 *
 * @param host      internet host
 * @param serv      internet service or port
 * @param backlog   maximum length to which the queue of pending connections
 * @param cb        callback before socket listen
 * @param user_data user data pass to cb
 *
 * @return
 *   - on success, listen socket description is returned
 *   - on failed, return MUGGLE_INVALID_SOCKET
 */
MUGGLE_C_EXPORT
muggle_socket_t muggle_tcp_listen_with_cb(
	const char *host, const char *serv, int backlog,
	fn_muggle_tcp_create_callback cb, void *user_data);

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
 * @brief tcp connect with callback
 *
 * @param host        target host
 * @param serv        target service or port
 * @param timeout_sec max seconds for wait connect complete
 * @param cb          callback before socket connect
 * @param user_data   user data pass to cb
 *
 * @return
 *   - on success, connected socket description is returned
 *   - on failed, return MUGGLE_INVALID_SOCKET
 */
MUGGLE_C_EXPORT
muggle_socket_t muggle_tcp_connect_with_cb(
	const char *host, const char *serv, int timeout_sec,
	fn_muggle_tcp_create_callback cb, void *user_data);

/**
 * @brief bind tcp
 *
 * @param bind_host  bind host
 * @param bind_serv  bind service or port, if it's null, bind port randomly
 *
 * @return
 *     - on success, connected socket description is returned
 *     - otherwise return MUGGLE_INVALID_SOCKET
 */
MUGGLE_C_EXPORT
muggle_socket_t muggle_tcp_bind(const char *bind_host, const char *bind_serv);

/**
 * @brief tcp bind and connect
 *
 * @param bind_host    bind host
 * @param bind_serv  bind service or port, if it's null, bind port randomly
 * @param host         target host
 * @param serv         target service or port
 * @param timeout_sec  max seconds for wait connect complete
 *
 * @return
 *     - on success, connected socket description is returned
 *     - otherwise return MUGGLE_INVALID_SOCKET
 */
MUGGLE_C_EXPORT
muggle_socket_t muggle_tcp_bind_connect(
		const char *bind_host, const char *bind_serv,
		const char *host, const char *serv,
		int timeout_sec);

/**
 * @brief tcp bind and connect with callback
 *
 * @param bind_host   bind host
 * @param bind_serv   bind service or port, if it's null, bind port randomly
 * @param host        target host
 * @param serv        target service or port
 * @param timeout_sec max seconds for wait connect complete
 * @param cb          callback before socket connect
 * @param user_data   user data pass to cb
 *
 * @return
 *     - on success, connected socket description is returned
 *     - otherwise return MUGGLE_INVALID_SOCKET
 */
MUGGLE_C_EXPORT
muggle_socket_t muggle_tcp_bind_connect_with_cb(
		const char *bind_host, const char *bind_serv,
		const char *host, const char *serv, int timeout_sec,
		fn_muggle_tcp_create_callback cb, void *user_data);

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

/**
 * @brief create a pair of connected sockets
 *
 * @param domain       socket domain(ignore in windows)
 * @param socket_type  socket type(ignore in windows)
 * @param protocol     protocol(ignore in windows)
 * @param fds[2]       socket file descriptors
 *
 * @return
 *     - on success, return 0
 *     - otherwise return -1
 */
MUGGLE_C_EXPORT
int muggle_socketpair(
	int domain,
	int socket_type,
	int protocol,
	muggle_socket_t fds[2]);

EXTERN_C_END

#endif
