#include "socket_event_select.h"
#include <string.h>
#include "muggle/c/log/log.h"
#include "muggle/c/memory/memory_pool.h"

struct muggle_peer_list_node
{
	struct muggle_peer_list_node *next;
	muggle_socket_peer_t    peer;
};

void muggle_socket_event_select(muggle_socket_event_t *ev, muggle_socket_ev_arg_t *ev_arg)
{
	MUGGLE_TRACE("socket event select run...");

	// set fd capacity
	int capacity = ev_arg->hints_max_peer;
	if (capacity <= 0 || capacity > FD_SETSIZE)
	{
		capacity = FD_SETSIZE;
	}

	if (capacity < ev_arg->cnt_peer)
	{
		MUGGLE_WARNING("capacity space not enough for all peers");
		return;
	}

	// set timeout
	struct timeval timeout;
	struct timeval *p_timeout = &timeout;
	memset(&timeout, 0, sizeof(timeout));
	if (ev->timeout_ms < 0)
	{
		p_timeout = NULL;
	}
	else if (ev->timeout_ms > 0)
	{
		timeout.tv_sec = ev->timeout_ms / 1000;
		timeout.tv_usec = (ev->timeout_ms % 1000) * 1000;
	}

	// fixed size pool for peers
	muggle_memory_pool_t pool;
	if (!muggle_memory_pool_init(&pool, capacity, sizeof(struct muggle_peer_list_node)))
	{
		MUGGLE_ERROR("failed init memory pool for capacity: %d, unit size: %d",
			capacity, sizeof(struct muggle_peer_list_node));
		return;
	}
	muggle_memory_pool_set_flag(&pool, MUGGLE_MEMORY_POOL_CONSTANT_SIZE);
	struct muggle_peer_list_node head;
	head.next = NULL;

	// convert sockets into peer
	for (int i = 0; i < ev_arg->cnt_peer; i++)
	{
		struct muggle_peer_list_node *node = (struct muggle_peer_list_node*)muggle_memory_pool_alloc(&pool);
		node->next = head.next;
		head.next = node;
		node->peer.peer_type = ev_arg->peers[i].peer_type;
		node->peer.fd = ev_arg->peers[i].fd;
		node->peer.data = ev_arg->peers[i].data;
		if (node->peer.peer_type == MUGGLE_SOCKET_PEER_TYPE_TCP_LISTEN ||
			node->peer.peer_type == MUGGLE_SOCKET_PEER_TYPE_TCP_PEER)
		{
			muggle_socket_set_nonblock(node->peer.fd, 1);
		}
	}

	// select
	int nfds = 0;
	fd_set rset, allset;
	FD_ZERO(&allset);
	struct muggle_peer_list_node *node = head.next, *prev_node = NULL;
	while (node)
	{
#if !MUGGLE_PLATFORM_WINDOWS
		if (node->peer.fd > nfds)
		{
			nfds = node->peer.fd;
		}
#endif
		FD_SET(node->peer.fd, &allset);
	}
	muggle_socket_peer_t tmp_peer, *p_peer;
	while (1)
	{
		rset = allset;
		// TODO:
		MUGGLE_ASSERT_MSG(0, "to be continued...");
		int n = select(nfds + 1, &rset, NULL, NULL, p_timeout);
	}


	// free memory pool
    muggle_memory_pool_destroy(&pool);
}
