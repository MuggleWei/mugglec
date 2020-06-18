#include "socket_event_memmgr.h"
#include "muggle/c/base/sleep.h"
#include "muggle/c/log/log.h"

#if MUGGLE_ENABLE_TRACE

void muggle_socket_event_memmgr_debug_print(
	muggle_socket_peer_list_node_t *node,
	char *buf, int offset, int bufsize)
{
	while (node)
	{
		char straddr[MUGGLE_SOCKET_ADDR_STRLEN];
		if (node->peer.addr_len == 0 ||
			muggle_socket_ntop((struct sockaddr*)&node->peer.addr, straddr, MUGGLE_SOCKET_ADDR_STRLEN, 0) == NULL)
		{
			snprintf(straddr, MUGGLE_SOCKET_ADDR_STRLEN, "?:?");
		}

#if MUGGLE_PLATFORM_WINDOWS
		offset += snprintf(buf + offset, bufsize - offset, "[%s] -> ", straddr);
#else
		offset += snprintf(buf + offset, bufsize - offset, "%d[%s] -> ", node->peer.fd, straddr);
#endif

		if (offset >= bufsize - 1)
		{
			break;
		}

		node = node->next;
	}

	if (offset < bufsize - 1)
	{
		snprintf(buf + offset, bufsize - offset, "NULL");
	}
	MUGGLE_DEBUG_INFO(buf);
}

#endif

void muggle_socket_event_memmgr_insert_node(muggle_socket_peer_list_node_t *head, muggle_socket_peer_list_node_t *node)
{
	node->next = head->next;
	if (node->next)
	{
		node->next->prev = node;
	}
	head->next = node;
	node->prev = head;
}

void muggle_socket_event_memmgr_remove_node(muggle_socket_peer_list_node_t *node)
{
	if (node->prev)
	{
		node->prev->next = node->next;
	}

	if (node->next)
	{
		node->next->prev = node->prev;
	}

	node->prev = NULL;
	node->next = NULL;
}

int muggle_socket_event_memmgr_init(
	muggle_socket_event_t *ev, muggle_socket_ev_arg_t *ev_arg, muggle_socket_event_memmgr_t *mgr)
{
	// fixed size pool for peers
	if (!muggle_memory_pool_init(&mgr->peer_pool, ev->capacity, sizeof(muggle_socket_peer_list_node_t)))
	{
		MUGGLE_LOG_ERROR("failed init memory pool for capacity: %d, unit size: %d",
			ev->capacity, sizeof(muggle_socket_peer_list_node_t));
		for (int i = 0; i < ev_arg->cnt_peer; ++i)
		{
			muggle_socket_close(ev_arg[i].peers->fd);
		}
		return -1;
	}
	muggle_memory_pool_set_flag(&mgr->peer_pool, MUGGLE_MEMORY_POOL_CONSTANT_SIZE);

	// active and recycle peer node list
	mgr->active_head.prev = NULL;
	mgr->active_head.next = NULL;
	mgr->recycle_head.prev = NULL;
	mgr->recycle_head.next = NULL;

	// put input peers into active list
	for (int i = 0; i < ev_arg->cnt_peer; ++i)
	{
		muggle_socket_peer_list_node_t* node = (muggle_socket_peer_list_node_t*)muggle_memory_pool_alloc(&mgr->peer_pool);
		if (node == NULL)
		{
			MUGGLE_ASSERT(node != NULL);
			muggle_memory_pool_destroy(&mgr->peer_pool);
			for (int j = 0; j < ev_arg->cnt_peer; ++j)
			{
				muggle_socket_close(ev_arg[j].peers->fd);
			}
			return -1;
		}

		memcpy(&node->peer, &ev_arg->peers[i], sizeof(muggle_socket_peer_list_node_t));
		muggle_socket_set_nonblock(node->peer.fd, 1);
		node->peer.status = MUGGLE_SOCKET_PEER_STATUS_ACTIVE;

		muggle_socket_event_memmgr_insert_node(&mgr->active_head, node);
	}

	return 0;
}

muggle_socket_peer_list_node_t* muggle_socket_event_memmgr_allocate(muggle_socket_event_memmgr_t *mgr)
{
	muggle_socket_peer_list_node_t* node = (muggle_socket_peer_list_node_t*)muggle_memory_pool_alloc(&mgr->peer_pool);
	if (node)
	{
		memset(node, 0, sizeof(muggle_socket_peer_list_node_t));
		muggle_socket_event_memmgr_insert_node(&mgr->active_head, node);
	}

	return node;
}

muggle_socket_peer_list_node_t* muggle_socket_event_memmgr_get_node(muggle_socket_event_memmgr_t *mgr)
{
	return mgr->active_head.next;
}

void muggle_socket_event_memmgr_free(muggle_socket_event_memmgr_t *mgr, muggle_socket_peer_list_node_t *node)
{
	muggle_socket_event_memmgr_remove_node(node);
	muggle_memory_pool_free(&mgr->peer_pool, node);
}

void muggle_socket_event_memmgr_recycle(muggle_socket_event_memmgr_t *mgr, muggle_socket_peer_list_node_t *node)
{
	MUGGLE_ASSERT(node->prev != NULL);

	muggle_socket_event_memmgr_remove_node(node);

	int ref_cnt = muggle_socket_peer_release(&node->peer);
	if (ref_cnt == 0)
	{
		muggle_memory_pool_free(&mgr->peer_pool, node);
	}
	else
	{
		muggle_socket_event_memmgr_insert_node(&mgr->recycle_head, node);
	}

#if MUGGLE_ENABLE_TRACE
	char debug_buf[4096];
	int debug_offset = 0;
	debug_offset = snprintf(debug_buf, sizeof(debug_buf) - debug_offset, "active list | ");
	muggle_socket_event_memmgr_debug_print(mgr->active_head.next, debug_buf, debug_offset, sizeof(debug_buf));

	debug_offset = 0;
	debug_offset = snprintf(debug_buf, sizeof(debug_buf) - debug_offset, "recycle list | ");
	muggle_socket_event_memmgr_debug_print(mgr->recycle_head.next, debug_buf, debug_offset, sizeof(debug_buf));
#endif
}

void muggle_socket_event_memmgr_clear(muggle_socket_event_memmgr_t *mgr)
{
	muggle_socket_peer_list_node_t *node = mgr->recycle_head.next;
	while (node)
	{
		muggle_socket_peer_list_node_t *next_node = node->next;
		if (node->peer.ref_cnt == 0)
		{
			muggle_socket_event_memmgr_free(mgr, node);
#if MUGGLE_ENABLE_TRACE
			char debug_buf[4096];
			int debug_offset = 0;
			debug_offset = snprintf(debug_buf, sizeof(debug_buf) - debug_offset, "active list | ");
			muggle_socket_event_memmgr_debug_print(mgr->active_head.next, debug_buf, debug_offset, sizeof(debug_buf));

			debug_offset = 0;
			debug_offset = snprintf(debug_buf, sizeof(debug_buf) - debug_offset, "recycle list | ");
			muggle_socket_event_memmgr_debug_print(mgr->recycle_head.next, debug_buf, debug_offset, sizeof(debug_buf));
#endif
		}
		node = next_node;
	}
}

void muggle_socket_event_memmgr_destroy(muggle_socket_event_memmgr_t *mgr)
{
	muggle_socket_peer_list_node_t *node = mgr->active_head.next;
	while (node)
	{
		muggle_socket_peer_list_node_t *next_node = node->next;
		muggle_socket_event_memmgr_recycle(mgr, node);
		node = next_node;
	}

	int retry_cnt = 0;
	node = mgr->recycle_head.next;
	while (node)
	{
		muggle_socket_peer_list_node_t *next_node = node->next;
		while (node->peer.ref_cnt != 0)
		{
			retry_cnt++;
			muggle_msleep(100);
			if (retry_cnt > 100)
			{
				retry_cnt = 0;
				MUGGLE_LOG_WARNING("socket event memory manager destroy retry 100 times");
			}
		}
		muggle_socket_event_memmgr_free(mgr, node);
		node = next_node;
	}

	muggle_memory_pool_destroy(&mgr->peer_pool);
}
