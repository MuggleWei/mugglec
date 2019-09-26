#include <thread>
#include <chrono>
#include <vector>
#include "muggle/c/muggle_c.h"

#define ASSERT_GE(x, y) if ((x) < (y)) printf("expect %d >= %d\n", (x), (y))
#define EXPECT_EQ(x, y) if ((x) != (y)) printf("expect %d == %d\n", (x), (y))

int suggest_w_flags[] = {
	MUGGLE_RINGBUFFER_FLAG_WRITE_LOCK,
	MUGGLE_RINGBUFFER_FLAG_SINGLE_WRITER,
	MUGGLE_RINGBUFFER_FLAG_WRITE_BUSY_LOOP
};
int suggest_r_flags[] = {
	MUGGLE_RINGBUFFER_FLAG_READ_ALL | MUGGLE_RINGBUFFER_FLAG_READ_WAIT,
	MUGGLE_RINGBUFFER_FLAG_READ_ALL | MUGGLE_RINGBUFFER_FLAG_READ_BUSY_LOOP,
	MUGGLE_RINGBUFFER_FLAG_SINGLE_READER | MUGGLE_RINGBUFFER_FLAG_READ_BUSY_LOOP,
	MUGGLE_RINGBUFFER_FLAG_SINGLE_READER | MUGGLE_RINGBUFFER_FLAG_READ_WAIT,
	MUGGLE_RINGBUFFER_FLAG_MSG_READ_ONCE 
};
int invalid_r_flags[] = {
	MUGGLE_RINGBUFFER_FLAG_MSG_READ_ONCE | MUGGLE_RINGBUFFER_FLAG_READ_BUSY_LOOP
};

void test_write_read_in_single_thread(int flag)
{
	muggle_ringbuffer_t r;
	muggle_atomic_int capacity = 16;
	muggle_atomic_int pos = 0;
	int arr[160];

	muggle_ringbuffer_init(&r, capacity, flag);

	printf("write mode: %d, read mode: %d\n", r.write_mode, r.read_mode);

	// push capacity / 2, and get all
	for (muggle_atomic_int i = 0; i < capacity / 2; ++i)
	{
		arr[i] = (int)i;
		muggle_ringbuffer_write(&r, &arr[i]);
	}
	for (muggle_atomic_int i = 0; i < capacity / 2; ++i)
	{
		int data = *(int*)muggle_ringbuffer_read(&r, pos++);
		EXPECT_EQ(data, (int)i);
	}

	// push one and get one
	for (muggle_atomic_int i = 0; i < capacity * 5; ++i)
	{
		arr[i] = (int)i;
		muggle_ringbuffer_write(&r, &arr[i]);

		int data = *(int*)muggle_ringbuffer_read(&r, pos++);
		EXPECT_EQ(data, (int)i);
	}

	muggle_ringbuffer_destroy(&r);
	printf("complete\n");
}

void get_case_name(char *buf, size_t max_len, int cnt_producer, int cnt_consumer, int w_mode, int r_mode)
{
	const char *str_w_mode[] = {
		"lock",
		"single",
		"busy_loop"
	};
	const char *str_r_mode[] = {
		"wait",
		"single_wait",
		"busy_loop",
		"lock"
	};

	snprintf(buf, max_len, "%dw%s-%dr%s",
		cnt_producer, str_w_mode[w_mode], 
		cnt_consumer, str_r_mode[r_mode]);
}


void producer_consumer(int flag, int cnt_producer, int cnt_consumer, int cnt_interval, int interval_ms)
{
	muggle_ringbuffer_t r;
	muggle_atomic_int capacity = 1024 * 16;
	muggle_atomic_int total = 1024 * 100;
	int *arr = (int*)malloc(sizeof(int) * total);
	for (int i = 0; i < total; ++i)
	{
		arr[i] = i;
	}
	muggle_atomic_int consumer_ready = 0;
	muggle_ringbuffer_init(&r, capacity, flag);
	char case_name[1024];
	get_case_name(case_name, sizeof(case_name)-1, cnt_producer, cnt_consumer, r.write_mode, r.read_mode);

	printf("launch %s\n", case_name);

	std::vector<std::thread> consumers;
	for (int i = 0; i < cnt_consumer; ++i)
	{
		consumers.push_back(std::thread([&, i]{
			muggle_atomic_fetch_add(&consumer_ready, 1, muggle_memory_order_relaxed);

			muggle_atomic_int pos = 0;
			int recv_idx = 0;
			int cnt = 0;
			while (1)
			{
				void *data = muggle_ringbuffer_read(&r, pos++);
				if (data == nullptr)
				{
					break;
				}

				if (cnt_producer == 1)
				{
					if (flag & MUGGLE_RINGBUFFER_FLAG_MSG_READ_ONCE)
					{
						ASSERT_GE(*(int*)data, recv_idx);
					}
					else
					{
						EXPECT_EQ(*(int*)data, recv_idx);
					}
				}

				if (*(int*)data != recv_idx)
				{
					// message loss
					recv_idx = *(int*)data;
				}
				++recv_idx;
				++cnt;
			}

			printf("consumer[%d] %d\n", i, cnt);
		}));
	}

	std::vector<std::thread> producers;
	muggle_atomic_int fetch_id = 0;
	for (int i = 0; i < cnt_producer; ++i)
	{
		producers.push_back(std::thread([&, i]{
			while (muggle_atomic_load(&consumer_ready, muggle_memory_order_acquire) != cnt_consumer);

			muggle_atomic_int idx;
			int cnt = 0;
			while (1)
			{
				idx = muggle_atomic_fetch_add(&fetch_id, 1, muggle_memory_order_relaxed);
				if (idx == total - 1)
				{
					if (flag & MUGGLE_RINGBUFFER_FLAG_MSG_READ_ONCE)
					{
						for (int i = 0; i < cnt_consumer; ++i)
						{
							muggle_ringbuffer_write(&r, nullptr);
						}
					}
					else
					{
						muggle_ringbuffer_write(&r, nullptr);
					}
					break;
				}

				if (idx < total)
				{
					muggle_ringbuffer_write(&r, &arr[idx]);
				}
				else
				{
					break;
				}

				if (idx == cnt_interval)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
				}

				++cnt;
			}
			printf("producer [%d] %d\n", i, cnt);
		}));
	}

	for (auto &producer : producers)
	{
		producer.join();
	}
	for (auto &consumer : consumers)
	{
		consumer.join();
	}

	muggle_ringbuffer_destroy(&r);
	free(arr);

	printf("complete\n");
}

int main()
{
	int hc = (int)std::thread::hardware_concurrency();
	hc /= 2;
	if (hc <= 1)
	{
		hc = 2;
	}

	int cnt_interval = 1024;
	int interval_ms = 1;

	for (int w_flag = 0; w_flag < (int)(sizeof(suggest_w_flags) / sizeof(suggest_w_flags[0])); ++w_flag)
	{
		for (int r_flag = 0; r_flag < (int)(sizeof(suggest_r_flags) / sizeof(suggest_r_flags[0])); ++r_flag)
		{
			test_write_read_in_single_thread(suggest_w_flags[w_flag] | suggest_r_flags[r_flag]);

			printf("flag: %x | %x, p(%d), c(%d)\n", suggest_w_flags[w_flag], suggest_r_flags[r_flag], 1, 1);
			producer_consumer(suggest_w_flags[w_flag] | suggest_r_flags[r_flag], 1, 1, cnt_interval, interval_ms);

			printf("flag: %x | %x, p(%d), c(%d)\n", suggest_w_flags[w_flag], suggest_r_flags[r_flag], 1, hc);
			producer_consumer(suggest_w_flags[w_flag] | suggest_r_flags[r_flag], 1, hc, cnt_interval, interval_ms);

			printf("flag: %x | %x, p(%d), c(%d)\n", suggest_w_flags[w_flag], suggest_r_flags[r_flag], hc, 1);
			producer_consumer(suggest_w_flags[w_flag] | suggest_r_flags[r_flag], hc, 1, cnt_interval, interval_ms);

			printf("flag: %x | %x, p(%d), c(%d)\n", suggest_w_flags[w_flag], suggest_r_flags[r_flag], hc, hc);
			producer_consumer(suggest_w_flags[w_flag] | suggest_r_flags[r_flag], hc, hc, cnt_interval, interval_ms);
		}
	}

	return 0;
}
