/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_NET_SOCKET_EVENT_MEMMGR_H_
#define MUGGLE_C_NET_SOCKET_EVENT_MEMMGR_H_

#include "muggle/c/net/socket_event.h"
#include "muggle/c/memory/memory_pool.h"

EXTERN_C_BEGIN


typedef struct muggle_socket_peer_list_node
{
	struct muggle_socket_peer_list_node *prev;
	struct muggle_socket_peer_list_node *next;
	muggle_socket_peer_t                peer;
}muggle_socket_peer_list_node_t;

typedef struct muggle_socket_event_memmgr
{
	muggle_memory_pool_t           peer_pool;    // socket peer list node memory pool
	muggle_socket_peer_list_node_t active_head;  // active peer node list head
	muggle_socket_peer_list_node_t recycle_head; // recycle peer node list head
}muggle_socket_event_memmgr_t;

#if MUGGLE_ENABLE_TRACE

void muggle_socket_event_memmgr_debug_print(
	muggle_socket_peer_list_node_t *node,
	char *buf, int offset, int bufsize);

#endif


void muggle_socket_event_memmgr_insert_node(muggle_socket_peer_list_node_t *head, muggle_socket_peer_list_node_t *node);
void muggle_socket_event_memmgr_remove_node(muggle_socket_peer_list_node_t *node);

/*
 * initialize socket event memory manager
 * RETURN: 0 - success, otherwise failed init
 * */
int muggle_socket_event_memmgr_init(
	muggle_socket_event_t *ev, muggle_socket_ev_arg_t *ev_arg, muggle_socket_event_memmgr_t *mgr);

/*
 * allocate socket peer list node
 * */
muggle_socket_peer_list_node_t* muggle_socket_event_memmgr_allocate(muggle_socket_event_memmgr_t *mgr);

/*
 * get first active socket peer list node
 * */
muggle_socket_peer_list_node_t* muggle_socket_event_memmgr_get_node(muggle_socket_event_memmgr_t *mgr);

/*
 * free socket peer list node
 * */
void muggle_socket_event_memmgr_free(muggle_socket_event_memmgr_t *mgr, muggle_socket_peer_list_node_t *node);

/*
 * recycle active socket peer list node
 * */
void muggle_socket_event_memmgr_recycle(muggle_socket_event_memmgr_t *mgr, muggle_socket_peer_list_node_t *node);

/*
 * clear closed socket peer list node
 * */
void muggle_socket_event_memmgr_clear(muggle_socket_event_memmgr_t *mgr);

/*
 * destroy socket event memory manager
 */
void muggle_socket_event_memmgr_destroy(muggle_socket_event_memmgr_t *mgr);

EXTERN_C_END

#endif