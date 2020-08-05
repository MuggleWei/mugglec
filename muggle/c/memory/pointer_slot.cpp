/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "pointer_slot.h"
#include <stdlib.h>
#include <string.h>
#include "muggle/c/base/err.h"
#include "muggle/c/base/utils.h"

int muggle_pointer_slot_init(muggle_pointer_slot_t *pointer_slot, unsigned int capacity)
{
	memset(pointer_slot, 0, sizeof(muggle_pointer_slot_t));
	capacity = capacity > 0 ? capacity : 1;
	pointer_slot->capacity = (unsigned int)next_pow_of_2((uint64_t)capacity);

	pointer_slot->slots = (muggle_pointer_slot_item_t*)malloc(sizeof(muggle_pointer_slot_item_t) * capacity);
	pointer_slot->pp_slots = (muggle_pointer_slot_item_t**)malloc(sizeof(muggle_pointer_slot_item_t*) * capacity);
	if (pointer_slot->slots == NULL || pointer_slot->pp_slots == NULL)
	{
		if (pointer_slot->slots)
		{
			free(pointer_slot->slots);
			pointer_slot->slots = NULL;
		}
		if (pointer_slot->pp_slots)
		{
			free(pointer_slot->pp_slots);
			pointer_slot->pp_slots = NULL;
		}
		return MUGGLE_ERR_MEM_ALLOC;
	}

	for (unsigned int i = 0; i < capacity; i++)
	{
		pointer_slot->slots[i].slot_idx = i;
		pointer_slot->slots[i].in_used = 0;
		pointer_slot->slots[i].data = NULL;

		pointer_slot->pp_slots[i] = &pointer_slot->slots[i];
	}

	pointer_slot->alloc_index = 0;
	pointer_slot->free_index = 0;

	return 0;
}

void muggle_pointer_slot_destroy(muggle_pointer_slot_t *pointer_slot)
{
	if (pointer_slot->slots)
	{
		free(pointer_slot->slots);
		pointer_slot->slots = NULL;
	}

	if (pointer_slot->pp_slots)
	{
		free(pointer_slot->pp_slots);
		pointer_slot->pp_slots = NULL;
	}
}

int muggle_pointer_slot_insert(muggle_pointer_slot_t *pointer_slot, void *data, unsigned int *slot_idx)
{
	unsigned int alloc_idx = IDX_IN_POW_OF_2_RING(pointer_slot->alloc_index, pointer_slot->capacity);
	if (pointer_slot->pp_slots[alloc_idx]->in_used == 1)
	{
		MUGGLE_ASSERT(alloc_idx == IDX_IN_POW_OF_2_RING(pointer_slot->free_index, pointer_slot->capacity));
		return MUGGLE_ERR_MEM_ALLOC;
	}

	pointer_slot->pp_slots[alloc_idx]->data = data;
	pointer_slot->pp_slots[alloc_idx]->in_used = 1;
	*slot_idx = pointer_slot->pp_slots[alloc_idx]->slot_idx;

	pointer_slot->alloc_index++;

	return 0;
}

int muggle_pointer_slot_remove(muggle_pointer_slot_t *pointer_slot, unsigned int slot_idx)
{
	if (slot_idx >= pointer_slot->capacity)
	{
		MUGGLE_ASSERT(slot_idx < pointer_slot->capacity);
		return MUGGLE_ERR_BEYOND_RANGE;
	}

	if (pointer_slot->slots[slot_idx].in_used == 0)
	{
		MUGGLE_ASSERT(pointer_slot->slots[slot_idx].in_used == 1);
		return MUGGLE_ERR_MEM_DUPLICATE_FREE;
	}

	unsigned int free_idx = IDX_IN_POW_OF_2_RING(pointer_slot->free_index, pointer_slot->capacity);
	pointer_slot->pp_slots[free_idx] = &pointer_slot->slots[slot_idx];

	pointer_slot->pp_slots[free_idx]->in_used = 0;
	pointer_slot->pp_slots[free_idx]->data = NULL;

	pointer_slot->free_index++;

	return 0;
}

void* muggle_pointer_slot_get(muggle_pointer_slot_t *pointer_slot, unsigned int idx)
{
	if (idx >= pointer_slot->capacity)
	{
		MUGGLE_ASSERT(idx < pointer_slot->capacity);
		return NULL;
	}

	if (pointer_slot->slots[idx].in_used == 0)
	{
		return NULL;
	}

	return pointer_slot->slots[idx].data;
}

unsigned int muggle_pointer_slot_iter_begin(muggle_pointer_slot_t *pointer_slot)
{
	return pointer_slot->free_index;
}

unsigned int muggle_pointer_slot_iter_end(muggle_pointer_slot_t *pointer_slot)
{
	return pointer_slot->alloc_index;
}

void* muggle_pointer_slot_iter_data(muggle_pointer_slot_t *pointer_slot, unsigned int iter)
{
	unsigned int idx = IDX_IN_POW_OF_2_RING(iter, pointer_slot->capacity);
	unsigned int slot_idx = pointer_slot->pp_slots[idx]->slot_idx;
	return muggle_pointer_slot_get(pointer_slot, slot_idx);
}
