#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"
#include "test_utils/test_utils.h"

#define TEST_LINKED_LIST_LEN 64

class TestLinkedListFixture : public ::testing::Test
{
public:
	void SetUp()
	{
		muggle_debug_memory_leak_start(&mem_state_);

		bool ret;

		ret = muggle_linked_list_init(&list_[0], 0);
		ASSERT_TRUE(ret);

		ret = muggle_linked_list_init(&list_[1], 8);
		ASSERT_TRUE(ret);

		for (int index = 0; index < (int)(sizeof(list_) / sizeof(list_[index])); index++)
		{
			ASSERT_EQ(muggle_linked_list_size(&list_[index]), (size_t)0);
		}
	}

	void TearDown()
	{
		muggle_linked_list_destroy(&list_[0], test_utils_free_int, &test_utils_);
		muggle_linked_list_destroy(&list_[1], test_utils_free_int, &test_utils_);

		muggle_debug_memory_leak_end(&mem_state_);
	}

protected:
	muggle_linked_list_t list_[2];

	TestUtils test_utils_;
	muggle_debug_memory_state mem_state_;
};

TEST_F(TestLinkedListFixture, empty)
{
	for (int index = 0; index < (int)(sizeof(list_) / sizeof(list_[0])); index++)
	{
		bool ret = muggle_linked_list_is_empty(&list_[index]);
		ASSERT_TRUE(ret);
	}
}

TEST_F(TestLinkedListFixture, insert)
{
	for (int index = 0; index < (int)(sizeof(list_) / sizeof(list_[0])); index++)
	{
		muggle_linked_list_t *list = &list_[index];

		for (int i = 0; i < TEST_LINKED_LIST_LEN; i++)
		{
			int *p = test_utils_.allocateInteger();
			ASSERT_TRUE(p != NULL);

			*p = i;

			muggle_linked_list_node_t *node = muggle_linked_list_insert(list, NULL, p);

			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(node->data, p);
			ASSERT_EQ(muggle_linked_list_size(list), (size_t)(i + 1));
		}

		int expect = TEST_LINKED_LIST_LEN - 1;
		muggle_linked_list_node_t *node = muggle_linked_list_first(list);
		for (; node; node = muggle_linked_list_next(list, node))
		{
			ASSERT_EQ(*(int*)node->data, expect);
			expect--;
		}
		ASSERT_EQ(expect, -1);

		expect = 0;
		node = muggle_linked_list_last(list);
		for (; node; node = muggle_linked_list_prev(list, node))
		{
			ASSERT_EQ(*(int*)node->data, expect);
			expect++;
		}
		ASSERT_EQ(expect, TEST_LINKED_LIST_LEN);
	}
}

TEST_F(TestLinkedListFixture, append)
{
	for (int index = 0; index < (int)(sizeof(list_) / sizeof(list_[0])); index++)
	{
		muggle_linked_list_t *list = &list_[index];

		for (int i = 0; i < TEST_LINKED_LIST_LEN; i++)
		{
			int *p = test_utils_.allocateInteger();
			ASSERT_TRUE(p != NULL);

			*p = i;

			muggle_linked_list_node_t *node = muggle_linked_list_append(list, NULL, p);

			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(node->data, p);
			ASSERT_EQ(muggle_linked_list_size(list), (size_t)(i + 1));
		}

		int expect = 0;
		muggle_linked_list_node_t *node = muggle_linked_list_first(list);
		for (; node; node = muggle_linked_list_next(list, node))
		{
			ASSERT_EQ(*(int*)node->data, expect);
			expect++;
		}
		ASSERT_EQ(expect, TEST_LINKED_LIST_LEN);

		expect = TEST_LINKED_LIST_LEN - 1;;
		node = muggle_linked_list_last(list);
		for (; node; node = muggle_linked_list_prev(list, node))
		{
			ASSERT_EQ(*(int*)node->data, expect);
			expect--;
		}
		ASSERT_EQ(expect, -1);
	}
}

TEST_F(TestLinkedListFixture, remove)
{
	for (int index = 0; index < (int)(sizeof(list_) / sizeof(list_[0])); index++)
	{
		muggle_linked_list_t *list = &list_[index];

		for (int i = 0; i < TEST_LINKED_LIST_LEN; i++)
		{
			int *p = test_utils_.allocateInteger();
			ASSERT_TRUE(p != NULL);

			*p = i;

			muggle_linked_list_node_t *node = muggle_linked_list_append(list, NULL, p);

			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(node->data, p);
			ASSERT_EQ(muggle_linked_list_size(list), (size_t)(i + 1));
		}

		int expect_cnt = TEST_LINKED_LIST_LEN;
		muggle_linked_list_node_t *node = muggle_linked_list_first(list);
		while (node)
		{
			ASSERT_EQ(muggle_linked_list_size(list), (size_t)(expect_cnt));
			node = muggle_linked_list_remove(list, node, test_utils_free_int, &test_utils_);
			expect_cnt--;
		}
	}
}

TEST_F(TestLinkedListFixture, clear)
{
	for (int index = 0; index < (int)(sizeof(list_) / sizeof(list_[0])); index++)
	{
		muggle_linked_list_t *list = &list_[index];

		for (int i = 0; i < TEST_LINKED_LIST_LEN; i++)
		{
			int *p = test_utils_.allocateInteger();
			ASSERT_TRUE(p != NULL);

			*p = i;

			muggle_linked_list_insert(list, NULL, p);
		}

		bool ret = muggle_linked_list_is_empty(list);
		ASSERT_FALSE(ret);

		muggle_linked_list_clear(list, test_utils_free_int, &test_utils_);

		ret = muggle_linked_list_is_empty(list);
		ASSERT_TRUE(ret);
		ASSERT_EQ(muggle_linked_list_size(list), (size_t)0);
	}
}

TEST_F(TestLinkedListFixture, find)
{
	for (int index = 0; index < (int)(sizeof(list_) / sizeof(list_[0])); index++)
	{
		muggle_linked_list_t *list = &list_[index];

		for (int i = 0; i < TEST_LINKED_LIST_LEN; i++)
		{
			int *p = test_utils_.allocateInteger();
			ASSERT_TRUE(p != NULL);

			*p = i;

			muggle_linked_list_node_t *node = muggle_linked_list_append(list, NULL, p);

			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(node->data, p);
			ASSERT_EQ(muggle_linked_list_size(list), (size_t)(i + 1));
		}

		muggle_linked_list_node_t *last_node = NULL;
		for (int i = 0; i < TEST_LINKED_LIST_LEN; i++)
		{
			muggle_linked_list_node_t *node = muggle_linked_list_find(list, NULL, &i, test_utils_cmp_int);
			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(*(int*)node->data, i);

			node = muggle_linked_list_find(list, last_node, &i, test_utils_cmp_int);
			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(*(int*)node->data, i);

			last_node = node;
		}
	}
}
