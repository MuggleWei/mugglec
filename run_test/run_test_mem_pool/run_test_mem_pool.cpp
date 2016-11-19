#if defined(_WIN32) && ! defined(NDEBUG)
#define _CRTDBG_MAP_ALLOC 
#include <crtdbg.h>
#endif
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <vector>
#include <assert.h>

#include "muggle/cpp_utils/cpp_utils.h"
#include "muggle/mem_pool/memory_pool.h"

#define TEST_NUM 1024

struct TestData
{
	int i;
	char str[16];
	int j;
	float f;
};

void TestFunction()
{
	int* arr[TEST_NUM];

	MemoryPool pool;
	MemoryPoolInit(&pool, 16, sizeof(int));

	/*
	*   f               m
	*  [x] [x] ... [x] [0] [0] [0] ... [0]
	*/
	for (int i = 0; i < 10; ++i)
	{
		arr[i] = (int*)MemoryPoolAlloc(&pool);
		*arr[i] = i;
	}
	MemoryPoolEnsureSpace(&pool, 32);

	/*
	*                   m   f
	*  [x] [x] ... [x] [0] [x] [x] [x] [x] ... [x]
	*/
	for (int i = 0; i < 16; ++i)
	{
		arr[i] = (int*)MemoryPoolAlloc(&pool);
		*arr[i] = i;
	}
	for (int i = 0; i < 15; ++i)
	{
		MemoryPoolFree(&pool, &arr[i]);
		arr[i] = NULL;
	}
	for (int i = 0; i < 30; ++i)
	{
		arr[i] = (int*)MemoryPoolAlloc(&pool);
		*arr[i] = i;
	}
	MemoryPoolEnsureSpace(&pool, 64);

	// alloc
	MemoryPoolDestroy(&pool);
	MemoryPoolInit(&pool, 16, sizeof(int));
	for (int i = 0; i < TEST_NUM; ++i)
	{
		arr[i] = (int*)MemoryPoolAlloc(&pool);
		*arr[i] = i;
	}

	MemoryPoolDestroy(&pool);
}

void WriteTimeData(FILE *fp, const char* name, std::vector<int>& nums, std::vector<double>& times)
{
	fprintf(fp, "%s\t", name);
	for (decltype(nums.size()) i = 0; i < nums.size(); ++i)
	{
		fprintf(fp, "%f\t", times[i]);
	}
	fprintf(fp, "\n");
}

void UnitTestPerformance_Origin(std::vector<int>& nums, std::vector<double>& times_alloc, std::vector<double>& times_free)
{
	srand((unsigned int)time(NULL));

	int max_size = nums[nums.size() - 1];
	int** arr = (int**)malloc(sizeof(int*) * max_size);

	muggle::DeltaTimer t;

	for (decltype(nums.size()) cur_num = 0; cur_num < nums.size(); ++cur_num)
	{
		t.Start();
		for (int i = 0; i < nums[cur_num]; ++i)
		{
			arr[i] = (int*)malloc(sizeof(int));
		}
		t.End();
		times_alloc.push_back(t.GetElapsedMilliseconds());
		printf("malloc: %d - %lf\n", nums[cur_num], t.GetElapsedMilliseconds());

		t.Start();
		for (int i = 0; i < nums[cur_num]; ++i)
		{
			free(arr[i]);
		}
		t.End();
		times_free.push_back(t.GetElapsedMilliseconds());
		printf("free: %d - %lf\n", nums[cur_num], t.GetElapsedMilliseconds());
	}

	free(arr);
}
void UnitTestPerformance_Pool(std::vector<int>& nums, std::vector<double>& times_alloc, std::vector<double>& times_free)
{
	srand((unsigned int)time(NULL));

	int max_size = nums[nums.size() - 1];
	int** arr = (int**)malloc(sizeof(int*) * max_size);

	muggle::DeltaTimer t;
	MemoryPool pool;
	MemoryPoolInit(&pool, 8, sizeof(int));

	for (decltype(nums.size()) cur_num = 0; cur_num < nums.size(); ++cur_num)
	{
		MemoryPoolEnsureSpace(&pool, (unsigned int)nums[cur_num]);

		t.Start();
		for (int i = 0; i < nums[cur_num]; ++i)
		{
			arr[i] = (int*)MemoryPoolAlloc(&pool);
		}
		t.End();
		times_alloc.push_back(t.GetElapsedMilliseconds());
		printf("malloc: %d - %lf\n", nums[cur_num], t.GetElapsedMilliseconds());

		t.Start();
		for (int i = 0; i < nums[cur_num]; ++i)
		{
			MemoryPoolFree(&pool, arr[i]);
		}
		t.End();
		times_free.push_back(t.GetElapsedMilliseconds());
		printf("free: %d - %lf\n", nums[cur_num], t.GetElapsedMilliseconds());
	}

	MemoryPoolDestroy(&pool);
	free(arr);
}

void TestPerformance()
{
	std::vector<int> nums;
	std::vector<double> times_malloc;
	std::vector<double> times_malloc_pool;
	std::vector<double> times_free;
	std::vector<double> times_free_pool;

	for (int i = 1; i <= 32; ++i)
	{
		nums.push_back(i * 102400);
	}

	// performance - insert and make empty
	UnitTestPerformance_Origin(nums, times_malloc, times_free);
	UnitTestPerformance_Pool(nums, times_malloc_pool, times_free_pool);

	// open file
	FILE *fp = fopen("MemoryPoolPerformance.txt", "w+");
	if (!fp)
	{
		printf("can't open MemoryPoolPerformance.txt!\n");
		assert(0);
		return;
	}

	// write file
	fprintf(fp, "\t");
	for (decltype(nums.size()) i = 0; i < nums.size(); ++i)
	{
		fprintf(fp, "%lu\t", (unsigned long)nums[i]);
	}
	fprintf(fp, "\n");

	// write performance data
	WriteTimeData(fp, "malloc", nums, times_malloc);
	WriteTimeData(fp, "free", nums, times_free);
	WriteTimeData(fp, "memory pool alloc", nums, times_malloc_pool);
	WriteTimeData(fp, "memory pool free", nums, times_free_pool);

	// close file
	fclose(fp);
}

int main()
{
#if defined(_WIN32) && ! defined(NDEBUG)
	_CrtMemState s1, s2, s3;
	_CrtMemCheckpoint(&s1);
#endif

	TestFunction();
	TestPerformance();

#if defined(_WIN32) && ! defined(NDEBUG)
	_CrtMemCheckpoint(&s2);
	if (_CrtMemDifference(&s3, &s1, &s2))
	{
		_CrtMemDumpStatistics(&s3);
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}