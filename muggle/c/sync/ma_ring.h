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
#include "muggle/c/sync/sync_obj.h"

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
	union {
		muggle_sync_t wpos; //!< writer position
		MUGGLE_STRUCT_CACHE_LINE_PADDING(0);
	};
	MUGGLE_STRUCT_CACHE_LINE_X2_PADDING(0);
	union {
		muggle_sync_t rpos; //!< reader position
		MUGGLE_STRUCT_CACHE_LINE_PADDING(1);
	};
	muggle_thread_readable_id tid; //!< thread id
	muggle_sync_t status; //!< status of ring
} muggle_ma_ring_t;

/**
 * @brief ma_ring list
 */
typedef struct muggle_ma_ring_list_node {
	struct muggle_ma_ring_list_node *next; //!< next node
	muggle_ma_ring_t *ring; //!< am_ring pointer
} muggle_ma_ring_list_node_t;

/**
 * @brief ma_ring callback function
 *
 * @param data  data pointer
 */
typedef void (*muggle_ma_ring_callback)(muggle_ma_ring_t *ring, void *data);

/**
 * @brief before ma_ring backend run callback
 */
typedef void (*muggle_ma_ring_before_run_callback)();

/**
 * @brief ma_ring context
 */
typedef struct muggle_ma_ring_context {
	muggle_spinlock_t spinlock; //!< context spinlock
	muggle_sync_t capacity; //!< capacity of ring elements
	muggle_sync_t block_size; //!< size of ring block
	muggle_ma_ring_list_node_t add_list; //!< add list
	muggle_ma_ring_list_node_t ring_list; //!< ring list
	muggle_ma_ring_callback cb; //!< message callback
	muggle_ma_ring_before_run_callback before_run_cb; //!< before run callback
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
muggle_ma_ring_context_t *muggle_ma_ring_ctx_get();

/**
 * @brief set ma_ring capacity
 *
 * @param capacity  capacity of ma_ring
 */
MUGGLE_C_EXPORT
void muggle_ma_ring_ctx_set_capacity(muggle_sync_t capacity);

/**
 * @brief set ma_ring data size
 *
 * @param data_size  data size of ma_ring
 */
MUGGLE_C_EXPORT
void muggle_ma_ring_ctx_set_data_size(muggle_sync_t data_size);

/**
 * @brief set callback of ma_ring
 *
 * @param fn  callback function
 */
MUGGLE_C_EXPORT
void muggle_ma_ring_ctx_set_callback(muggle_ma_ring_callback fn);

/**
 * @brief set before run callback of ma_ring
 *
 * @param fn  before run callback function
 */
MUGGLE_C_EXPORT
void muggle_ma_ring_ctx_set_before_run_callback(
	muggle_ma_ring_before_run_callback fn);

/**
 * @brief get thread-local ma_ring
 *
 * @return ma_ring
 */
MUGGLE_C_EXPORT
muggle_ma_ring_t *muggle_ma_ring_thread_ctx_get();

/**
 * @brief initialize ma_ring thread context
 *
 * @return ma_ring
 */
MUGGLE_C_EXPORT
muggle_ma_ring_t *muggle_ma_ring_thread_ctx_init();

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
void *muggle_ma_ring_alloc(muggle_ma_ring_t *ring);

/**
 * @brief move ring writer forward
 *
 * @param ring  am_ring pointer
 */
MUGGLE_C_EXPORT
void muggle_ma_ring_move(muggle_ma_ring_t *ring);

EXTERN_C_END

#endif // !MUGGLE_C_AM_RING_H_
