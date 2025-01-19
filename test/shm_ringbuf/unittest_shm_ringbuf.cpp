#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

class ShmRingBufFixture : public testing::Test {
public:
	virtual void SetUp() override
	{
		k_name = "/tmp/";
		k_num = 1;
		uint32_t n_cacheline = muggle_next_pow_of_2(1024);
		uint32_t n_bytes = n_cacheline * MUGGLE_CACHE_LINE_SIZE;

		// close already exists shm_ringbuf
		shm_rbuf = muggle_shm_ringbuf_open(&shm, k_name, k_num,
										   MUGGLE_SHM_FLAG_OPEN, 0);
		if (shm_rbuf) {
			muggle_shm_detach(&shm);
			muggle_shm_rm(&shm);
			shm_rbuf = NULL;
		}

		// create new one
		shm_rbuf = muggle_shm_ringbuf_open(&shm, k_name, k_num,
										   MUGGLE_SHM_FLAG_CREAT, n_bytes);
		ASSERT_TRUE(shm_rbuf != NULL);
		ASSERT_EQ(shm_rbuf->n_cacheline, n_cacheline);
		ASSERT_EQ(shm_rbuf->n_bytes, n_bytes);
		ASSERT_TRUE(muggle_shm_ringbuf_is_ready(shm_rbuf));
	}

	virtual void TearDown() override
	{
		if (shm_rbuf) {
			muggle_shm_detach(&shm);
			muggle_shm_rm(&shm);
			shm_rbuf = NULL;
		}
	}

public:
	const char *k_name;
	int k_num;

	muggle_shm_ringbuf_t *shm_rbuf;
	muggle_shm_t shm;
};

TEST_F(ShmRingBufFixture, cal_bytes_cacheline)
{
	uint32_t k = MUGGLE_CACHE_LINE_SIZE - sizeof(muggle_shm_ringbuf_data_hdr_t);

	for (uint32_t n = 1; n < k; ++n) {
		uint32_t n_cacheline = MUGGLE_SHM_RINGBUF_CAL_BYTES_CACHELINE(n);
		ASSERT_EQ(n_cacheline, 3);
	}
	for (uint32_t i = 0; i < 10; ++i) {
		for (uint32_t n = 1 + k + MUGGLE_CACHE_LINE_SIZE * i;
			 n < 1 + k + MUGGLE_CACHE_LINE_SIZE * (i + 1); ++n) {
			uint32_t n_cacheline = MUGGLE_SHM_RINGBUF_CAL_BYTES_CACHELINE(n);
			ASSERT_EQ(n_cacheline, i + 4);
		}
	}
}

TEST_F(ShmRingBufFixture, cap)
{
	uint32_t data_size = 8;
	uint32_t data_cacheline = MUGGLE_SHM_RINGBUF_CAL_BYTES_CACHELINE(data_size);
	ASSERT_EQ(data_cacheline, 3);

	uint32_t n = (uint32_t)shm_rbuf->n_cacheline / data_cacheline;
	for (uint32_t i = 0; i < n; ++i) {
		void *ptr = muggle_shm_ringbuf_w_alloc_bytes(shm_rbuf, data_size);
		ASSERT_TRUE(ptr != NULL);
		muggle_shm_ringbuf_w_move(shm_rbuf);
	}
	void *ptr = muggle_shm_ringbuf_w_alloc_bytes(shm_rbuf, data_size);
	ASSERT_TRUE(ptr == NULL);
}

// 1. w move until remain is not enough
// 2. r move to w
// 3. w move until remain is not enough
TEST_F(ShmRingBufFixture, w_r_case1)
{
	uint32_t data_size = 8;
	void *ptr = NULL;
	uint32_t n_bytes = 0;
	uint32_t w_cnt = 0;
	uint32_t r_cnt = 0;

	// w move until remain is not enough
	ASSERT_EQ(shm_rbuf->cached_remain, shm_rbuf->n_cacheline - 1);
	while (true) {
		ptr = muggle_shm_ringbuf_w_alloc_bytes(shm_rbuf, data_size);
		if (ptr != NULL) {
			*(uint32_t *)ptr = w_cnt++;
			muggle_shm_ringbuf_w_move(shm_rbuf);
			ASSERT_EQ(shm_rbuf->cached_remain,
					  shm_rbuf->n_cacheline - 1 - shm_rbuf->write_cursor);
		} else {
			break;
		}
	}
	ASSERT_EQ(shm_rbuf->cached_remain,
			  shm_rbuf->n_cacheline - 1 - shm_rbuf->write_cursor);
	ASSERT_LE(shm_rbuf->cached_remain, 3);

	// r move to w
	while (true) {
		n_bytes = 0;
		ptr = muggle_shm_ringbuf_r_fetch(shm_rbuf, &n_bytes);
		if (ptr != NULL) {
			ASSERT_EQ(n_bytes, data_size);
			ASSERT_EQ(*(uint32_t *)ptr, r_cnt++);
			muggle_shm_ringbuf_r_move(shm_rbuf);
		} else {
			break;
		}
	}
	ASSERT_EQ(shm_rbuf->write_cursor, shm_rbuf->read_cursor);
	ASSERT_EQ(shm_rbuf->cached_remain,
			  shm_rbuf->n_cacheline - 1 - shm_rbuf->write_cursor);

	// w move until remain is not enough
	while (true) {
		ptr = muggle_shm_ringbuf_w_alloc_bytes(shm_rbuf, data_size);
		if (ptr != NULL) {
			muggle_shm_ringbuf_w_move(shm_rbuf);
			ASSERT_EQ(shm_rbuf->cached_remain,
					  shm_rbuf->read_cursor - shm_rbuf->write_cursor - 1);
		} else {
			break;
		}
	}
}

// 1. w move 1 step
// 2. r move 1 step
// 3. w move to end(n_cacheline)
// 4. w move 1 step
TEST_F(ShmRingBufFixture, w_r_case2)
{
	uint32_t data_size = 8;
	void *ptr = NULL;
	uint32_t n_bytes = 0;
	uint32_t w_cnt = 0;
	uint32_t r_cnt = 0;

	// w move 1 step
	ptr = muggle_shm_ringbuf_w_alloc_bytes(shm_rbuf, data_size);
	if (ptr != NULL) {
		*(uint32_t *)ptr = w_cnt++;
		muggle_shm_ringbuf_w_move(shm_rbuf);
		ASSERT_EQ(shm_rbuf->cached_remain,
				  shm_rbuf->n_cacheline - 1 - shm_rbuf->write_cursor);
	}

	// r move 1 step
	n_bytes = 0;
	ptr = muggle_shm_ringbuf_r_fetch(shm_rbuf, &n_bytes);
	ASSERT_EQ(*(uint32_t *)ptr, r_cnt++);
	muggle_shm_ringbuf_r_move(shm_rbuf);

	// w move to end
	n_bytes = (shm_rbuf->n_cacheline - shm_rbuf->write_cursor - 2) *
				  MUGGLE_CACHE_LINE_SIZE -
			  sizeof(muggle_shm_ringbuf_data_hdr_t);
	ptr = muggle_shm_ringbuf_w_alloc_bytes(shm_rbuf, n_bytes);
	ASSERT_TRUE(ptr != NULL);
	*(uint32_t *)ptr = w_cnt++;
	muggle_shm_ringbuf_w_move(shm_rbuf);
	ASSERT_EQ(shm_rbuf->cached_remain, 0);
	ASSERT_EQ(shm_rbuf->write_cursor, shm_rbuf->n_cacheline);

	// w move 1 step
	ptr = muggle_shm_ringbuf_w_alloc_bytes(shm_rbuf, data_size);
	ASSERT_TRUE(ptr == NULL);
	ASSERT_EQ(shm_rbuf->write_cursor, shm_rbuf->n_cacheline);
	ASSERT_EQ(shm_rbuf->read_cursor, 3);
}

// 1. w move until remain is not enough
// 2. r move to w
// 3. w alloc bytes and not move
// 4. r fetch is NULL
// 5. w move
// 6. w move 1 step
TEST_F(ShmRingBufFixture, w_r_case3)
{
	uint32_t data_size = 8;
	void *ptr = NULL;
	uint32_t n_bytes = 0;
	uint32_t w_cnt = 0;
	uint32_t r_cnt = 0;

	// w move until remain is not enough
	ASSERT_EQ(shm_rbuf->cached_remain, shm_rbuf->n_cacheline - 1);
	while (true) {
		ptr = muggle_shm_ringbuf_w_alloc_bytes(shm_rbuf, data_size);
		if (ptr != NULL) {
			*(uint32_t *)ptr = w_cnt++;
			muggle_shm_ringbuf_w_move(shm_rbuf);
			ASSERT_EQ(shm_rbuf->cached_remain,
					  shm_rbuf->n_cacheline - 1 - shm_rbuf->write_cursor);
		} else {
			break;
		}
	}
	ASSERT_EQ(shm_rbuf->cached_remain,
			  shm_rbuf->n_cacheline - 1 - shm_rbuf->write_cursor);
	ASSERT_LE(shm_rbuf->cached_remain, 3);

	// r move to w
	while (true) {
		n_bytes = 0;
		ptr = muggle_shm_ringbuf_r_fetch(shm_rbuf, &n_bytes);
		if (ptr != NULL) {
			ASSERT_EQ(n_bytes, data_size);
			ASSERT_EQ(*(uint32_t *)ptr, r_cnt++);
			muggle_shm_ringbuf_r_move(shm_rbuf);
		} else {
			break;
		}
	}
	ASSERT_EQ(shm_rbuf->write_cursor, shm_rbuf->read_cursor);
	ASSERT_EQ(shm_rbuf->cached_remain,
			  shm_rbuf->n_cacheline - 1 - shm_rbuf->write_cursor);

	// w alloc bytes and not move
	ptr = muggle_shm_ringbuf_w_alloc_bytes(shm_rbuf, data_size);
	ASSERT_TRUE(ptr != NULL);
	*(uint32_t *)ptr = w_cnt++;
	ASSERT_EQ(shm_rbuf->write_cursor, 0);
	ASSERT_EQ(shm_rbuf->cached_remain, shm_rbuf->read_cursor - 1);

	// r fetch is NULL
	ptr = muggle_shm_ringbuf_r_fetch(shm_rbuf, &n_bytes);
	ASSERT_TRUE(ptr == NULL);
	ASSERT_NE(shm_rbuf->read_cursor, 0);

	// w move
	muggle_shm_ringbuf_w_move(shm_rbuf);

	// r fetch is not NULL
	ptr = muggle_shm_ringbuf_r_fetch(shm_rbuf, &n_bytes);
	ASSERT_TRUE(ptr != NULL);
	ASSERT_EQ(*(uint32_t *)ptr, r_cnt++);
	ASSERT_EQ(shm_rbuf->read_cursor, 0);

	muggle_shm_ringbuf_r_move(shm_rbuf);
	ASSERT_EQ(shm_rbuf->read_cursor, shm_rbuf->write_cursor);
}
