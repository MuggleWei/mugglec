#include "muggle/c/muggle_c.h"

#define TOTAL_DATA 16384

typedef struct user_data
{
	char key[32];
	int val;
} user_data_t;

int cmp_str(const void *p1, const void *p2)
{
	if (p1 == p2)
	{
		return 0;
	}

	if (p1 == NULL && p2 != NULL)
	{
		return -1;
	}

	if (p1 != NULL && p2 == NULL)
	{
		return 1;
	}

	return strcmp((char*)p1, (char*)p2);
}

void do_free(void *pool, void *data)
{
	free(data);
}

int main()
{
	muggle_log_complicated_init(LOG_LEVEL_TRACE, -1, NULL);

	srand((unsigned int)time(NULL));

	const char **key_array = (const char**)malloc(sizeof(const char*) * TOTAL_DATA);

	muggle_hash_table_t table;
	muggle_hash_table_init(&table, 0, NULL, cmp_str, 0);

	int cnt = 0;
	for (int i = 0; i < TOTAL_DATA; i++)
	{
		user_data_t *data = (user_data_t*)malloc(sizeof(user_data_t));
		memset(data, 0, sizeof(*data));
		data->val = i;
		for (int j = 0; j < 31; ++j)
		{
			data->key[j] = (char)('A' + rand() % 26);
		}

		if (muggle_hash_table_put(&table, data->key, data) == NULL)
		{
			muggle_hash_table_node_t *node = muggle_hash_table_find(&table, data->key);
			if (node != NULL)
			{
				LOG_WARNING("failed put data: %s, cause duplicated key", data->key);
			}
			else
			{
				LOG_ERROR("failed put data: %s", data->key);
			}
			free(data);
			key_array[i] = NULL;
			continue;
		}
		++cnt;

		key_array[i] = data->key;
	}
	LOG_INFO("total put data: %d", cnt);

	cnt = 0;
	for (int i = 0; i < TOTAL_DATA; i++)
	{
		if (key_array[i] == NULL)
		{
			continue;
		}

		muggle_hash_table_node_t *node = muggle_hash_table_find(&table, (void*)key_array[i]);
		const char *key = (const char*)node->key;
		int value = ((user_data_t*)node->value)->val;
		++cnt;
	}
	LOG_INFO("total found data: %d", cnt);

	muggle_hash_table_destroy(&table, NULL, NULL, do_free, NULL);

	free((void*)key_array);

	return 0;
}