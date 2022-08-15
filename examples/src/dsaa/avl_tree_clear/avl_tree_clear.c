#include "muggle/c/muggle_c.h"

typedef struct user_data
{
	int idx;
	int val;
} user_data_t;

int cmp_int(const void *p1, const void *p2)
{
	int i1 = *(int*)p1;
	int i2 = *(int*)p2;
	return i1 - i2;
}

void do_free(void *pool, void *data)
{
	free(data);
}

int main()
{
	muggle_log_complicated_init(LOG_LEVEL_TRACE, -1, NULL);

	muggle_avl_tree_t tree;
	muggle_avl_tree_init(&tree, cmp_int, 0);

	for (int i = 0; i < 16; i++)
	{
		user_data_t *data = (user_data_t*)malloc(sizeof(user_data_t));
		data->idx = i;
		data->val = i / 2;
		muggle_avl_tree_insert(&tree, &data->idx, data);
		LOG_INFO("insert data: [%d]%d", data->idx, data->val);
	}

	for (int i = 0; i < 16; i++)
	{
		muggle_avl_tree_node_t *node = muggle_avl_tree_find(&tree, &i);
		MUGGLE_ASSERT(node != NULL);
		user_data_t *data = (user_data_t*)node->value;
		LOG_INFO("found data: [%d]%d", *(int*)node->key, data->val);
	}

	muggle_avl_tree_clear(&tree, NULL, NULL, do_free, NULL);

	muggle_avl_tree_destroy(&tree, NULL, NULL, NULL, NULL);

	return 0;
}