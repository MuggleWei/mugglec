/******************************************************************************
 *  @file         socket_event_memmgr.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-21
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec socket event memory manager
 *****************************************************************************/

#ifndef MUGGLE_C_NET_SOCKET_EVENT_MEMMGR_H_
#define MUGGLE_C_NET_SOCKET_EVENT_MEMMGR_H_

#include "muggle/c/net/socket_event.h"
#include "muggle/c/memory/memory_pool.h"

EXTERN_C_BEGIN


/**
 * @brief socket peer node in memory manager's linked list
 */
typedef struct muggle_socket_peer_list_node
{
	struct muggle_socket_peer_list_node *prev;
	struct muggle_socket_peer_list_node *next;
	muggle_socket_peer_t                peer;
}muggle_socket_peer_list_node_t;

/**
 * @brief socket event memory manager
 */
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


/**
 * @brief socket event memory manager insert node
 *
 * @param head  head of socket event memory manager's linked list
 * @param node  socket peer list node
 */
void muggle_socket_event_memmgr_insert_node(muggle_socket_peer_list_node_t *head, muggle_socket_peer_list_node_t *node);

/**
 * @brief remove socket peer node from socket event memory manager's linked list
 *
 * @param node socket peer list node
 */
void muggle_socket_event_memmgr_remove_node(muggle_socket_peer_list_node_t *node);

/**
 * 
 *
 * RETURN: 
 */
/**
 * @brief initialize socket event memory manager
 *
 * @param ev          socket event
 * @param ev_init_arg socket event initialize arguments
 * @param mgr         socket event memory manager pointer
 *
 * @return 
 *     - success return 0
 *     - otherwise failed init
 */
int muggle_socket_event_memmgr_init(
	muggle_socket_event_t *ev, muggle_socket_event_init_arg_t *ev_init_arg, muggle_socket_event_memmgr_t *mgr);

/**
 * @brief allocate socket peer list node
 *
 * @param mgr   socket event memory manager pointer
 *
 * @return allocated list node
 */
muggle_socket_peer_list_node_t* muggle_socket_event_memmgr_allocate(muggle_socket_event_memmgr_t *mgr);

/**
 * @brief get first active socket peer list node
 *
 * @param mgr   socket event memory manager pointer
 *
 * @return first node in socket event memory manager
 */
muggle_socket_peer_list_node_t* muggle_socket_event_memmgr_get_node(muggle_socket_event_memmgr_t *mgr);

/**
 * @brief free socket peer list node
 *
 * @param mgr   socket event memory manager pointer
 * @param node  node need to be free
 */
void muggle_socket_event_memmgr_free(muggle_socket_event_memmgr_t *mgr, muggle_socket_peer_list_node_t *node);

/**
 * @brief recycle active socket peer list node
 *
 * @param mgr   socket event memory manager pointer
 * @param node  node need to be recycle
 */
void muggle_socket_event_memmgr_recycle(muggle_socket_event_memmgr_t *mgr, muggle_socket_peer_list_node_t *node);

/**
 * @brief clear closed socket peer list node
 *
 * @param mgr   socket event memory manager pointer
 */
void muggle_socket_event_memmgr_clear(muggle_socket_event_memmgr_t *mgr);

/**
 * @brief destroy socket event memory manager
 *
 * @param mgr   socket event memory manager pointer
 */
void muggle_socket_event_memmgr_destroy(muggle_socket_event_memmgr_t *mgr);

EXTERN_C_END

#endif
