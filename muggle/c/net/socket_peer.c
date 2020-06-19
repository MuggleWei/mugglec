/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "socket_peer.h"
#include <string.h>
#include "muggle/c/log/log.h"
#include "socket_utils.h"

void muggle_socket_peer_init(
	muggle_socket_peer_t *peer, muggle_socket_t fd,
	int peer_type, const struct sockaddr *addr, muggle_socklen_t addr_len)
{
	memset(peer, 0, sizeof(muggle_socket_peer_t));

	peer->ref_cnt = 1;
	peer->fd = fd;
	peer->peer_type = peer_type;
	peer->status = MUGGLE_SOCKET_PEER_STATUS_ACTIVE;
	if (addr)
	{
		memcpy(&peer->addr, addr, addr_len);
		peer->addr_len = addr_len;
	}
	peer->data = NULL;
}

int muggle_socket_peer_retain(muggle_socket_peer_t *peer)
{
	muggle_atomic_int ref_cnt = 0, desired = 0;
	do {
		MUGGLE_ASSERT(peer->ref_cnt >= 0);

		ref_cnt = peer->ref_cnt;
		if (ref_cnt == 0)
		{
			// try to retain released peer
			MUGGLE_ASSERT(ref_cnt != 0);
			return ref_cnt;
		}
		desired = ref_cnt + 1;
	} while (!muggle_atomic_cmp_exch_weak(&peer->ref_cnt, &ref_cnt, desired, muggle_memory_order_relaxed));

	return desired;
}

int muggle_socket_peer_release(muggle_socket_peer_t *peer)
{
	muggle_atomic_int ref_cnt = 0, desired = 0;
	do {
		MUGGLE_ASSERT(peer->ref_cnt >= 0);

		// consider single thread optimize, the last holder of this socket peer don't need to use CAS
		if (peer->ref_cnt == 1)
		{
			peer->ref_cnt = 0;
			break;
		}

		ref_cnt = peer->ref_cnt;
		if (ref_cnt == 0)
		{
			// repeated release error
			MUGGLE_ASSERT(ref_cnt != 0);
			return ref_cnt;
		}
		desired = ref_cnt - 1;
	} while (!muggle_atomic_cmp_exch_weak(&peer->ref_cnt, &ref_cnt, desired, muggle_memory_order_relaxed));

	if (desired == 0)
	{
#if MUGGLE_ENABLE_TRACE
		char straddr[MUGGLE_SOCKET_ADDR_STRLEN];
		if (peer->addr_len == 0 ||
			muggle_socket_ntop((struct sockaddr*)&peer->addr, straddr, MUGGLE_SOCKET_ADDR_STRLEN, 0) == NULL)
		{
			snprintf(straddr, MUGGLE_SOCKET_ADDR_STRLEN, "?:?");
		}
#if MUGGLE_PLATFORM_WINDOWS
		MUGGLE_DEBUG_INFO("close socket [%s]", straddr);
#else
		MUGGLE_DEBUG_INFO("close socket %d[%s]", peer->fd, straddr);
#endif
#endif
		muggle_socket_close(peer->fd);
		peer->fd = 0;
	}

	return desired;
}

void muggle_socket_peer_close(muggle_socket_peer_t *peer)
{
	peer->status = MUGGLE_SOCKET_PEER_STATUS_CLOSED;
	muggle_socket_shutdown(peer->fd, MUGGLE_SOCKET_SHUT_RDWR);
}

int muggle_socket_peer_recvfrom(
	muggle_socket_peer_t *peer, void *buf, size_t len, int flags,
	struct sockaddr *addr, muggle_socklen_t *addrlen)
{
	int n = 0;
	while (1)
	{
#if MUGGLE_PLATFORM_WINDOWS
		n = recvfrom(peer->fd, buf, (int)len, flags, addr, addrlen);
#else
		n = recvfrom(peer->fd, buf, len, flags, addr, addrlen);
#endif
		if (n > 0)
		{
			break;
		}
		else
		{
			if (n < 0)
			{
				if (MUGGLE_SOCKET_LAST_ERRNO == MUGGLE_SYS_ERRNO_INTR)
				{
					continue;
				}
				else if (MUGGLE_SOCKET_LAST_ERRNO == MUGGLE_SYS_ERRNO_WOULDBLOCK)
				{
					break;
				}
			}

			muggle_socket_peer_close(peer);
			break;
		}
	}

	return n;
}

int muggle_socket_peer_recv(muggle_socket_peer_t *peer, void *buf, size_t len, int flags)
{
	int n = 0;
	while (1)
	{
#if MUGGLE_PLATFORM_WINDOWS
		n = recv(peer->fd, buf, (int)len, flags);
#else
		n = recv(peer->fd, buf, len, flags);
#endif
		if (n > 0)
		{
			break;
		}
		else
		{
			if (n < 0)
			{
				if (MUGGLE_SOCKET_LAST_ERRNO == MUGGLE_SYS_ERRNO_INTR)
				{
					continue;
				}
				else if (MUGGLE_SOCKET_LAST_ERRNO == MUGGLE_SYS_ERRNO_WOULDBLOCK)
				{
					break;
				}
			}

			muggle_socket_peer_close(peer);
			break;
		}
	}

	return n;
}

int muggle_socket_peer_sendto(muggle_socket_peer_t *peer, const void *buf, size_t len, int flags,
	const struct sockaddr *dest_addr, socklen_t addrlen)
{
#if MUGGLE_PLATFORM_WINDOWS
	int num_bytes = sendto(peer->fd, buf, (int)len, flags, dest_addr, addrlen);
#else
	int num_bytes = sendto(peer->fd, buf, len, flags, dest_addr, addrlen);
#endif
	if (num_bytes != len)
	{
		if (num_bytes == MUGGLE_SOCKET_ERROR)
		{
			char err_msg[1024] = { 0 };
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_LOG_TRACE("failed send msg - %s", err_msg);
		}
		else
		{
			MUGGLE_LOG_TRACE("send buffer full");
		}

		muggle_socket_peer_close(peer);
	}
	return num_bytes;
}

int muggle_socket_peer_send(muggle_socket_peer_t *peer, const void *buf, size_t len, int flags)
{
#if MUGGLE_PLATFORM_WINDOWS
	int num_bytes = send(peer->fd, buf, (int)len, flags);
#else
	int num_bytes = send(peer->fd, buf, len, flags);
#endif
	if (num_bytes != len)
	{
		if (num_bytes == MUGGLE_SOCKET_ERROR)
		{
			char err_msg[1024] = { 0 };
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_LOG_TRACE("failed send msg - %s", err_msg);
		}
		else
		{
			MUGGLE_LOG_TRACE("send buffer full");
		}

		muggle_socket_peer_close(peer);
	}
	return num_bytes;
}
