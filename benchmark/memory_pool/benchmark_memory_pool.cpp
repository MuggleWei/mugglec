/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include <stdint.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <future>
#include <vector>
#include "muggle/cpp/memory_pool/memory_pool.h"
#include "muggle/cpp/memory_pool/thread_safe_memory_pool.h"

typedef std::chrono::duration<double, std::milli> double_dura;
typedef std::function<void(intptr_t*, int, unsigned int)> allocFunc;
typedef std::function<void(intptr_t*, int)> freeFunc;

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

void memorypoolAlloc(muggle::MemoryPool &pool, intptr_t *buf, int times, unsigned int block_size)
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

void threadsafeMemorypoolAlloc(muggle::ThreadSafeMemoryPool &pool, intptr_t *buf, int times, unsigned int block_size)
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


void benchmarkOrigin(int times, unsigned int block_size, double &alloc_ms, double &free_ms)
{
	intptr_t *buf = (intptr_t*)malloc(sizeof(void*) * times);

	allocFunc alloc_func = std::bind(&originAlloc, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	alloc_ms = invokeAlloc(buf, times, block_size, alloc_func);

	freeFunc free_func = std::bind(&originFree, std::placeholders::_1, std::placeholders::_2);
	free_ms = invokeFree(buf, times, free_func);

	free(buf);
}

void benchmarkMemoryPool(int times, unsigned int block_size, double &alloc_ms, double &free_ms)
{
	intptr_t *buf = (intptr_t*)malloc(sizeof(void*) * times);

	muggle::MemoryPool pool((unsigned int)times, block_size);

	allocFunc alloc_func = std::bind(&memorypoolAlloc, std::ref(pool), 
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	alloc_ms = invokeAlloc(buf, times, block_size, alloc_func);

	freeFunc free_func = std::bind(&memorypoolFree, std::ref(pool),
		std::placeholders::_1, std::placeholders::_2);
	free_ms = invokeFree(buf, times, free_func);

	free(buf);
}

void benchmarkThreadsafeMemoryPool(int times, unsigned int block_size, double &alloc_ms, double &free_ms)
{
	intptr_t *buf = (intptr_t*)malloc(sizeof(void*) * times);

	muggle::ThreadSafeMemoryPool pool((unsigned int)times, block_size);

	allocFunc alloc_func = std::bind(&threadsafeMemorypoolAlloc, std::ref(pool),
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	alloc_ms = invokeAlloc(buf, times, block_size, alloc_func);

	freeFunc free_func = std::bind(&threadsafeMemorypoolFree, std::ref(pool),
		std::placeholders::_1, std::placeholders::_2);
	free_ms = invokeFree(buf, times, free_func);

	free(buf);
}


int times[] = { 128, 256, 512, 1024, 2048, 4096, 10000, 100000 };
unsigned int block_size[] = { 8, 16, 32, 64, 128, 512, 1024, 2048, 4096 };

const int len_time = sizeof(times) / sizeof(times[0]);
const int len_block_size = sizeof(block_size) / sizeof(block_size[0]);

struct Records
{
	double alloc_records[len_time][len_block_size];
	double free_records[len_time][len_block_size];
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

void benchmarkSingleThread()
{
	double alloc_records[len_time][len_block_size] = {};
	double free_records[len_time][len_block_size] = {};

	// origin
	for (int i = 0; i < len_time; ++i)
	{
		for (int j = 0; j < len_block_size; ++j)
		{
			benchmarkOrigin(times[i], block_size[j], alloc_records[i][j], free_records[i][j]);
		}
	}
	writeToFile("origin_alloc", &alloc_records[0][0]);
	writeToFile("origin_free", &free_records[0][0]);

	// pool
	for (int i = 0; i < len_time; ++i)
	{
		for (int j = 0; j < len_block_size; ++j)
		{
			benchmarkMemoryPool(times[i], block_size[j], alloc_records[i][j], free_records[i][j]);
		}
	}
	writeToFile("pool_alloc", &alloc_records[0][0]);
	writeToFile("pool_free", &free_records[0][0]);

	// thread safe pool
	for (int i = 0; i < len_time; ++i)
	{
		for (int j = 0; j < len_block_size; ++j)
		{
			benchmarkThreadsafeMemoryPool(times[i], block_size[j], alloc_records[i][j], free_records[i][j]);
		}
	}
	writeToFile("threadsafe_pool_alloc", &alloc_records[0][0]);
	writeToFile("threadsafe_pool_free", &free_records[0][0]);
}

void accuRecords(std::vector<std::future<Records>> &futures, double *alloc_records, double *free_records)
{
	for (auto &future : futures)
	{
		Records rec = future.get();
		for (int i = 0; i < len_time; ++i)
		{
			for (int j = 0; j < len_block_size; ++j)
			{
				*(alloc_records + i * len_block_size + j) += rec.alloc_records[i][j];
				*(free_records + i * len_block_size + j) += rec.free_records[i][j];
			}
		}
	}
}

void benchmarkMultipleThread()
{
	double alloc_records[len_time][len_block_size] = {};
	double free_records[len_time][len_block_size] = {};

	unsigned int thread_nums = std::thread::hardware_concurrency();
	thread_nums = thread_nums > 1 ? thread_nums : 2;

	std::vector<std::future<Records>> futures;

	// origin
	futures.clear();
	memset(&alloc_records[0][0], 0, sizeof(double) * len_time * len_block_size);
	memset(&free_records[0][0], 0, sizeof(double) * len_time * len_block_size);
	for (unsigned int i = 0; i < thread_nums; ++i)
	{
		futures.push_back(std::async(std::launch::async, []()->Records {
			Records rec;
			for (int i = 0; i < len_time; ++i)
			{
				for (int j = 0; j < len_block_size; ++j)
				{
					benchmarkOrigin(times[i], block_size[j], rec.alloc_records[i][j], rec.free_records[i][j]);
				}
			}
			return rec;
		}));
	}
	accuRecords(futures, &alloc_records[0][0], &free_records[0][0]);
	writeToFile("mul_th_origin_alloc", &alloc_records[0][0]);
	writeToFile("mul_th_origin_free", &free_records[0][0]);

	// thread safe pool
	futures.clear();
	memset(&alloc_records[0][0], 0, sizeof(double) * len_time * len_block_size);
	memset(&free_records[0][0], 0, sizeof(double) * len_time * len_block_size);
	for (unsigned int i = 0; i < thread_nums; ++i)
	{
		futures.push_back(std::async(std::launch::async, []()->Records {
			Records rec;
			for (int i = 0; i < len_time; ++i)
			{
				for (int j = 0; j < len_block_size; ++j)
				{
					benchmarkThreadsafeMemoryPool(times[i], block_size[j], rec.alloc_records[i][j], rec.free_records[i][j]);
				}
			}
			return rec;
		}));
	}
	accuRecords(futures, &alloc_records[0][0], &free_records[0][0]);
	writeToFile("mul_th_threadsafe_pool_alloc", &alloc_records[0][0]);
	writeToFile("mul_th_threadsafe_pool_free", &free_records[0][0]);
}

int main()
{
	benchmarkSingleThread();
	benchmarkMultipleThread();

	return 0;
}
