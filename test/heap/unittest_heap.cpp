#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"
#include "test_utils/test_utils.h"

#define TEST_HEAP_LEN 64

class TestHeapFixture : public ::testing::Test
{
public:
	void SetUp()
	{
		muggle_debug_memory_leak_start(&mem_state_);

		bool ret;

		ret = muggle_heap_init(&heap_[0], test_utils_cmp_int, 0);
		ASSERT_TRUE(ret);

		ret = muggle_heap_init(&heap_[1], test_utils_cmp_int, TEST_HEAP_LEN);
		ASSERT_TRUE(ret);
	}

	void TearDown()
	{
		muggle_heap_destroy(&heap_[0], test_utils_free_int, &test_utils_, test_utils_free_str, &test_utils_);
		muggle_heap_destroy(&heap_[1], test_utils_free_int, &test_utils_, test_utils_free_str, &test_utils_);

		muggle_debug_memory_leak_end(&mem_state_);
	}

protected:
	muggle_heap_t heap_[2];

	TestUtils test_utils_;
	muggle_debug_memory_state mem_state_;
};

void TestHeapCheckValid(muggle_heap_t *p_heap)
{
	for (uint32_t i = 1; i <= p_heap->size; i++)
	{
		muggle_heap_node_t *node = &p_heap->nodes[i];
		ASSERT_TRUE(node != NULL);
		ASSERT_TRUE(node->key != NULL);
		ASSERT_TRUE(node->value != NULL);

		uint32_t left = i * 2;
		uint32_t right = left + 1;

		if (left <= p_heap->size)
		{
			muggle_heap_node_t *left_node = &p_heap->nodes[left];

			ASSERT_TRUE(left_node != NULL);
			ASSERT_TRUE(left_node->key != NULL);
			ASSERT_TRUE(left_node->value != NULL);
			ASSERT_LE(*(int*)node->key, *(int*)left_node->key);
		}

		if (right <= p_heap->size)
		{
			muggle_heap_node_t *right_node = &p_heap->nodes[right];

			ASSERT_TRUE(right_node != NULL);
			ASSERT_TRUE(right_node->key != NULL);
			ASSERT_TRUE(right_node->value != NULL);
			ASSERT_LE(*(int*)node->key, *(int*)right_node->key);
		}
	}
}

TEST_F(TestHeapFixture, insert)
{
	for (int index = 0; index < (int)(sizeof(heap_) / sizeof(heap_[0])); index++)
	{
		muggle_heap_t *p_heap = &heap_[index];

		for (int i = 0; i < TEST_HEAP_LEN; i++)
		{
			int *p = test_utils_.allocateInteger();
			ASSERT_TRUE(p != NULL);

			char *s = test_utils_.allocateString();

			*p = i;
			snprintf(s, TEST_UTILS_STR_SIZE, "%d", i);

			bool ret = muggle_heap_insert(p_heap, p, s);

			ASSERT_TRUE(ret);

			TestHeapCheckValid(p_heap);
		}
	}
}

// insert extract integer array: 1, 2, 3 ...... n
TEST_F(TestHeapFixture, insert_extract_case1)
{
	for (int index = 0; index < (int)(sizeof(heap_) / sizeof(heap_[0])); index++)
	{
		muggle_heap_t *p_heap = &heap_[index];

		for (int i = 0; i < TEST_HEAP_LEN; i++)
		{
			int *p = test_utils_.allocateInteger();
			ASSERT_TRUE(p != NULL);

			char *s = test_utils_.allocateString();

			*p = i;
			snprintf(s, TEST_UTILS_STR_SIZE, "%d", i);

			bool ret = muggle_heap_insert(p_heap, p, s);

			ASSERT_TRUE(ret);

			TestHeapCheckValid(p_heap);
		}

		muggle_heap_node_t node;
		for (int i = 0; i < TEST_HEAP_LEN; i++)
		{
			bool ret = muggle_heap_extract(p_heap, &node);

			ASSERT_TRUE(ret);

			TestHeapCheckValid(p_heap);

			ASSERT_EQ(*(int*)node.key, i);
			char buf[32];
			snprintf(buf, sizeof(buf), "%d", *(int*)node.key);
			ASSERT_STREQ(buf, (char*)node.value);

			test_utils_free_int(&test_utils_, node.key);
			test_utils_free_str(&test_utils_, node.value);
		}
	}
}

// insert extract integer array: n, n-1, n-2 ...... 1
TEST_F(TestHeapFixture, insert_extract_case2)
{
	for (int index = 0; index < (int)(sizeof(heap_) / sizeof(heap_[0])); index++)
	{
		muggle_heap_t *p_heap = &heap_[index];

		for (int i = TEST_HEAP_LEN - 1; i >= 0; i--)
		{
			int *p = test_utils_.allocateInteger();
			ASSERT_TRUE(p != NULL);

			char *s = test_utils_.allocateString();

			*p = i;
			snprintf(s, TEST_UTILS_STR_SIZE, "%d", i);

			bool ret = muggle_heap_insert(p_heap, p, s);

			ASSERT_TRUE(ret);

			TestHeapCheckValid(p_heap);
		}

		muggle_heap_node_t node;
		for (int i = 0; i < TEST_HEAP_LEN; i++)
		{
			bool ret = muggle_heap_extract(p_heap, &node);

			ASSERT_TRUE(ret);

			TestHeapCheckValid(p_heap);

			ASSERT_EQ(*(int*)node.key, i);
			char buf[32];
			snprintf(buf, sizeof(buf), "%d", *(int*)node.key);
			ASSERT_STREQ(buf, (char*)node.value);

			test_utils_free_int(&test_utils_, node.key);
			test_utils_free_str(&test_utils_, node.value);
		}
	}
}

// insert extract integer array: random
TEST_F(TestHeapFixture, insert_extract_case3)
{
	srand((unsigned int)time(NULL));
	for (int index = 0; index < (int)(sizeof(heap_) / sizeof(heap_[0])); index++)
	{
		muggle_heap_t *p_heap = &heap_[index];

		int *arr = (int*)malloc(sizeof(int*) * TEST_HEAP_LEN);
		for (int i = 0; i < TEST_HEAP_LEN; i++)
		{
			arr[i] = i;
		}

		// shuffle
		for (int i = 0; i < TEST_HEAP_LEN; i++)
		{
			int idx = rand() % TEST_HEAP_LEN;
			int tmp = arr[i];
			arr[i] = arr[idx];
			arr[idx] = tmp;
		}

		// insert
		printf("----------------------------------\n");
		printf("insert: ");
		for (int i = 0; i < TEST_HEAP_LEN; i++)
		{
			char *s = test_utils_.allocateString();
			snprintf(s, TEST_UTILS_STR_SIZE, "%d", arr[i]);

			bool ret = muggle_heap_insert(p_heap, &arr[i], s);

			ASSERT_TRUE(ret);

			TestHeapCheckValid(p_heap);
			printf(" %d", arr[i]);
		}
		printf("\n");

		printf("----------------------------------\n");
		printf("extract: ");
		muggle_heap_node_t node;
		for (int i = 0; i < TEST_HEAP_LEN; i++)
		{
			bool ret = muggle_heap_extract(p_heap, &node);

			ASSERT_TRUE(ret);

			TestHeapCheckValid(p_heap);

			ASSERT_EQ(*(int*)node.key, i);

			char buf[32];
			snprintf(buf, sizeof(buf), "%d", *(int*)node.key);
			ASSERT_STREQ(buf, (char*)node.value);

			test_utils_free_str(&test_utils_, node.value);

			printf(" %s", buf);
		}
		printf("\n");

		free(arr);
	}
}

// insert integer array: random
TEST_F(TestHeapFixture, insert_extract_find_case)
{
	srand((unsigned int)time(NULL));
	for (int index = 0; index < (int)(sizeof(heap_) / sizeof(heap_[0])); index++)
	{
		muggle_heap_t *p_heap = &heap_[index];

		int *arr = (int*)malloc(sizeof(int*) * TEST_HEAP_LEN);
		for (int i = 0; i < TEST_HEAP_LEN; i++)
		{
			arr[i] = i;
		}

		// shuffle
		for (int i = 0; i < TEST_HEAP_LEN; i++)
		{
			int idx = rand() % TEST_HEAP_LEN;
			int tmp = arr[i];
			arr[i] = arr[idx];
			arr[idx] = tmp;
		}

		// insert
		for (int i = 0; i < TEST_HEAP_LEN; i++)
		{
			char *s = test_utils_.allocateString();
			snprintf(s, TEST_UTILS_STR_SIZE, "%d", arr[i]);

			bool ret = muggle_heap_insert(p_heap, &arr[i], s);

			ASSERT_TRUE(ret);

			TestHeapCheckValid(p_heap);

			muggle_heap_node_t *p_node = muggle_heap_find(p_heap, &arr[i]);
			ASSERT_TRUE(p_node != NULL);
			ASSERT_EQ(*(int*)p_node->key, arr[i]);
			ASSERT_STREQ((char*)p_node->value, s);
		}

		muggle_heap_node_t node;
		for (int i = 0; i < TEST_HEAP_LEN; i++)
		{
			int tmp = i;

			muggle_heap_node_t *p_node = muggle_heap_find(p_heap, &tmp);
			ASSERT_TRUE(p_node != NULL);
			ASSERT_EQ(*(int*)p_node->key, tmp);

			bool ret = muggle_heap_extract(p_heap, &node);
			ASSERT_TRUE(ret);

			p_node = muggle_heap_find(p_heap, &tmp);
			ASSERT_TRUE(p_node == NULL);

			TestHeapCheckValid(p_heap);

			ASSERT_EQ(*(int*)node.key, tmp);

			char buf[32];
			snprintf(buf, sizeof(buf), "%d", *(int*)node.key);
			ASSERT_STREQ(buf, (char*)node.value);

			test_utils_free_str(&test_utils_, node.value);
		}

		free(arr);
	}
}

// insert remove integer array: 1, 2, 3 ...... n
TEST_F(TestHeapFixture, insert_remove_case1)
{
	for (int index = 0; index < (int)(sizeof(heap_) / sizeof(heap_[0])); index++)
	{
		muggle_heap_t *p_heap = &heap_[index];

		for (int i = 0; i < TEST_HEAP_LEN; i++)
		{
			int *p = test_utils_.allocateInteger();
			ASSERT_TRUE(p != NULL);

			char *s = test_utils_.allocateString();

			*p = i;
			snprintf(s, TEST_UTILS_STR_SIZE, "%d", i);

			bool ret = muggle_heap_insert(p_heap, p, s);

			ASSERT_TRUE(ret);

			TestHeapCheckValid(p_heap);
		}

		for (int i = 0; i < TEST_HEAP_LEN; i++)
		{
			muggle_heap_node_t *node = muggle_heap_find(p_heap, &i);
			ASSERT_TRUE(node != NULL);

			ASSERT_EQ(*(int*)node->key, i);
			char buf[32];
			snprintf(buf, sizeof(buf), "%d", *(int*)node->key);
			ASSERT_STREQ((char*)node->value, buf);

			bool ret = muggle_heap_remove(p_heap, node, test_utils_free_int, &test_utils_, test_utils_free_str, &test_utils_);
			ASSERT_TRUE(ret);

			node = muggle_heap_find(p_heap, &i);
			ASSERT_TRUE(node == NULL);

			TestHeapCheckValid(p_heap);
		}
	}
}

// insert remove integer array: n, n-1, n-2 ...... 1
TEST_F(TestHeapFixture, insert_remove_case2)
{
	for (int index = 0; index < (int)(sizeof(heap_) / sizeof(heap_[0])); index++)
	{
		muggle_heap_t *p_heap = &heap_[index];

		for (int i = TEST_HEAP_LEN - 1; i >= 0; i--)
		{
			int *p = test_utils_.allocateInteger();
			ASSERT_TRUE(p != NULL);

			char *s = test_utils_.allocateString();

			*p = i;
			snprintf(s, TEST_UTILS_STR_SIZE, "%d", i);

			bool ret = muggle_heap_insert(p_heap, p, s);

			ASSERT_TRUE(ret);

			TestHeapCheckValid(p_heap);
		}

		for (int i = TEST_HEAP_LEN - 1; i >= 0; i--)
		{
			muggle_heap_node_t *node = muggle_heap_find(p_heap, &i);
			ASSERT_TRUE(node != NULL);

			ASSERT_EQ(*(int*)node->key, i);
			char buf[32];
			snprintf(buf, sizeof(buf), "%d", *(int*)node->key);
			ASSERT_STREQ((char*)node->value, buf);

			bool ret = muggle_heap_remove(p_heap, node, test_utils_free_int, &test_utils_, test_utils_free_str, &test_utils_);
			ASSERT_TRUE(ret);

			node = muggle_heap_find(p_heap, &i);
			ASSERT_TRUE(node == NULL);

			TestHeapCheckValid(p_heap);
		}
	}
}

// insert remove integer array: random
TEST_F(TestHeapFixture, insert_remove_case3)
{
	for (int index = 0; index < (int)(sizeof(heap_) / sizeof(heap_[0])); index++)
	{
		muggle_heap_t *p_heap = &heap_[index];

		int *arr = (int*)malloc(sizeof(int*) * TEST_HEAP_LEN);
		for (int i = 0; i < TEST_HEAP_LEN; i++)
		{
			arr[i] = i;
		}

		// shuffle
		for (int i = 0; i < TEST_HEAP_LEN; i++)
		{
			int idx = rand() % TEST_HEAP_LEN;
			int tmp = arr[i];
			arr[i] = arr[idx];
			arr[idx] = tmp;
		}

		// insert
		printf("----------------------------------\n");
		printf("insert: ");
		for (int i = 0; i < TEST_HEAP_LEN; i++)
		{
			int *p = test_utils_.allocateInteger();
			ASSERT_TRUE(p != NULL);
			*p = arr[i];

			char *s = test_utils_.allocateString();
			snprintf(s, TEST_UTILS_STR_SIZE, "%d", arr[i]);

			bool ret = muggle_heap_insert(p_heap, p, s);

			ASSERT_TRUE(ret);

			TestHeapCheckValid(p_heap);

			muggle_heap_node_t *p_node = muggle_heap_find(p_heap, &arr[i]);
			ASSERT_TRUE(p_node != NULL);
			ASSERT_EQ(*(int*)p_node->key, arr[i]);
			ASSERT_STREQ((char*)p_node->value, s);

			printf(" %d", arr[i]);
		}
		printf("\n");

		// shuffle
		for (int i = 0; i < TEST_HEAP_LEN; i++)
		{
			int idx = rand() % TEST_HEAP_LEN;
			int tmp = arr[i];
			arr[i] = arr[idx];
			arr[idx] = tmp;
		}

		// remove
		printf("----------------------------------\n");
		printf("remove: ");
		for (int i = 0; i < TEST_HEAP_LEN; i++)
		{
			muggle_heap_node_t *node = muggle_heap_find(p_heap, &arr[i]);
			ASSERT_TRUE(node != NULL);

			ASSERT_EQ(*(int*)node->key, arr[i]);
			char buf[32];
			snprintf(buf, sizeof(buf), "%d", *(int*)node->key);
			ASSERT_STREQ((char*)node->value, buf);

			bool ret = muggle_heap_remove(p_heap, node, test_utils_free_int, &test_utils_, test_utils_free_str, &test_utils_);
			ASSERT_TRUE(ret);

			node = muggle_heap_find(p_heap, &arr[i]);
			ASSERT_TRUE(node == NULL);

			TestHeapCheckValid(p_heap);

			printf(" %d", arr[i]);
		}
		printf("\n");

		free(arr);
	}
}
