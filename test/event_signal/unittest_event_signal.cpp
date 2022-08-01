#include <vector>
#include <thread>
#include <chrono>
#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

#define PRODUCER_NUM 16

TEST(event_signal, wakeup_clearup)
{
	// init event lib
	muggle_event_lib_init();

	// init event signal
	muggle_event_signal ev_signal;
	int ret = muggle_event_signal_init(&ev_signal);
	ASSERT_EQ(ret, 0);

	muggle_atomic_int completed = 0;

	// producer
	int producer_cnt[PRODUCER_NUM];
	memset(producer_cnt, 0, sizeof(producer_cnt));

	std::vector<std::thread*> threads;
	for (int i = 0; i < PRODUCER_NUM; i++)
	{
		threads.push_back(new std::thread([i, &producer_cnt, &completed, &ev_signal]{
			int &cnt = producer_cnt[i];
			do {
				int ret = muggle_event_signal_wakeup(&ev_signal);
				ASSERT_NE(ret, MUGGLE_EVENT_ERROR);
				++cnt;
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
			} while (muggle_atomic_load(&completed, muggle_memory_order_relaxed) == 0);
		}));
	}

	// consumer
	int n = 0;
	int consumer_cnt = 0;
	while (1)
	{
		n = muggle_event_signal_clearup(&ev_signal);

		ASSERT_NE(n, MUGGLE_EVENT_ERROR);
		consumer_cnt += n;
		if (consumer_cnt >= 512)
		{
			break;
		}
		std::this_thread::sleep_for(std::chrono::microseconds(200));
	}

	// notify producer stop
	muggle_atomic_store(&completed, 1, muggle_memory_order_relaxed);

	// producer join
	for (auto th : threads)
	{
		th->join();
		delete th;
	}
	threads.clear();

	// clearup event signal
	n = muggle_event_signal_clearup(&ev_signal);
	ASSERT_NE(n, MUGGLE_EVENT_ERROR);
	consumer_cnt += n;

	int total_producer_cnt = 0;
	for (int i = 0; i < PRODUCER_NUM; i++)
	{
		total_producer_cnt += producer_cnt[i];
	}
	ASSERT_EQ(total_producer_cnt, consumer_cnt);

	// destroy event signal
	muggle_event_signal_destroy(&ev_signal);
}
