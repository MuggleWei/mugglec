#include <thread>
#include <vector>
#include <map>
#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

struct chan_data
{
	int idx;
	int thread_idx;
	int thread_msg_idx;
};

void test_chan(int flags, int cnt_writer)
{
	muggle_atomic_int capacity = 1024 * 4;
	int cnt_msg = capacity * 32;
	muggle_atomic_int msg_idx = 0;
	chan_data *datas = (chan_data*)malloc(cnt_msg * sizeof(chan_data));

	muggle_channel_t chan;
	muggle_channel_init(&chan, capacity, flags);

	std::map<int, int> thread_cnts;
	for (int i = 0; i < cnt_writer; i++)
	{
		thread_cnts[i] = 0;
	}

	// write
	std::vector<std::thread> threads;
	for (int i = 0; i < cnt_writer; i++)
	{
		threads.push_back(std::thread([i, &chan, &msg_idx, &cnt_msg, &datas]{
			muggle_msleep(1);
			int thread_msg_idx = 0;

			muggle_atomic_int cur_idx = 0;
			while (true)
			{
				cur_idx = muggle_atomic_fetch_add(&msg_idx, 1, muggle_memory_order_relaxed);
				if (cur_idx >= cnt_msg)
				{
					break;
				}

				datas[cur_idx].idx = cur_idx;
				datas[cur_idx].thread_idx = i;
				datas[cur_idx].thread_msg_idx = thread_msg_idx++;

				while (muggle_channel_write(&chan, &datas[cur_idx]) == MUGGLE_ERR_FULL)
				{
					muggle_msleep(1);
				}
			} 
		}));
	}

	// read
	int recv_cnt = 0;
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
	for (int i = 0; i < cnt_writer; i++)
	{
		threads[i].join();
	}

	// free resource
	free(datas);
	muggle_channel_destroy(&chan);
}

int getProducerNum()
{
	int hc = (int)muggle_thread_hardware_concurrency();
	if (hc <= 0)
	{
		hc = 2;
	}

	return hc * 2;
}

TEST(channel, futex_w_futex_r)
{
	int flags = MUGGLE_CHANNEL_FLAG_WRITE_FUTEX | MUGGLE_CHANNEL_FLAG_READ_FUTEX;
	test_chan(flags, getProducerNum());
}

TEST(channel, futex_w_mutex_r)
{
	int flags = MUGGLE_CHANNEL_FLAG_WRITE_FUTEX | MUGGLE_CHANNEL_FLAG_READ_MUTEX;
	test_chan(flags, getProducerNum());
}

TEST(channel, futex_w_busy_r)
{
	int flags = MUGGLE_CHANNEL_FLAG_WRITE_FUTEX | MUGGLE_CHANNEL_FLAG_READ_BUSY;
	test_chan(flags, getProducerNum());
}

TEST(channel, mutex_w_futex_r)
{
	int flags = MUGGLE_CHANNEL_FLAG_WRITE_MUTEX | MUGGLE_CHANNEL_FLAG_READ_FUTEX;
	test_chan(flags, getProducerNum());
}

TEST(channel, mutex_w_mutex_r)
{
	int flags = MUGGLE_CHANNEL_FLAG_WRITE_MUTEX | MUGGLE_CHANNEL_FLAG_READ_MUTEX;
	test_chan(flags, getProducerNum());
}

TEST(channel, mutex_w_busy_r)
{
	int flags = MUGGLE_CHANNEL_FLAG_WRITE_MUTEX | MUGGLE_CHANNEL_FLAG_READ_BUSY;
	test_chan(flags, getProducerNum());
}

TEST(channel, spin_w_futex_r)
{
	int flags = MUGGLE_CHANNEL_FLAG_WRITE_SPIN | MUGGLE_CHANNEL_FLAG_READ_FUTEX;
	test_chan(flags, getProducerNum());
}

TEST(channel, spin_w_mutex_r)
{
	int flags = MUGGLE_CHANNEL_FLAG_WRITE_SPIN | MUGGLE_CHANNEL_FLAG_READ_MUTEX;
	test_chan(flags, getProducerNum());
}

TEST(channel, spin_w_busy_r)
{
	int flags = MUGGLE_CHANNEL_FLAG_WRITE_SPIN | MUGGLE_CHANNEL_FLAG_READ_BUSY;
	test_chan(flags, getProducerNum());
}

TEST(channel, single_w_futex_r)
{
	int flags = MUGGLE_CHANNEL_FLAG_WRITE_SINGLE | MUGGLE_CHANNEL_FLAG_READ_FUTEX;
	test_chan(flags, 1);
}

TEST(channel, single_w_mutex_r)
{
	int flags = MUGGLE_CHANNEL_FLAG_WRITE_SINGLE | MUGGLE_CHANNEL_FLAG_READ_MUTEX;
	test_chan(flags, 1);
}

TEST(channel, single_w_busy_r)
{
	int flags = MUGGLE_CHANNEL_FLAG_WRITE_SINGLE | MUGGLE_CHANNEL_FLAG_READ_BUSY;
	test_chan(flags, 1);
}
