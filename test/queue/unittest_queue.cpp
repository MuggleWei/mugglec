#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"
#include "test_utils/test_utils.h"

#define TEST_QUEUE_LEN 64

class TestQueueFixture : public ::testing::Test
{
public:
	void SetUp()
	{
		muggle_debug_memory_leak_start(&mem_state_);

		bool ret;

		ret = muggle_queue_init(&queue_[0], 0);
		ASSERT_TRUE(ret);

		ret = muggle_queue_init(&queue_[1], 8);
		ASSERT_TRUE(ret);

		for (int index = 0; index < (int)(sizeof(queue_) / sizeof(queue_[index])); index++)
		{
			ASSERT_EQ(muggle_queue_size(&queue_[index]), (size_t)0);
		}
	}

	void TearDown()
	{
		muggle_queue_destroy(&queue_[0], test_utils_free_int, &test_utils_);
		muggle_queue_destroy(&queue_[1], test_utils_free_int, &test_utils_);

		muggle_debug_memory_leak_end(&mem_state_);
	}

protected:
	muggle_queue_t queue_[2];

	TestUtils test_utils_;
	muggle_debug_memory_state mem_state_;
};

TEST_F(TestQueueFixture, empty)
{
	for (int index = 0; index < (int)(sizeof(queue_) / sizeof(queue_[0])); index++)
	{
		bool ret = muggle_queue_is_empty(&queue_[index]);
		ASSERT_TRUE(ret);
		ASSERT_EQ(muggle_queue_size(&queue_[index]), (size_t)0);
	}
}

TEST_F(TestQueueFixture, enqueue_dequeue)
{
	for (int index = 0; index < (int)(sizeof(queue_) / sizeof(queue_[0])); index++)
	{
		muggle_queue_t *p_queue = &queue_[index];

		for (int i = 0; i < TEST_QUEUE_LEN; i++)
		{
			int *p = test_utils_.allocateInteger();
			ASSERT_TRUE(p != NULL);

			*p = i;

			muggle_queue_node_t *node = muggle_queue_enqueue(p_queue, p);

			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(node->data, p);
			ASSERT_EQ(muggle_queue_size(p_queue), (size_t)(i + 1));
		}

		int expect = 0;
		while (!muggle_queue_is_empty(p_queue))
		{
			muggle_queue_node_t *node = muggle_queue_front(p_queue);
			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(*(int*)node->data, expect);
			expect++;

			muggle_queue_dequeue(p_queue, test_utils_free_int, &test_utils_);
		}
	}
}

TEST_F(TestQueueFixture, clear)
{
	for (int index = 0; index < (int)(sizeof(queue_) / sizeof(queue_[0])); index++)
	{
		muggle_queue_t *p_queue = &queue_[index];

		for (int i = 0; i < TEST_QUEUE_LEN; i++)
		{
			int *p = test_utils_.allocateInteger();
			ASSERT_TRUE(p != NULL);

			*p = i;

			muggle_queue_node_t *node = muggle_queue_enqueue(p_queue, p);

			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(node->data, p);
			ASSERT_EQ(muggle_queue_size(p_queue), (size_t)(i + 1));
		}

		bool ret = muggle_queue_is_empty(p_queue);
		ASSERT_FALSE(ret);

		muggle_queue_clear(p_queue, test_utils_free_int, &test_utils_);

		ret = muggle_queue_is_empty(p_queue);
		ASSERT_TRUE(ret);
		ASSERT_EQ(muggle_queue_size(p_queue), (size_t)0);
	}
}
