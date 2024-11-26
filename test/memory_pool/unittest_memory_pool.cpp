#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

typedef struct block_data {
	uint64_t u64;
	uint32_t u32;
	int32_t i32;
} block_data_t;

#define INIT_CAPACITY 8

class TestMemoryPoolFixture : public ::testing::Test {
public:
	void SetUp()
	{
		bool ret = muggle_memory_pool_init(&pool_, INIT_CAPACITY,
										   sizeof(block_data_t));
		ASSERT_TRUE(ret);
	}

	void TearDown()
	{
		muggle_memory_pool_destroy(&pool_);
	}

protected:
	muggle_memory_pool_t pool_;
};

TEST_F(TestMemoryPoolFixture, init_destroy)
{
}

TEST_F(TestMemoryPoolFixture, ensure_space1)
{
	bool ret = true;
	unsigned int cap = 0;

	cap = 4;
	ret = muggle_memory_pool_ensure_space(&pool_, cap);
	ASSERT_TRUE(ret);
	ASSERT_LE(cap, INIT_CAPACITY);
	ASSERT_EQ(pool_.capacity, INIT_CAPACITY);

	cap = 8;
	ret = muggle_memory_pool_ensure_space(&pool_, cap);
	ASSERT_TRUE(ret);
	ASSERT_LE(cap, INIT_CAPACITY);
	ASSERT_EQ(pool_.capacity, INIT_CAPACITY);

	cap = 16;
	ret = muggle_memory_pool_ensure_space(&pool_, cap);
	ASSERT_TRUE(ret);
	ASSERT_GT(cap, INIT_CAPACITY);
	ASSERT_EQ(pool_.capacity, 16);
}

TEST_F(TestMemoryPoolFixture, ensure_space2)
{
	bool ret = true;
	unsigned int cap = 0;

	cap = 4;
	ret = muggle_memory_pool_ensure_space(&pool_, cap);
	ASSERT_TRUE(ret);
	ASSERT_LE(cap, INIT_CAPACITY);
	ASSERT_EQ(pool_.capacity, INIT_CAPACITY);

	cap = 8;
	ret = muggle_memory_pool_ensure_space(&pool_, cap);
	ASSERT_TRUE(ret);
	ASSERT_LE(cap, INIT_CAPACITY);
	ASSERT_EQ(pool_.capacity, INIT_CAPACITY);

	muggle_memory_pool_set_flag(&pool_, MUGGLE_MEMORY_POOL_CONSTANT_SIZE);

	cap = 16;
	ret = muggle_memory_pool_ensure_space(&pool_, cap);
	ASSERT_FALSE(ret);
	ASSERT_EQ(pool_.capacity, INIT_CAPACITY);
}

TEST_F(TestMemoryPoolFixture, alloc_free_ensure_space1)
{
	bool ret = true;
	unsigned int cap = 0;
	block_data_t *arr[INIT_CAPACITY];
	memset(arr, 0, sizeof(arr));

	// alloc_index > free_index
	/*
	 *       f   m            
	 *  [0] [x] [0] [0] [0] [0] [0] [0]
	 */
	arr[0] = (block_data_t *)muggle_memory_pool_alloc(&pool_);
	ASSERT_TRUE(arr[0] != NULL);
	arr[1] = (block_data_t *)muggle_memory_pool_alloc(&pool_);
	ASSERT_TRUE(arr[1] != NULL);
	muggle_memory_pool_free(&pool_, arr[1]);
	ASSERT_EQ(pool_.alloc_index, 2);
	ASSERT_EQ(pool_.free_index, 1);
	ASSERT_EQ(pool_.capacity, INIT_CAPACITY);

	cap = 8;
	ret = muggle_memory_pool_ensure_space(&pool_, cap);
	ASSERT_TRUE(ret);
	ASSERT_LE(cap, INIT_CAPACITY);
	ASSERT_EQ(pool_.capacity, INIT_CAPACITY);

	cap = 16;
	ret = muggle_memory_pool_ensure_space(&pool_, cap);
	ASSERT_TRUE(ret);
	ASSERT_GT(cap, INIT_CAPACITY);
	ASSERT_EQ(pool_.capacity, 16);
}

TEST_F(TestMemoryPoolFixture, alloc_free_ensure_space2)
{
	bool ret = true;
	unsigned int cap = 0;
	block_data_t *arr[INIT_CAPACITY];
	memset(arr, 0, sizeof(arr));

	// free_index > alloc_index
	/*
	 *   m       f            
	 *  [0] [0] [x] [x] [x] [x] [x] [x]
	 */
	for (int i = 0; i < INIT_CAPACITY; i++) {
		arr[i] = (block_data_t *)muggle_memory_pool_alloc(&pool_);
		ASSERT_TRUE(arr[i] != NULL);
	}
	muggle_memory_pool_free(&pool_, arr[0]);
	muggle_memory_pool_free(&pool_, arr[1]);
	ASSERT_EQ(pool_.alloc_index, 0);
	ASSERT_EQ(pool_.free_index, 2);
	ASSERT_EQ(pool_.capacity, INIT_CAPACITY);

	cap = 8;
	ret = muggle_memory_pool_ensure_space(&pool_, cap);
	ASSERT_TRUE(ret);
	ASSERT_LE(cap, INIT_CAPACITY);
	ASSERT_EQ(pool_.capacity, INIT_CAPACITY);

	cap = 16;
	ret = muggle_memory_pool_ensure_space(&pool_, cap);
	ASSERT_TRUE(ret);
	ASSERT_GT(cap, INIT_CAPACITY);
	ASSERT_EQ(pool_.capacity, 16);
}

TEST_F(TestMemoryPoolFixture, alloc_free_ensure_space3)
{
	bool ret = true;
	unsigned int cap = 0;
	block_data_t *arr[INIT_CAPACITY];
	memset(arr, 0, sizeof(arr));

	// free_index = alloc_index && used == capacity
	/*
	 *          fm             
	 *  [x] [x] [x] [x] [x] [x] [x] [x]
	 */
	for (int i = 0; i < INIT_CAPACITY; i++) {
		arr[i] = (block_data_t *)muggle_memory_pool_alloc(&pool_);
		ASSERT_TRUE(arr[i] != NULL);
	}
	muggle_memory_pool_free(&pool_, arr[0]);
	muggle_memory_pool_free(&pool_, arr[1]);
	arr[0] = (block_data_t *)muggle_memory_pool_alloc(&pool_);
	ASSERT_TRUE(arr[0] != NULL);
	arr[1] = (block_data_t *)muggle_memory_pool_alloc(&pool_);
	ASSERT_TRUE(arr[1] != NULL);

	ASSERT_EQ(pool_.alloc_index, 2);
	ASSERT_EQ(pool_.free_index, 2);
	ASSERT_EQ(pool_.used, pool_.capacity);
	ASSERT_EQ(pool_.capacity, INIT_CAPACITY);

	cap = 8;
	ret = muggle_memory_pool_ensure_space(&pool_, cap);
	ASSERT_TRUE(ret);
	ASSERT_LE(cap, INIT_CAPACITY);
	ASSERT_EQ(pool_.capacity, INIT_CAPACITY);

	cap = 16;
	ret = muggle_memory_pool_ensure_space(&pool_, cap);
	ASSERT_TRUE(ret);
	ASSERT_GT(cap, INIT_CAPACITY);
	ASSERT_EQ(pool_.capacity, 16);
}

TEST_F(TestMemoryPoolFixture, alloc_free_ensure_space4)
{
	bool ret = true;
	unsigned int cap = 0;
	block_data_t *arr[INIT_CAPACITY];
	memset(arr, 0, sizeof(arr));

	// free_index = alloc_index && used == capacity
	/*
	 *          fm             
	 *  [0] [0] [0] [0] [0] [0] [0] [0]
	 */
	for (int i = 0; i < INIT_CAPACITY; i++) {
		arr[i] = (block_data_t *)muggle_memory_pool_alloc(&pool_);
		ASSERT_TRUE(arr[i] != NULL);
	}
	muggle_memory_pool_free(&pool_, arr[0]);
	muggle_memory_pool_free(&pool_, arr[1]);
	arr[0] = (block_data_t *)muggle_memory_pool_alloc(&pool_);
	ASSERT_TRUE(arr[0] != NULL);
	arr[1] = (block_data_t *)muggle_memory_pool_alloc(&pool_);
	ASSERT_TRUE(arr[1] != NULL);
	for (int i = 0; i < INIT_CAPACITY; i++) {
		int idx = (i + 2) % INIT_CAPACITY;
		muggle_memory_pool_free(&pool_, arr[idx]);
	}

	ASSERT_EQ(pool_.alloc_index, 2);
	ASSERT_EQ(pool_.free_index, 2);
	ASSERT_EQ(pool_.used, 0);
	ASSERT_EQ(pool_.capacity, INIT_CAPACITY);

	cap = 8;
	ret = muggle_memory_pool_ensure_space(&pool_, cap);
	ASSERT_TRUE(ret);
	ASSERT_LE(cap, INIT_CAPACITY);
	ASSERT_EQ(pool_.capacity, INIT_CAPACITY);

	cap = 16;
	ret = muggle_memory_pool_ensure_space(&pool_, cap);
	ASSERT_TRUE(ret);
	ASSERT_GT(cap, INIT_CAPACITY);
	ASSERT_EQ(pool_.capacity, 16);
}

TEST_F(TestMemoryPoolFixture, alloc_increase_space1)
{
	block_data_t *data = NULL;

	for (int i = 0; i < INIT_CAPACITY; i++) {
		data = (block_data_t *)muggle_memory_pool_alloc(&pool_);
		ASSERT_TRUE(data != NULL);
		ASSERT_EQ(pool_.capacity, INIT_CAPACITY);
	}

	for (int i = 0; i < INIT_CAPACITY; i++) {
		data = (block_data_t *)muggle_memory_pool_alloc(&pool_);
		ASSERT_TRUE(data != NULL);
		ASSERT_EQ(pool_.capacity, 2 * INIT_CAPACITY);
	}

	for (int i = 0; i < 2 * INIT_CAPACITY; i++) {
		data = (block_data_t *)muggle_memory_pool_alloc(&pool_);
		ASSERT_TRUE(data != NULL);
		ASSERT_EQ(pool_.capacity, 4 * INIT_CAPACITY);
	}

	for (int i = 0; i < 4 * INIT_CAPACITY; i++) {
		data = (block_data_t *)muggle_memory_pool_alloc(&pool_);
		ASSERT_TRUE(data != NULL);
		ASSERT_EQ(pool_.capacity, 8 * INIT_CAPACITY);
	}
}

TEST_F(TestMemoryPoolFixture, alloc_increase_space2)
{
	block_data_t *data = NULL;

	muggle_memory_pool_set_max_delta_cap(&pool_, 1);

	for (int i = 0; i < INIT_CAPACITY; i++) {
		data = (block_data_t *)muggle_memory_pool_alloc(&pool_);
		ASSERT_TRUE(data != NULL);
		ASSERT_EQ(pool_.capacity, INIT_CAPACITY);
	}

	for (int i = 0; i < INIT_CAPACITY; i++) {
		data = (block_data_t *)muggle_memory_pool_alloc(&pool_);
		ASSERT_TRUE(data != NULL);
		ASSERT_EQ(pool_.capacity, INIT_CAPACITY + i + 1);
	}
}

TEST_F(TestMemoryPoolFixture, alloc_increase_space3)
{
	block_data_t *data = NULL;

	muggle_memory_pool_set_max_delta_cap(&pool_, 8);

	for (int i = 0; i < INIT_CAPACITY; i++) {
		data = (block_data_t *)muggle_memory_pool_alloc(&pool_);
		ASSERT_TRUE(data != NULL);
		ASSERT_EQ(pool_.capacity, INIT_CAPACITY);
	}

	for (int i = 0; i < INIT_CAPACITY; i++) {
		data = (block_data_t *)muggle_memory_pool_alloc(&pool_);
		ASSERT_TRUE(data != NULL);
		ASSERT_EQ(pool_.capacity, 2 * INIT_CAPACITY);
	}

	for (int i = 0; i < INIT_CAPACITY; i++) {
		data = (block_data_t *)muggle_memory_pool_alloc(&pool_);
		ASSERT_TRUE(data != NULL);
		ASSERT_EQ(pool_.capacity, 3 * INIT_CAPACITY);
	}

	for (int i = 0; i < INIT_CAPACITY; i++) {
		data = (block_data_t *)muggle_memory_pool_alloc(&pool_);
		ASSERT_TRUE(data != NULL);
		ASSERT_EQ(pool_.capacity, 4 * INIT_CAPACITY);
	}
}
