/******************************************************************************
 *  @file         pointer_slot.h
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2021-06-17
 *  @copyright    Copyright 2021 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec pointer slot
 *****************************************************************************/
 
#ifndef MUGGLE_C_POINTER_SLOT_H_
#define MUGGLE_C_POINTER_SLOT_H_

#include "muggle/c/base/macro.h"

EXTERN_C_BEGIN

/**
 * @brief pointer slot item
 */
typedef struct muggle_pointer_slot_item
{
	struct muggle_pointer_slot_item *prev;  //!< previous item
	struct muggle_pointer_slot_item *next;  //!< next item
	unsigned int slot_idx;                  //!< slot index
	unsigned int in_used;                   //!< is slot in used
	void *data;                             //!< data in this slot item
}muggle_pointer_slot_item_t;

/**
 * @brief pointer slot
 */
typedef struct muggle_pointer_slot
{
	muggle_pointer_slot_item_t *slots;      //!< slot item pointer
	muggle_pointer_slot_item_t **pp_slots;  //!< pointer to slot item address

	unsigned int capacity;     //!< capacity of slot
	unsigned int alloc_index;  //!< allocate cursor in slot
	unsigned int free_index;   //!< recycle cursor in slot

	muggle_pointer_slot_item_t head;  //!< slot head
	muggle_pointer_slot_item_t tail;  //!< slot tail
}muggle_pointer_slot_t;

/**
 * @brief init muggle_pointer_slot_t
 *
 * @param pointer_slot pointer to a muggle_pointer_slot_t
 * @param capacity expect slot capacity
 *
 * @return
 *     - return 0 on success
 *     - otherwise return error code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_pointer_slot_init(muggle_pointer_slot_t *pointer_slot, unsigned int capacity);

/**
 * @brief destroy muggle_pointer_slot_t
 *
 * @param pointer_slot  pointer to a muggle_pointer_slot_t
 */
MUGGLE_C_EXPORT
void muggle_pointer_slot_destroy(muggle_pointer_slot_t *pointer_slot);

/**
 * @brief insert data into muggle_pointer_slot_t
 *
 * @param pointer_slot pointer to a muggle_pointer_slot_t
 * @param data data need to insert into slot
 * @param idx return slot idx
 *
 * @return
 *     - return 0 on success
 *     - otherwise return error code in muggle/c/base/err.h
 */
MUGGLE_C_EXPORT
int muggle_pointer_slot_insert(muggle_pointer_slot_t *pointer_slot, void *data, unsigned int *slot_idx);

/**
 * @brief remove pointer data from muggle_pointer_slot_t
 *
 * @param pointer_slot pointer to a muggle_pointer_slot_t
 * @param idx slot index that pointer data need to be removed
 *
 * @return 0 - success, otherwise return error code
 */
MUGGLE_C_EXPORT
int muggle_pointer_slot_remove(muggle_pointer_slot_t *pointer_slot, unsigned int slot_idx);

/**
 * @brief get pointer data from muggle_pointer_slot_t
 *
 * @param pointer_slot pointer to a muggle_pointer_slot_t
 * @param idx slot index that pointer data need to be removed
 *
 * @return pointer data
 */
MUGGLE_C_EXPORT
void* muggle_pointer_slot_get(muggle_pointer_slot_t *pointer_slot, unsigned int idx);

/**
 * @brief get muggle_pointer_slot_t iterator begin
 *
 * @param pointer_slot pointer to a muggle_pointer_slot_t
 *
 * @return begin item in muggle_pointer_slot_t
 */
MUGGLE_C_EXPORT
muggle_pointer_slot_item_t* muggle_pointer_slot_iter_begin(muggle_pointer_slot_t *pointer_slot);

/**
 * @brief get muggle_pointer_slot_t iterator end
 *
 * @param pointer_slot pointer to a muggle_pointer_slot_t
 *
 * @return end item in muggle_pointer_slot_t
 */
MUGGLE_C_EXPORT
muggle_pointer_slot_item_t* muggle_pointer_slot_iter_end(muggle_pointer_slot_t *pointer_slot);

/**
 * @brief get muggle_pointer_slot_t iterator data
 *
 * @param pointer_slot pointer to a muggle_pointer_slot_t
 * @param iter         item in slot
 *
 * @return data in slot item
 */
MUGGLE_C_EXPORT
void* muggle_pointer_slot_iter_data(muggle_pointer_slot_t *pointer_slot, muggle_pointer_slot_item_t *iter);

EXTERN_C_END

#endif
