#include "shm_ring_buffer.h"
#include "muggle/c/base/utils.h"
#include <string.h>
#include <assert.h>

#define MUGGLE_SHM_ALIGN_4K_PAGE(n) MUGGLE_ROUND_UP_POW_OF_2_MUL(n, 4096)

// 'M', 'S', 'H', 'M'
#define MUGGLE_SHM_RINGBUF_MAGIC 0x4D53484D

typedef struct {
	char placeholder[MUGGLE_CACHE_LINE_SIZE];
} muggle_shm_ringbuf_block_t;
static_assert(sizeof(muggle_shm_ringbuf_block_t) == MUGGLE_CACHE_LINE_SIZE, "");

muggle_shm_ringbuf_t *muggle_shm_ringbuf_open(muggle_shm_t *shm,
											  const char *k_name, int k_num,
											  int flag, uint32_t nbytes)
{
	uint32_t data_bytes = 0;
	uint32_t n_cacheline = 0;
	uint32_t total_bytes = 0;

	if (nbytes == 0) {
		total_bytes = 0;
	} else {
		data_bytes =
			MUGGLE_ROUND_UP_POW_OF_2_MUL(nbytes, MUGGLE_CACHE_LINE_SIZE);
		n_cacheline = data_bytes / MUGGLE_CACHE_LINE_SIZE;
		n_cacheline = (uint32_t)muggle_next_pow_of_2(n_cacheline);
		data_bytes = n_cacheline * MUGGLE_CACHE_LINE_SIZE;

		total_bytes = sizeof(muggle_shm_ringbuf_t) + data_bytes;
		total_bytes = MUGGLE_SHM_ALIGN_4K_PAGE(total_bytes);
	}

	muggle_shm_ringbuf_t *ptr = (muggle_shm_ringbuf_t *)muggle_shm_open(
		shm, k_name, k_num, flag, total_bytes);
	if (ptr == NULL) {
		return NULL;
	}

	// if create new shm, do init
	if (flag & MUGGLE_SHM_FLAG_CREAT) {
		memset(ptr, 0, sizeof(*ptr));

		ptr->n_bytes = data_bytes;
		ptr->total_bytes = total_bytes;
		ptr->n_cacheline = n_cacheline;

		ptr->write_cursor = 0;
		ptr->cached_remain = ptr->n_cacheline - 1;

		ptr->read_cursor = 0;

		muggle_spinlock_init(&ptr->write_lock);
		muggle_spinlock_init(&ptr->read_lock);

		ptr->magic = MUGGLE_SHM_RINGBUF_MAGIC;
		muggle_atomic_store(&ptr->ready, 1, muggle_memory_order_release);
	}

	return ptr;
}

bool muggle_shm_ringbuf_is_ready(muggle_shm_ringbuf_t *shm_rbuf)
{
	muggle_sync_t ready =
		muggle_atomic_load(&shm_rbuf->ready, muggle_memory_order_acquire);
	uint32_t magic =
		muggle_atomic_load(&shm_rbuf->magic, muggle_memory_order_relaxed);

	if (magic != MUGGLE_SHM_RINGBUF_MAGIC) {
		return false;
	}
	if (ready != 1) {
		return false;
	}

	return true;
}

muggle_spinlock_t *muggle_shm_ringbuf_get_wlock(muggle_shm_ringbuf_t *shm_rbuf)
{
	return &shm_rbuf->write_lock;
}

muggle_spinlock_t *muggle_shm_ringbuf_get_rlock(muggle_shm_ringbuf_t *shm_rbuf)
{
	return &shm_rbuf->read_lock;
}

void *muggle_shm_ringbuf_get_data(muggle_shm_ringbuf_t *shm_rbuf, uint32_t pos)
{
	return (muggle_shm_ringbuf_block_t *)(shm_rbuf + 1) + pos;
}

void muggle_shm_ringbuf_update_cached_remain(muggle_shm_ringbuf_t *shm_rbuf,
											 uint32_t required_cacheline)
{
	muggle_sync_t r_pos =
		muggle_atomic_load(&shm_rbuf->read_cursor, muggle_memory_order_relaxed);
	if (r_pos > shm_rbuf->write_cursor) {
		//         w         r
		// ================================
		//
		// contiguous writable is: r - w - 1
		shm_rbuf->cached_remain = r_pos - shm_rbuf->write_cursor - 1;
	} else {
		if (r_pos != 0) {
			//             r        w          end
			// ================================
			//
			//               (rw)              end
			// ================================
			//
			// contiguous writable is: (end - w) or (r - 1)
			uint32_t right_remain =
				shm_rbuf->n_cacheline - shm_rbuf->write_cursor;
			uint32_t left_remain = r_pos - 1;
			if (right_remain >= required_cacheline) {
				shm_rbuf->cached_remain = right_remain;
			} else if (left_remain >= required_cacheline) {
				// fillup current hdr 0 and move w to 0
				muggle_shm_ringbuf_data_hdr_t *hdr =
					muggle_shm_ringbuf_get_data(shm_rbuf,
												shm_rbuf->write_cursor);

				// if w == end, right_remain is 0
				//
				//               r                 (w|end)
				// ================================
				//
				if (right_remain != 0) {
					hdr->n_bytes = 0;
					hdr->n_cachelines = 0;
				}
				muggle_atomic_store(&shm_rbuf->write_cursor, 0,
									muggle_memory_order_release);

				shm_rbuf->cached_remain = left_remain;
			} else {
				// there has no enough space
			}
		} else {
			// r                    w          end
			// ================================
			//
			// contiguous writable is: (end - w - 1)
			//
			// NOTE:
			//   don't need worry about end == w, cause r is 0, w will not move
			//   to end
			if (shm_rbuf->n_cacheline == shm_rbuf->write_cursor) {
				// there has no enough space
			} else {
				shm_rbuf->cached_remain =
					shm_rbuf->n_cacheline - shm_rbuf->write_cursor - 1;
			}
		}
	}
}

void *muggle_shm_ringbuf_w_alloc_bytes(muggle_shm_ringbuf_t *shm_rbuf,
									   uint32_t n_bytes)
{
	uint32_t n_cacheline = MUGGLE_SHM_RINGBUF_CAL_BYTES_CACHELINE(n_bytes);
	return muggle_shm_ringbuf_w_alloc_cachelines(shm_rbuf, n_bytes,
												 n_cacheline);
}

void *muggle_shm_ringbuf_w_alloc_cachelines(muggle_shm_ringbuf_t *shm_rbuf,
											uint32_t n_bytes,
											uint32_t n_cacheline)
{
	if (shm_rbuf->cached_remain < n_cacheline) {
		muggle_shm_ringbuf_update_cached_remain(shm_rbuf, n_cacheline);
		if (shm_rbuf->cached_remain < n_cacheline) {
			return NULL;
		}
	}

	muggle_shm_ringbuf_data_hdr_t *hdr =
		muggle_shm_ringbuf_get_data(shm_rbuf, shm_rbuf->write_cursor);
	hdr->n_bytes = n_bytes;
	hdr->n_cachelines = n_cacheline;

	shm_rbuf->cached_w_hdr = hdr;

	return (void *)(hdr + 1);
}

void muggle_shm_ringbuf_w_move(muggle_shm_ringbuf_t *shm_rbuf)
{
	uint32_t n = shm_rbuf->cached_w_hdr->n_cachelines;
	shm_rbuf->cached_remain -= n;
	muggle_atomic_store(&shm_rbuf->write_cursor, shm_rbuf->write_cursor + n,
						muggle_memory_order_release);
}

void *muggle_shm_ringbuf_r_fetch(muggle_shm_ringbuf_t *shm_rbuf,
								 uint32_t *n_bytes)
{
	muggle_sync_t w_pos = muggle_atomic_load(&shm_rbuf->write_cursor,
											 muggle_memory_order_acquire);

	// there has no data wait for read
	if (w_pos == shm_rbuf->read_cursor) {
		return NULL;
	}

	// normal status
	shm_rbuf->cached_r_hdr =
		muggle_shm_ringbuf_get_data(shm_rbuf, shm_rbuf->read_cursor);
	if (shm_rbuf->cached_r_hdr->n_bytes != 0) {
		if (n_bytes) {
			*n_bytes = shm_rbuf->cached_r_hdr->n_bytes;
		}
		return shm_rbuf->cached_r_hdr + 1;
	}

	// write cursor already move to start
	if (w_pos == 0) {
		// data not write yet
		return NULL;
	}

	// write cursor already move away from 0
	muggle_atomic_store(&shm_rbuf->read_cursor, 0, muggle_memory_order_relaxed);
	shm_rbuf->cached_r_hdr =
		muggle_shm_ringbuf_get_data(shm_rbuf, shm_rbuf->read_cursor);
	if (shm_rbuf->cached_r_hdr->n_bytes != 0) {
		if (n_bytes) {
			*n_bytes = shm_rbuf->cached_r_hdr->n_bytes;
		}
		return shm_rbuf->cached_r_hdr + 1;
	}

	return NULL;
}

void muggle_shm_ringbuf_r_move(muggle_shm_ringbuf_t *shm_rbuf)
{
	uint32_t n = shm_rbuf->cached_r_hdr->n_cachelines;
	uint32_t r_pos = MUGGLE_IDX_IN_POW_OF_2_RING(shm_rbuf->read_cursor + n,
												 shm_rbuf->n_cacheline);
	muggle_atomic_store(&shm_rbuf->read_cursor, r_pos,
						muggle_memory_order_release);
}
