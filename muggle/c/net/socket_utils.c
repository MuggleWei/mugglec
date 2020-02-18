#include "socket_utils.h"
#include <string.h>
#include "muggle/c/log/log.h"

const char* muggle_socket_ntop(const struct sockaddr *sa, void *buf, size_t bufsize, int host_only)
{
	switch (sa->sa_family)
	{
	case AF_INET:
		{
			const struct sockaddr_in *sin = (struct sockaddr_in*)sa;
			if (inet_ntop(AF_INET, &sin->sin_addr, buf, (socklen_t)bufsize) == NULL)
			{
				char err_msg[1024] = {0};
				muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
				MUGGLE_ERROR("failed inet_ntop - %s", err_msg);
				return NULL;
			}

			if (!host_only)
			{
				size_t offset = strlen(buf);
				if (bufsize - offset < 8)
				{
					MUGGLE_ERROR("buffer size(%d) is not enough for socket address presentation string", (int)bufsize);
					return NULL;
				}
				snprintf((char*)buf + offset, bufsize - offset, ":%d", (int)ntohs(sin->sin_port));
			}

			return (const char*)buf;
		}break;
	case AF_INET6:
		{
			const struct sockaddr_in6 *sin6 = (struct sockaddr_in6*)sa;
			if (inet_ntop(AF_INET6, &sin6->sin6_addr, buf, (socklen_t)bufsize) == NULL)
			{
				char err_msg[1024] = {0};
				muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
				MUGGLE_ERROR("failed inet_ntop - %s", err_msg);
				return NULL;
			}

			if (!host_only)
			{
				size_t offset = strlen(buf);
				if (bufsize - offset < 8)
				{
					MUGGLE_ERROR("buffer size(%d) is not enough for socket address presentation string", (int)bufsize);
					return NULL;
				}
				snprintf((char*)buf + offset, bufsize - offset, ":%d", (int)ntohs(sin6->sin6_port));
			}

			return (const char*)buf;
		}break;
	default:
		{
			MUGGLE_ERROR("invalid AF_* family(%d) for socket_ntop", sa->sa_family);
		}
	}

	return NULL;

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
		MUGGLE_ERROR("failed tcp listen for %s:%s - getaddrinfo return '%s'",
				host, serv, err_msg);
#else
		MUGGLE_ERROR("failed tcp listen for %s:%s - getaddrinfo return '%s'",
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

		// always set SO_REUSEADDR for listen socket
		int on = 1;
		if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, (void*)&on, sizeof(on)) != 0)
		{
			char err_msg[1024] = {0};
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_WARNING("failed setsockopt SO_REUSEADDR on - %s", err_msg);
		}

		if (bind(listen_socket, res->ai_addr, (muggle_socklen_t)res->ai_addrlen) == 0)
		{
			break;
		}
		else
		{
			char err_msg[1024] = {0};
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_WARNING("failed bind %s:%s - %s", host, serv, err_msg);
		}

		muggle_socket_close(listen_socket);
		listen_socket = MUGGLE_INVALID_SOCKET;
	}

	if (res == NULL)
	{
		MUGGLE_ERROR("failed to create and bind tcp socket for %s:%s", host, serv);
		freeaddrinfo(ressave);
		return MUGGLE_INVALID_SOCKET;
	}

	if (listen(listen_socket, backlog) != 0)
	{
		char err_msg[1024] = {0};
		muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
		MUGGLE_ERROR("failed listen for %s:%s - %s", host, serv, err_msg);

		muggle_socket_close(listen_socket);
		freeaddrinfo(ressave);
		return MUGGLE_INVALID_SOCKET;
	}

	freeaddrinfo(ressave);

	return listen_socket;

}

muggle_socket_t muggle_tcp_connect(const char *host, const char *serv, int timeout_sec, muggle_socket_peer_t *peer)
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
        MUGGLE_ERROR("failed nonblocking tcp connect for %s:%s - getaddrinfo return '%s'",
            host, serv, err_msg);
#else
        MUGGLE_ERROR("failed nonblocking tcp connect for %s:%s - getaddrinfo return '%s'",
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
        MUGGLE_ERROR("failed create socket for %s:%s", host, serv);
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
            char err_msg[1024] = {0};
            muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
            MUGGLE_ERROR("failed connect for %s:%s - %s", host, serv, err_msg);

            muggle_socket_close(client);
            freeaddrinfo(ressave);
            return MUGGLE_INVALID_SOCKET;
        }
    }

	// set peer
	if (peer)
	{
		peer->fd = client;
		peer->peer_type = MUGGLE_SOCKET_PEER_TYPE_TCP_PEER;
		memcpy(&peer->addr, res->ai_addr, sizeof(res->ai_addrlen));
		peer->addr_len = res->ai_addrlen;
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
        MUGGLE_ERROR("failed nonblocking tcp connect for %s:%s - timeout(%d sec)", host, serv, timeout_sec);
        return MUGGLE_INVALID_SOCKET;
    }
    else if (n == MUGGLE_SOCKET_ERROR)
    {
        char err_msg[1024] = {0};
        muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
        MUGGLE_ERROR("failed nonblocking tcp connect for %s:%s - %s", host, serv, err_msg);

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
            MUGGLE_ERROR("failed nonblocking tcp connect for %s:%s - %s", host, serv, err_msg);

            muggle_socket_close(client);
            return MUGGLE_INVALID_SOCKET;
        }
    }
    else
    {
        MUGGLE_ERROR("failed nonblocking tcp connect for %s:%s - socket not set", host, serv);
        muggle_socket_close(client);
        return MUGGLE_INVALID_SOCKET;
    }

    if (error != 0)
    {
        char err_msg[1024] = {0};
        muggle_socket_strerror(error, err_msg, sizeof(err_msg));
        MUGGLE_ERROR("failed nonblocking tcp connect for %s:%s - %s", host, serv, err_msg);

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
