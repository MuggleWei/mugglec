/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#ifndef MUGGLE_C_POINTER_SLOT_H_
#define MUGGLE_C_POINTER_SLOT_H_

#include "muggle/c/base/macro.h"

EXTERN_C_BEGIN

typedef struct muggle_pointer_slot_item
{
	struct muggle_pointer_slot_item *prev;
	struct muggle_pointer_slot_item *next;
	unsigned int slot_idx;
	unsigned int in_used;
	void *data;
}muggle_pointer_slot_item_t;

typedef struct muggle_pointer_slot
{
	muggle_pointer_slot_item_t *slots;
	muggle_pointer_slot_item_t **pp_slots;

	unsigned int capacity;
	unsigned int alloc_index;
	unsigned int free_index;

	muggle_pointer_slot_item_t head;
	muggle_pointer_slot_item_t tail;
}muggle_pointer_slot_t;

// init muggle_pointer_slot_t
// @param pointer_slot pointer to a muggle_pointer_slot_t
// @param capacity expect slot capacity
// @return 0 - success, otherwise return error code
MUGGLE_CC_EXPORT
int muggle_pointer_slot_init(muggle_pointer_slot_t *pointer_slot, unsigned int capacity);

// destroy muggle_pointer_slot_t
// @param pointer_slot pointer to a muggle_pointer_slot_t
MUGGLE_CC_EXPORT
void muggle_pointer_slot_destroy(muggle_pointer_slot_t *pointer_slot);

// insert data into muggle_pointer_slot_t
// @param pointer_slot pointer to a muggle_pointer_slot_t
// @param data data need to insert into slot
// @param idx return slot idx
// @return 0 - success, otherwise return error code
MUGGLE_CC_EXPORT
int muggle_pointer_slot_insert(muggle_pointer_slot_t *pointer_slot, void *data, unsigned int *slot_idx);

// remove pointer data from muggle_pointer_slot_t
// @param pointer_slot pointer to a muggle_pointer_slot_t
// @param idx slot index that pointer data need to be removed
// @return 0 - success, otherwise return error code
MUGGLE_CC_EXPORT
int muggle_pointer_slot_remove(muggle_pointer_slot_t *pointer_slot, unsigned int slot_idx);

// get pointer data from muggle_pointer_slot_t
// @param pointer_slot pointer to a muggle_pointer_slot_t
// @param idx slot index that pointer data need to be removed
// @return pointer data
MUGGLE_CC_EXPORT
void* muggle_pointer_slot_get(muggle_pointer_slot_t *pointer_slot, unsigned int idx);

// get muggle_pointer_slot_t iterator begin and end
MUGGLE_CC_EXPORT
muggle_pointer_slot_item_t* muggle_pointer_slot_iter_begin(muggle_pointer_slot_t *pointer_slot);

MUGGLE_CC_EXPORT
muggle_pointer_slot_item_t* muggle_pointer_slot_iter_end(muggle_pointer_slot_t *pointer_slot);

// get muggle_pointer_slot_t iterator data
MUGGLE_CC_EXPORT
void* muggle_pointer_slot_iter_data(muggle_pointer_slot_t *pointer_slot, muggle_pointer_slot_item_t *iter);

EXTERN_C_END

#endif
