#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"
#include <mutex>
#include <thread>

typedef struct {
	uint32_t magic_num;
	uint32_t val;
} pipe_data_t;

std::once_flag init_socket_flag;

class TestEventPipeFixture : public ::testing::Test {
public:
	virtual void SetUp() override
	{
		std::call_once(init_socket_flag, []() { muggle_socket_lib_init(); });

		muggle_socket_evloop_pipe_init(&ev_pipe);

		num = 1024;
		arr = (pipe_data_t *)malloc(sizeof(pipe_data_t) * num);
		for (size_t i = 0; i < num; ++i) {
			arr[i].magic_num = 0xFF00FF00;
			arr[i].val = (uint32_t)i;
		}
		batch_n = 1024;
	}

	virtual void TearDown() override
	{
		muggle_socket_evloop_pipe_destroy(&ev_pipe);

		if (arr) {
			free(arr);
			arr = nullptr;
		}
	}

	void batch_read(int &cnt_read, bool &is_end)
	{
		const int cap = 128;
		void *ret[cap];
		int offset = 0;
		int remain = cap * (int)sizeof(void *);
		int n = 0;
		while (!is_end) {
			char *p = (char *)ret + offset;
			n = muggle_socket_evloop_pipe_read_n(&ev_pipe, p, remain);
			if (n <= 0) {
				if (offset % sizeof(void *) != 0) {
					remain = sizeof(void *) - (offset % sizeof(void *));
					n = muggle_socket_evloop_pipe_block_read_n(&ev_pipe, p,
															   remain);
					ASSERT_EQ(n, remain);
					offset += n;
				}

				break;
			}

			offset += n;
			remain -= n;
			if (remain == 0) {
				for (int i = 0; i < cap; ++i) {
					void *data = ret[i];
					if (data == &end_data) {
						is_end = true;
						return;
					}

					int idx = cnt_read % num;
					void *expect_data = &arr[idx];
					ASSERT_EQ(data, expect_data);

					++cnt_read;
				}

				offset = 0;
				remain = cap * sizeof(void *);
			}
		}

		for (size_t i = 0; i < offset / sizeof(void *); ++i) {
			void *data = ret[i];
			if (data == &end_data) {
				is_end = true;
				return;
			}

			int idx = cnt_read % num;
			void *expect_data = &arr[idx];
			ASSERT_EQ(data, expect_data);

			++cnt_read;
		}
	}

public:
	muggle_socket_evloop_pipe_t ev_pipe;
	pipe_data_t end_data;
	pipe_data_t *arr;
	size_t num;
	int batch_n;
};

// write and read data one by one in the same thread
TEST_F(TestEventPipeFixture, case01)
{
	bool ret = true;
	pipe_data_t *data = nullptr;

	ret = muggle_socket_evloop_pipe_write(&ev_pipe, &arr[0]);
	ASSERT_TRUE(ret);
	ret = muggle_socket_evloop_pipe_write(&ev_pipe, &end_data);
	ASSERT_TRUE(ret);

	data = (pipe_data_t *)muggle_socket_evloop_pipe_read(&ev_pipe);
	ASSERT_TRUE(data != nullptr);
	ASSERT_EQ(data, &arr[0]);

	data = (pipe_data_t *)muggle_socket_evloop_pipe_read(&ev_pipe);
	ASSERT_TRUE(data != nullptr);
	ASSERT_EQ(data, &end_data);

	data = (pipe_data_t *)muggle_socket_evloop_pipe_read(&ev_pipe);
	ASSERT_TRUE(data == nullptr);
}

// write and batch read data in the same thread
TEST_F(TestEventPipeFixture, case02)
{
	bool ret = true;

	ret = muggle_socket_evloop_pipe_write(&ev_pipe, &arr[0]);
	ASSERT_TRUE(ret);
	ret = muggle_socket_evloop_pipe_write(&ev_pipe, &end_data);
	ASSERT_TRUE(ret);

	int cnt_read = 0;
	bool is_end = false;
	batch_read(cnt_read, is_end);
	ASSERT_EQ(cnt_read, 1);
	ASSERT_TRUE(is_end);
}

// write and read data one by one in the different thread
TEST_F(TestEventPipeFixture, case03)
{
	bool ret = true;

	ret = muggle_socket_evloop_pipe_write(&ev_pipe, &arr[0]);
	ASSERT_TRUE(ret);
	ret = muggle_socket_evloop_pipe_write(&ev_pipe, &end_data);
	ASSERT_TRUE(ret);

	std::thread th_consumer([this] {
		pipe_data_t *data = nullptr;
		data = (pipe_data_t *)muggle_socket_evloop_pipe_read(&ev_pipe);
		ASSERT_TRUE(data != nullptr);
		ASSERT_EQ(data, &arr[0]);

		data = (pipe_data_t *)muggle_socket_evloop_pipe_read(&ev_pipe);
		ASSERT_TRUE(data != nullptr);
		ASSERT_EQ(data, &end_data);

		data = (pipe_data_t *)muggle_socket_evloop_pipe_read(&ev_pipe);
		ASSERT_TRUE(data == nullptr);
	});
	th_consumer.join();
}

// write and batch read data in the different thread
TEST_F(TestEventPipeFixture, case04)
{
	bool ret = true;

	ret = muggle_socket_evloop_pipe_write(&ev_pipe, &arr[0]);
	ASSERT_TRUE(ret);
	ret = muggle_socket_evloop_pipe_write(&ev_pipe, &end_data);
	ASSERT_TRUE(ret);

	std::thread th_consumer([this] {
		int cnt_read = 0;
		bool is_end = false;
		batch_read(cnt_read, is_end);
		ASSERT_EQ(cnt_read, 1);
		ASSERT_TRUE(is_end);
	});
	th_consumer.join();
}

// write double size of read size, read one by one
TEST_F(TestEventPipeFixture, case05)
{
	bool ret = true;
	int cnt_read = 0;
	int cnt_write = 0;

	int rcv_size = muggle_socket_evloop_pipe_get_r_size(&ev_pipe);
	for (int i = 0; i < rcv_size / (int)sizeof(void *); ++i) {
		int idx = cnt_write % num;
		ret = muggle_socket_evloop_pipe_write(&ev_pipe, &arr[idx]);
		ASSERT_TRUE(ret);
		++cnt_write;
	}

	std::thread th_consumer([this, &cnt_read] {
		muggle_msleep(5);

		pipe_data_t *data = nullptr;
		pipe_data_t *expect_data = nullptr;
		while (true) {
			int idx = cnt_read % num;

			data = (pipe_data_t *)muggle_socket_evloop_pipe_read(&ev_pipe);
			if (data == nullptr) {
				continue;
			}

			expect_data = &arr[idx];
			if (data == &end_data) {
				break;
			}
			ASSERT_EQ(data, expect_data);

			++cnt_read;
		}
	});

	for (int i = 0; i < rcv_size / (int)sizeof(void *); ++i) {
		int idx = cnt_write % num;
		ret = muggle_socket_evloop_pipe_write(&ev_pipe, &arr[idx]);
		ASSERT_TRUE(ret);
		++cnt_write;
	}

	ret = muggle_socket_evloop_pipe_write(&ev_pipe, &end_data);
	ASSERT_TRUE(ret);

	th_consumer.join();

	ASSERT_EQ(cnt_read, cnt_write);
}

// write double size of read size, batch read
TEST_F(TestEventPipeFixture, case06)
{
	bool ret = true;
	int cnt_read = 0;
	int cnt_write = 0;

	int rcv_size = muggle_socket_evloop_pipe_get_r_size(&ev_pipe);
	for (int i = 0; i < rcv_size / (int)sizeof(void *); ++i) {
		int idx = cnt_write % num;
		ret = muggle_socket_evloop_pipe_write(&ev_pipe, &arr[idx]);
		ASSERT_TRUE(ret);
		++cnt_write;
	}

	std::thread th_consumer([this, &cnt_read] {
		muggle_msleep(5);

		bool is_end = false;
		while (!is_end) {
			batch_read(cnt_read, is_end);
		}
		ASSERT_TRUE(is_end);
	});

	for (int i = 0; i < rcv_size / (int)sizeof(void *); ++i) {
		int idx = cnt_write % num;
		ret = muggle_socket_evloop_pipe_write(&ev_pipe, &arr[idx]);
		ASSERT_TRUE(ret);
		++cnt_write;
	}

	ret = muggle_socket_evloop_pipe_write(&ev_pipe, &end_data);
	ASSERT_TRUE(ret);

	th_consumer.join();

	ASSERT_EQ(cnt_read, cnt_write);
}

// half write and read one by one
TEST_F(TestEventPipeFixture, case07)
{
	bool ret = true;
	int n = 0;

	muggle_socket_context_t *writer =
		muggle_socket_evloop_pipe_get_writer(&ev_pipe);

	char *p = (char *)&arr[0];

	int len1 = 0;
	int len2 = 0;
	if (sizeof(void *) == 8) {
		len1 = 2;
		len2 = 6;
	} else if (sizeof(void *) == 4) {
		len1 = 1;
		len2 = 3;
	} else {
		len1 = sizeof(void *) / 2;
		len2 = sizeof(void *) / 2;
	}

	// write half
	muggle_spinlock_lock(&ev_pipe.lock);
	muggle_atomic_thread_fence(muggle_memory_order_release);
	n = muggle_socket_block_write(writer->base.fd, &p, len1, 400);
	muggle_spinlock_unlock(&ev_pipe.lock);
	ASSERT_EQ(n, len1);

	// run consumer
	std::thread th_consumer([this] {
		pipe_data_t *data = nullptr;
		data = (pipe_data_t *)muggle_socket_evloop_pipe_read(&ev_pipe);
		ASSERT_TRUE(data != nullptr);
		ASSERT_EQ(data, &arr[0]);

		data = nullptr;
		while (data == nullptr) {
			data = (pipe_data_t *)muggle_socket_evloop_pipe_read(&ev_pipe);
		}
		ASSERT_EQ(data, &end_data);

		data = (pipe_data_t *)muggle_socket_evloop_pipe_read(&ev_pipe);
		ASSERT_TRUE(data == nullptr);
	});

	muggle_msleep(5);

	// write another half
	muggle_spinlock_lock(&ev_pipe.lock);
	muggle_atomic_thread_fence(muggle_memory_order_release);
	n = muggle_socket_block_write(writer->base.fd, (char *)&p + len1, len2,
								  400);
	muggle_spinlock_unlock(&ev_pipe.lock);
	ASSERT_EQ(n, len2);

	// write end
	ret = muggle_socket_evloop_pipe_write(&ev_pipe, &end_data);
	ASSERT_TRUE(ret);

	th_consumer.join();
}

// half write and batch read
TEST_F(TestEventPipeFixture, case08)
{
	bool ret = true;
	int n = 0;

	muggle_socket_context_t *writer =
		muggle_socket_evloop_pipe_get_writer(&ev_pipe);

	char *p = (char *)&arr[0];

	int len1 = 0;
	int len2 = 0;
	if (sizeof(void *) == 8) {
		len1 = 2;
		len2 = 6;
	} else if (sizeof(void *) == 4) {
		len1 = 1;
		len2 = 3;
	} else {
		len1 = sizeof(void *) / 2;
		len2 = sizeof(void *) / 2;
	}

	// write half
	muggle_spinlock_lock(&ev_pipe.lock);
	muggle_atomic_thread_fence(muggle_memory_order_release);
	n = muggle_socket_block_write(writer->base.fd, &p, len1, 400);
	muggle_spinlock_unlock(&ev_pipe.lock);
	ASSERT_EQ(n, len1);

	// run consumer
	std::thread th_consumer([this] {
		int cnt_read = 0;
		bool is_end = false;
		while (!is_end) {
			batch_read(cnt_read, is_end);
		}
		ASSERT_EQ(cnt_read, 1);
		ASSERT_TRUE(is_end);
	});

	muggle_msleep(5);

	// write another half
	muggle_spinlock_lock(&ev_pipe.lock);
	muggle_atomic_thread_fence(muggle_memory_order_release);
	n = muggle_socket_block_write(writer->base.fd, (char *)&p + len1, len2,
								  400);
	muggle_spinlock_unlock(&ev_pipe.lock);
	ASSERT_EQ(n, len2);

	// write end
	ret = muggle_socket_evloop_pipe_write(&ev_pipe, &end_data);
	ASSERT_TRUE(ret);

	th_consumer.join();
}
