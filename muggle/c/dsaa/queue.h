/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_DSAA_QUEUE_H_
#define MUGGLE_C_DSAA_QUEUE_H_

#include "muggle/c/dsaa/dsaa_utils.h"

EXTERN_C_BEGIN

typedef struct muggle_queue_node
{
	struct muggle_queue_node *prev;
	struct muggle_queue_node *next;
	void                     *data;
}muggle_queue_node_t;

typedef struct muggle_queue
{
	muggle_queue_node_t  head;   // head node of queue
	muggle_queue_node_t  tail;   // tail node of queue
	muggle_memory_pool_t *pool;  // memory pool of queue nodes, if it's NULL, use malloc and free by default
	uint64_t             size;   // number of elements in queue
}muggle_queue_t;

// initialize queue
// @param p_queue    pointer to queue
// @param capacity   init capacity for nodes memory pool, if 0, don't use memory pool
MUGGLE_C_EXPORT
bool muggle_queue_init(muggle_queue_t *p_queue, size_t capacity);

// destroy queue
// @param p_queue    pointer to queue
// @param func_free  function for free data, if it's NULL, do nothing for data
// @param pool       the memory pool passed to func_free
MUGGLE_C_EXPORT
void muggle_queue_destroy(muggle_queue_t *p_queue, muggle_dsaa_data_free func_free, void *pool);

// detect queue is empty
// @param p_queue    pointer to queue
MUGGLE_C_EXPORT
bool muggle_queue_is_empty(muggle_queue_t *p_queue);

// clear queue
// @param p_queue    pointer to queue
// @param func_free  function for free data, if it's NULL, do nothing for data
// @param pool       the memory pool passed to func_free
MUGGLE_C_EXPORT
void muggle_queue_clear(muggle_queue_t *p_queue, muggle_dsaa_data_free func_free, void *pool);

// get number of elements in queue
// @param p_queue  pointer to queue
// @return number of elements in queue
MUGGLE_C_EXPORT
size_t muggle_queue_size(muggle_queue_t *p_queue);

// enqueue data
// @param p_queue  pointer to queue
// @return the node hold enqueue data, if it's NULL, failed enqueue
MUGGLE_C_EXPORT
muggle_queue_node_t* muggle_queue_enqueue(muggle_queue_t *p_queue, void *data);

// dequeue data
// @param p_queue  pointer to queue
// @param func_free  function for free data, if it's NULL, do nothing for data
// @param pool       the memory pool passed to func_free
MUGGLE_C_EXPORT
void muggle_queue_dequeue(muggle_queue_t *p_queue, muggle_dsaa_data_free func_free, void *pool);

// get first node of queue
// @param p_queue  pointer to queue
// @return first node of queue, if it's NULL, queue is empty
MUGGLE_C_EXPORT
muggle_queue_node_t* muggle_queue_front(muggle_queue_t *p_queue);

EXTERN_C_END

#endif
