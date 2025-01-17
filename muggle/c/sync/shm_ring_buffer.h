/******************************************************************************
 *  @file         shm_ring_buffer.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2025-01-17
 *  @copyright    Copyright 2025 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec shared memory ring buffer
 *****************************************************************************/

#ifndef MUGGLE_C_SHM_RING_BUFFER_H_
#define MUGGLE_C_SHM_RING_BUFFER_H_

#include "muggle/c/base/macro.h"
#include "muggle/c/sync/spinlock.h"
#include "muggle/c/sync/sync_obj.h"
#include "muggle/c/sync/shm.h"
#include <stdint.h>
#include <stdbool.h>

EXTERN_C_BEGIN

typedef struct {
	uint32_t n_bytes; //!< number of bytes
	uint32_t n_cachelines; //!< number of cachelines
} muggle_shm_ringbuf_data_hdr_t;

typedef struct {
	union {
		struct {
			uint32_t magic; //!< magic word
			uint32_t n_bytes; //!< number of data bytes
			muggle_sync_t n_cacheline; //!< number of data cachelines
			muggle_sync_t ready; //!< is ready
		};
		MUGGLE_STRUCT_CACHE_LINE_PADDING(0);
	};
	MUGGLE_STRUCT_CACHE_LINE_X2_PADDING(0);

	union {
		struct {
			muggle_sync_t write_cursor; //!< write cursor
			muggle_sync_t cached_remain; //!< contiguous len of cachelines
			muggle_shm_ringbuf_data_hdr_t *cached_w_hdr; //!< current write hdr
		};
		MUGGLE_STRUCT_CACHE_LINE_PADDING(1);
	};
	MUGGLE_STRUCT_CACHE_LINE_X2_PADDING(1);

	union {
		struct {
			muggle_sync_t read_cursor; //!< read cursor
			muggle_shm_ringbuf_data_hdr_t *cached_r_hdr; //!< current read hdr
		};
		MUGGLE_STRUCT_CACHE_LINE_PADDING(2);
	};
	MUGGLE_STRUCT_CACHE_LINE_X2_PADDING(2);

	union {
		struct {
			muggle_spinlock_t write_lock; //!< write lock
		};
		MUGGLE_STRUCT_CACHE_LINE_PADDING(3);
	};
	MUGGLE_STRUCT_CACHE_LINE_X2_PADDING(3);

	union {
		struct {
			muggle_spinlock_t read_lock; //!< read lock
		};
		MUGGLE_STRUCT_CACHE_LINE_PADDING(4);
	};
	MUGGLE_STRUCT_CACHE_LINE_X2_PADDING(4);
} muggle_shm_ringbuf_t;

/**
 * @brief open shared memory ring buffer
 *
 * @param shm     pointer to muggle_shm_t
 * @param k_name  key's name of shm, in *nix, gurantee it's an exists filepath
 * @param k_num   key's number, range in [1, 255]
 * @param flag    bit or of MUGGLE_SHM_FLAG_*
 * @param nbytes  expected size of data's memory
 *
 * @return
 *   - on success, return pointer to memory
 *   - on failed, return NULL
 *
 * @NOTE: arguments and return value is the same as muggle_shm_open
 */
MUGGLE_C_EXPORT
muggle_shm_ringbuf_t *muggle_shm_ringbuf_open(muggle_shm_t *shm,
											  const char *k_name, int k_num,
											  int flag, uint32_t nbytes);

/**
 * @brief check shared memory buffer is ready
 *
 * @param shm_rbuf  pointer to muggle_shm_ringbuf_t
 *
 * @return boolean
 */
MUGGLE_C_EXPORT
bool muggle_shm_ringbuf_is_ready(muggle_shm_ringbuf_t *shm_rbuf);

/**
 * @brief get write lock
 *
 * @param shm_rbuf  pointer to muggle_shm_ringbuf_t
 *
 * @return write lock
 */
MUGGLE_C_EXPORT
muggle_spinlock_t *muggle_shm_ringbuf_get_wlock(muggle_shm_ringbuf_t *shm_rbuf);

/**
 * @brief get read lock
 *
 * @param shm_rbuf  pointer to muggle_shm_ringbuf_t
 *
 * @return read lock
 */
MUGGLE_C_EXPORT
muggle_spinlock_t *muggle_shm_ringbuf_get_rlock(muggle_shm_ringbuf_t *shm_rbuf);

/**
 * @brief get cache line data
 *
 * @param shm_rbuf  pointer to muggle_shm_ringbuf_t
 * @param pos       position in cachelines
 *
 * @return data
 */
MUGGLE_C_EXPORT
void *muggle_shm_ringbuf_get_data(muggle_shm_ringbuf_t *shm_rbuf, uint32_t pos);

/**
 * @brief update cached_remain
 *
 * @param shm_rbuf            pointer to muggle_shm_ringbuf_t
 * @param required_cacheline  required number of cacheline
 */
MUGGLE_C_EXPORT
void muggle_shm_ringbuf_update_cached_remain(muggle_shm_ringbuf_t *shm_rbuf,
											 uint32_t required_cacheline);

/**
 * @brief allocate bytes for write
 *
 * @param shm_rbuf  pointer to muggle_shm_ringbuf_t
 * @param n_bytes   number of bytes required
 *
 * @return
 *   - on success, return pointer to memory addr
 *   - on failed, return NULL
 */
MUGGLE_C_EXPORT
void *muggle_shm_ringbuf_w_alloc_bytes(muggle_shm_ringbuf_t *shm_rbuf,
									   uint32_t n_bytes);

/**
 * @brief allocate bytes for write
 *
 * @param shm_rbuf    pointer to muggle_shm_ringbuf_t
 * @param n_bytes     number of bytes required
 * @param n_cacheline number of cacheline required
 *
 * @return 
 *   - on success, return pointer to memory addr
 *   - on failed, return NULL
 */
MUGGLE_C_EXPORT
void *muggle_shm_ringbuf_w_alloc_cachelines(muggle_shm_ringbuf_t *shm_rbuf,
											uint32_t n_bytes,
											uint32_t n_cacheline);

/**
 * @brief write move
 *
 * @param shm_rbuf  pointer to muggle_shm_ringbuf_t
 */
MUGGLE_C_EXPORT
void muggle_shm_ringbuf_w_move(muggle_shm_ringbuf_t *shm_rbuf);

/**
 * @brief fetch next data for read
 *
 * @param shm_rbuf  pointer to muggle_shm_ringbuf_t
 * @param n_bytes   number of bytes read
 *
 * @return
 *   - when has data wait for read, return pointer to data
 *   - when no data wait for read, return NULL
 */
MUGGLE_C_EXPORT
void *muggle_shm_ringbuf_r_fetch(muggle_shm_ringbuf_t *shm_rbuf,
								 uint32_t *n_bytes);

/**
 * @brief read move
 *
 * @param shm_rbuf  pointer to muggle_shm_ringbuf_t
 */
MUGGLE_C_EXPORT
void muggle_shm_ringbuf_r_move(muggle_shm_ringbuf_t *shm_rbuf);

EXTERN_C_END

#endif // !MUGGLE_C_SHM_RING_BUFFER_H_
