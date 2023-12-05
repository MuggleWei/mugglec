#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"
#include "test_utils/test_utils.h"

#define TEST_SORT_LEN 1000000

class TestSortFixture : public ::testing::Test
{
public:
	void SetUp()
	{
		muggle_debug_memory_leak_start(&mem_state_);

		integer_arr_ = (int*)malloc(sizeof(int) * TEST_SORT_LEN);
		ptr_ = (void**)malloc(sizeof(void*) * TEST_SORT_LEN);

		for (int i = 0; i < TEST_SORT_LEN; i++)
		{
			// integer_arr_[i] = i;
			integer_arr_[i] = (i * 2) / 2; // for have same element
			ptr_[i] = &integer_arr_[i];
		}
		
		srand((unsigned int)time(NULL));
		for (int i = 0; i < TEST_SORT_LEN; i++)
		{
			int idx = rand() % TEST_SORT_LEN;
			void *tmp = ptr_[i];
			ptr_[i] = ptr_[idx];
			ptr_[idx] = tmp;
		}
	}

	void TearDown()
	{
		for (int i = 0; i < TEST_SORT_LEN; i++)
		{
			ASSERT_EQ(*(int*)ptr_[i], integer_arr_[i]);
		}

		free(ptr_);
		free(integer_arr_);

		muggle_debug_memory_leak_end(&mem_state_);
	}

	void printPtr()
	{
		printf("------------------------------\n");
		for (int i = 0; i < TEST_SORT_LEN; i++)
		{
			printf(" %d", *(int*)ptr_[i]);
		}
		printf("\n");
	}

protected:
	int *integer_arr_;
	void **ptr_;

	muggle_debug_memory_state mem_state_;
};

bool c_qsort(void **ptr, size_t count, muggle_dsaa_data_cmp cmp)
{
	qsort(ptr, count, sizeof(void*), cmp);
	return true;
}

void run_sort(void **ptr, muggle_func_sort func, const char *name)
{
	timespec t1, t2;

	muggle_realtime_get(t1);
	bool ret = func(ptr, TEST_SORT_LEN, test_utils_cmp_int);
	muggle_realtime_get(t2);

	ASSERT_TRUE(ret);

	uint64_t ns = (t2.tv_sec - t1.tv_sec) * 1000000000 + t2.tv_nsec - t1.tv_nsec;
	printf("%s sort %d elements, elapsed time: %llu ns\n",
		name, TEST_SORT_LEN, (unsigned long long)ns);
}

// TEST_F(TestSortFixture, insertion_sort)
// {
// 	run_sort(ptr_, insertion_sort, __FUNCTION__);
// }

TEST_F(TestSortFixture, shell_short)
{
	run_sort(ptr_, muggle_shell_sort, __FUNCTION__);
}

TEST_F(TestSortFixture, heap_sort)
{
	run_sort(ptr_, muggle_heap_sort, __FUNCTION__);
}

TEST_F(TestSortFixture, merge_sort)
{
	run_sort(ptr_, muggle_merge_sort, __FUNCTION__);
}

TEST_F(TestSortFixture, c_qsort)
{
	run_sort(ptr_, c_qsort, __FUNCTION__);
}

TEST_F(TestSortFixture, quick_sort)
{
	run_sort(ptr_, muggle_quick_sort, __FUNCTION__);
}
