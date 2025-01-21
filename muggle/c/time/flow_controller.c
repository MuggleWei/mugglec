#include "flow_controller.h"
#include <string.h>
#include <stdlib.h>

bool muggle_flow_ctl_init(muggle_flow_controller_t *flow_ctl,
						  int64_t time_range_sec, uint32_t n,
						  int64_t init_forward_sec)
{
	memset(flow_ctl, 0, sizeof(*flow_ctl));
	if (n == 0) {
		return false;
	}
	if (time_range_sec <= 0) {
		return false;
	}

	flow_ctl->arr = (int64_t *)malloc(sizeof(int64_t) * n);
	if (flow_ctl->arr == NULL) {
		return false;
	}

	flow_ctl->t = time_range_sec * 1000000000LL;
	flow_ctl->n = n;
	flow_ctl->cursor = 0;

	muggle_time_counter_init(&flow_ctl->tc);
	muggle_time_counter_start(&flow_ctl->tc);

	int64_t elapsed_ns = -init_forward_sec * 1000000000LL;
	for (uint32_t i = 0; i < n; ++i) {
		flow_ctl->arr[i] = elapsed_ns;
	}

	return true;
}

void muggle_flow_ctl_destroy(muggle_flow_controller_t *flow_ctl)
{
	if (flow_ctl->arr) {
		free(flow_ctl->arr);
		flow_ctl->arr = NULL;
	}
}

bool muggle_flow_ctl_check_and_update(muggle_flow_controller_t *flow_ctl)
{
	int64_t elapsed_ns = muggle_flow_ctl_get_curr_elapsed(flow_ctl);
	if (!muggle_flow_ctl_check(flow_ctl, elapsed_ns)) {
		return false;
	}

	muggle_flow_ctl_update(flow_ctl, elapsed_ns);

	return true;
}

bool muggle_flow_ctl_check_and_force_update(muggle_flow_controller_t *flow_ctl)
{
	int64_t elapsed_ns = muggle_flow_ctl_get_curr_elapsed(flow_ctl);
	bool ret = muggle_flow_ctl_check(flow_ctl, elapsed_ns);
	muggle_flow_ctl_update(flow_ctl, elapsed_ns);

	return ret;
}

int64_t muggle_flow_ctl_get_curr_elapsed(muggle_flow_controller_t *flow_ctl)
{
	muggle_time_counter_end(&flow_ctl->tc);
	return muggle_time_counter_interval_ns(&flow_ctl->tc);
}

bool muggle_flow_ctl_check(muggle_flow_controller_t *flow_ctl,
						   int64_t elapsed_ns)
{
	int64_t diff = elapsed_ns - flow_ctl->arr[flow_ctl->cursor];
	return diff >= flow_ctl->t;
}

void muggle_flow_ctl_update(muggle_flow_controller_t *flow_ctl,
							int64_t elapsed_ns)
{
	flow_ctl->arr[flow_ctl->cursor] = elapsed_ns;
	flow_ctl->cursor = (flow_ctl->cursor + 1) % flow_ctl->n;
}
