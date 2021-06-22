/******************************************************************************
 *  @file         sort.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-17
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec sort
 *****************************************************************************/
 
#include "sort.h"
#include <string.h>
#include <stdlib.h>
#include "heap.h"

bool muggle_insertion_sort(void **ptr, size_t count, muggle_dsaa_data_cmp cmp)
{
	size_t j = 0;
	for (size_t i = 1; i < count; i++)
	{
		void *tmp = ptr[i];

		for (j = i; j > 0; j--)
		{
			if (cmp(ptr[j - 1], tmp) > 0)
			{
				ptr[j] = ptr[j - 1];
			}
			else
			{
				break;
			}
		}
		ptr[j] = tmp;
	}

	return true;
}

bool muggle_shell_sort(void **ptr, size_t count, muggle_dsaa_data_cmp cmp)
{
	size_t j = 0;
	for (size_t increment = count / 2; increment > 0; increment /= 2)
	{
		for (size_t i = increment; i < count; i++)
		{
			void *tmp = ptr[i];
			for (j = i; j >= increment; j -= increment)
			{
				if (cmp(tmp, ptr[j - increment]) < 0)
				{
					ptr[j] = ptr[j - increment];
				}
				else
				{
					break;
				}
			}
			ptr[j] = tmp;
		}
	}

	return true;
}

bool muggle_heap_sort(void **ptr, size_t count, muggle_dsaa_data_cmp cmp)
{
	muggle_heap_t heap;
	if (!muggle_heap_init(&heap, cmp, (uint32_t)count + 1))
	{
		return false;
	}

	for (size_t i = 0; i < count; i++)
	{
		muggle_heap_insert(&heap, ptr[i], NULL);
	}

	muggle_heap_node_t node;
	for (size_t i = 0; i < count; i++)
	{
		muggle_heap_extract(&heap, &node);

		// NOTE: need reverse insert, cause heap is max-heap
		ptr[i] = node.key;
	}

	muggle_heap_destroy(&heap, NULL, NULL, NULL, NULL);

	return true;
}

static void muggle_merge_sort_recursive(void **ptr, void **arr, size_t left, size_t right, muggle_dsaa_data_cmp cmp)
{
	if (left < right)
	{
		size_t center = (left + right) / 2;
		muggle_merge_sort_recursive(ptr, arr, left, center, cmp);
		muggle_merge_sort_recursive(ptr, arr, center + 1, right, cmp);

		// merge left and right
		size_t l = left;
		size_t r = center + 1;
		size_t idx = l;
		while (l <= center && r <= right)
		{
			if (cmp(ptr[l], ptr[r]) <= 0)
			{
				arr[idx++] = ptr[l++];
			}
			else
			{
				arr[idx++] = ptr[r++];
			}
		}

		while (l <= center)
		{
			arr[idx++] = ptr[l++];
		}
		while (r <= right)
		{
			arr[idx++] = ptr[r++];
		}

		for (idx = left; idx <= right; idx++)
		{
			ptr[idx] = arr[idx];
		}
	}
}

bool muggle_merge_sort(void **ptr, size_t count, muggle_dsaa_data_cmp cmp)
{
	void **arr = (void**)malloc(sizeof(void*) * count);
	if (arr == NULL)
	{
		return false;
	}

	muggle_merge_sort_recursive(ptr, arr, 0, count - 1, cmp);

	free(arr);

	return true;
}

static void* muggle_quick_sort_median3(void **ptr, size_t left, size_t right, muggle_dsaa_data_cmp cmp)
{
	void *tmp = NULL;
	size_t center = (left + right) / 2;
	if (cmp(ptr[left], ptr[center]) > 0)
	{
		tmp = ptr[left];
		ptr[left] = ptr[center];
		ptr[center] = tmp;
	}
	if (cmp(ptr[left], ptr[right]) > 0)
	{
		tmp = ptr[left];
		ptr[left] = ptr[right];
		ptr[right] = tmp;
	}
	if (cmp(ptr[center], ptr[right]) > 0)
	{
		tmp = ptr[center];
		ptr[center] = ptr[right];
		ptr[right] = tmp;
	}

	tmp = ptr[center];
	ptr[center] = ptr[right - 1];
	ptr[right - 1] = tmp;

	return ptr[right - 1];
}

#define QUICK_SORT_CUTOFF 10
static void muggle_quick_sort_recursive(void **ptr, size_t left, size_t right, muggle_dsaa_data_cmp cmp)
{
	if (left + QUICK_SORT_CUTOFF <= right)
	{
		void *pivot = muggle_quick_sort_median3(ptr, left, right, cmp);
		size_t i = left;
		size_t j = right - 1;
		void *tmp;
		while (true)
		{
			while (cmp(ptr[++i], pivot) < 0) {}
			while (cmp(ptr[--j], pivot) > 0) {}
			if (i < j)
			{
				tmp = ptr[i];
				ptr[i] = ptr[j];
				ptr[j] = tmp;
			}
			else
			{
				break;
			}
		}

		// restore pivot
		tmp = ptr[i];
		ptr[i] = ptr[right - 1];
		ptr[right - 1] = tmp;

		muggle_quick_sort_recursive(ptr, left, i - 1, cmp);
		muggle_quick_sort_recursive(ptr, i + 1, right, cmp);
	}
	else
	{
		// do an insertion sort on the subarray
		muggle_insertion_sort(ptr + left, right + 1 - left, cmp);
	}
}

bool muggle_quick_sort(void **ptr, size_t count, muggle_dsaa_data_cmp cmp)
{
	muggle_quick_sort_recursive(ptr, 0, count - 1, cmp);

	return true;
}
