#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"
#include "test_utils/test_utils.h"

#define TEST_AVL_TREE_LEN 64

class TestAvlTreeFixture : public ::testing::Test
{
public:
	void SetUp()
	{
		muggle_debug_memory_leak_start(&mem_state_);

		bool ret;

		ret = muggle_avl_tree_init(&tree_[0], test_utils_cmp_int, 0);
		ASSERT_TRUE(ret);

		ret = muggle_avl_tree_init(&tree_[1], test_utils_cmp_int, 8);
		ASSERT_TRUE(ret);
	}

	void TearDown()
	{
		muggle_avl_tree_destroy(&tree_[0], test_utils_free_int, &test_utils_, test_utils_free_str, &test_utils_);
		muggle_avl_tree_destroy(&tree_[1], test_utils_free_int, &test_utils_, test_utils_free_str, &test_utils_);

		muggle_debug_memory_leak_end(&mem_state_);
	}

protected:
	muggle_avl_tree_t tree_[2];

	TestUtils test_utils_;
	muggle_debug_memory_state mem_state_;
};

void TestAvlTreeNodeDeep(muggle_avl_tree_node_t *node, int &deep, int &max_val)
{
	int deep_left = -1;
	int deep_right = -1;

	if (node->left)
	{
		TestAvlTreeNodeDeep(node->left, deep_left, max_val);
	}

	ASSERT_LT(max_val, *(int*)node->key);
	max_val = *(int*)node->key;

	if (node->right)
	{
		TestAvlTreeNodeDeep(node->right, deep_right, max_val);
	}

	int balance_factor = deep_right - deep_left;
	ASSERT_EQ(balance_factor, node->balance);
	ASSERT_LE(balance_factor, 1);
	ASSERT_GE(balance_factor, -1);

	deep = deep_left > deep_right ? deep_left : deep_right;
	deep += 1;
}

void TestAvlTreeCheckValid(muggle_avl_tree_t *tree)
{
	if (tree->root == NULL)
	{
		return;
	}

	int deep = 0;
	int max_val = -1;
	TestAvlTreeNodeDeep(tree->root, deep, max_val);
}

void TestAvlTreeNodePrint(muggle_avl_tree_node_t *node)
{
	if (node->left)
	{
		TestAvlTreeNodePrint(node->left);
	}

	printf("%d ", *(int*)node->key);

	if (node->right)
	{
		TestAvlTreeNodePrint(node->right);
	}
}

void TestAvlTreePrint(muggle_avl_tree_t *tree)
{
	if (tree->root)
	{
		TestAvlTreeNodePrint(tree->root);
	}
	printf("\n");
}

TEST_F(TestAvlTreeFixture, insert_find)
{
	for (int index = 0; index < (int)(sizeof(tree_) / sizeof(tree_[0])); index++)
	{
		muggle_avl_tree_t *tree = &tree_[index];

		for (int i = 0; i < TEST_AVL_TREE_LEN; i++)
		{
			int *p = test_utils_.allocateInteger();
			ASSERT_TRUE(p != NULL);

			char *s = test_utils_.allocateString();

			*p = i;
			snprintf(s, TEST_UTILS_STR_SIZE, "%d", i);

			muggle_avl_tree_node_t *node = muggle_avl_tree_insert(tree, p, s);

			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(node->key, p);
			ASSERT_EQ(node->value, s);

			TestAvlTreeCheckValid(tree);
		}

		for (int i = 0; i < TEST_AVL_TREE_LEN; i++)
		{
			muggle_avl_tree_node_t *node = muggle_avl_tree_find(tree, &i);

			ASSERT_TRUE(node != NULL);
			ASSERT_LE((int)node->balance, 1);
			ASSERT_GE((int)node->balance, -1);

			char buf[TEST_UTILS_STR_SIZE];
			snprintf(buf, TEST_UTILS_STR_SIZE, "%d", i);

			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(*(int*)node->key, i);
			ASSERT_STREQ((char*)node->value, buf);

			TestAvlTreeCheckValid(tree);
		}
	}
}

// insert/remove integer array: 1, 2, 3 ...... n
TEST_F(TestAvlTreeFixture, insert_remove_case1)
{
	for (int index = 0; index < (int)(sizeof(tree_) / sizeof(tree_[0])); index++)
	{
		muggle_avl_tree_t *tree = &tree_[index];

		for (int i = 0; i < TEST_AVL_TREE_LEN; i++)
		{
			int *p = test_utils_.allocateInteger();
			ASSERT_TRUE(p != NULL);

			char *s = test_utils_.allocateString();

			*p = i;
			snprintf(s, TEST_UTILS_STR_SIZE, "%d", i);

			muggle_avl_tree_node_t *node = muggle_avl_tree_insert(tree, p, s);

			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(node->key, p);
			ASSERT_EQ(node->value, s);

			ASSERT_LE((int)node->balance, 1);
			ASSERT_GE((int)node->balance, -1);

			TestAvlTreeCheckValid(tree);
		}

		for (int i = 0; i < TEST_AVL_TREE_LEN; i++)
		{
			muggle_avl_tree_node_t *node = muggle_avl_tree_find(tree, &i);

			ASSERT_TRUE(node != NULL);
			ASSERT_LE((int)node->balance, 1);
			ASSERT_GE((int)node->balance, -1);
		}

		for (int i = 0; i < TEST_AVL_TREE_LEN; i++)
		{
			muggle_avl_tree_node_t *node = muggle_avl_tree_find(tree, &i);

			ASSERT_TRUE(node != NULL);
			ASSERT_LE((int)node->balance, 1);
			ASSERT_GE((int)node->balance, -1);

			char buf[TEST_UTILS_STR_SIZE];
			snprintf(buf, TEST_UTILS_STR_SIZE, "%d", i);

			ASSERT_EQ(*(int*)node->key, i);
			ASSERT_STREQ((char*)node->value, buf);

			muggle_avl_tree_remove(tree, node, test_utils_free_int, &test_utils_, test_utils_free_str, &test_utils_);

			TestAvlTreeCheckValid(tree);
		}

		for (int i = 0; i < TEST_AVL_TREE_LEN; i++)
		{
			muggle_avl_tree_node_t *node = muggle_avl_tree_find(tree, &i);
			ASSERT_TRUE(node == NULL);
		}
	}
}

// insert/remove integer array: n, n-1, n-2 ...... 1
TEST_F(TestAvlTreeFixture, insert_remove_case2)
{
	for (int index = 0; index < (int)(sizeof(tree_) / sizeof(tree_[0])); index++)
	{
		muggle_avl_tree_t *tree = &tree_[index];

		for (int i = TEST_AVL_TREE_LEN - 1; i >= 0; i--)
		{
			int *p = test_utils_.allocateInteger();
			ASSERT_TRUE(p != NULL);

			char *s = test_utils_.allocateString();

			*p = i;
			snprintf(s, TEST_UTILS_STR_SIZE, "%d", i);

			muggle_avl_tree_node_t *node = muggle_avl_tree_insert(tree, p, s);

			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(node->key, p);
			ASSERT_EQ(node->value, s);
			ASSERT_LE((int)node->balance, 1);
			ASSERT_GE((int)node->balance, -1);

			TestAvlTreeCheckValid(tree);
		}

		for (int i = TEST_AVL_TREE_LEN - 1; i >= 0; i--)
		{
			muggle_avl_tree_node_t *node = muggle_avl_tree_find(tree, &i);

			ASSERT_TRUE(node != NULL);
			ASSERT_LE((int)node->balance, 1);
			ASSERT_GE((int)node->balance, -1);
		}

		for (int i = TEST_AVL_TREE_LEN - 1; i >= 0; i--)
		{
			muggle_avl_tree_node_t *node = muggle_avl_tree_find(tree, &i);

			ASSERT_LE((int)node->balance, 1);
			ASSERT_GE((int)node->balance, -1);

			char buf[TEST_UTILS_STR_SIZE];
			snprintf(buf, TEST_UTILS_STR_SIZE, "%d", i);

			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(*(int*)node->key, i);
			ASSERT_STREQ((char*)node->value, buf);

			muggle_avl_tree_remove(tree, node, test_utils_free_int, &test_utils_, test_utils_free_str, &test_utils_);

			TestAvlTreeCheckValid(tree);
		}

		for (int i = TEST_AVL_TREE_LEN - 1; i >= 0; i--)
		{
			muggle_avl_tree_node_t *node = muggle_avl_tree_find(tree, &i);
			ASSERT_TRUE(node == NULL);
		}
	}
}

// insert remove integer array: 1, 0, 3, 2 ... 2k + 1, 2k ... 2n + 1, 2n
TEST_F(TestAvlTreeFixture, insert_remove_case3)
{
	for (int index = 0; index < (int)(sizeof(tree_) / sizeof(tree_[0])); index++)
	{
		muggle_avl_tree_t *tree = &tree_[index];

		for (int i = 0; i < TEST_AVL_TREE_LEN; i++)
		{
			for (int j = 1; j >= 0; j--)
			{
				int *p = test_utils_.allocateInteger();
				ASSERT_TRUE(p != NULL);

				char *s = test_utils_.allocateString();

				*p = 2 * i + j;
				snprintf(s, TEST_UTILS_STR_SIZE, "%d", *p);

				muggle_avl_tree_node_t *node = muggle_avl_tree_insert(tree, p, s);

				ASSERT_TRUE(node != NULL);
				ASSERT_EQ(node->key, p);
				ASSERT_EQ(node->value, s);

				TestAvlTreeCheckValid(tree);
			}
		}

		for (int i = 0; i < TEST_AVL_TREE_LEN; i++)
		{
			for (int j = 1; j >= 0; j--)
			{
				int tmp = 2 * i + j;
				muggle_avl_tree_node_t *node = muggle_avl_tree_find(tree, &tmp);

				ASSERT_TRUE(node != NULL);
				ASSERT_LE((int)node->balance, 1);
				ASSERT_GE((int)node->balance, -1);
			}
		}

		for (int i = 0; i < TEST_AVL_TREE_LEN; i++)
		{
			muggle_avl_tree_node_t *node = NULL;
			for (int j = 1; j >= 0; j--)
			{
				int tmp = 2 * i + j;
				node = muggle_avl_tree_find(tree, &tmp);

				ASSERT_TRUE(node != NULL);
				ASSERT_LE((int)node->balance, 1);
				ASSERT_GE((int)node->balance, -1);

				char buf[TEST_UTILS_STR_SIZE];
				snprintf(buf, TEST_UTILS_STR_SIZE, "%d", tmp);

				ASSERT_EQ(*(int*)node->key, tmp);
				ASSERT_STREQ((char*)node->value, buf);

				muggle_avl_tree_remove(tree, node, test_utils_free_int, &test_utils_, test_utils_free_str, &test_utils_);

				// TestAvlTreePrint(tree);

				TestAvlTreeCheckValid(tree);
			}
		}

		for (int i = 0; i < TEST_AVL_TREE_LEN; i++)
		{
			for (int j = 1; j >= 0; j--)
			{
				int tmp = 2 * i + j;

				muggle_avl_tree_node_t *node = muggle_avl_tree_find(tree, &tmp);
				ASSERT_TRUE(node == NULL);
			}
		}
	}
}

// insert remove integer array: 2n + 1, 2n ... 2k + 1, 2k ... 3, 2, 0, 1
TEST_F(TestAvlTreeFixture, insert_remove_case4)
{
	for (int index = 0; index < (int)(sizeof(tree_) / sizeof(tree_[0])); index++)
	{
		muggle_avl_tree_t *tree = &tree_[index];

		for (int i = TEST_AVL_TREE_LEN - 1; i >= 0; i--)
		{
			for (int j = 1; j >= 0; j--)
			{
				int *p = test_utils_.allocateInteger();
				ASSERT_TRUE(p != NULL);

				char *s = test_utils_.allocateString();

				*p = 2 * i + j;
				snprintf(s, TEST_UTILS_STR_SIZE, "%d", *p);

				muggle_avl_tree_node_t *node = muggle_avl_tree_insert(tree, p, s);

				ASSERT_TRUE(node != NULL);
				ASSERT_EQ(node->key, p);
				ASSERT_EQ(node->value, s);

				TestAvlTreeCheckValid(tree);
			}
		}

		for (int i = TEST_AVL_TREE_LEN - 1; i >= 0; i--)
		{
			for (int j = 1; j >= 0; j--)
			{
				int tmp = 2 * i + j;
				muggle_avl_tree_node_t *node = muggle_avl_tree_find(tree, &tmp);

				ASSERT_TRUE(node != NULL);
				ASSERT_LE((int)node->balance, 1);
				ASSERT_GE((int)node->balance, -1);
			}
		}

		for (int i = TEST_AVL_TREE_LEN - 1; i >= 0; i--)
		{
			muggle_avl_tree_node_t *node = NULL;
			for (int j = 1; j >= 0; j--)
			{
				int tmp = 2 * i + j;
				node = muggle_avl_tree_find(tree, &tmp);

				ASSERT_LE((int)node->balance, 1);
				ASSERT_GE((int)node->balance, -1);

				char buf[TEST_UTILS_STR_SIZE];
				snprintf(buf, TEST_UTILS_STR_SIZE, "%d", tmp);

				ASSERT_TRUE(node != NULL);
				ASSERT_EQ(*(int*)node->key, tmp);
				ASSERT_STREQ((char*)node->value, buf);

				muggle_avl_tree_remove(tree, node, test_utils_free_int, &test_utils_, test_utils_free_str, &test_utils_);

				TestAvlTreeCheckValid(tree);
			}
		}

		for (int i = TEST_AVL_TREE_LEN - 1; i >= 0; i--)
		{
			for (int j = 1; j >= 0; j--)
			{
				int tmp = 2 * i + j;

				muggle_avl_tree_node_t *node = muggle_avl_tree_find(tree, &tmp);
				ASSERT_TRUE(node == NULL);
			}
		}
	}
}

// insert remove integer array: manual set array
TEST_F(TestAvlTreeFixture, insert_remove_case5)
{
	int arr[] = {
		1,5,6,0,3,4,7,2
	};

	for (int index = 0; index < (int)(sizeof(tree_) / sizeof(tree_[0])); index++)
	{
		muggle_avl_tree_t *tree = &tree_[index];

		// printf("----------------------------------\n");
		for (size_t i = 0; i < sizeof(arr) / sizeof(arr[0]); i++)
		{
			// printf("insert %d\n", arr[i]);
			muggle_avl_tree_node_t *node = muggle_avl_tree_insert(tree, &arr[i], &arr[i]);
			// TestAvlTreePrint(tree);
			ASSERT_TRUE(node != NULL);
			TestAvlTreeCheckValid(tree);
		}

		// find
		for (size_t i = 0; i < sizeof(arr) / sizeof(arr[0]); i++)
		{
			muggle_avl_tree_node_t *node = muggle_avl_tree_find(tree, &arr[i]);
			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(*(int*)node->key, arr[i]);
		}

		// remove
		for (size_t i = 0; i < sizeof(arr) / sizeof(arr[0]); i++)
		{
			// printf("remove %d\n", arr[i]);
			muggle_avl_tree_node_t *node = muggle_avl_tree_find(tree, &arr[i]);
			ASSERT_TRUE(node != NULL);

			muggle_avl_tree_remove(tree, node, NULL, NULL, NULL, NULL);
			// TestAvlTreePrint(tree);

			TestAvlTreeCheckValid(tree);
		}

		// find
		for (size_t i = 0; i < sizeof(arr) / sizeof(arr[0]); i++)
		{
			muggle_avl_tree_node_t *node = muggle_avl_tree_find(tree, &arr[i]);
			ASSERT_TRUE(node == NULL);
		}
	}
}

// insert remove integer array: random
TEST_F(TestAvlTreeFixture, insert_remove_case6)
{
	srand((unsigned int)time(NULL));
	for (int index = 0; index < (int)(sizeof(tree_) / sizeof(tree_[0])); index++)
	{
		muggle_avl_tree_t *tree = &tree_[index];

		int *arr = (int*)malloc(sizeof(int*) * TEST_AVL_TREE_LEN);
		for (int i = 0; i < TEST_AVL_TREE_LEN; i++)
		{
			arr[i] = i;
		}

		// shuffle
		for (int i = 0; i < TEST_AVL_TREE_LEN; i++)
		{
			int idx = rand() % TEST_AVL_TREE_LEN;
			int tmp = arr[i];
			arr[i] = arr[idx];
			arr[idx] = tmp;
		}

		// insert
		printf("----------------------------------\n");
		printf("insert: ");
		for (int i = 0; i < TEST_AVL_TREE_LEN; i++)
		{
			printf("%d ", arr[i]);
			muggle_avl_tree_node_t *node = muggle_avl_tree_insert(tree, &arr[i], &arr[i]);
			ASSERT_TRUE(node != NULL);
			TestAvlTreeCheckValid(tree);
		}
		printf("\n");
		printf("traversal: ");
		TestAvlTreePrint(tree);

		// find
		for (int i = 0; i < TEST_AVL_TREE_LEN; i++)
		{
			muggle_avl_tree_node_t *node = muggle_avl_tree_find(tree, &arr[i]);
			ASSERT_TRUE(node != NULL);
			ASSERT_EQ(*(int*)node->key, arr[i]);
		}

		// shuffle
		int *arr2 = (int*)malloc(sizeof(int*) * TEST_AVL_TREE_LEN);
		for (int i = 0; i < TEST_AVL_TREE_LEN; i++)
		{
			arr2[i] = arr[i];
		}
		for (int i = 0; i < TEST_AVL_TREE_LEN; i++)
		{
			int idx = rand() % TEST_AVL_TREE_LEN;
			int tmp = arr2[i];
			arr2[i] = arr2[idx];
			arr2[idx] = tmp;
		}

		// remove
		printf("remove: ");
		for (int i = 0; i < TEST_AVL_TREE_LEN; i++)
		{
			printf("%d ", arr2[i]);
			muggle_avl_tree_node_t *node = muggle_avl_tree_find(tree, &arr2[i]);
			ASSERT_TRUE(node != NULL);

			muggle_avl_tree_remove(tree, node, NULL, NULL, NULL, NULL);
			TestAvlTreeCheckValid(tree);
		}
		printf("\n");

		// find
		for (int i = 0; i < TEST_AVL_TREE_LEN; i++)
		{
			muggle_avl_tree_node_t *node = muggle_avl_tree_find(tree, &arr[i]);
			ASSERT_TRUE(node == NULL);
		}

		free(arr2);
		free(arr);
	}
}
