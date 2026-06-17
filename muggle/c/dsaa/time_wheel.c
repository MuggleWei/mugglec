#include "time_wheel.h"
#include <stdlib.h>
#include <string.h>

bool muggle_time_wheel_init(muggle_time_wheel_t *wheel, uint32_t n,
							uint32_t unit_ms, uint32_t insert_mode)
{
	bool ret = true;

	memset(wheel, 0, sizeof(muggle_time_wheel_t));

	if (!muggle_time_counter_init(&wheel->tc)) {
		return false;
	}
	muggle_time_counter_start(&wheel->tc);

	wheel->slots = (muggle_time_wheel_slot_t *)malloc(
		sizeof(muggle_time_wheel_slot_t) * n);
	if (wheel->slots == NULL) {
		return false;
	}
	for (uint32_t i = 0; i < n; ++i) {
		muggle_time_wheel_node_t *head = &wheel->slots[i].head;
		muggle_time_wheel_node_t *tail = &wheel->slots[i].tail;
		head->prev = NULL;
		head->next = tail;
		tail->prev = head;
		tail->next = NULL;
	}
	wheel->n = n;
	wheel->unit_ms = unit_ms;
	wheel->insert_mode = insert_mode;
	wheel->cur = 0;
	wheel->insert_cur = 0;

	return ret;
}

void muggle_time_wheel_destroy(muggle_time_wheel_t *wheel)
{
	if (wheel->slots) {
		free(wheel->slots);
		wheel->slots = 0;
	}
}

void muggle_time_wheel_update(muggle_time_wheel_t *wheel,
							  fn_muggle_time_wheel_cb cb, void *user_data)
{
	muggle_time_counter_end(&wheel->tc);
	int64_t elapsed_ms = muggle_time_counter_interval_ms(&wheel->tc);
	uint32_t n_slot = elapsed_ms / wheel->unit_ms;
	if (n_slot > wheel->n) {
		n_slot = wheel->n;
	}
	muggle_time_wheel_update_n(wheel, n_slot, cb, user_data);
	muggle_time_counter_move_end_to_start(&wheel->tc);
}

void muggle_time_wheel_update_n(muggle_time_wheel_t *wheel, uint32_t n_slot,
								fn_muggle_time_wheel_cb cb, void *user_data)
{
	for (uint32_t i = 0; i < n_slot; ++i) {
		uint32_t idx = wheel->cur++;
		wheel->cur %= wheel->n;
		muggle_time_wheel_slot_t *slot = &wheel->slots[idx];
		muggle_time_wheel_node_t *node = slot->head.next;
		while (node != &slot->tail) {
			cb(node, user_data);
			node = node->next;
		}
	}
}

void muggle_time_wheel_insert(muggle_time_wheel_t *wheel,
							  muggle_time_wheel_node_t *node)
{
	uint32_t idx = 0;
	switch (wheel->insert_mode) {
	case MUGGLE_TIME_WHEEL_INSERT_MODE_CURRENT: {
		idx = wheel->cur;
	} break;
	case MUGGLE_TIME_WHEEL_INSERT_MODE_LOOP: {
		++wheel->insert_cur;
		wheel->insert_cur %= wheel->n;
		idx = wheel->insert_cur;
	} break;
	}

	muggle_time_wheel_slot_t *slot = &wheel->slots[idx];
	muggle_time_wheel_node_t *head = &slot->head;

	head->next->prev = node;
	node->next = head->next;
	node->prev = head;
	head->next = node;
}

void muggle_time_wheel_remove(muggle_time_wheel_t *wheel,
							  muggle_time_wheel_node_t *node)
{
	MUGGLE_UNUSED(wheel);
	node->next->prev = node->prev;
	node->prev->next = node->next;
}
