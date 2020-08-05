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
	ASSERT_EQ(idx, 0);

	ret = muggle_pointer_slot_insert(&ptr_slot, &arr[1], &idx);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(idx, 1);

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
	ASSERT_EQ(idx, 1);

	ret = muggle_pointer_slot_insert(&ptr_slot, &arr[1], &idx);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(idx, 0);

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
