/******************************************************************************
 *  @file         ref_cnt.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-08-24
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec reference count
 *****************************************************************************/

#include "ref_cnt.h"

int muggle_ref_cnt_init(muggle_ref_cnt_t *ref, int init_val)
{
	if (init_val <= 0)
	{
		return -1;
	}

	*ref = init_val;
	return 0;
}

int muggle_ref_cnt_retain(muggle_ref_cnt_t *ref)
{
	muggle_atomic_int v = 0, desired = 0;
	do {
		v = *ref;
		if (v == 0)
		{
			// try to retain released ctx
			return -1;
		}
		desired = v + 1;
	} while (!muggle_atomic_cmp_exch_strong(ref, &v, desired, muggle_memory_order_relaxed));

	return desired;
}

int muggle_ref_cnt_release(muggle_ref_cnt_t *ref)
{
	muggle_atomic_int v = 0, desired = 0;
	do {
		v = *ref;
		if (v == 0)
		{
			// repeated release error
			return -1;
		}
		desired = v - 1;
	} while (!muggle_atomic_cmp_exch_strong(ref, &v, desired, muggle_memory_order_relaxed));

	return desired;
}

int muggle_ref_cnt_val(muggle_ref_cnt_t *ref)
{
	return *ref;
}

int muggle_ref_cnt_load(muggle_ref_cnt_t *ref, int memorder)
{
	return muggle_atomic_load(ref, memorder);
}
