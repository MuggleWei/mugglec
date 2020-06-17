#include "socket_peer.h"
#include "muggle/c/log/log.h"

int muggle_socket_peer_retain(muggle_socket_peer_t *peer)
{
	muggle_atomic_int ref_cnt = 0, desired = 0;
	do {
		MUGGLE_ASSERT(peer->ref_cnt >= 0);

		ref_cnt = peer->ref_cnt;
		if (ref_cnt == 0)
		{
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

		ref_cnt = peer->ref_cnt;
		if (ref_cnt == 0)
		{
			return ref_cnt;
		}
		desired = ref_cnt - 1;
	} while (!muggle_atomic_cmp_exch_weak(&peer->ref_cnt, &ref_cnt, desired, muggle_memory_order_relaxed));

	if (desired == 0)
	{
		muggle_socket_close(peer->fd);
	}

	return desired;
}

int muggle_socket_peer_close(muggle_socket_peer_t *peer)
{
	return muggle_socket_peer_release(peer);
}