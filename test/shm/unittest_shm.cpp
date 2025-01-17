#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

class ShmFixture : public testing::Test {
public:
	virtual void SetUp() override
	{
		muggle_shm_t shm;
		void *ptr = NULL;

		k_name = "/tmp/";
		k_num = 1;

		// ensure previous shm closed
		ptr = muggle_shm_open(&shm, k_name, k_num, MUGGLE_SHM_FLAG_OPEN, 0);
		if (ptr) {
			muggle_shm_detach(&shm);
			muggle_shm_rm(&shm);
		}
	}

	virtual void TearDown() override
	{
	}

public:
	const char *k_name;
	int k_num;
	void *ptr_;
};

TEST_F(ShmFixture, repeated_create)
{
	int ret = 0;
	muggle_shm_t shm;

	void *ptr =
		muggle_shm_open(&shm, k_name, k_num, MUGGLE_SHM_FLAG_CREAT, 4096);
	ASSERT_TRUE(ptr != NULL);

#if MUGGLE_PLATFORM_WINDOWS
	void *ptr2 = muggle_shm_open(&shm, k_name, k_num, MUGGLE_SHM_FLAG_CREAT, 0);
	ASSERT_TRUE(ptr2 == NULL);
#else
	ret = muggle_shm_detach(&shm);
	ASSERT_EQ(ret, 0);

	void *ptr2 = muggle_shm_open(&shm, k_name, k_num, MUGGLE_SHM_FLAG_CREAT, 0);
	ASSERT_TRUE(ptr2 == NULL);

	ptr = muggle_shm_open(&shm, k_name, k_num, MUGGLE_SHM_FLAG_OPEN, 4096);
	ASSERT_TRUE(ptr != NULL);
#endif

	// cleanup
	ret = muggle_shm_detach(&shm);
	ASSERT_EQ(ret, 0);
	ret = muggle_shm_rm(&shm);
	ASSERT_EQ(ret, 0);
}

TEST_F(ShmFixture, open_size_zero)
{
	int ret = 0;
	muggle_shm_t shm;
	void *ptr = NULL;

	uint32_t nbytes = 4096;

	ptr = muggle_shm_open(&shm, k_name, k_num, MUGGLE_SHM_FLAG_CREAT, nbytes);
	ASSERT_TRUE(ptr != NULL);

	memcpy(ptr, &nbytes, sizeof(nbytes));

#if MUGGLE_PLATFORM_WINDOWS
	muggle_shm_t shm2;

	void *ptr2 = muggle_shm_open(&shm2, k_name, k_num, MUGGLE_SHM_FLAG_OPEN, 0);
	ASSERT_EQ(*(uint32_t *)ptr2, nbytes);

	ret = muggle_shm_detach(&shm2);
	ASSERT_EQ(ret, 0);
#else
	ret = muggle_shm_detach(&shm);
	ASSERT_EQ(ret, 0);

	ptr = muggle_shm_open(&shm, k_name, k_num, MUGGLE_SHM_FLAG_OPEN, 0);
	ASSERT_TRUE(ptr != NULL);
	ASSERT_EQ(*(uint32_t *)ptr, nbytes);
#endif

	// cleanup
	ret = muggle_shm_detach(&shm);
	ASSERT_EQ(ret, 0);
	ret = muggle_shm_rm(&shm);
	ASSERT_EQ(ret, 0);
}
