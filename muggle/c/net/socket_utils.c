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

int muggle_socket_getaddrinfo(const char *host, const char *serv, struct addrinfo *hints, struct addrinfo *addrinfo, struct sockaddr *addr)
{
	struct addrinfo *res;
	int n;
	if ((n = getaddrinfo(host, serv, hints, &res)) != 0)
	{
#if MUGGLE_PLATFORM_WINDOWS
		char err_msg[1024] = { 0 };
		muggle_socket_strerror(WSAGetLastError(), err_msg, sizeof(err_msg));
		MUGGLE_ERROR("failed muggle_socket_getaddrinfo for %s:%s - getaddrinfo return '%s'",
			host, serv, err_msg);
#else
		MUGGLE_ERROR("failed muggle_socket_getaddrinfo for %s:%s - getaddrinfo return '%s'",
			host, serv, gai_strerror(n));
#endif
		return -1;
	}

	int ret = -1;
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

muggle_socket_t muggle_tcp_listen(const char *host, const char *serv, int backlog, muggle_socket_peer_t *peer)
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

		// always set SO_REUSEADDR for bind socket
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

	// set peer
	if (peer)
	{
		peer->fd = listen_socket;
		peer->peer_type = MUGGLE_SOCKET_PEER_TYPE_TCP_LISTEN;
		memcpy(&peer->addr, res->ai_addr, res->ai_addrlen);
		peer->addr_len = (muggle_socklen_t)res->ai_addrlen;
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
		memcpy(&peer->addr, res->ai_addr, res->ai_addrlen);
		peer->addr_len = (muggle_socklen_t)res->ai_addrlen;
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

muggle_socket_t muggle_udp_bind(const char *host, const char *serv, muggle_socket_peer_t *peer)
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
		MUGGLE_ERROR("failed udp bind for %s:%s - getaddrinfo return '%s'",
				host, serv, err_msg);
#else
		MUGGLE_ERROR("failed udp bind for %s:%s - getaddrinfo return '%s'",
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
		if (setsockopt(udp_socket, SOL_SOCKET, SO_REUSEADDR, (void*)&on, sizeof(on)) != 0)
		{
			char err_msg[1024] = {0};
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_WARNING("failed setsockopt SO_REUSEADDR on - %s", err_msg);
		}

		if (bind(udp_socket, res->ai_addr, (muggle_socklen_t)res->ai_addrlen) == 0)
		{
			break;
		}
		else
		{
			char err_msg[1024] = {0};
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_WARNING("failed bind %s:%s - %s", host, serv, err_msg);
		}

		muggle_socket_close(udp_socket);
		udp_socket = MUGGLE_INVALID_SOCKET;
	}

	if (res == NULL)
	{
		MUGGLE_ERROR("failed to create and bind udp socket for %s:%s", host, serv);
		freeaddrinfo(ressave);
		return MUGGLE_INVALID_SOCKET;
	}

	// set peer
	if (peer)
	{
		peer->fd = udp_socket;
		peer->peer_type = MUGGLE_SOCKET_PEER_TYPE_UDP_PEER;
		memcpy(&peer->addr, res->ai_addr, res->ai_addrlen);
		peer->addr_len = (muggle_socklen_t)res->ai_addrlen;
	}

	freeaddrinfo(ressave);

	return udp_socket;
}

muggle_socket_t muggle_udp_connect(const char *host, const char *serv, muggle_socket_peer_t *peer)
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
		MUGGLE_ERROR("failed udp connect for %s:%s - getaddrinfo return '%s'",
				host, serv, err_msg);
#else
		MUGGLE_ERROR("failed udp connect for %s:%s - getaddrinfo return '%s'",
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
		MUGGLE_ERROR("failed to create and connect udp socket for %s:%s", host, serv);
		freeaddrinfo(ressave);
		return MUGGLE_INVALID_SOCKET;
	}

	// set peer
	if (peer)
	{
		peer->fd = udp_socket;
		peer->peer_type = MUGGLE_SOCKET_PEER_TYPE_UDP_PEER;
		memcpy(&peer->addr, res->ai_addr, res->ai_addrlen);
		peer->addr_len = (muggle_socklen_t)res->ai_addrlen;
	}

	freeaddrinfo(ressave);

	return udp_socket;
}

#if MUGGLE_PLATFORM_WINDOWS

muggle_socket_t muggle_mcast_join(
	const char *host,
	const char *serv,
	const char *iface,
	const char *src_grp,
	muggle_socket_peer_t *peer)
{
	muggle_socket_t fd = MUGGLE_INVALID_SOCKET;

	if (host == NULL)
	{
		MUGGLE_ERROR("failed muggle_mcast_join - with null host");
		return MUGGLE_INVALID_SOCKET;
	}

	if (serv == NULL)
	{
		MUGGLE_ERROR("failed muggle_mcast_join - with null serv");
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
		MUGGLE_ERROR("failed get multicast address info for %s:%s", host, serv);
		return MUGGLE_INVALID_SOCKET;
	}

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
		MUGGLE_ERROR("failed get local address info for %s:%s", iface, serv);
		return MUGGLE_INVALID_SOCKET;
	}

	// create and bind local socket
	fd = muggle_socket_create(local_addrinfo.ai_family, local_addrinfo.ai_socktype, local_addrinfo.ai_protocol);
	if (fd == MUGGLE_INVALID_SOCKET)
	{
		char err_msg[1024] = { 0 };
		muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
		MUGGLE_ERROR("failed muggle_socket_create info for %s:%s - %s", iface, serv, err_msg);
		return MUGGLE_INVALID_SOCKET;
	}

	// always set SO_REUSEADDR for bind socket
	int on = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*)&on, sizeof(on)) != 0)
	{
		char err_msg[1024] = { 0 };
		muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
		MUGGLE_WARNING("failed setsockopt SO_REUSEADDR on - %s", err_msg);
	}

	if (bind(fd, local_addrinfo.ai_addr, (muggle_socklen_t)local_addrinfo.ai_addrlen) != 0)
	{
		char err_msg[1024] = { 0 };
		muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
		MUGGLE_WARNING("failed bind %s:%s - %s", iface, serv, err_msg);

		muggle_socket_close(fd);
		return MUGGLE_INVALID_SOCKET;
	}

	// join mcast
	int ret = 0;
	if (local_addrinfo.ai_family == AF_INET)
	{
		if (src_grp == NULL)
		{
			struct ip_mreq mreq;
			mreq.imr_interface.s_addr = ((struct sockaddr_in*)&local_addr)->sin_addr.s_addr;
			mreq.imr_multiaddr.s_addr = ((struct sockaddr_in*)&multicast_addr)->sin_addr.s_addr;
			ret = setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq));
			if (ret != 0)
			{
				char err_msg[1024] = { 0 };
				muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
				MUGGLE_ERROR("failed setsockopt IP_ADD_MEMBERSHIP - %s", err_msg);
			}
		}
		else
		{
			struct sockaddr_in src_addr;
			if (inet_pton(multicast_addrinfo.ai_family, src_grp, &src_addr.sin_addr) <= 0)
			{
				char err_msg[1024] = { 0 };
				muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
				MUGGLE_ERROR("failed inet_pton for %s - %s", src_grp, err_msg);
				ret = -1;
			}
			else
			{
				struct ip_mreq_source mreq;
				mreq.imr_interface.s_addr = ((struct sockaddr_in*)&local_addr)->sin_addr.s_addr;
				mreq.imr_sourceaddr.s_addr = src_addr.sin_addr.s_addr;
				mreq.imr_multiaddr.s_addr = ((struct sockaddr_in*)&multicast_addr)->sin_addr.s_addr;
				ret = setsockopt(fd, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, (char *)&mreq, sizeof(mreq));
				if (ret != 0)
				{
					char err_msg[1024] = { 0 };
					muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
					MUGGLE_ERROR("failed setsockopt IP_ADD_SOURCE_MEMBERSHIP - %s", err_msg);
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
			ret = setsockopt(fd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (char *)&mreqv6, sizeof(mreqv6));
			if (ret != 0)
			{
				char err_msg[1024] = { 0 };
				muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
				MUGGLE_ERROR("failed setsockopt IPV6_ADD_MEMBERSHIP - %s", err_msg);
			}
		}
		else
		{
			MUGGLE_ERROR("unimplemented windows ipv6 mcast join source group!");
			ret = -1;
		}
	}

	if (ret != 0)
	{
		muggle_socket_close(fd);
		return MUGGLE_INVALID_SOCKET;
	}

	// set peer
	if (peer)
	{
		peer->fd = fd;
		peer->peer_type = MUGGLE_SOCKET_PEER_TYPE_UDP_PEER;
		memcpy(&peer->addr, local_addrinfo.ai_addr, local_addrinfo.ai_addrlen);
		peer->addr_len = (muggle_socklen_t)local_addrinfo.ai_addrlen;
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
		MUGGLE_ERROR("failed muggle_mcast_leave - with null host");
		return -1;
	}

	if (serv == NULL)
	{
		MUGGLE_ERROR("failed muggle_mcast_leave - with null serv");
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
		MUGGLE_ERROR("failed get multicast address info for %s:%s", host, serv);
		return -1;
	}

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
		MUGGLE_ERROR("failed get local address info for %s:%s", iface, serv);
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
			if (setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) != 0)
			{
				char err_msg[1024] = { 0 };
				muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
				MUGGLE_ERROR("failed setsockopt IP_DROP_MEMBERSHIP - %s", err_msg);
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
				MUGGLE_ERROR("failed inet_pton for %s - %s", src_grp, err_msg);
				return -1;
			}
			else
			{
				struct ip_mreq_source mreq;
				mreq.imr_interface.s_addr = ((struct sockaddr_in*)&local_addr)->sin_addr.s_addr;
				mreq.imr_sourceaddr.s_addr = src_addr.sin_addr.s_addr;
				mreq.imr_multiaddr.s_addr = ((struct sockaddr_in*)&multicast_addr)->sin_addr.s_addr;
				if (setsockopt(fd, IPPROTO_IP, IP_DROP_SOURCE_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) != 0)
				{
					char err_msg[1024] = { 0 };
					muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
					MUGGLE_ERROR("failed setsockopt IP_DROP_SOURCE_MEMBERSHIP - %s", err_msg);
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
			if (setsockopt(fd, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP, (char *)&mreqv6, sizeof(mreqv6)) != 0)
			{
				char err_msg[1024] = { 0 };
				muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
				MUGGLE_ERROR("failed setsockopt IPV6_DROP_MEMBERSHIP - %s", err_msg);
			}
		}
		else
		{
			MUGGLE_ERROR("unimplemented windows ipv6 mcast leave source group!");
			return -1;
		}
	}

	return 0;
}

#else

static int muggle_mcast_join_all(
	muggle_socket_t fd,
	const struct sockaddr *grp,
	muggle_socklen_t grplen,
	const char *iface)
{
#ifdef MCAST_JOIN_GROUP
	struct group_req req;
	if (iface == NULL)
	{
		req.gr_interface = 0;
	}
	else
	{
		req.gr_interface = if_nametoindex(iface);
		if (req.gr_interface == 0)
		{
			char err_msg[1024] = {0};
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_WARNING("failed convert net interface(%s) to index, let kernel select - %s", iface, err_msg);
		}
	}

	if (grplen > sizeof(req.gr_group))
	{
		MUGGLE_ERROR("failed mcast join, group size too big");
		return -1;
	}
	memcpy(&req.gr_group, grp, grplen);

	int level = -1;
	switch (grp->sa_family)
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
			MUGGLE_ERROR("unsupported socket family: %d", grp->sa_family);
			return -1;
		}break;
	}

	return setsockopt(fd, level, MCAST_JOIN_GROUP, &req, sizeof(req));
#else
	switch (grp->sa_family)
	{
	case AF_INET:
		{
            struct ip_mreq mreq;
            memcpy(&mreq.imr_multiaddr, &(((const struct sockaddr_in*)grp)->sin_addr), sizeof(struct in_addr));

            if (iface != NULL)
            {
                struct ifreq ifreq;
                memset(&ifreq, 0, sizeof(ifreq));
                strncpy(ifreq.ifr_name, iface, IFNAMSIZ - 1);
                if (ioctl(fd, SIOCGIFADDR, &ifreq) < 0)
                {
                    MUGGLE_ERROR("failed ioctl(SIOCGIFADDR)");
                    return -1;
                }
                memcpy(&mreq.imr_interface, &(((struct sockaddr_in*)&ifreq.ifr_addr)->sin_addr), sizeof(struct in_addr));
            }
            else
            {
                mreq.imr_interface.s_addr = htonl(INADDR_ANY);
            }

            return setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
		}break;
	case AF_INET6:
		{
#ifndef IPV6_JOIN_GROUP
#define IPV6_JOIN_GROUP IPV6_ADD_MEMBERSHIP
#endif
			struct ipv6_mreq mreq6;
			memcpy(&mreq6.ipv6mr_multiaddr, &(((const struct sockaddr_in6*)grp)->sin6_addr), sizeof(struct in6_addr));

			if (iface == NULL)
			{
				mreq6.ipv6mr_interface = 0;
			}
			else
			{
				mreq6.ipv6mr_interface = if_nametoindex(iface);
				if (mreq6.ipv6mr_interface == 0)
				{
					char err_msg[1024] = {0};
					muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
					MUGGLE_WARNING("failed convert net interface(%s) to index, let kernel select - %s", iface, err_msg);
				}
			}

			return setsockopt(fd, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq6, sizeof(mreq6));
		}break;
	default:
		{
			MUGGLE_ERROR("unsupported socket family: %d", grp->sa_family);
			return -1;
		}break;
	}
#endif

	return -1;
}

static int muggle_mcast_join_souce_group(
	muggle_socket_t fd,
	const struct sockaddr *grp,
	muggle_socklen_t grplen,
	const char *iface,
	const char *src_grp)
{
	// get source group address
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	int n;
	if ((n = getaddrinfo(src_grp, NULL, &hints, &res)) != 0)
	{
#if MUGGLE_PLATFORM_WINDOWS
		char err_msg[1024] = {0};
		muggle_socket_strerror(WSAGetLastError(), err_msg, sizeof(err_msg));
		MUGGLE_ERROR("failed mcast join source group for %s - getaddrinfo return '%s'",
			src_grp, err_msg);
#else
		MUGGLE_ERROR("failed mcast join source group for %s - getaddrinfo return '%s'",
			src_grp, gai_strerror(n));
#endif
		return -1;
	}

	struct sockaddr_storage srcaddr;
	muggle_socklen_t srcaddr_len;
	if (res == NULL)
	{
		return -1;
	}
	else
	{
		memcpy(&srcaddr, res->ai_addr, res->ai_addrlen);
		srcaddr_len = (muggle_socklen_t)res->ai_addrlen;
	}
	freeaddrinfo(res);

	// join multicast souce group
#ifdef MCAST_JOIN_GROUP
    struct group_source_req req;

	if (iface == NULL)
	{
        req.gsr_interface = 0; // kernel select interface
	}
	else
	{
        req.gsr_interface = if_nametoindex(iface);
        if (req.gsr_interface == 0)
		{
			char err_msg[1024] = {0};
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_WARNING("failed convert net interface(%s) to index, let kernel select - %s", iface, err_msg);
		}
	}

    if (grplen > sizeof(req.gsr_group) || srcaddr_len > sizeof(req.gsr_source))
    {
        MUGGLE_ERROR("grplen or srclen too big");
        return -1;
    }

    memcpy(&req.gsr_group, grp, grplen);
    memcpy(&req.gsr_source, &srcaddr, srcaddr_len);

    int level = -1;
    switch (grp->sa_family)
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
			MUGGLE_ERROR("unsupported socket family: %d", grp->sa_family);
			return -1;
        }break;
    }

    return setsockopt(fd, level, MCAST_JOIN_SOURCE_GROUP, &req, sizeof(req));
#endif

	return -1;
}


muggle_socket_t muggle_mcast_join(
	const char *host,
	const char *serv,
	const char *iface,
	const char *src_grp,
	muggle_socket_peer_t *peer)
{
	muggle_socket_t fd = MUGGLE_INVALID_SOCKET;

	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	int n;
	if ((n = getaddrinfo(host, serv, &hints, &res)) != 0)
	{
#if MUGGLE_PLATFORM_WINDOWS
		char err_msg[1024] = {0};
		muggle_socket_strerror(WSAGetLastError(), err_msg, sizeof(err_msg));
		MUGGLE_ERROR("failed mcast join for %s:%s - getaddrinfo return '%s'",
				host, serv, err_msg);
#else
		MUGGLE_ERROR("failed mcast join for %s:%s - getaddrinfo return '%s'",
				host, serv, gai_strerror(n));
#endif
		return MUGGLE_INVALID_SOCKET;
	}

	struct addrinfo *ressave = res;
	for (; res != NULL; res = res->ai_next)
	{
		fd = muggle_socket_create(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (fd == MUGGLE_INVALID_SOCKET)
		{
			continue;
		}

		// always set SO_REUSEADDR for bind socket
		int on = 1;
		if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*)&on, sizeof(on)) != 0)
		{
			char err_msg[1024] = {0};
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_WARNING("failed setsockopt SO_REUSEADDR on - %s", err_msg);
		}

		if (bind(fd, res->ai_addr, (muggle_socklen_t)res->ai_addrlen) != 0)
		{
			char err_msg[1024] = {0};
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_WARNING("failed bind %s:%s - %s", host, serv, err_msg);

			muggle_socket_close(fd);
			fd = MUGGLE_INVALID_SOCKET;
			continue;
		}

		int ret = 0;
		if (src_grp == NULL)
		{
			ret = muggle_mcast_join_all(fd, res->ai_addr, (muggle_socklen_t)res->ai_addrlen, iface);
		}
		else
		{
			ret = muggle_mcast_join_souce_group(fd, res->ai_addr, (muggle_socklen_t)res->ai_addrlen, iface, src_grp);
		}
		if (ret == 0)
		{
			break;
		}

		muggle_socket_close(fd);
		fd = MUGGLE_INVALID_SOCKET;
	}

	if (res == NULL)
	{
		MUGGLE_ERROR("failed to create and bind udp socket for %s:%s", host, serv);
		freeaddrinfo(ressave);
		return MUGGLE_INVALID_SOCKET;
	}

	// set peer
	if (peer)
	{
		peer->fd = fd;
		peer->peer_type = MUGGLE_SOCKET_PEER_TYPE_UDP_PEER;
		memcpy(&peer->addr, res->ai_addr, res->ai_addrlen);
		peer->addr_len = (muggle_socklen_t)res->ai_addrlen;
	}

	freeaddrinfo(ressave);

	return fd;
}

#endif