#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"
#include <thread>

struct block_data {
	int info;
	int idx;
	int is_end;
};

TEST(ring_memory_pool, basic)
{
	const muggle_sync_t capacity = 8;

	muggle_ring_memory_pool_t pool;
	muggle_ring_memory_pool_init(&pool, capacity, sizeof(block_data));

	block_data *p = NULL;
	muggle_ring_mpool_block_head_t *head = NULL;

	block_data *arr[capacity];
	for (muggle_sync_t i = 0; i < capacity; i++) {
		p = (block_data *)muggle_ring_memory_pool_alloc(&pool);
		ASSERT_TRUE(p != NULL);

		head = (muggle_ring_mpool_block_head_t *)p - 1;
		ASSERT_EQ(head->pool, &pool);
		ASSERT_EQ(head->block_idx, i);
		arr[i] = p;
	}

	muggle_ring_memory_pool_free(arr[0]);
	p = (block_data *)muggle_ring_memory_pool_alloc(&pool);
	ASSERT_TRUE(p != NULL);
	head = (muggle_ring_mpool_block_head_t *)p - 1;
	ASSERT_EQ(head->pool, &pool);
	ASSERT_EQ(head->block_idx, 0);

	muggle_ring_memory_pool_free(arr[5]);
	p = (block_data *)muggle_ring_memory_pool_alloc(&pool);
	ASSERT_TRUE(p != NULL);
	head = (muggle_ring_mpool_block_head_t *)p - 1;
	ASSERT_EQ(head->pool, &pool);
	ASSERT_EQ(head->block_idx, 5);

	muggle_ring_memory_pool_destroy(&pool);
}

TEST(ring_memory_pool, mul_consume)
{
	const int num_threads = 8;
	muggle_channel_t chans[num_threads];

	const muggle_sync_t capacity = 8;
	muggle_ring_memory_pool_t pool;
	muggle_ring_memory_pool_init(&pool, capacity, sizeof(block_data));

	// run consumers
	std::thread *p_th[num_threads];
	for (int i = 0; i < num_threads; ++i) {
		muggle_channel_init(&chans[i], 16, 0);
		muggle_channel_t *p_chan = &chans[i];
		p_th[i] = new std::thread([p_chan] {
			while (true) {
				block_data *data = (block_data *)muggle_channel_read(p_chan);
				if (data == nullptr) {
					break;
				} else {
					muggle_ring_memory_pool_free(data);
				}
			}
		});
	}

	// allocate data and push to threads
	for (int i = 0; i < 2048; ++i) {
		block_data *data = (block_data *)muggle_ring_memory_pool_alloc(&pool);
		ASSERT_TRUE(data != NULL);

		int idx_thread = i % num_threads;
		while (muggle_channel_write(&chans[idx_thread], data) != 0) {
			muggle_nsleep(100);
			continue;
		}
	}

	// write end message
	for (int i = 0; i < num_threads; ++i) {
		while (muggle_channel_write(&chans[i], nullptr) != 0) {
			muggle_nsleep(100);
			continue;
		}
	}

	// wait threads exit
	for (int i = 0; i < num_threads; ++i) {
		p_th[i]->join();
		muggle_channel_destroy(&chans[i]);
	}

	// can allocate all blocks
	for (muggle_sync_t i = 0; i < capacity; i++) {
		block_data *p = (block_data *)muggle_ring_memory_pool_alloc(&pool);
		ASSERT_TRUE(p != NULL);

		muggle_ring_mpool_block_head_t *head =
			(muggle_ring_mpool_block_head_t *)p - 1;
		ASSERT_EQ(head->pool, &pool);
	}

	muggle_ring_memory_pool_destroy(&pool);
}
