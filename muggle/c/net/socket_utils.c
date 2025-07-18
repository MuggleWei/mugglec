/******************************************************************************
 *  @file         socket_utils.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-21
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec socket utils
 *****************************************************************************/

#include "socket_utils.h"
#include <stdio.h>
#include <string.h>
#include "muggle/c/log/log.h"

#if MUGGLE_PLATFORM_WINDOWS
#else
#include <fcntl.h>
#endif

const char* muggle_socket_ntop(const struct sockaddr *sa, void *buf, size_t bufsize, int host_only)
{
	switch (sa->sa_family)
	{
	case AF_INET:
		{
			const struct sockaddr_in *sin = (struct sockaddr_in*)sa;
			if (inet_ntop(AF_INET, (void*)&sin->sin_addr, buf, (socklen_t)bufsize) == NULL)
			{
				char err_msg[1024] = {0};
				muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
				MUGGLE_LOG_ERROR("failed inet_ntop - %s", err_msg);
				return NULL;
			}

			if (!host_only)
			{
				size_t offset = strlen(buf);
				if ((int)bufsize - (int)offset < 8)
				{
					MUGGLE_LOG_ERROR("buffer size(%d) is not enough for socket address presentation string", (int)bufsize);
					return NULL;
				}
				snprintf((char*)buf + offset, bufsize - offset, ":%d", (int)ntohs(sin->sin_port));
			}

			return (const char*)buf;
		}break;
	case AF_INET6:
		{
			const struct sockaddr_in6 *sin6 = (struct sockaddr_in6*)sa;

			int offset = 0;
			char *cbuf = (char*)buf;
			if (!host_only)
			{
				cbuf[0] = '[';
				offset += 1;
			}

			if (inet_ntop(AF_INET6, (void*)&sin6->sin6_addr, cbuf + offset, (socklen_t)bufsize) == NULL)
			{
				char err_msg[1024] = {0};
				muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
				MUGGLE_LOG_ERROR("failed inet_ntop - %s", err_msg);
				return NULL;
			}

			if (!host_only)
			{
				size_t offset = strlen(cbuf);
				int remain = (int)bufsize - (int)offset;
				if (remain < 8)
				{
					MUGGLE_LOG_ERROR("buffer size(%d) is not enough for socket address presentation string", (int)bufsize);
					return NULL;
				}
				snprintf(cbuf + offset, remain, "]:%d", (int)ntohs(sin6->sin6_port));
			}

			return (const char*)buf;
		}break;
	default:
		{
			MUGGLE_LOG_ERROR("invalid AF_* family(%d) for socket_ntop", sa->sa_family);
		}
	}

	return NULL;
}

int muggle_socket_nto_ip_port(const struct sockaddr *sa, void *buf, size_t bufsize, int *port)
{
	switch (sa->sa_family)
	{
	case AF_INET:
		{
			const struct sockaddr_in *sin = (struct sockaddr_in*)sa;
			if (inet_ntop(AF_INET, (void*)&sin->sin_addr, buf, (socklen_t)bufsize) == NULL)
			{
				char err_msg[1024] = {0};
				muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
				MUGGLE_LOG_ERROR("failed inet_ntop - %s", err_msg);
				return -1;
			}

			*port = (int)ntohs(sin->sin_port);
			return 0;
		}break;
	case AF_INET6:
		{
			const struct sockaddr_in6 *sin6 = (struct sockaddr_in6*)sa;

			if (inet_ntop(AF_INET6, (void*)&sin6->sin6_addr, buf, (socklen_t)bufsize) == NULL)
			{
				char err_msg[1024] = {0};
				muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
				MUGGLE_LOG_ERROR("failed inet_ntop - %s", err_msg);
				return -1;
			}

			*port = (int)ntohs(sin6->sin6_port);

			return 0;
		}break;
	default:
		{
			MUGGLE_LOG_ERROR("invalid AF_* family(%d) for socket_nto_host_port", sa->sa_family);
		}
	}

	return -1;
}

int muggle_socket_getaddrinfo(const char *host, const char *serv, struct addrinfo *hints, struct addrinfo *addrinfo, struct sockaddr *addr)
{
	struct addrinfo *res;
	int n;
	if ((n = getaddrinfo(host, serv, hints, &res)) != 0)
	{
#if MUGGLE_PLATFORM_WINDOWS
		char err_msg[1024] = { 0 };
		muggle_socket_strerror(WSAGetLastError(), err_msg, sizeof(err_msg));
		MUGGLE_LOG_ERROR("failed muggle_socket_getaddrinfo for %s:%s - getaddrinfo return '%s'",
			host, serv, err_msg);
#else
		MUGGLE_LOG_ERROR("failed muggle_socket_getaddrinfo for %s:%s - getaddrinfo return '%s'",
			host, serv, gai_strerror(n));
#endif
		return -1;
	}

	if (res)
	{
		memcpy(addrinfo, res, sizeof(struct addrinfo));
		memcpy(addr, res->ai_addr, res->ai_addrlen);
		freeaddrinfo(res);
		addrinfo->ai_addr = addr;
		addrinfo->ai_next = NULL;
		return 0;
	}

	return -1;
}

const char* muggle_socket_local_addr(muggle_socket_t fd, char *buf, size_t bufsize, int host_only)
{
	struct sockaddr_storage sin;
	memset(&sin, 0, sizeof(sin));
	muggle_socklen_t len = sizeof(sin);

	if (getsockname(fd, (struct sockaddr*)&sin, &len) != 0)
	{
		return NULL;
	}

	return muggle_socket_ntop((struct sockaddr*)&sin, buf, bufsize, host_only);
}

int muggle_socket_local_ip_port(muggle_socket_t fd, char *buf, size_t bufsize, int *port)
{
	struct sockaddr_storage sin;
	memset(&sin, 0, sizeof(sin));
	muggle_socklen_t len = sizeof(sin);

	if (getsockname(fd, (struct sockaddr*)&sin, &len) != 0)
	{
		return -1;
	}

	return muggle_socket_nto_ip_port((struct sockaddr*)&sin, buf, bufsize, port);
}

const char* muggle_socket_remote_addr(muggle_socket_t fd, char *buf, size_t bufsize, int host_only)
{
	struct sockaddr_storage sin;
	memset(&sin, 0, sizeof(sin));
	muggle_socklen_t len = sizeof(sin);

	if (getpeername(fd, (struct sockaddr*)&sin, &len) != 0)
	{
		return NULL;
	}

	return muggle_socket_ntop((struct sockaddr*)&sin, buf, bufsize, host_only);
}

int muggle_socket_remote_ip_port(muggle_socket_t fd, char *buf, size_t bufsize, int *port)
{
	struct sockaddr_storage sin;
	memset(&sin, 0, sizeof(sin));
	muggle_socklen_t len = sizeof(sin);

	if (getpeername(fd, (struct sockaddr*)&sin, &len) != 0)
	{
		return -1;
	}

	return muggle_socket_nto_ip_port((struct sockaddr*)&sin, buf, bufsize, port);
}

muggle_socket_t muggle_tcp_listen(const char *host, const char *serv, int backlog)
{
	muggle_socket_t listen_socket = MUGGLE_INVALID_SOCKET;

	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int n;
	if ((n = getaddrinfo(host, serv, &hints, &res)) != 0)
	{
#if MUGGLE_PLATFORM_WINDOWS
		char err_msg[1024] = {0};
		muggle_socket_strerror(WSAGetLastError(), err_msg, sizeof(err_msg));
		MUGGLE_LOG_ERROR("failed tcp listen for %s:%s - getaddrinfo return '%s'",
				host, serv, err_msg);
#else
		MUGGLE_LOG_ERROR("failed tcp listen for %s:%s - getaddrinfo return '%s'",
				host, serv, gai_strerror(n));
#endif
		return MUGGLE_INVALID_SOCKET;
	}

	struct addrinfo *ressave = res;
	for (; res != NULL; res = res->ai_next)
	{
		listen_socket = muggle_socket_create(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (listen_socket == MUGGLE_INVALID_SOCKET)
		{
			continue;
		}

		// always set SO_REUSEADDR for bind socket
		int on = 1;
		if (muggle_setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, (const void*)&on, sizeof(on)) != 0)
		{
			char err_msg[1024] = {0};
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_LOG_WARNING("failed setsockopt SO_REUSEADDR on - %s", err_msg);
		}

		if (bind(listen_socket, res->ai_addr, (muggle_socklen_t)res->ai_addrlen) == 0)
		{
			break;
		}
		else
		{
			char err_msg[1024] = {0};
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_LOG_WARNING("failed bind %s:%s - %s", host, serv, err_msg);
		}

		muggle_socket_close(listen_socket);
		listen_socket = MUGGLE_INVALID_SOCKET;
	}

	if (res == NULL)
	{
		MUGGLE_LOG_ERROR("failed to create and bind tcp socket for %s:%s", host, serv);
		freeaddrinfo(ressave);
		return MUGGLE_INVALID_SOCKET;
	}

	if (listen(listen_socket, backlog) != 0)
	{
		char err_msg[1024] = {0};
		muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
		MUGGLE_LOG_ERROR("failed listen for %s:%s - %s", host, serv, err_msg);

		muggle_socket_close(listen_socket);
		freeaddrinfo(ressave);
		return MUGGLE_INVALID_SOCKET;
	}

	freeaddrinfo(ressave);

	return listen_socket;
}

#define MUGGLE_OUTPUT_WAIT_TCP_CONN_LAST_ERROR(host, serv)                    \
	int last_errnum = MUGGLE_SOCKET_LAST_ERRNO;                           \
	char err_msg[1024] = { 0 };                                           \
	muggle_socket_strerror(last_errnum, err_msg, sizeof(err_msg));        \
	MUGGLE_LOG_DEBUG(                                                     \
		"failed nonblocking tcp connect for %s:%s - (errno=%d) %s", host, \
		serv, last_errnum, err_msg);

#define MUGGLE_WAIT_TCP_CONN_USE_POLL 1

#if MUGGLE_WAIT_TCP_CONN_USE_POLL

static bool muggle_wait_tcp_connect_poll(
	muggle_socket_t client, int timeout_sec, const char *host, const char *serv)
{
	struct pollfd pfd;
	memset(&pfd, 0, sizeof(pfd));
	pfd.fd = client;
	pfd.events = POLLIN | POLLOUT;

#if MUGGLE_PLATFORM_WINDOWS
	int n = WSAPoll(&pfd, 1, timeout_sec * 1000);
#else
	int n = poll(&pfd, 1, timeout_sec * 1000);
#endif
	if (n == 0)
	{
		MUGGLE_LOG_DEBUG(
			"failed nonblocking tcp connect for %s:%s - timeout(%d sec)",
			host, serv, timeout_sec);
		return false;
	}
	else if (n == MUGGLE_SOCKET_ERROR)
	{
		MUGGLE_OUTPUT_WAIT_TCP_CONN_LAST_ERROR(host, serv);
		return false;
	}

	if (pfd.revents & POLLOUT)
	{
		int err = 0;
        muggle_socklen_t len = sizeof(err);
        if (getsockopt(client, SOL_SOCKET, SO_ERROR, (void*)&err, &len) < 0)
        {
			MUGGLE_OUTPUT_WAIT_TCP_CONN_LAST_ERROR(host, serv);
            return false;
        }
		if (err != 0)
		{
			char err_msg[1024] = {0};
			muggle_socket_strerror(err, err_msg, sizeof(err_msg));
			MUGGLE_LOG_DEBUG(
				"failed nonblocking tcp connect for %s:%s - (so_error=%d) %s",
				host, serv, err, err_msg);
			return false;
		}
	}
	else
	{
        MUGGLE_LOG_DEBUG(
			"failed nonblocking tcp connect for %s:%s - socket not set",
			host, serv);
        return false;
	}

	return true;
}

#else

static bool muggle_wait_tcp_connect_select(
	muggle_socket_t client, int timeout_sec, const char *host, const char *serv)
{
    // select
    fd_set rset, wset;
    FD_ZERO(&rset);
    FD_SET(client, &rset);
    wset = rset;

    struct timeval tval;
    tval.tv_sec = timeout_sec;
    tval.tv_usec = 0;

    int nfds = 0;
#if !MUGGLE_PLATFORM_WINDOWS
    nfds = client + 1;
#endif

    int n = select(nfds, &rset, &wset, NULL, timeout_sec ? &tval : NULL);
    if (n == 0)
    {
        MUGGLE_LOG_DEBUG(
			"failed nonblocking tcp connect for %s:%s - timeout(%d sec)",
			host, serv, timeout_sec);
        return false;
    }
    else if (n == MUGGLE_SOCKET_ERROR)
    {
		MUGGLE_OUTPUT_WAIT_TCP_CONN_LAST_ERROR(host, serv);
        return false;
    }

    if (FD_ISSET(client, &rset) || FD_ISSET(client, &wset))
    {
		int err = 0;
        muggle_socklen_t len = sizeof(err);
        if (getsockopt(client, SOL_SOCKET, SO_ERROR, (void*)&err, &len) < 0)
        {
			MUGGLE_OUTPUT_WAIT_TCP_CONN_LAST_ERROR(host, serv);
            return false;
        }
		if (err != 0)
		{
			char err_msg[1024] = {0};
			muggle_socket_strerror(err, err_msg, sizeof(err_msg));
			MUGGLE_LOG_DEBUG(
				"failed nonblocking tcp connect for %s:%s - (so_error=%d) %s",
				host, serv, err, err_msg);
			return false;
		}
    }
    else
    {
        MUGGLE_LOG_DEBUG(
			"failed nonblocking tcp connect for %s:%s - socket not set",
			host, serv);
        return false;
    }

	return true;
}

#endif

static bool muggle_wait_tcp_connect(
	muggle_socket_t client, int timeout_sec, const char *host, const char *serv)
{
#if MUGGLE_WAIT_TCP_CONN_USE_POLL
	return muggle_wait_tcp_connect_poll(client, timeout_sec, host, serv);
#else
	return muggle_wait_tcp_connect_select(client, timeout_sec, host, serv);
#endif
}

muggle_socket_t muggle_tcp_connect(const char *host, const char *serv, int timeout_sec)
{
    muggle_socket_t client = MUGGLE_INVALID_SOCKET;

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;

    int n;
    if ((n = getaddrinfo(host, serv, &hints, &res)) != 0)
    {
#if MUGGLE_PLATFORM_WINDOWS
        char err_msg[1024] = {0};
        muggle_socket_strerror(WSAGetLastError(), err_msg, sizeof(err_msg));
        MUGGLE_LOG_ERROR("failed nonblocking tcp connect for %s:%s - getaddrinfo return '%s'",
            host, serv, err_msg);
#else
        MUGGLE_LOG_ERROR("failed nonblocking tcp connect for %s:%s - getaddrinfo return '%s'",
            host, serv, gai_strerror(n));
#endif
        return MUGGLE_INVALID_SOCKET;
    }

    struct addrinfo *ressave = res;
    for (; res != NULL; res = res->ai_next)
    {
        client = muggle_socket_create(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (client != MUGGLE_INVALID_SOCKET)
        {
            break;
        }
    }

    if (res == NULL)
    {
		int last_errnum = MUGGLE_SOCKET_LAST_ERRNO;
		MUGGLE_LOG_ERROR("failed create socket for %s:%s - (errno=%d)", 
				host, serv, last_errnum);
        freeaddrinfo(ressave);
        return MUGGLE_INVALID_SOCKET;
    }

    // set nonblock
#if MUGGLE_PLATFORM_WINDOWS
    u_long iMode = 1;

    // If iMode = 0, blocking is enabled;
    // If iMode != 0, non-blocking mode is enabled.
    ioctlsocket(client, FIONBIO, &iMode);
#else
    int flags = 0;
    flags = fcntl(client, F_GETFL, 0);
    fcntl(client, F_SETFL, flags | O_NONBLOCK);
#endif

    // connect
    n = connect(client, res->ai_addr, (muggle_socklen_t)res->ai_addrlen);
    if (n < 0)
    {
#if MUGGLE_PLATFORM_WINDOWS
        if (MUGGLE_SOCKET_LAST_ERRNO != WSAEWOULDBLOCK)
#else
        if (MUGGLE_SOCKET_LAST_ERRNO != EINPROGRESS)
#endif
        {
			int last_errnum = MUGGLE_SOCKET_LAST_ERRNO;
            char err_msg[1024] = {0};
            muggle_socket_strerror(last_errnum, err_msg, sizeof(err_msg));
            MUGGLE_LOG_ERROR("failed connect for %s:%s - (errno=%d) %s",
				host, serv, last_errnum, err_msg);

            muggle_socket_close(client);
            freeaddrinfo(ressave);
            return MUGGLE_INVALID_SOCKET;
        }
    }

    freeaddrinfo(ressave);

    // connect completed immediately
    if (n == 0)
    {
        // restore file status flags
#if MUGGLE_PLATFORM_WINDOWS
        iMode = 0;
        ioctlsocket(client, FIONBIO, &iMode);
#else
        fcntl(client, F_SETFL, flags);
#endif
        return client;
    }

	// wait connect completed
	if (!muggle_wait_tcp_connect(client, timeout_sec, host, serv))
	{
        muggle_socket_close(client);
		client = MUGGLE_INVALID_SOCKET;
		return MUGGLE_INVALID_SOCKET;
	}

    // restore file status flags
#if MUGGLE_PLATFORM_WINDOWS
    iMode = 0;
    ioctlsocket(client, FIONBIO, &iMode);
#else
    fcntl(client, F_SETFL, flags);
#endif

    return client;
}

muggle_socket_t muggle_tcp_bind(const char *bind_host, const char *bind_serv)
{
	muggle_socket_t client = MUGGLE_INVALID_SOCKET;

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;

    int n;
    if ((n = getaddrinfo(bind_host, bind_serv, &hints, &res)) != 0) {
#if MUGGLE_PLATFORM_WINDOWS
        char err_msg[1024] = {0};
        muggle_socket_strerror(WSAGetLastError(), err_msg, sizeof(err_msg));
		MUGGLE_LOG_ERROR("failed tcp bind -> %s %s"
			" - getaddrinfo return '%s'",
			bind_host, bind_serv == NULL ? "(null)" : bind_serv, err_msg);
#else
		MUGGLE_LOG_ERROR("failed tcp bind %s:%s - getaddrinfo return '%s'",
			bind_host, bind_serv == NULL ? "(null)" : bind_serv,
			gai_strerror(n));
#endif
        return MUGGLE_INVALID_SOCKET;
    }

    struct addrinfo *ressave = res;
    for (; res != NULL; res = res->ai_next)
    {
        client = muggle_socket_create(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (client == MUGGLE_INVALID_SOCKET)
        {
			continue;
        }

		// always set SO_REUSEADDR for bind socket
		int on = 1;
		if (muggle_setsockopt(client, SOL_SOCKET, SO_REUSEADDR, (void*)&on, sizeof(on)) != 0)
		{
			char err_msg[1024] = {0};
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_LOG_WARNING("failed setsockopt SO_REUSEADDR on - %s", err_msg);
		}

		if (bind(client, res->ai_addr, (muggle_socklen_t)res->ai_addrlen) == 0) {
			break;
		} else {
			char err_msg[1024] = {0};
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_LOG_WARNING("failed tcp bind %s:%s - bind return '%s'",
				bind_host, bind_serv == NULL ? "(null)" : bind_serv, err_msg);
		}

		muggle_socket_close(client);
		client = MUGGLE_INVALID_SOCKET;
    }

	if (res == NULL) {
		MUGGLE_LOG_ERROR("failed tcp bind %s:%s - no valid addrinfo",
			bind_host, bind_serv == NULL ? "(null)" : bind_serv);
		freeaddrinfo(ressave);
		return MUGGLE_INVALID_SOCKET;
	}

	freeaddrinfo(ressave);

	return client;
}

static muggle_socket_t muggle_tcp_binded_socket_connect(
		muggle_socket_t client, const char *host, const char *serv, int timeout_sec)
{
    // set nonblock
#if MUGGLE_PLATFORM_WINDOWS
    u_long iMode = 1;

    // If iMode = 0, blocking is enabled;
    // If iMode != 0, non-blocking mode is enabled.
    ioctlsocket(client, FIONBIO, &iMode);
#else
    int flags = 0;
    flags = fcntl(client, F_GETFL, 0);
    fcntl(client, F_SETFL, flags | O_NONBLOCK);
#endif

	// get remote addr
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;

    int n;
    if ((n = getaddrinfo(host, serv, &hints, &res)) != 0)
    {
#if MUGGLE_PLATFORM_WINDOWS
        char err_msg[1024] = {0};
        muggle_socket_strerror(WSAGetLastError(), err_msg, sizeof(err_msg));
		MUGGLE_LOG_ERROR("failed nonblocking tcp bind and connect for %s:%s"
			" - getaddrinfo return '%s'",
            host, serv, err_msg);
#else
		MUGGLE_LOG_ERROR("failed nonblocking tcp bind and connect for %s:%s"
			" - getaddrinfo return '%s'",
            host, serv, gai_strerror(n));
#endif
        return MUGGLE_INVALID_SOCKET;
    }

	if (res == NULL) {
		MUGGLE_LOG_ERROR("failed nonblocking tcp bind and connect for %s:%s"
			" - 0 valid addrinfo",
			host, serv);
		muggle_socket_close(client);
		return MUGGLE_INVALID_SOCKET;
	}

	// connect
    n = connect(client, res->ai_addr, (muggle_socklen_t)res->ai_addrlen);
    if (n < 0)
    {
#if MUGGLE_PLATFORM_WINDOWS
        if (MUGGLE_SOCKET_LAST_ERRNO != WSAEWOULDBLOCK)
#else
        if (MUGGLE_SOCKET_LAST_ERRNO != EINPROGRESS)
#endif
        {
            char err_msg[1024] = {0};
            muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
            MUGGLE_LOG_ERROR("failed connect for %s:%s - %s", host, serv, err_msg);

            muggle_socket_close(client);
            freeaddrinfo(res);
            return MUGGLE_INVALID_SOCKET;
        }
    }

    freeaddrinfo(res);

    // connect completed immediately
    if (n == 0)
    {
        // restore file status flags
#if MUGGLE_PLATFORM_WINDOWS
        iMode = 0;
        ioctlsocket(client, FIONBIO, &iMode);
#else
        fcntl(client, F_SETFL, flags);
#endif
        return client;
    }

    // select
    fd_set rset, wset;
    FD_ZERO(&rset);
    FD_SET(client, &rset);
    wset = rset;

    struct timeval tval;
    tval.tv_sec = timeout_sec;
    tval.tv_usec = 0;

    int nfds = 0;
#if !MUGGLE_PLATFORM_WINDOWS
    nfds = client + 1;
#endif

    n = select(nfds, &rset, &wset, NULL, timeout_sec ? &tval : NULL);
    if (n == 0)
    {
        muggle_socket_close(client);
        MUGGLE_LOG_DEBUG("failed nonblocking tcp connect for %s:%s - timeout(%d sec)", host, serv, timeout_sec);
        return MUGGLE_INVALID_SOCKET;
    }
    else if (n == MUGGLE_SOCKET_ERROR)
    {
        char err_msg[1024] = {0};
        muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
        MUGGLE_LOG_DEBUG("failed nonblocking tcp connect for %s:%s - %s", host, serv, err_msg);

        muggle_socket_close(client);
        return MUGGLE_INVALID_SOCKET;
    }

    int error = 0;
    if (FD_ISSET(client, &rset) || FD_ISSET(client, &wset))
    {
        muggle_socklen_t len = sizeof(error);
        if (getsockopt(client, SOL_SOCKET, SO_ERROR, (void*)&error, &len) < 0)
        {
            char err_msg[1024] = {0};
            muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
            MUGGLE_LOG_DEBUG("failed nonblocking tcp connect for %s:%s - %s", host, serv, err_msg);

            muggle_socket_close(client);
            return MUGGLE_INVALID_SOCKET;
        }
    }
    else
    {
        MUGGLE_LOG_DEBUG("failed nonblocking tcp connect for %s:%s - socket not set", host, serv);
        muggle_socket_close(client);
        return MUGGLE_INVALID_SOCKET;
    }

    if (error != 0)
    {
        char err_msg[1024] = {0};
        muggle_socket_strerror(error, err_msg, sizeof(err_msg));
        MUGGLE_LOG_DEBUG("failed nonblocking tcp connect for %s:%s - %s", host, serv, err_msg);

        muggle_socket_close(client);
        return MUGGLE_INVALID_SOCKET;
    }

    // restore file status flags
#if MUGGLE_PLATFORM_WINDOWS
    iMode = 0;
    ioctlsocket(client, FIONBIO, &iMode);
#else
    fcntl(client, F_SETFL, flags);
#endif

	return client;
}

muggle_socket_t muggle_tcp_bind_connect(
		const char *bind_host, const char *bind_serv,
		const char *host, const char *serv,
		int timeout_sec)
{
	muggle_socket_t client = muggle_tcp_bind(bind_host, bind_serv);
	if (client == MUGGLE_INVALID_SOCKET) {
		MUGGLE_LOG_ERROR("failed tcp bind %s:%s",
			bind_host, bind_serv == NULL ? "(null)" : bind_serv);
		return client;
	}

	return muggle_tcp_binded_socket_connect(client, host, serv, timeout_sec);
}

muggle_socket_t muggle_udp_bind(const char *host, const char *serv)
{
	muggle_socket_t udp_socket = MUGGLE_INVALID_SOCKET;

	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	int n;
	if ((n = getaddrinfo(host, serv, &hints, &res)) != 0)
	{
#if MUGGLE_PLATFORM_WINDOWS
		char err_msg[1024] = {0};
		muggle_socket_strerror(WSAGetLastError(), err_msg, sizeof(err_msg));
		MUGGLE_LOG_ERROR("failed udp bind for %s:%s - getaddrinfo return '%s'",
				host, serv, err_msg);
#else
		MUGGLE_LOG_ERROR("failed udp bind for %s:%s - getaddrinfo return '%s'",
				host, serv, gai_strerror(n));
#endif
		return MUGGLE_INVALID_SOCKET;
	}

	struct addrinfo *ressave = res;
	for (; res != NULL; res = res->ai_next)
	{
		udp_socket = muggle_socket_create(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (udp_socket == MUGGLE_INVALID_SOCKET)
		{
			continue;
		}

		// always set SO_REUSEADDR for bind socket
		int on = 1;
		if (muggle_setsockopt(udp_socket, SOL_SOCKET, SO_REUSEADDR, (void*)&on, sizeof(on)) != 0)
		{
			char err_msg[1024] = {0};
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_LOG_WARNING("failed setsockopt SO_REUSEADDR on - %s", err_msg);
		}

		if (bind(udp_socket, res->ai_addr, (muggle_socklen_t)res->ai_addrlen) == 0)
		{
			break;
		}
		else
		{
			char err_msg[1024] = {0};
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_LOG_WARNING("failed bind %s:%s - %s", host, serv, err_msg);
		}

		muggle_socket_close(udp_socket);
		udp_socket = MUGGLE_INVALID_SOCKET;
	}

	if (res == NULL)
	{
		MUGGLE_LOG_ERROR("failed to create and bind udp socket for %s:%s", host, serv);
		freeaddrinfo(ressave);
		return MUGGLE_INVALID_SOCKET;
	}

	freeaddrinfo(ressave);

	return udp_socket;
}

muggle_socket_t muggle_udp_connect(const char *host, const char *serv)
{
	muggle_socket_t udp_socket = MUGGLE_INVALID_SOCKET;

	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	int n;
	if ((n = getaddrinfo(host, serv, &hints, &res)) != 0)
	{
#if MUGGLE_PLATFORM_WINDOWS
		char err_msg[1024] = {0};
		muggle_socket_strerror(WSAGetLastError(), err_msg, sizeof(err_msg));
		MUGGLE_LOG_ERROR("failed udp connect for %s:%s - getaddrinfo return '%s'",
				host, serv, err_msg);
#else
		MUGGLE_LOG_ERROR("failed udp connect for %s:%s - getaddrinfo return '%s'",
				host, serv, gai_strerror(n));
#endif
		return MUGGLE_INVALID_SOCKET;
	}

	struct addrinfo *ressave = res;
	for (; res != NULL; res = res->ai_next)
	{
		udp_socket = muggle_socket_create(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (udp_socket == MUGGLE_INVALID_SOCKET)
		{
			continue;
		}

		if (connect(udp_socket, res->ai_addr, (muggle_socklen_t)res->ai_addrlen) == 0)
		{
			break;
		}

		muggle_socket_close(udp_socket);
		udp_socket = MUGGLE_INVALID_SOCKET;
	}

	if (res == NULL)
	{
		MUGGLE_LOG_ERROR("failed to create and connect udp socket for %s:%s", host, serv);
		freeaddrinfo(ressave);
		return MUGGLE_INVALID_SOCKET;
	}

	freeaddrinfo(ressave);

	return udp_socket;
}

muggle_socket_t muggle_mcast_join(
	const char *host,
	const char *serv,
	const char *iface,
	const char *src_grp)
{
	muggle_socket_t fd = MUGGLE_INVALID_SOCKET;

	if (host == NULL)
	{
		MUGGLE_LOG_ERROR("failed muggle_mcast_join - with null host");
		return MUGGLE_INVALID_SOCKET;
	}

	if (serv == NULL)
	{
		MUGGLE_LOG_ERROR("failed muggle_mcast_join - with null serv");
		return MUGGLE_INVALID_SOCKET;
	}

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	// get multicast address info
	struct addrinfo multicast_addrinfo;
	struct sockaddr_storage multicast_addr;
	if (muggle_socket_getaddrinfo(host, serv, &hints, &multicast_addrinfo, (struct sockaddr*)&multicast_addr) != 0)
	{
		MUGGLE_LOG_ERROR("failed get multicast address info for %s:%s", host, serv);
		return MUGGLE_INVALID_SOCKET;
	}

	struct addrinfo *bind_addrinfo = NULL;
	// struct sockaddr_storage *bind_addr = NULL;

#if MUGGLE_PLATFORM_WINDOWS
	// get local address
	const char *ipv4_host_any = "0.0.0.0";
	const char *ipv6_host_any = "::";
	if (iface == NULL)
	{
		if (multicast_addrinfo.ai_family == AF_INET)
		{
			iface = ipv4_host_any;
		}
		else if (multicast_addrinfo.ai_family = AF_INET6)
		{
			iface = ipv6_host_any;
		}
	}
	struct addrinfo local_addrinfo;
	struct sockaddr_storage local_addr;
	if (muggle_socket_getaddrinfo(iface, serv, &hints, &local_addrinfo, (struct sockaddr*)&local_addr) != 0)
	{
		MUGGLE_LOG_ERROR("failed get local address info for %s:%s", iface, serv);
		return MUGGLE_INVALID_SOCKET;
	}

	bind_addrinfo = &local_addrinfo;
	// bind_addr = &local_addr;
#else
	unsigned int iface_idx = 0;
	if (iface != NULL)
	{
		iface_idx = if_nametoindex(iface);
		if (iface_idx == 0)
		{
			char err_msg[1024] = { 0 };
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_LOG_WARNING("failed convert net interface(%s) to index, let kernel select - %s", iface, err_msg);
		}
	}

	bind_addrinfo = &multicast_addrinfo;
	// bind_addr = &multicast_addr;
#endif

	// create socket
	fd = muggle_socket_create(bind_addrinfo->ai_family, bind_addrinfo->ai_socktype, bind_addrinfo->ai_protocol);
	if (fd == MUGGLE_INVALID_SOCKET)
	{
		char err_msg[1024] = { 0 };
		muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
		MUGGLE_LOG_ERROR("failed muggle_socket_create info for multicast[%s:%s] iface[%s] - %s", host, serv, iface, err_msg);
		return MUGGLE_INVALID_SOCKET;
	}

	// always set SO_REUSEADDR for bind socket
	int on = 1;
	if (muggle_setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*)&on, sizeof(on)) != 0)
	{
		char err_msg[1024] = { 0 };
		muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
		MUGGLE_LOG_WARNING("failed setsockopt SO_REUSEADDR on - %s", err_msg);
	}

	// bind
	if (bind(fd, bind_addrinfo->ai_addr, (muggle_socklen_t)bind_addrinfo->ai_addrlen) != 0)
	{
		char err_msg[1024] = { 0 };
		muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
		MUGGLE_LOG_WARNING("failed bind %s:%s - %s", iface, serv, err_msg);

		muggle_socket_close(fd);
		return MUGGLE_INVALID_SOCKET;
	}

	int level = -1;
	switch (multicast_addrinfo.ai_family)
	{
	case AF_INET:
	{
		level = IPPROTO_IP;
	}break;
	case AF_INET6:
	{
		level = IPPROTO_IPV6;
	}break;
	default:
	{
		MUGGLE_LOG_ERROR("unsupported socket family: %d", multicast_addrinfo.ai_family);
		muggle_socket_close(fd);
		return MUGGLE_INVALID_SOCKET;
	}break;
	}

	// join mcast
	int ret = 0;
#if MUGGLE_PLATFORM_WINDOWS
	if (level == IPPROTO_IP)
	{
		if (src_grp == NULL)
		{
			struct ip_mreq mreq;
			mreq.imr_interface.s_addr = ((struct sockaddr_in*)&local_addr)->sin_addr.s_addr;
			mreq.imr_multiaddr.s_addr = ((struct sockaddr_in*)&multicast_addr)->sin_addr.s_addr;
			ret = muggle_setsockopt(fd, level, IP_ADD_MEMBERSHIP, (void*)&mreq, sizeof(mreq));
			if (ret != 0)
			{
				char err_msg[1024] = { 0 };
				muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
				MUGGLE_LOG_ERROR("failed setsockopt IP_ADD_MEMBERSHIP - %s", err_msg);
			}
		}
		else
		{
			struct sockaddr_in src_addr;
			if (inet_pton(multicast_addrinfo.ai_family, src_grp, &src_addr.sin_addr) <= 0)
			{
				char err_msg[1024] = { 0 };
				muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
				MUGGLE_LOG_ERROR("failed inet_pton for %s - %s", src_grp, err_msg);
				ret = -1;
			}
			else
			{
				struct ip_mreq_source mreq;
				mreq.imr_interface.s_addr = ((struct sockaddr_in*)&local_addr)->sin_addr.s_addr;
				mreq.imr_sourceaddr.s_addr = src_addr.sin_addr.s_addr;
				mreq.imr_multiaddr.s_addr = ((struct sockaddr_in*)&multicast_addr)->sin_addr.s_addr;
				ret = muggle_setsockopt(fd, level, IP_ADD_SOURCE_MEMBERSHIP, (void*)&mreq, sizeof(mreq));
				if (ret != 0)
				{
					char err_msg[1024] = { 0 };
					muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
					MUGGLE_LOG_ERROR("failed setsockopt IP_ADD_SOURCE_MEMBERSHIP - %s", err_msg);
				}
			}
		}
	}
	else if (level == IPPROTO_IPV6)
	{
		if (src_grp == NULL)
		{
			struct ipv6_mreq mreqv6;
			mreqv6.ipv6mr_interface = ((struct sockaddr_in6*)&local_addr)->sin6_scope_id;
			mreqv6.ipv6mr_multiaddr = ((struct sockaddr_in6*)&multicast_addr)->sin6_addr;
			ret = muggle_setsockopt(fd, level, IPV6_ADD_MEMBERSHIP, (void*)&mreqv6, sizeof(mreqv6));
			if (ret != 0)
			{
				char err_msg[1024] = { 0 };
				muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
				MUGGLE_LOG_ERROR("failed setsockopt IPV6_ADD_MEMBERSHIP - %s", err_msg);
			}
		}
		else
		{
			MUGGLE_LOG_ERROR("unimplemented windows ipv6 mcast join source group!");
			ret = -1;
		}
	}
#elif MCAST_JOIN_GROUP
	if (src_grp)
	{
		struct group_source_req req;
		req.gsr_interface = iface_idx;

		struct addrinfo src_addrinfo;
		struct sockaddr_storage src_addr;
		if (muggle_socket_getaddrinfo(src_grp, NULL, &hints, &src_addrinfo, (struct sockaddr*)&src_addr) != 0)
		{
			MUGGLE_LOG_ERROR("failed get source group address info for %s", src_grp);
			return MUGGLE_INVALID_SOCKET;
		}
		else
		{
			memcpy(&req.gsr_group, multicast_addrinfo.ai_addr, multicast_addrinfo.ai_addrlen);
			memcpy(&req.gsr_source, src_addrinfo.ai_addr, src_addrinfo.ai_addrlen);
		}

		ret = muggle_setsockopt(fd, level, MCAST_JOIN_SOURCE_GROUP, (void*)&req, sizeof(req));
		if (ret != 0)
		{
			char err_msg[1024] = { 0 };
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_LOG_ERROR("failed setsockopt MCAST_JOIN_SOURCE_GROUP - %s", err_msg);
		}
	}
	else
	{
		struct group_req req;
		req.gr_interface = iface_idx;

		if (multicast_addrinfo.ai_addrlen > sizeof(req.gr_group))
		{
			MUGGLE_LOG_ERROR("failed mcast join, group size too big");
			return -1;
		}
		memcpy(&req.gr_group, multicast_addrinfo.ai_addr, multicast_addrinfo.ai_addrlen);

		ret = muggle_setsockopt(fd, level, MCAST_JOIN_GROUP, (void*)&req, sizeof(req));
		if (ret != 0)
		{
			char err_msg[1024] = { 0 };
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_LOG_ERROR("failed setsockopt MCAST_JOIN_GROUP - %s", err_msg);
		}
	}
#else
	ret = -1;
#endif

	if (ret != 0)
	{
		muggle_socket_close(fd);
		return MUGGLE_INVALID_SOCKET;
	}

	return fd;
}

int muggle_mcast_leave(
	muggle_socket_t fd,
	const char *host,
	const char *serv,
	const char *iface,
	const char *src_grp)
{
	if (host == NULL)
	{
		MUGGLE_LOG_ERROR("failed muggle_mcast_leave - with null host");
		return -1;
	}

	if (serv == NULL)
	{
		MUGGLE_LOG_ERROR("failed muggle_mcast_leave - with null serv");
		return -1;
	}

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	// get multicast address info
	struct addrinfo multicast_addrinfo;
	struct sockaddr_storage multicast_addr;
	if (muggle_socket_getaddrinfo(host, serv, &hints, &multicast_addrinfo, (struct sockaddr*)&multicast_addr) != 0)
	{
		MUGGLE_LOG_ERROR("failed get multicast address info for %s:%s", host, serv);
		return -1;
	}

	int level = -1;
	switch (multicast_addrinfo.ai_family)
	{
	case AF_INET:
	{
		level = IPPROTO_IP;
	}break;
	case AF_INET6:
	{
		level = IPPROTO_IPV6;
	}break;
	default:
	{
		MUGGLE_LOG_ERROR("unsupported socket family: %d", multicast_addrinfo.ai_family);
		return -1;
	}break;
	}

#if MUGGLE_PLATFORM_WINDOWS
	// get local address
	const char *ipv4_host_any = "0.0.0.0";
	const char *ipv6_host_any = "::";
	if (iface == NULL)
	{
		if (multicast_addrinfo.ai_family == AF_INET)
		{
			iface = ipv4_host_any;
		}
		else if (multicast_addrinfo.ai_family = AF_INET6)
		{
			iface = ipv6_host_any;
		}
	}
	struct addrinfo local_addrinfo;
	struct sockaddr_storage local_addr;
	if (muggle_socket_getaddrinfo(iface, serv, &hints, &local_addrinfo, (struct sockaddr*)&local_addr) != 0)
	{
		MUGGLE_LOG_ERROR("failed get local address info for %s:%s", iface, serv);
		return -1;
	}

	// leave mcast
	if (local_addrinfo.ai_family == AF_INET)
	{
		if (src_grp == NULL)
		{
			struct ip_mreq mreq;
			mreq.imr_interface.s_addr = ((struct sockaddr_in*)&local_addr)->sin_addr.s_addr;
			mreq.imr_multiaddr.s_addr = ((struct sockaddr_in*)&multicast_addr)->sin_addr.s_addr;
			if (muggle_setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void*)&mreq, sizeof(mreq)) != 0)
			{
				char err_msg[1024] = { 0 };
				muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
				MUGGLE_LOG_ERROR("failed setsockopt IP_DROP_MEMBERSHIP - %s", err_msg);
				return -1;
			}
		}
		else
		{
			struct sockaddr_in src_addr;
			if (inet_pton(multicast_addrinfo.ai_family, src_grp, &src_addr.sin_addr) <= 0)
			{
				char err_msg[1024] = { 0 };
				muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
				MUGGLE_LOG_ERROR("failed inet_pton for %s - %s", src_grp, err_msg);
				return -1;
			}
			else
			{
				struct ip_mreq_source mreq;
				mreq.imr_interface.s_addr = ((struct sockaddr_in*)&local_addr)->sin_addr.s_addr;
				mreq.imr_sourceaddr.s_addr = src_addr.sin_addr.s_addr;
				mreq.imr_multiaddr.s_addr = ((struct sockaddr_in*)&multicast_addr)->sin_addr.s_addr;
				if (muggle_setsockopt(fd, IPPROTO_IP, IP_DROP_SOURCE_MEMBERSHIP, (void*)&mreq, sizeof(mreq)) != 0)
				{
					char err_msg[1024] = { 0 };
					muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
					MUGGLE_LOG_ERROR("failed setsockopt IP_DROP_SOURCE_MEMBERSHIP - %s", err_msg);
					return -1;
				}
			}
		}
	}
	else if (local_addrinfo.ai_family == AF_INET6)
	{
		if (src_grp == NULL)
		{
			struct ipv6_mreq mreqv6;
			mreqv6.ipv6mr_interface = ((struct sockaddr_in6*)&local_addr)->sin6_scope_id;
			mreqv6.ipv6mr_multiaddr = ((struct sockaddr_in6*)&multicast_addr)->sin6_addr;
			if (muggle_setsockopt(fd, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP, (void*)&mreqv6, sizeof(mreqv6)) != 0)
			{
				char err_msg[1024] = { 0 };
				muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
				MUGGLE_LOG_ERROR("failed setsockopt IPV6_DROP_MEMBERSHIP - %s", err_msg);
			}
		}
		else
		{
			MUGGLE_LOG_ERROR("unimplemented windows ipv6 mcast leave source group!");
			return -1;
		}
	}

#elif MCAST_JOIN_GROUP
	unsigned int iface_idx = 0;
	if (iface != NULL)
	{
		iface_idx = if_nametoindex(iface);
		if (iface_idx == 0)
		{
			char err_msg[1024] = { 0 };
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_LOG_WARNING("failed convert net interface(%s) to index, let kernel select - %s", iface, err_msg);
		}
	}

	if (src_grp)
	{
		struct group_source_req req;
		req.gsr_interface = iface_idx;

		struct addrinfo src_addrinfo;
		struct sockaddr_storage src_addr;
		if (muggle_socket_getaddrinfo(src_grp, NULL, &hints, &src_addrinfo, (struct sockaddr*)&src_addr) != 0)
		{
			MUGGLE_LOG_ERROR("failed get source group address info for %s", src_grp);
			return -1;
		}
		else
		{
			memcpy(&req.gsr_group, multicast_addrinfo.ai_addr, multicast_addrinfo.ai_addrlen);
			memcpy(&req.gsr_source, src_addrinfo.ai_addr, src_addrinfo.ai_addrlen);
		}

		if (muggle_setsockopt(fd, level, MCAST_LEAVE_SOURCE_GROUP, (void*)&req, sizeof(req)) != 0)
		{
			char err_msg[1024] = { 0 };
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_LOG_ERROR("failed setsockopt MCAST_LEAVE_SOURCE_GROUP - %s", err_msg);
			return -1;
		}
	}
	else
	{
		struct group_req req;
		req.gr_interface = iface_idx;

		if (multicast_addrinfo.ai_addrlen > sizeof(req.gr_group))
		{
			MUGGLE_LOG_ERROR("failed mcast join, group size too big");
			return -1;
		}
		memcpy(&req.gr_group, multicast_addrinfo.ai_addr, multicast_addrinfo.ai_addrlen);

		if (muggle_setsockopt(fd, level, MCAST_LEAVE_GROUP, (void*)&req, sizeof(req)) != 0)
		{
			char err_msg[1024] = { 0 };
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_LOG_ERROR("failed setsockopt MCAST_LEAVE_GROUP - %s", err_msg);
			return -1;
		}
	}
#else
	return -1;
#endif

	return 0;
}

#if MUGGLE_PLATFORM_WINDOWS

typedef struct {
	muggle_socket_t *fd;
	muggle_socket_t listen_fd;
} muggle_socketpair_listen_th_args_t;

static muggle_thread_ret_t muggle_socketpair_listen_routine(void *p_args)
{
	muggle_socketpair_listen_th_args_t *listen_args =
		(muggle_socketpair_listen_th_args_t*)p_args;

	muggle_socket_t fd = accept(listen_args->listen_fd, NULL, NULL);
	if (fd == MUGGLE_INVALID_SOCKET) {
		MUGGLE_LOG_ERROR("socketpair failed accept");
		goto socketpair_listen_th_exit;
	}
	*listen_args->fd = fd;

socketpair_listen_th_exit:
	muggle_socket_close(listen_args->listen_fd);
	return 0;
}

#endif

int muggle_socketpair(
	int domain,
	int socket_type,
	int protocol,
	muggle_socket_t fds[2])
{
	fds[0] = MUGGLE_INVALID_SOCKET;
	fds[1] = MUGGLE_INVALID_SOCKET;

#if MUGGLE_PLATFORM_WINDOWS
	muggle_socket_t listen_fd = muggle_tcp_listen("127.0.0.1", 0, 1);
	if (listen_fd == MUGGLE_INVALID_SOCKET) {
		char errmsg[256];
		int errid = muggle_socket_lasterror();
		muggle_socket_strerror(errid, errmsg, sizeof(errmsg));
		MUGGLE_LOG_ERROR("failed socketpair listen, errno: %d, errmsg: %s",
			errid, errmsg);
		return -1;
	}

	char host[32];
	int port;
	if (muggle_socket_local_ip_port(
			listen_fd, host, sizeof(host), &port) != 0) {
		muggle_socket_close(listen_fd);
		MUGGLE_LOG_ERROR("failed socketpair get listen port");
		return -1;
	}

	muggle_socketpair_listen_th_args_t listen_args;
	listen_args.listen_fd = listen_fd;
	listen_args.fd = &fds[0];
	muggle_thread_t th_listen;
	muggle_thread_create(
		&th_listen, muggle_socketpair_listen_routine, &listen_args);

	char str_port[16];
	snprintf(str_port, sizeof(str_port), "%d", port);

	fds[1] = muggle_tcp_connect(host, str_port, 3);
	muggle_thread_join(&th_listen);

	if (fds[0] == MUGGLE_INVALID_SOCKET || fds[1] == MUGGLE_INVALID_SOCKET) {
		if (fds[0] != MUGGLE_INVALID_SOCKET) {
			muggle_socket_close(fds[0]);
			fds[0] = MUGGLE_INVALID_SOCKET;
		}
		if (fds[1] != MUGGLE_INVALID_SOCKET) {
			muggle_socket_close(fds[1]);
			fds[1] = MUGGLE_INVALID_SOCKET;
		}
		return -1;
	}

	return 0;
#else
	return socketpair(domain, socket_type, protocol, fds);
#endif
}
