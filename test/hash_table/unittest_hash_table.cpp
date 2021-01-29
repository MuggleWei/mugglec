#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"
#include "test_utils/test_utils.h"

#define TEST_HASH_TABLE_LEN 1024

class TestHashTableFixture : public ::testing::Test
{
public:
	void SetUp()
	{
		muggle_debug_memory_leak_start(&mem_state_);

		bool ret;

		ret = muggle_hash_table_init(&tables_[0], 0, NULL, test_utils_cmp_str, 0);
		ASSERT_TRUE(ret);

		ret = muggle_hash_table_init(&tables_[1], 0, NULL, test_utils_cmp_str, 16);
		ASSERT_TRUE(ret);
	}

	void TearDown()
	{
		muggle_hash_table_destroy(&tables_[0], test_utils_free_str, &test_utils_, test_utils_free_int, &test_utils_);
		muggle_hash_table_destroy(&tables_[1], test_utils_free_str, &test_utils_, test_utils_free_int, &test_utils_);

		muggle_debug_memory_leak_end(&mem_state_);
	}

protected:
	muggle_hash_table_t tables_[2];

	TestUtils test_utils_;
	muggle_debug_memory_state mem_state_;
};

void TestHashTablePrint(muggle_hash_table_t *table)
{
	int cnt_dict[8];
	memset(cnt_dict, 0, sizeof(cnt_dict));

	for (uint64_t i = 0; i < table->table_size; i++)
	{
		int cnt = 0;
		muggle_hash_table_node_t *node = &table->nodes[i];
		if (node->next)
		{
			node = node->next;

			// printf("%5d | ", (int)i);
			while (node)
			{
				// int v = *(int*)node->value;
				// printf("%5d ", v);

				node = node->next;
				++cnt;
			}
			// printf("\n");
		}

		if (cnt >= (int)(sizeof(cnt_dict) / sizeof(cnt_dict[0]) - 1))
		{
			cnt_dict[sizeof(cnt_dict) / sizeof(cnt_dict[0]) - 1] += 1;
		}
		else
		{
			cnt_dict[cnt] += 1;
		}
	}

	printf("hash table count dict: \n");
	printf("node number | count\n");
	for (int i = 0; i < (int)(sizeof(cnt_dict) / sizeof(cnt_dict[0])); i++)
	{
		printf("%11d | %d\n", i, cnt_dict[i]);
	}
}

TEST_F(TestHashTableFixture, put_find)
{
	for (int index = 0; index < (int)(sizeof(tables_) / sizeof(tables_[0])); index++)
	{
		muggle_hash_table_t *table = &tables_[index];

		for (int i = 0; i < TEST_HASH_TABLE_LEN; i++)
		{
			int *p = test_utils_.allocateInteger();
			ASSERT_TRUE(p != NULL);

			char *s = test_utils_.allocateString();

			*p = i;
			snprintf(s, TEST_UTILS_STR_SIZE, "%d", i);

			muggle_hash_table_node_t *node = muggle_hash_table_put(table, s, p);

			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(node->key, s);
			ASSERT_EQ(node->value, p);
		}

		for (int i = 0; i < TEST_HASH_TABLE_LEN; i++)
		{
			char buf[16];
			snprintf(buf, sizeof(buf), "%d", i);

			muggle_hash_table_node_t *node = muggle_hash_table_find(table, buf);

			ASSERT_TRUE(node != NULL);
			ASSERT_STREQ((char*)node->key, buf);
			ASSERT_EQ(*(int*)node->value, i);
		}

		printf("------------------------------------\n");
		TestHashTablePrint(table);
	}
}

TEST_F(TestHashTableFixture, put_remove)
{
	for (int index = 0; index < (int)(sizeof(tables_) / sizeof(tables_[0])); index++)
	{
		muggle_hash_table_t *table = &tables_[index];

		for (int i = 0; i < TEST_HASH_TABLE_LEN; i++)
		{
			int *p = test_utils_.allocateInteger();
			ASSERT_TRUE(p != NULL);

			char *s = test_utils_.allocateString();

			*p = i;
			snprintf(s, TEST_UTILS_STR_SIZE, "%d", i);

			muggle_hash_table_node_t *node = muggle_hash_table_put(table, s, p);

			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(node->key, s);
			ASSERT_EQ(node->value, p);
		}

		for (int i = 0; i < TEST_HASH_TABLE_LEN; i++)
		{
			char buf[16];
			snprintf(buf, sizeof(buf), "%d", i);

			muggle_hash_table_node_t *node = muggle_hash_table_find(table, buf);

			ASSERT_TRUE(node != NULL);
			ASSERT_STREQ((char*)node->key, buf);
			ASSERT_EQ(*(int*)node->value, i);
		}

		for (int i = 0; i < TEST_HASH_TABLE_LEN; i++)
		{
			char buf[16];
			snprintf(buf, sizeof(buf), "%d", i);

			muggle_hash_table_node_t *node = muggle_hash_table_find(table, buf);
			ASSERT_TRUE(node != NULL);
			muggle_hash_table_remove(table, node, test_utils_free_str, &test_utils_, test_utils_free_int, &test_utils_);
		}

		for (int i = 0; i < TEST_HASH_TABLE_LEN; i++)
		{
			char buf[16];
			snprintf(buf, sizeof(buf), "%d", i);

			muggle_hash_table_node_t *node = muggle_hash_table_find(table, buf);

			ASSERT_TRUE(node == NULL);
		}
	}
}
