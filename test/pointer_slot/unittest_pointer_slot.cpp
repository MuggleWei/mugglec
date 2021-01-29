#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

TEST(pointer_slot, insert_get_remove)
{
	muggle_pointer_slot_t ptr_slot;
	int ret = muggle_pointer_slot_init(&ptr_slot, 2);
	ASSERT_EQ(ret, 0);

	void *data;
	unsigned int idx;
	int arr[3] = { 1, 2, 3 };

	ret = muggle_pointer_slot_insert(&ptr_slot, &arr[0], &idx);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(idx, (unsigned int)0);

	ret = muggle_pointer_slot_insert(&ptr_slot, &arr[1], &idx);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(idx, (unsigned int)1);

	ret = muggle_pointer_slot_insert(&ptr_slot, &arr[2], &idx);
	ASSERT_NE(ret, 0);

	data = muggle_pointer_slot_get(&ptr_slot, 0);
	ASSERT_EQ(data, &arr[0]);

	data = muggle_pointer_slot_get(&ptr_slot, 1);
	ASSERT_EQ(data, &arr[1]);

	ret = muggle_pointer_slot_remove(&ptr_slot, 1);
	ASSERT_EQ(ret, 0);

	data = muggle_pointer_slot_get(&ptr_slot, 1);
	ASSERT_EQ(data, nullptr);

	ret = muggle_pointer_slot_remove(&ptr_slot, 0);
	ASSERT_EQ(ret, 0);

	data = muggle_pointer_slot_get(&ptr_slot, 0);
	ASSERT_EQ(data, nullptr);

	ret = muggle_pointer_slot_insert(&ptr_slot, &arr[0], &idx);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(idx, (unsigned int)1);

	ret = muggle_pointer_slot_insert(&ptr_slot, &arr[1], &idx);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(idx, (unsigned int)0);

	data = muggle_pointer_slot_get(&ptr_slot, 0);
	ASSERT_EQ(data, &arr[1]);

	data = muggle_pointer_slot_get(&ptr_slot, 1);
	ASSERT_EQ(data, &arr[0]);


	ret = muggle_pointer_slot_insert(&ptr_slot, &arr[2], &idx);
	ASSERT_NE(ret, 0);

	for (int i = 0; i < 20; i++)
	{
		ret = muggle_pointer_slot_remove(&ptr_slot, 1);
		ASSERT_EQ(ret, 0);
		ret = muggle_pointer_slot_remove(&ptr_slot, 0);
		ASSERT_EQ(ret, 0);

		ret = muggle_pointer_slot_insert(&ptr_slot, &arr[0], &idx);
		ASSERT_EQ(ret, 0);

		ret = muggle_pointer_slot_insert(&ptr_slot, &arr[1], &idx);
		ASSERT_EQ(ret, 0);

		ret = muggle_pointer_slot_insert(&ptr_slot, &arr[2], &idx);
		ASSERT_NE(ret, 0);

		data = muggle_pointer_slot_get(&ptr_slot, 0);
		ASSERT_NE(data, nullptr);

		data = muggle_pointer_slot_get(&ptr_slot, 1);
		ASSERT_NE(data, nullptr);
	}

	muggle_pointer_slot_destroy(&ptr_slot);
}

TEST(pointer_slot, idx_truncate)
{
	unsigned int capacity = 4;
	muggle_pointer_slot_t ptr_slot;
	int ret = muggle_pointer_slot_init(&ptr_slot, capacity);
	ASSERT_EQ(ret, 0);

	ptr_slot.alloc_index = ~(unsigned int)0;
	ptr_slot.free_index = ptr_slot.alloc_index;

	unsigned int idx, start_idx;
	int arr[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };

	ret = muggle_pointer_slot_insert(&ptr_slot, &arr[0], &idx);
	ASSERT_EQ(ret, 0);
	start_idx = idx;

	ret = muggle_pointer_slot_insert(&ptr_slot, &arr[1], &idx);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(IDX_IN_POW_OF_2_RING(start_idx + 1, capacity), IDX_IN_POW_OF_2_RING(idx, capacity));

	ret = muggle_pointer_slot_insert(&ptr_slot, &arr[2], &idx);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(IDX_IN_POW_OF_2_RING(start_idx + 2, capacity), IDX_IN_POW_OF_2_RING(idx, capacity));

	ret = muggle_pointer_slot_insert(&ptr_slot, &arr[3], &idx);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(IDX_IN_POW_OF_2_RING(start_idx + 3, capacity), IDX_IN_POW_OF_2_RING(idx, capacity));

	muggle_pointer_slot_destroy(&ptr_slot);
}

TEST(pointer_slot, iter)
{
	unsigned int capacity = 16;
	muggle_pointer_slot_t ptr_slot;
	int ret = muggle_pointer_slot_init(&ptr_slot, capacity);
	ASSERT_EQ(ret, 0);

	int arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
	unsigned int slot_idx_arr[sizeof(arr) / sizeof(arr[0])];
	for (int i = 0; i < 256; i++)
	{
		// insert
		for (int idx = 0; idx < (int)(sizeof(arr) / sizeof(arr[0])); idx++)
		{
			ret = muggle_pointer_slot_insert(&ptr_slot, &arr[idx], &slot_idx_arr[idx]);
			ASSERT_EQ(ret, 0);
		}

		// iter
		int j = 0;
		for (muggle_pointer_slot_item_t *it = muggle_pointer_slot_iter_begin(&ptr_slot); it != muggle_pointer_slot_iter_end(&ptr_slot); it = it->next)
		{
			int *data = (int*)muggle_pointer_slot_iter_data(&ptr_slot, it);
			ASSERT_EQ(data, &arr[j]);
			j++;
		}

		// rand sort and remove
		srand(time(nullptr));
		for (int idx = 0; idx < (int)(sizeof(arr) / sizeof(arr[0])); idx++)
		{
			int swap_idx = rand() % (int)(sizeof(arr) / sizeof(arr[0]));
			unsigned int tmp = slot_idx_arr[idx];
			slot_idx_arr[idx] = slot_idx_arr[swap_idx];
			slot_idx_arr[swap_idx] = tmp;
		}
		for (int idx = 0; idx < (int)(sizeof(arr) / sizeof(arr[0])); idx++)
		{
			ret = muggle_pointer_slot_remove(&ptr_slot, slot_idx_arr[idx]);
			ASSERT_EQ(ret, 0);
		}
	}

	muggle_pointer_slot_destroy(&ptr_slot);
}

TEST(pointer_slot, insert_remove_iter)
{
	muggle_pointer_slot_t ptr_slot;
	int ret = muggle_pointer_slot_init(&ptr_slot, 4);
	ASSERT_EQ(ret, 0);

	unsigned int idx;
	int arr[3] = { 1, 2, 3 };
	int *p_arr[3] = { nullptr, nullptr, nullptr };

	ret = muggle_pointer_slot_insert(&ptr_slot, &arr[0], &idx);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(idx, (unsigned int)0);

	ret = muggle_pointer_slot_insert(&ptr_slot, &arr[1], &idx);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(idx, (unsigned int)1);

	ret = muggle_pointer_slot_insert(&ptr_slot, &arr[2], &idx);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(idx, (unsigned int)2);

	ret = muggle_pointer_slot_remove(&ptr_slot, 1);
	ASSERT_EQ(ret, 0);

	// iter
	int cnt = 0;
	for (muggle_pointer_slot_item_t *it = muggle_pointer_slot_iter_begin(&ptr_slot); it != muggle_pointer_slot_iter_end(&ptr_slot); it = it->next)
	{
		int *data = (int*)muggle_pointer_slot_iter_data(&ptr_slot, it);
		p_arr[cnt++] = data;
	}
	ASSERT_EQ(cnt, 2);
	ASSERT_EQ(p_arr[0], &arr[0]);
	ASSERT_EQ(p_arr[1], &arr[2]);
}
