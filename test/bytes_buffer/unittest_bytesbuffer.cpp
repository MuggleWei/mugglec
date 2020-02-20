#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

TEST(bytes_buffer, empty)
{
	int capacity = 1024 * 16;
	muggle_bytes_buffer_t bytes_buf;
	bool ret = muggle_bytes_buffer_init(&bytes_buf, capacity);
	ASSERT_TRUE(ret);
	
	void *ptr;
	int n;
	int32_t ival;

	// empty status
	{
		n = muggle_bytes_buffer_writable(&bytes_buf);
		ASSERT_EQ(n, capacity - 1);

		n = muggle_bytes_buffer_readable(&bytes_buf);
		ASSERT_EQ(n, 0);

		ret = muggle_bytes_buffer_fetch(&bytes_buf, sizeof(ival), &ival);
		ASSERT_FALSE(ret);

		ret = muggle_bytes_buffer_read(&bytes_buf, sizeof(ival), &ival);
		ASSERT_FALSE(ret);

		ptr = muggle_bytes_buffer_reader_get(&bytes_buf, sizeof(ival));
		ASSERT_TRUE(ptr == NULL);
	}

	muggle_bytes_buffer_destroy(&bytes_buf);
}

TEST(bytes_buffer, w_lead_r)
{
	muggle_bytes_buffer_t bytes_buf;
	int capacity = sizeof(int32_t) * 8 + 1;
	
	bool ret = muggle_bytes_buffer_init(&bytes_buf, capacity);
	ASSERT_TRUE(ret);
	
	int n;
	int32_t ival_arr[16];
	int ival_size = sizeof(int32_t);
	for (int32_t i = 0; i < sizeof(ival_arr)/ sizeof(ival_arr[0]); ++i)
	{
		ival_arr[i] = i;
	}

	// write 1 int and read 1 int
	{
		int32_t idx = 5;
		ret = muggle_bytes_buffer_write(&bytes_buf, ival_size, &ival_arr[idx]);
		ASSERT_TRUE(ret);

		n = muggle_bytes_buffer_writable(&bytes_buf);
		ASSERT_EQ(n, capacity - 1 - ival_size);

		n = muggle_bytes_buffer_readable(&bytes_buf);
		ASSERT_EQ(n, ival_size);

		int32_t ival = 99999;
		ret = muggle_bytes_buffer_fetch(&bytes_buf, ival_size, &ival);
		ASSERT_TRUE(ret);
		ASSERT_EQ(ival, ival_arr[idx]);

		ival = 99999;
		ret = muggle_bytes_buffer_read(&bytes_buf, ival_size, &ival);
		ASSERT_TRUE(ret);
		ASSERT_EQ(ival, ival_arr[idx]);

		n = muggle_bytes_buffer_readable(&bytes_buf);
		ASSERT_EQ(n, 0);
	}

	// writer_get write 1 int and read 1 int
	{
		muggle_bytes_buffer_clear(&bytes_buf);

		int32_t idx = 5;
		int32_t *ptr = (int32_t*)muggle_bytes_buffer_writer_get(&bytes_buf, ival_size);
		ASSERT_TRUE(ptr != NULL);
		*ptr = ival_arr[idx];
		ASSERT_TRUE(ret);

		n = muggle_bytes_buffer_writable(&bytes_buf);
		ASSERT_EQ(n, capacity - 1 - ival_size);

		n = muggle_bytes_buffer_readable(&bytes_buf);
		ASSERT_EQ(n, ival_size);

		int32_t ival = 99999;
		ret = muggle_bytes_buffer_fetch(&bytes_buf, ival_size, &ival);
		ASSERT_TRUE(ret);
		ASSERT_EQ(ival, ival_arr[idx]);

		ival = 99999;
		ret = muggle_bytes_buffer_read(&bytes_buf, ival_size, &ival);
		ASSERT_TRUE(ret);
		ASSERT_EQ(ival, ival_arr[idx]);

		n = muggle_bytes_buffer_readable(&bytes_buf);
		ASSERT_EQ(n, 0);
	}

	// write 1 int and reader_get read 1 int
	{
		muggle_bytes_buffer_clear(&bytes_buf);

		int32_t idx = 5;
		ret = muggle_bytes_buffer_write(&bytes_buf, ival_size, &ival_arr[idx]);
		ASSERT_TRUE(ret);

		n = muggle_bytes_buffer_writable(&bytes_buf);
		ASSERT_EQ(n, capacity - 1 - ival_size);

		n = muggle_bytes_buffer_readable(&bytes_buf);
		ASSERT_EQ(n, ival_size);

		int32_t ival = 99999;
		ret = muggle_bytes_buffer_fetch(&bytes_buf, ival_size, &ival);
		ASSERT_TRUE(ret);
		ASSERT_EQ(ival, ival_arr[idx]);

		void *ptr = muggle_bytes_buffer_reader_get(&bytes_buf, ival_size);
		ASSERT_TRUE(ptr != NULL);
		ASSERT_EQ(*(int32_t*)ptr, ival_arr[idx]);

		n = muggle_bytes_buffer_readable(&bytes_buf);
		ASSERT_EQ(n, 0);

		n = muggle_bytes_buffer_writable(&bytes_buf);
		ASSERT_EQ(n, capacity - 1);

		ptr = muggle_bytes_buffer_writer_get(&bytes_buf, capacity - 1);
		ASSERT_TRUE(ptr == NULL);

		muggle_bytes_buffer_refresh(&bytes_buf);

		ptr = muggle_bytes_buffer_writer_get(&bytes_buf, capacity - 1);
		ASSERT_TRUE(ptr != NULL);

		n = muggle_bytes_buffer_writable(&bytes_buf);
		ASSERT_EQ(n, 0);
	}
	
	// write 2 int and read 1 int success and failed read 2 int
	{
		muggle_bytes_buffer_clear(&bytes_buf);

		int32_t idx = 5;
		ret = muggle_bytes_buffer_write(&bytes_buf, ival_size, &ival_arr[idx]);
		ASSERT_TRUE(ret);

		ret = muggle_bytes_buffer_write(&bytes_buf, ival_size, &ival_arr[idx]);
		ASSERT_TRUE(ret);

		int32_t ival = 99999;
		ret = muggle_bytes_buffer_read(&bytes_buf, ival_size, &ival);
		ASSERT_TRUE(ret);
		ASSERT_EQ(ival, ival_arr[idx]);

		int32_t tmp_arr[2];
		ret = muggle_bytes_buffer_read(&bytes_buf, ival_size * 2, tmp_arr);
		ASSERT_FALSE(ret);
	}

	// write and writer_get
	{
		muggle_bytes_buffer_clear(&bytes_buf);
		char *tmp_space = (char*)malloc(capacity * 2);

		ret = muggle_bytes_buffer_write(&bytes_buf, 4, &tmp_space);
		ASSERT_TRUE(ret);

		int32_t *ptr = (int32_t*)muggle_bytes_buffer_writer_get(&bytes_buf, capacity - 1 - 4);
		ASSERT_TRUE(ptr != NULL);

		n = muggle_bytes_buffer_writable(&bytes_buf);
		ASSERT_EQ(n, 0);

		free(tmp_space);
	}

	muggle_bytes_buffer_destroy(&bytes_buf);
}

TEST(bytes_buffer, r_lead_w)
{
	int capacity = 64;
	char space[64];

	muggle_bytes_buffer_t bytes_buf;
	bool ret = muggle_bytes_buffer_init(&bytes_buf, capacity);
	ASSERT_TRUE(ret);

	// TODO:

	muggle_bytes_buffer_destroy(&bytes_buf);
}