#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

class ShmRingBufFixture : public testing::Test {
public:
	virtual void SetUp() override
	{
		k_name = "/tmp/";
		k_num = 1;
		uint32_t n_cacheline = next_pow_of_2(1024 + 1);
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

TEST_F(ShmRingBufFixture, cap)
{
	uint32_t data_size = 8;
	uint32_t data_cacheline =
		ROUND_UP_POW_OF_2_MUL(data_size, MUGGLE_CACHE_LINE_SIZE) /
		MUGGLE_CACHE_LINE_SIZE;
	uint32_t data_required_cacheline = data_cacheline + 2;
	ASSERT_EQ(data_cacheline, 1);
	ASSERT_EQ(data_required_cacheline, 3);

	uint32_t n = (uint32_t)shm_rbuf->n_cacheline / data_required_cacheline;
	for (uint32_t i = 0; i < n; ++i) {
		void *ptr = muggle_shm_ringbuf_w_alloc_bytes(shm_rbuf, data_size);
		ASSERT_TRUE(ptr != NULL);
		muggle_shm_ringbuf_w_move(shm_rbuf);
	}
	void *ptr = muggle_shm_ringbuf_w_alloc_bytes(shm_rbuf, data_size);
	ASSERT_TRUE(ptr == NULL);
}
