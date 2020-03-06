/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#if MUGGLE_BUILD_C_ONLY

#include "muggle/c/muggle_c.h"

int main()
{
	muggle_log_simple_init(MUGGLE_LOG_LEVEL_ERROR, MUGGLE_LOG_LEVEL_ERROR);
	MUGGLE_LOG_ERROR("benchmark_memory_pool not run in BUILD_C_ONLY");

	return 0;
}

#else

#include <stdint.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <future>
#include <vector>
#include <functional>
#include "muggle/c/memory/memory_detect.h"
#include "muggle/cpp/memory/memory_pool.h"
#include "muggle/cpp/memory/thread_safe_memory_pool.h"

typedef std::chrono::duration<double, std::milli> double_dura;
typedef std::function<void(intptr_t*, int, unsigned int)> allocFunc;
typedef std::function<void(intptr_t*, int)> freeFunc;
typedef std::function<void(intptr_t*, int, unsigned int, int*)> randFunc;

double invokeAlloc(
	intptr_t *buf, int times, unsigned int block_size,
	allocFunc &func)
{
	auto start = std::chrono::high_resolution_clock::now();

	func(buf, times, block_size);

	auto end = std::chrono::high_resolution_clock::now();

	return std::chrono::duration_cast<double_dura>(end - start).count();
}
double invokeFree(intptr_t* buf, int times, freeFunc &func)
{
	auto start = std::chrono::high_resolution_clock::now();

	func(buf, times);

	auto end = std::chrono::high_resolution_clock::now();

	return std::chrono::duration_cast<double_dura>(end - start).count();
}
double invokeRand(intptr_t *buf, int times, unsigned int block_size,
	int* op_flag, randFunc &func)
{
	auto start = std::chrono::high_resolution_clock::now();

	func(buf, times, block_size, op_flag);

	auto end = std::chrono::high_resolution_clock::now();

	return std::chrono::duration_cast<double_dura>(end - start).count();
}

void originAlloc(intptr_t *buf, int times, unsigned int block_size)
{
	for (int i = 0; i < times; ++i)
	{
		buf[i] = (intptr_t)malloc(block_size);
	}
}
void originFree(intptr_t* buf, int times)
{
	for (int i = 0; i < times; ++i)
	{
		free((void*)buf[i]);
	}
}
void originRand(intptr_t *buf, int times, unsigned int block_size, int *op_flag)
{
	int idx = 0;
	for (int i = 0; i < times; ++i)
	{
		if (op_flag[i] == 1)
		{
			buf[idx++] = (intptr_t)malloc(block_size);
		}
		else
		{
			free((void*)buf[--idx]);
		}
	}

	for (int i = 0; i < idx; ++i)
	{
		free((void*)buf[i]);
	}
}

void memorypoolAlloc(muggle::MemoryPool &pool, intptr_t *buf, int times, unsigned int /*block_size*/)
{
	for (int i = 0; i < times; ++i)
	{
		buf[i] = (intptr_t)pool.alloc();
	}
}
void memorypoolFree(muggle::MemoryPool &pool, intptr_t* buf, int times)
{
	for (int i = 0; i < times; ++i)
	{
		pool.recycle((void*)buf[i]);
	}
}
void memorypoolRand(muggle::MemoryPool &pool, intptr_t *buf, int times, unsigned int /*block_size*/, int *op_flag)
{
	int idx = 0;
	for (int i = 0; i < times; ++i)
	{
		if (op_flag[i] == 1)
		{
			buf[idx++] = (intptr_t)pool.alloc();
		}
		else
		{
			pool.recycle((void*)buf[--idx]);
		}
	}

	for (int i = 0; i < idx; ++i)
	{
		pool.recycle((void*)buf[i]);
	}
}

void threadsafeMemorypoolAlloc(muggle::ThreadSafeMemoryPool &pool, intptr_t *buf, int times, unsigned int /*block_size*/)
{
	for (int i = 0; i < times; ++i)
	{
		buf[i] = (intptr_t)pool.alloc();
	}
}
void threadsafeMemorypoolFree(muggle::ThreadSafeMemoryPool &pool, intptr_t* buf, int times)
{
	for (int i = 0; i < times; ++i)
	{
		pool.recycle((void*)buf[i]);
	}
}
void threadsafeMemorypoolRand(muggle::ThreadSafeMemoryPool &pool, intptr_t *buf, int times, unsigned int /*block_size*/, int *op_flag)
{
	int idx = 0;
	for (int i = 0; i < times; ++i)
	{
		if (op_flag[i] == 1)
		{
			buf[idx++] = (intptr_t)pool.alloc();
		}
		else
		{
			pool.recycle((void*)buf[--idx]);
		}
	}

	for (int i = 0; i < idx; ++i)
	{
		pool.recycle((void*)buf[i]);
	}
}


void benchmarkOrigin(int times, unsigned int block_size, 
	double &alloc_ms, double &free_ms, double &rand_ms, int *op_flag)
{
	intptr_t *buf = (intptr_t*)malloc(sizeof(void*) * times);

	allocFunc alloc_func = std::bind(&originAlloc, 
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	alloc_ms = invokeAlloc(buf, times, block_size, alloc_func);

	freeFunc free_func = std::bind(&originFree, 
		std::placeholders::_1, std::placeholders::_2);
	free_ms = invokeFree(buf, times, free_func);

	randFunc rand_func = std::bind(&originRand, 
		std::placeholders::_1, std::placeholders::_2, 
		std::placeholders::_3, std::placeholders::_4);
	rand_ms = invokeRand(buf, times, block_size, op_flag, rand_func);

	free(buf);
}

void benchmarkMemoryPool(muggle::MemoryPool &pool, int times, 
	unsigned int block_size, double &alloc_ms, double &free_ms, 
	double &rand_ms, int *op_flag)
{
	intptr_t *buf = (intptr_t*)malloc(sizeof(void*) * times);

	allocFunc alloc_func = std::bind(&memorypoolAlloc, std::ref(pool), 
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	alloc_ms = invokeAlloc(buf, times, block_size, alloc_func);

	freeFunc free_func = std::bind(&memorypoolFree, std::ref(pool),
		std::placeholders::_1, std::placeholders::_2);
	free_ms = invokeFree(buf, times, free_func);

	randFunc rand_func = std::bind(&memorypoolRand, std::ref(pool),
		std::placeholders::_1, std::placeholders::_2,
		std::placeholders::_3, std::placeholders::_4);
	rand_ms = invokeRand(buf, times, block_size, op_flag, rand_func);

	free(buf);
}

void benchmarkThreadsafeMemoryPool(muggle::ThreadSafeMemoryPool &pool, 
	int times, unsigned int block_size, double &alloc_ms, double &free_ms, 
	double &rand_ms, int *op_flag)
{
	intptr_t *buf = (intptr_t*)malloc(sizeof(void*) * times);

	allocFunc alloc_func = std::bind(&threadsafeMemorypoolAlloc, std::ref(pool),
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	alloc_ms = invokeAlloc(buf, times, block_size, alloc_func);

	freeFunc free_func = std::bind(&threadsafeMemorypoolFree, std::ref(pool),
		std::placeholders::_1, std::placeholders::_2);
	free_ms = invokeFree(buf, times, free_func);

	randFunc rand_func = std::bind(&threadsafeMemorypoolRand, std::ref(pool),
		std::placeholders::_1, std::placeholders::_2,
		std::placeholders::_3, std::placeholders::_4);
	rand_ms = invokeRand(buf, times, block_size, op_flag, rand_func);

	free(buf);
}


int times[] = { 10, 100, 1000, 2000, 5000, 10000 };
unsigned int block_size[] = { 8, 16, 32, 64, 128, 512, 1024, 2048, 4096, 10240, 20480 };

const int len_time = sizeof(times) / sizeof(times[0]);
const int len_block_size = sizeof(block_size) / sizeof(block_size[0]);

int *op_flag[len_time] = { nullptr };	// operation flags: 1 - malloc, -1 - free

struct Records
{
	double alloc_records[len_time][len_block_size];
	double free_records[len_time][len_block_size];
	double rand_records[len_time][len_block_size];
};

void writeToFile(const std::string& name, double *records)
{
	std::string file_name = name + ".csv";

	std::ofstream out;
	out.open(file_name, std::ios::out);

	out << name.c_str() << ",";
	for (int i = 0; i < len_block_size; ++i)
	{
		out << block_size[i] << ",";
	}
	out << std::endl;

	for (int i = 0; i < len_time; ++i)
	{
		out << times[i] << ",";
		for (int j = 0; j < len_block_size; ++j)
		{
			out << *(records + i * len_block_size + j) << ",";
		}
		out << std::endl;
	}

	out.close();
}


template<typename T>
void shufflePool(T &pool)
{
	unsigned int capacity = pool.capacity();
	intptr_t *buf = (intptr_t*)malloc(sizeof(void*) * capacity);

	for (unsigned int j = 0; j < capacity; ++j)
	{
		buf[j] = (intptr_t)pool.alloc();
	}

	for (unsigned int j = 0; j < capacity; ++j)
	{
		int k = rand() % capacity;
		std::swap(buf[j], buf[k]);
	}

	for (unsigned int j = 0; j < capacity; ++j)
	{
		pool.recycle((void*)buf[j]);
	}

	free(buf);
}

void benchmarkSingleThread()
{
	double alloc_records[len_time][len_block_size] = {};
	double free_records[len_time][len_block_size] = {};
	double rand_records[len_time][len_block_size] = {};

	// origin
	for (int i = 0; i < len_time; ++i)
	{
		for (int j = 0; j < len_block_size; ++j)
		{
			std::cout << "single thread - malloc/free - " << times[i] << ", " << block_size[j] << std::endl;
			benchmarkOrigin(times[i], block_size[j], alloc_records[i][j], free_records[i][j], rand_records[i][j], op_flag[i]);
		}
	}
	writeToFile("origin_alloc", &alloc_records[0][0]);
	writeToFile("origin_free", &free_records[0][0]);
	writeToFile("origin_rand", &rand_records[0][0]);

	// pool
	for (int i = 0; i < len_time; ++i)
	{
		for (int j = 0; j < len_block_size; ++j)
		{
			std::cout << "single thread - pool - " << times[i] << ", " << block_size[j] << std::endl;
			muggle::MemoryPool pool(times[i], block_size[j]);
			shufflePool<muggle::MemoryPool>(pool);
			benchmarkMemoryPool(pool, times[i], block_size[j], alloc_records[i][j], 
				free_records[i][j], rand_records[i][j], op_flag[i]);
		}
	}
	writeToFile("pool_alloc", &alloc_records[0][0]);
	writeToFile("pool_free", &free_records[0][0]);
	writeToFile("pool_rand", &rand_records[0][0]);

	// thread safe pool
	for (int i = 0; i < len_time; ++i)
	{
		for (int j = 0; j < len_block_size; ++j)
		{
			std::cout << "single thread - thread safe pool - " << times[i] << ", " << block_size[j] << std::endl;
			muggle::ThreadSafeMemoryPool pool(times[i], block_size[j]);
			shufflePool<muggle::ThreadSafeMemoryPool>(pool);
			benchmarkThreadsafeMemoryPool(pool, times[i], block_size[j], alloc_records[i][j], 
				free_records[i][j], rand_records[i][j], op_flag[i]);
		}
	}
	writeToFile("threadsafe_pool_alloc", &alloc_records[0][0]);
	writeToFile("threadsafe_pool_free", &free_records[0][0]);
	writeToFile("threadsafe_pool_rand", &rand_records[0][0]);
}

void accuRecords(std::vector<Records> &records, double *alloc_records, double *free_records, double *rand_records)
{
	for (auto &rec : records)
	{
		for (int i = 0; i < len_time; ++i)
		{
			for (int j = 0; j < len_block_size; ++j)
			{
				*(alloc_records + i * len_block_size + j) += rec.alloc_records[i][j];
				*(free_records + i * len_block_size + j) += rec.free_records[i][j];
				*(rand_records + i * len_block_size + j) += rec.rand_records[i][j];
			}
		}
	}
}

void benchmarkMultipleThread()
{
	double alloc_records[len_time][len_block_size] = {};
	double free_records[len_time][len_block_size] = {};
	double rand_records[len_time][len_block_size] = {};

	unsigned int thread_nums = std::thread::hardware_concurrency();
	thread_nums = thread_nums > 1 ? thread_nums : 2;

	std::vector<Records> records;
	std::vector<std::future<Records>> futures;
	char name_buf[64];

	// origin
	records.clear();
	memset(&alloc_records[0][0], 0, sizeof(double) * len_time * len_block_size);
	memset(&free_records[0][0], 0, sizeof(double) * len_time * len_block_size);
	memset(&rand_records[0][0], 0, sizeof(double) * len_time * len_block_size);
	for (int i = 0; i < len_time; ++i)
	{
		for (int j = 0; j < len_block_size; ++j)
		{
			std::cout << "multiple thread " << thread_nums << " - malloc/free - " << times[i] << ", " << block_size[j] << std::endl;

			for (unsigned int th_num = 0; th_num < thread_nums; ++th_num)
			{
				futures.push_back(std::async(std::launch::async, [&i, &j]()->Records {
					Records rec;
					memset(&rec, 0, sizeof(rec));
					benchmarkOrigin(times[i], block_size[j], rec.alloc_records[i][j], 
						rec.free_records[i][j], rec.rand_records[i][j], op_flag[i]);
					return rec;
				}));
			}
			for (unsigned int th_num = 0; th_num < thread_nums; ++th_num)
			{
				records.push_back(futures[th_num].get());
			}
			futures.clear();
		}
	}
	accuRecords(records, &alloc_records[0][0], &free_records[0][0], &rand_records[0][0]);

	memset(name_buf, 0, sizeof(name_buf));
	snprintf(name_buf, 63, "mul_th_%d_origin_alloc", thread_nums);
	writeToFile(name_buf, &alloc_records[0][0]);

	memset(name_buf, 0, sizeof(name_buf));
	snprintf(name_buf, 63, "mul_th_%d_origin_free", thread_nums);
	writeToFile(name_buf, &free_records[0][0]);

	memset(name_buf, 0, sizeof(name_buf));
	snprintf(name_buf, 63, "mul_th_%d_origin_rand", thread_nums);
	writeToFile(name_buf, &rand_records[0][0]);

	// thread safe pool
	records.clear();
	memset(&alloc_records[0][0], 0, sizeof(double) * len_time * len_block_size);
	memset(&free_records[0][0], 0, sizeof(double) * len_time * len_block_size);
	memset(&rand_records[0][0], 0, sizeof(double) * len_time * len_block_size);
	for (int i = 0; i < len_time; ++i)
	{
		for (int j = 0; j < len_block_size; ++j)
		{
			std::cout << "multiple thread " << thread_nums << " - thread safe pool - " << times[i] << ", " << block_size[j] << std::endl;

			muggle::ThreadSafeMemoryPool pool(times[i] * thread_nums, block_size[j]);
			shufflePool<muggle::ThreadSafeMemoryPool>(pool);

			for (unsigned int th_num = 0; th_num < thread_nums; ++th_num)
			{
				futures.push_back(std::async(std::launch::async, [&i, &j, &pool]()->Records {
					Records rec;
					memset(&rec, 0, sizeof(rec));
					benchmarkThreadsafeMemoryPool(pool, times[i], block_size[j], rec.alloc_records[i][j], 
						rec.free_records[i][j], rec.rand_records[i][j], op_flag[i]);
					return rec;
				}));
			}
			for (unsigned int th_num = 0; th_num < thread_nums; ++th_num)
			{
				records.push_back(futures[th_num].get());
			}
			futures.clear();
		}
	}
	accuRecords(records, &alloc_records[0][0], &free_records[0][0], &rand_records[0][0]);

	memset(name_buf, 0, sizeof(name_buf));
	snprintf(name_buf, 63, "mul_th_%d_threadsafe_pool_alloc", thread_nums);
	writeToFile(name_buf, &alloc_records[0][0]);

	memset(name_buf, 0, sizeof(name_buf));
	snprintf(name_buf, 63, "mul_th_%d_threadsafe_pool_free", thread_nums);
	writeToFile(name_buf, &free_records[0][0]);

	memset(name_buf, 0, sizeof(name_buf));
	snprintf(name_buf, 63, "mul_th_%d_threadsafe_pool_rand", thread_nums);
	writeToFile(name_buf, &rand_records[0][0]);

	// pool with thread local
	records.clear();
	memset(&alloc_records[0][0], 0, sizeof(double) * len_time * len_block_size);
	memset(&free_records[0][0], 0, sizeof(double) * len_time * len_block_size);
	memset(&rand_records[0][0], 0, sizeof(double) * len_time * len_block_size);
	for (int i = 0; i < len_time; ++i)
	{
		for (int j = 0; j < len_block_size; ++j)
		{
			std::cout << "multiple thread " << thread_nums << " - pool thread local - " << times[i] << ", " << block_size[j] << std::endl;

			for (unsigned int th_num = 0; th_num < thread_nums; ++th_num)
			{
				futures.push_back(std::async(std::launch::async, [&i, &j]()->Records {
					Records rec;
					memset(&rec, 0, sizeof(rec));

					muggle::ThreadSafeMemoryPool pool(times[i], block_size[j]);
					shufflePool<muggle::ThreadSafeMemoryPool>(pool);

					benchmarkThreadsafeMemoryPool(pool, times[i], block_size[j], rec.alloc_records[i][j], 
						rec.free_records[i][j], rec.rand_records[i][j], op_flag[i]);
					return rec;
				}));
			}
			for (unsigned int th_num = 0; th_num < thread_nums; ++th_num)
			{
				records.push_back(futures[th_num].get());
			}
			futures.clear();
		}
	}
	accuRecords(records, &alloc_records[0][0], &free_records[0][0], &rand_records[0][0]);

	memset(name_buf, 0, sizeof(name_buf));
	snprintf(name_buf, 63, "mul_th_%d_pool_thread_local_alloc", thread_nums);
	writeToFile(name_buf, &alloc_records[0][0]);

	memset(name_buf, 0, sizeof(name_buf));
	snprintf(name_buf, 63, "mul_th_%d_pool_thread_local_free", thread_nums);
	writeToFile(name_buf, &free_records[0][0]);

	memset(name_buf, 0, sizeof(name_buf));
	snprintf(name_buf, 63, "mul_th_%d_pool_thread_local_rand", thread_nums);
	writeToFile(name_buf, &rand_records[0][0]);
}

void genOpFlag()
{
	for (int i = 0; i < len_time; ++i)
	{
		op_flag[i] = (int*)malloc(sizeof(int) * times[i]);
		int sum = 0;
		for (int j = 0; j < times[i]; ++j)
		{
			int k = rand() % 2;
			if (k == 0)
			{
				k = -1;
			}

			if (k == -1 && sum + k > 0)
			{
				k = -1;
			}
			else
			{
				k = 1;
			}

			op_flag[i][j] = k;
			sum += k;
		}
	}
}

void delOpFlag()
{
	for (int i = 0; i < len_time; ++i)
	{
		free(op_flag[i]);
	}
}

void run()
{
	srand((unsigned int)time(nullptr));

	genOpFlag();

	benchmarkSingleThread();
	benchmarkMultipleThread();

	delOpFlag();
}

int main()
{
#if MUGGLE_PLATFORM_WINDOWS & MUGGLE_DEBUG
	struct muggle_debug_memory_state mem_state;
	muggle_debug_memory_leak_start(&mem_state);
#endif

	run();

#if MUGGLE_PLATFORM_WINDOWS
	muggle_debug_memory_leak_end(&mem_state);
#endif

	return 0;
}

#endif