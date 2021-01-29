#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"
#include "test_utils/test_utils.h"

#define TEST_ARRAY_LIST_LEN 64

class TestArrayListFixture : public ::testing::Test
{
public:
	void SetUp()
	{
		muggle_debug_memory_leak_start(&mem_state_);

		bool ret;

		ret = muggle_array_list_init(&list_[0], 0);
		ASSERT_TRUE(ret);

		ret = muggle_array_list_init(&list_[1], 8);
		ASSERT_TRUE(ret);

		for (int index = 0; index < (int)(sizeof(list_) / sizeof(list_[index])); index++)
		{
			ASSERT_EQ(muggle_array_list_size(&list_[index]), (size_t)0);
		}
	}

	void TearDown()
	{
		muggle_array_list_destroy(&list_[0], test_utils_free_int, &test_utils_);
		muggle_array_list_destroy(&list_[1], test_utils_free_int, &test_utils_);

		muggle_debug_memory_leak_end(&mem_state_);
	}

protected:
	muggle_array_list_t list_[2];

	TestUtils test_utils_;
	muggle_debug_memory_state mem_state_;
};

TEST_F(TestArrayListFixture, empty)
{
	for (int index = 0; index < (int)(sizeof(list_) / sizeof(list_[0])); index++)
	{
		bool ret = muggle_array_list_is_empty(&list_[index]);
		ASSERT_TRUE(ret);

		muggle_array_list_node_t *node = muggle_array_list_index(&list_[index], 0);
		ASSERT_TRUE(node == NULL);
	}
}

TEST_F(TestArrayListFixture, insert)
{
	for (int index = 0; index < (int)(sizeof(list_) / sizeof(list_[0])); index++)
	{
		muggle_array_list_t *list = &list_[index];

		for (int i = 0; i < TEST_ARRAY_LIST_LEN; i++)
		{
			int *p = test_utils_.allocateInteger();
			ASSERT_TRUE(p != NULL);

			*p = i;

			muggle_array_list_node_t *node = muggle_array_list_insert(list, 0, p);

			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(node->data, p);
			ASSERT_EQ(muggle_array_list_size(list), (size_t)(i + 1));
		}

		int expect = TEST_ARRAY_LIST_LEN - 1;
		for (int i = 0; i < (int)muggle_array_list_size(list); i++)
		{
			muggle_array_list_node *node = muggle_array_list_index(list, (int32_t)i);
			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(*(int*)node->data, expect);
			expect--;
		}
		ASSERT_EQ(expect, -1);

		expect = 0;
		for (int i = -1; i >= -(int)muggle_array_list_size(list); i--)
		{
			muggle_array_list_node *node = muggle_array_list_index(list, (int32_t)i);
			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(*(int*)node->data, expect);
			expect++;
		}
		ASSERT_EQ(expect, TEST_ARRAY_LIST_LEN);
	}
}

TEST_F(TestArrayListFixture, reverse_insert)
{
	for (int index = 0; index < (int)(sizeof(list_) / sizeof(list_[0])); index++)
	{
		muggle_array_list_t *list = &list_[index];

		for (int i = 0; i < TEST_ARRAY_LIST_LEN; i++)
		{
			int *p = test_utils_.allocateInteger();
			ASSERT_TRUE(p != NULL);

			*p = i;

			muggle_array_list_node_t *node = muggle_array_list_insert(list, -1, p);

			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(node->data, p);
			ASSERT_EQ(muggle_array_list_size(list), (size_t)(i + 1));
		}

		muggle_array_list_node_t *node = muggle_array_list_index(list, (int32_t)-1);
		ASSERT_TRUE(node != NULL);
		ASSERT_TRUE(node->data != NULL);
		ASSERT_EQ(*(int*)node->data, 0);

		int expect = 1;
		for (int i = 0; i < (int)muggle_array_list_size(list) - 1; i++)
		{
			muggle_array_list_node *node = muggle_array_list_index(list, (int32_t)i);
			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(*(int*)node->data, expect);
			expect++;
		}
		ASSERT_EQ(expect, TEST_ARRAY_LIST_LEN);
	}
}

TEST_F(TestArrayListFixture, append)
{
	for (int index = 0; index < (int)(sizeof(list_) / sizeof(list_[0])); index++)
	{
		muggle_array_list_t *list = &list_[index];

		for (int i = 0; i < TEST_ARRAY_LIST_LEN; i++)
		{
			int *p = test_utils_.allocateInteger();
			ASSERT_TRUE(p != NULL);

			*p = i;

			muggle_array_list_node_t *node = muggle_array_list_append(list, -1, p);

			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(node->data, p);
			ASSERT_EQ(muggle_array_list_size(list), (size_t)(i + 1));
		}

		int expect = 0;
		for (int i = 0; i < (int)muggle_array_list_size(list); i++)
		{
			muggle_array_list_node *node = muggle_array_list_index(list, (int32_t)i);
			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(*(int*)node->data, expect);
			expect++;
		}
		ASSERT_EQ(expect, TEST_ARRAY_LIST_LEN);

		expect = TEST_ARRAY_LIST_LEN - 1;
		for (int i = -1; i >= -(int)muggle_array_list_size(list); i--)
		{
			muggle_array_list_node *node = muggle_array_list_index(list, (int32_t)i);
			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(*(int*)node->data, expect);
			expect--;
		}
		ASSERT_EQ(expect, -1);
	}
}

TEST_F(TestArrayListFixture, reverse_append)
{
	for (int index = 0; index < (int)(sizeof(list_) / sizeof(list_[0])); index++)
	{
		muggle_array_list_t *list = &list_[index];

		for (int i = 0; i < TEST_ARRAY_LIST_LEN; i++)
		{
			int *p = test_utils_.allocateInteger();
			ASSERT_TRUE(p != NULL);

			*p = i;

			muggle_array_list_node_t *node = muggle_array_list_append(list, 0, p);

			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(node->data, p);
			ASSERT_EQ(muggle_array_list_size(list), (size_t)(i + 1));
		}

		muggle_array_list_node_t *node = muggle_array_list_index(list, 0);
		ASSERT_TRUE(node != NULL);
		ASSERT_EQ(*(int*)node->data, 0);

		int expect = TEST_ARRAY_LIST_LEN - 1;
		for (int i = 1; i < (int)muggle_array_list_size(list); i++)
		{
			muggle_array_list_node *node = muggle_array_list_index(list, (int32_t)i);
			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(*(int*)node->data, expect);
			expect--;
		}
		ASSERT_EQ(expect, 0);
	}
}

TEST_F(TestArrayListFixture, remove)
{
	for (int index = 0; index < (int)(sizeof(list_) / sizeof(list_[0])); index++)
	{
		muggle_array_list_t *list = &list_[index];

		for (int i = 0; i < TEST_ARRAY_LIST_LEN; i++)
		{
			int *p = test_utils_.allocateInteger();
			ASSERT_TRUE(p != NULL);

			*p = i;

			muggle_array_list_node_t *node = muggle_array_list_append(list, -1, p);

			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(node->data, p);
			ASSERT_EQ(muggle_array_list_size(list), (size_t)(i + 1));
		}

		int expect_cnt = TEST_ARRAY_LIST_LEN;
		bool ret = 0;
		while (1)
		{
			ASSERT_EQ(muggle_array_list_size(list), (size_t)expect_cnt);
			ret = muggle_array_list_remove(list, 0, test_utils_free_int, &test_utils_);
			ASSERT_TRUE(ret);
			expect_cnt--;
			if (expect_cnt == 0)
			{
				break;
			}
		}
	}
}

TEST_F(TestArrayListFixture, clear)
{
	for (int index = 0; index < (int)(sizeof(list_) / sizeof(list_[0])); index++)
	{
		muggle_array_list_t *list = &list_[index];

		for (int i = 0; i < TEST_ARRAY_LIST_LEN; i++)
		{
			int *p = test_utils_.allocateInteger();
			ASSERT_TRUE(p != NULL);

			*p = i;

			muggle_array_list_node_t *node = muggle_array_list_append(list, -1, p);

			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(node->data, p);
			ASSERT_EQ(muggle_array_list_size(list), (size_t)(i + 1));
		}

		bool ret = muggle_array_list_is_empty(list);
		ASSERT_FALSE(ret);

		muggle_array_list_clear(list, test_utils_free_int, &test_utils_);

		ret = muggle_array_list_is_empty(list);
		ASSERT_TRUE(ret);
		ASSERT_EQ(muggle_array_list_size(list), (size_t)0);
	}
}

TEST_F(TestArrayListFixture, find)
{
	for (int index = 0; index < (int)(sizeof(list_) / sizeof(list_[0])); index++)
	{
		muggle_array_list_t *list = &list_[index];

		for (int i = 0; i < TEST_ARRAY_LIST_LEN; i++)
		{
			int *p = test_utils_.allocateInteger();
			ASSERT_TRUE(p != NULL);

			*p = i;

			muggle_array_list_node_t *node = muggle_array_list_append(list, -1, p);

			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(node->data, p);
			ASSERT_EQ(muggle_array_list_size(list), (size_t)(i + 1));
		}

		for (int i = 0; i < TEST_ARRAY_LIST_LEN; i++)
		{
			int idx = muggle_array_list_find(list, 0, &i, test_utils_cmp_int);
			ASSERT_EQ(idx, i);
		}
	}
}
