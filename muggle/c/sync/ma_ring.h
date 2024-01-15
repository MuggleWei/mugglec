/******************************************************************************
 *  @file         ma_ring.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2024-01-15
 *  @copyright    Copyright 2024 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec multiple-async ring
 *****************************************************************************/

#ifndef MUGGLE_C_AM_RING_H_
#define MUGGLE_C_AM_RING_H_

#include "muggle/c/base/atomic.h"
#include "muggle/c/base/macro.h"
#include "muggle/c/base/thread.h"
#include "muggle/c/sync/spinlock.h"

EXTERN_C_BEGIN

enum {
	MUGGLE_MA_RING_STATUS_NORMAL = 0,
	MUGGLE_MA_RING_STATUS_WAIT_REMOVE,
	MUGGLE_MA_RING_STATUS_DONE,
};

/**
 * @brief multiple-async ring
 */
typedef struct muggle_ma_ring {
	void *buffer; //!< ring buffer datas
	muggle_atomic_int wpos; //!< writer position
	muggle_atomic_int rpos; //!< reader position
	muggle_thread_readable_id tid; //!< thread id
	muggle_atomic_int status; //!< status of ring
} muggle_ma_ring_t;

/**
 * @brief ma_ring list
 */
typedef struct muggle_ma_ring_list_node {
	struct muggle_ma_ring_list_node *next; //!< next node
	muggle_ma_ring_t *ring; //!< am_ring pointer
} muggle_ma_ring_list_node_t;

/**
 * @brief am_ring callback function
 *
 * @param data  data pointer
 */
typedef void (*muggle_ma_ring_callback)(muggle_ma_ring_t *ring, void *data);

/**
 * @brief ma_ring context
 */
typedef struct muggle_ma_ring_context {
	muggle_spinlock_t spinlock; //!< context spinlock
	muggle_atomic_int capacity; //!< capacity of ring elements
	muggle_atomic_int block_size; //!< size of ring block
	muggle_ma_ring_list_node_t add_list; //!< add list
	muggle_ma_ring_list_node_t ring_list; //!< ring list
	muggle_ma_ring_callback cb; //!< message callback
} muggle_ma_ring_context_t;

/**
 * @brief run ma_ring backend thread
 */
MUGGLE_C_EXPORT
void muggle_ma_ring_backend_run();

/**
 * @brief get ma_ring context
 *
 * @return ma_ring context
 */
MUGGLE_C_EXPORT
muggle_ma_ring_context_t* muggle_ma_ring_ctx_get();

/**
 * @brief set ma_ring capacity
 *
 * @param capacity  capacity of ma_ring
 */
MUGGLE_C_EXPORT
void muggle_ma_ring_ctx_set_capacity(muggle_atomic_int capacity);

/**
 * @brief set ma_ring data size
 *
 * @param data_size  data size of ma_ring
 */
MUGGLE_C_EXPORT
void muggle_ma_ring_ctx_set_data_size(muggle_atomic_int data_size);

/**
 * @brief set callback of ma_ring
 *
 * @param fn  callback function
 */
MUGGLE_C_EXPORT
void muggle_ma_ring_ctx_set_callback(muggle_ma_ring_callback fn);

/**
 * @brief get thread-local ma_ring
 *
 * @return ma_ring
 */
MUGGLE_C_EXPORT
muggle_ma_ring_t* muggle_ma_ring_thread_ctx_get();

/**
 * @brief initialize ma_ring thread context
 *
 * @return ma_ring
 */
MUGGLE_C_EXPORT
muggle_ma_ring_t* muggle_ma_ring_thread_ctx_init();

/**
 * @brief cleanup thread context
 */
MUGGLE_C_EXPORT
void muggle_ma_ring_thread_ctx_cleanup();

/**
 * @brief allocate block size
 *
 * @param ring  am_ring pointer
 *
 * @return block pointer
 */
MUGGLE_C_EXPORT
void* muggle_ma_ring_alloc(muggle_ma_ring_t *ring);

/**
 * @brief move ring writer forward
 *
 * @param ring  am_ring pointer
 */
MUGGLE_C_EXPORT
void muggle_ma_ring_move(muggle_ma_ring_t *ring);

EXTERN_C_END

#endif // !MUGGLE_C_AM_RING_H_
