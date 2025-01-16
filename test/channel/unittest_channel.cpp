#include <thread>
#include <vector>
#include <map>
#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

struct chan_data
{
	uint32_t idx;
	uint32_t thread_idx;
	uint32_t thread_msg_idx;
};

void test_chan(int flags, uint32_t cnt_writer)
{
	uint32_t capacity = 1024 * 4;
	uint32_t cnt_msg = capacity * 32;
	muggle_atomic_int msg_idx = 0;
	chan_data *datas = (chan_data*)malloc(cnt_msg * sizeof(chan_data));

	muggle_channel_t chan;
	muggle_channel_init(&chan, capacity, flags);

	std::map<uint32_t, uint32_t> thread_cnts;
	for (uint32_t i = 0; i < cnt_writer; i++)
	{
		thread_cnts[i] = 0;
	}

	// write
	std::vector<std::thread> threads;
	for (uint32_t i = 0; i < cnt_writer; i++)
	{
		threads.push_back(std::thread([i, &chan, &msg_idx, &cnt_msg, &datas]{
			muggle_msleep(1);
			uint32_t thread_msg_idx = 0;

			muggle_atomic_int cur_idx = 0;
			while (true)
			{
				cur_idx = muggle_atomic_fetch_add(
					&msg_idx, 1, muggle_memory_order_relaxed);
				if ((uint32_t)cur_idx >= cnt_msg)
				{
					break;
				}

				datas[cur_idx].idx = (uint32_t)cur_idx;
				datas[cur_idx].thread_idx = i;
				datas[cur_idx].thread_msg_idx = thread_msg_idx++;

				while (muggle_channel_write(&chan, &datas[cur_idx]) ==
					MUGGLE_ERR_FULL)
				{
					muggle_msleep(1);
				}
			} 
		}));
	}

	// read
	uint32_t recv_cnt = 0;
	while (true)
	{
		chan_data *data = (chan_data*)muggle_channel_read(&chan);
		if (data)
		{
			ASSERT_LT(data->thread_idx, cnt_writer);
			ASSERT_EQ(data->thread_msg_idx, thread_cnts[data->thread_idx]);
			thread_cnts[data->thread_idx]++;
			recv_cnt++;
		}

		if (recv_cnt == cnt_msg)
		{
			break;
		}
	}

	// join thread
	for (uint32_t i = 0; i < cnt_writer; i++)
	{
		threads[i].join();
	}

	// free resource
	free(datas);
	muggle_channel_destroy(&chan);
}

uint32_t getProducerNum()
{
	int hc = (int)muggle_thread_hardware_concurrency();
	if (hc <= 0)
	{
		hc = 2;
	}

	return (uint32_t)(hc * 2);
}

TEST(channel, struct_size_and_align)
{
	MUGGLE_ALIGNAS(MUGGLE_CACHE_LINE_SIZE) muggle_channel_t chan;
	muggle_channel_init(&chan, 16, 0);

	uintptr_t addr_chan = (uintptr_t)&chan;
	uintptr_t addr_write_cursor = (uintptr_t)&chan.write_cursor;
	uintptr_t addr_read_cursor = (uintptr_t)&chan.read_cursor;
	uintptr_t addr_write_spin = (uintptr_t)&chan.write_spinlock;

#if MUGGLE_C_HAVE_ALIGNAS
	ASSERT_EQ(addr_write_cursor % MUGGLE_CACHE_LINE_SIZE, 0);
	ASSERT_EQ(addr_read_cursor % MUGGLE_CACHE_LINE_SIZE, 0);
	ASSERT_EQ(addr_write_spin % MUGGLE_CACHE_LINE_SIZE, 0);
#endif

	ASSERT_GE(addr_write_cursor - addr_chan, MUGGLE_CACHE_LINE_X2_SIZE);
	ASSERT_GE(addr_read_cursor - addr_write_cursor, MUGGLE_CACHE_LINE_X2_SIZE);
	ASSERT_GE(addr_write_spin - addr_read_cursor, MUGGLE_CACHE_LINE_X2_SIZE);

	ASSERT_EQ(sizeof(muggle_channel_block_t), MUGGLE_CACHE_LINE_X2_SIZE); 
#if MUGGLE_C_HAVE_ALIGNED_ALLOC 
	uintptr_t addr_blocks0 = (uintptr_t)&chan.blocks[0];
	uintptr_t addr_blocks1 = (uintptr_t)&chan.blocks[1];

	ASSERT_EQ(addr_blocks0 % MUGGLE_CACHE_LINE_X2_SIZE, 0);
	ASSERT_EQ(addr_blocks1 - addr_blocks0, MUGGLE_CACHE_LINE_X2_SIZE);
#endif

	muggle_channel_destroy(&chan);
}

TEST(channel, sync_w_sync_r)
{
	int flags =
		MUGGLE_CHANNEL_FLAG_WRITE_SYNC |
		MUGGLE_CHANNEL_FLAG_READ_SYNC;
	test_chan(flags, getProducerNum());
}

TEST(channel, sync_w_mutex_r)
{
	int flags =
		MUGGLE_CHANNEL_FLAG_WRITE_SYNC |
		MUGGLE_CHANNEL_FLAG_READ_MUTEX;
	test_chan(flags, getProducerNum());
}

TEST(channel, sync_w_busy_r)
{
	int flags =
		MUGGLE_CHANNEL_FLAG_WRITE_SYNC |
		MUGGLE_CHANNEL_FLAG_READ_BUSY;
	test_chan(flags, getProducerNum());
}

TEST(channel, mutex_w_sync_r)
{
	int flags =
		MUGGLE_CHANNEL_FLAG_WRITE_MUTEX |
		MUGGLE_CHANNEL_FLAG_READ_SYNC;
	test_chan(flags, getProducerNum());
}

TEST(channel, mutex_w_mutex_r)
{
	int flags =
		MUGGLE_CHANNEL_FLAG_WRITE_MUTEX |
		MUGGLE_CHANNEL_FLAG_READ_MUTEX;
	test_chan(flags, getProducerNum());
}

TEST(channel, mutex_w_busy_r)
{
	int flags =
		MUGGLE_CHANNEL_FLAG_WRITE_MUTEX |
		MUGGLE_CHANNEL_FLAG_READ_BUSY;
	test_chan(flags, getProducerNum());
}

TEST(channel, spin_w_sync_r)
{
	int flags =
		MUGGLE_CHANNEL_FLAG_WRITE_SPIN |
		MUGGLE_CHANNEL_FLAG_READ_SYNC;
	test_chan(flags, getProducerNum());
}

TEST(channel, spin_w_mutex_r)
{
	int flags =
		MUGGLE_CHANNEL_FLAG_WRITE_SPIN |
		MUGGLE_CHANNEL_FLAG_READ_MUTEX;
	test_chan(flags, getProducerNum());
}

TEST(channel, spin_w_busy_r)
{
	int flags =
		MUGGLE_CHANNEL_FLAG_WRITE_SPIN |
		MUGGLE_CHANNEL_FLAG_READ_BUSY;
	test_chan(flags, getProducerNum());
}

TEST(channel, single_w_sync_r)
{
	int flags =
		MUGGLE_CHANNEL_FLAG_WRITE_SINGLE |
		MUGGLE_CHANNEL_FLAG_READ_SYNC;
	test_chan(flags, 1);
}

TEST(channel, single_w_mutex_r)
{
	int flags =
		MUGGLE_CHANNEL_FLAG_WRITE_SINGLE |
		MUGGLE_CHANNEL_FLAG_READ_MUTEX;
	test_chan(flags, 1);
}

TEST(channel, single_w_busy_r)
{
	int flags =
		MUGGLE_CHANNEL_FLAG_WRITE_SINGLE |
		MUGGLE_CHANNEL_FLAG_READ_BUSY;
	test_chan(flags, 1);
}
