#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"
#include "test_utils/test_utils.h"

#define TEST_STACK_LEN 64

class TestStackFixture : public ::testing::Test
{
public:
	void SetUp()
	{
		muggle_debug_memory_leak_start(&mem_state_);

		bool ret;

		ret = muggle_stack_init(&stack_[0], 0);
		ASSERT_TRUE(ret);

		ret = muggle_stack_init(&stack_[1], 8);
		ASSERT_TRUE(ret);

		for (int index = 0; index < (int)(sizeof(stack_) / sizeof(stack_[index])); index++)
		{
			ASSERT_EQ(muggle_stack_size(&stack_[index]), (size_t)0);
		}
	}

	void TearDown()
	{
		muggle_stack_destroy(&stack_[0], test_utils_free_int, &test_utils_);
		muggle_stack_destroy(&stack_[1], test_utils_free_int, &test_utils_);

		muggle_debug_memory_leak_end(&mem_state_);
	}

protected:
	muggle_stack_t stack_[2];

	TestUtils test_utils_;
	muggle_debug_memory_state mem_state_;
};

TEST_F(TestStackFixture, empty)
{
	for (int index = 0; index < (int)(sizeof(stack_) / sizeof(stack_[0])); index++)
	{
		bool ret = muggle_stack_is_empty(&stack_[index]);
		ASSERT_TRUE(ret);

		muggle_stack_node_t *node = muggle_stack_top(&stack_[index]);
		ASSERT_TRUE(node == NULL);
	}
}

TEST_F(TestStackFixture, push_top_pop)
{
	for (int index = 0; index < (int)(sizeof(stack_) / sizeof(stack_[0])); index++)
	{
		muggle_stack_t *p_stack = &stack_[index];

		for (int i = 0; i < TEST_STACK_LEN; i++)
		{
			int *p = test_utils_.allocateInteger();
			ASSERT_TRUE(p != NULL);

			*p = i;

			muggle_stack_node_t *node = muggle_stack_push(p_stack, p);

			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(node->data, p);
			ASSERT_EQ(muggle_stack_size(p_stack), (size_t)(i + 1));
		}

		int expect = TEST_STACK_LEN - 1;
		while (!muggle_stack_is_empty(p_stack))
		{
			muggle_stack_node_t *node = muggle_stack_top(p_stack);
			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(*(int*)node->data, expect);
			expect--;

			muggle_stack_pop(p_stack, test_utils_free_int, &test_utils_);
		}
	}
}

TEST_F(TestStackFixture, clear)
{
	for (int index = 0; index < (int)(sizeof(stack_) / sizeof(stack_[0])); index++)
	{
		muggle_stack_t *p_stack = &stack_[index];

		for (int i = 0; i < TEST_STACK_LEN; i++)
		{
			int *p = test_utils_.allocateInteger();
			ASSERT_TRUE(p != NULL);

			*p = i;

			muggle_stack_node_t *node = muggle_stack_push(p_stack, p);

			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(node->data, p);
			ASSERT_EQ(muggle_stack_size(p_stack), (size_t)(i + 1));
		}

		bool ret = muggle_stack_is_empty(p_stack);
		ASSERT_FALSE(ret);

		muggle_stack_clear(p_stack, test_utils_free_int, &test_utils_);

		ret = muggle_stack_is_empty(p_stack);
		ASSERT_TRUE(ret);
		ASSERT_EQ(muggle_stack_size(p_stack), (size_t)0);
	}
}
