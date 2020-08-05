#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

TEST(pointer_slot, insert_remove)
{
	muggle_pointer_slot_t ptr_slot;
	int ret = muggle_pointer_slot_init(&ptr_slot, 2);
	ASSERT_EQ(ret, 0);

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

	ret = muggle_pointer_slot_remove(&ptr_slot, 1);
	ASSERT_EQ(ret, 0);
	ret = muggle_pointer_slot_remove(&ptr_slot, 0);
	ASSERT_EQ(ret, 0);

	ret = muggle_pointer_slot_insert(&ptr_slot, &arr[0], &idx);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(idx, 1);

	ret = muggle_pointer_slot_insert(&ptr_slot, &arr[1], &idx);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(idx, 0);

	ret = muggle_pointer_slot_insert(&ptr_slot, &arr[2], &idx);
	ASSERT_NE(ret, 0);

	muggle_pointer_slot_destroy(&ptr_slot);
}