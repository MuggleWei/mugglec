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


TEST(channel, default_wr)
{
	int cnt_writer = (int)std::thread::hardware_concurrency() * 2;
	if (cnt_writer <= 0)
	{
		cnt_writer = 4;
	}

	test_chan(0, cnt_writer);
}

TEST(channel, default_w_busyloop_r)
{
	int cnt_writer = (int)std::thread::hardware_concurrency() * 2;
	if (cnt_writer <= 0)
	{
		cnt_writer = 4;
	}

	test_chan(MUGGLE_CHANNEL_FLAG_READ_BUSY_LOOP, cnt_writer);
}

TEST(channel, single_w_default_r)
{
	test_chan(MUGGLE_CHANNEL_FLAG_SINGLE_WRITER, 1);
}

TEST(channel, single_w_busyloop_r)
{
	test_chan(MUGGLE_CHANNEL_FLAG_SINGLE_WRITER | MUGGLE_CHANNEL_FLAG_READ_BUSY_LOOP, 1);
}
