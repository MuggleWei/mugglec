#include "fast_flow_controller.h"
#include <stdlib.h>
#include <string.h>

#if MUGGLE_SUPPORT_FAST_FLOW_CONTROLLER

bool muggle_fast_flow_ctl_init(muggle_fast_flow_controller_t *flow_ctl,
							   int64_t time_range_sec, uint32_t n,
							   int64_t init_forward_sec, double tick_freq)
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
	flow_ctl->t_ticks = time_range_sec * (int64_t)tick_freq;
	flow_ctl->n = n;
	flow_ctl->cursor = 0;

	flow_ctl->start_ticks = muggle_rdtscp();

	int64_t elapsed_ticks = -init_forward_sec * (int64_t)tick_freq;
	for (uint32_t i = 0; i < n; ++i) {
		flow_ctl->arr[i] = elapsed_ticks;
	}

	return true;
}

void muggle_fast_flow_ctl_destroy(muggle_fast_flow_controller_t *flow_ctl)
{
	if (flow_ctl->arr) {
		free(flow_ctl->arr);
		flow_ctl->arr = NULL;
	}
}

bool muggle_fast_flow_ctl_check_and_update(
	muggle_fast_flow_controller_t *flow_ctl)
{
	int64_t elapsed_ticks = muggle_fast_flow_ctl_get_curr_elapsed(flow_ctl);
	if (!muggle_fast_flow_ctl_check(flow_ctl, elapsed_ticks)) {
		return false;
	}

	muggle_fast_flow_ctl_update(flow_ctl, elapsed_ticks);

	return true;
}

bool muggle_fast_flow_ctl_check_and_force_update(
	muggle_fast_flow_controller_t *flow_ctl)
{
	int64_t elapsed_ticks = muggle_fast_flow_ctl_get_curr_elapsed(flow_ctl);
	bool ret = muggle_fast_flow_ctl_check(flow_ctl, elapsed_ticks);
	muggle_fast_flow_ctl_update(flow_ctl, elapsed_ticks);
	return ret;
}

int64_t
muggle_fast_flow_ctl_get_curr_elapsed(muggle_fast_flow_controller_t *flow_ctl)
{
	uint64_t curr_ticks = muggle_rdtscp();
	return (int64_t)(curr_ticks - flow_ctl->start_ticks);
}

bool muggle_fast_flow_ctl_check(muggle_fast_flow_controller_t *flow_ctl,
								int64_t elapsed_ticks)
{
	int64_t diff = elapsed_ticks - flow_ctl->arr[flow_ctl->cursor];
	return diff >= flow_ctl->t_ticks;
}

void muggle_fast_flow_ctl_update(muggle_fast_flow_controller_t *flow_ctl,
								 int64_t elapsed_ticks)
{
	flow_ctl->arr[flow_ctl->cursor] = elapsed_ticks;
	flow_ctl->cursor = (flow_ctl->cursor + 1) % flow_ctl->n;
}

#else

bool muggle_fast_flow_ctl_init(muggle_fast_flow_controller_t *flow_ctl,
							   int64_t time_range_sec, uint32_t n,
							   int64_t init_forward_sec, double tick_freq)
{
	memset(flow_ctl, 0, sizeof(*flow_ctl));
	MUGGLE_UNUSED(time_range_sec);
	MUGGLE_UNUSED(n);
	MUGGLE_UNUSED(init_forward_sec);
	MUGGLE_UNUSED(tick_freq);
	return false;
}

void muggle_fast_flow_ctl_destroy(muggle_fast_flow_controller_t *flow_ctl)
{
	MUGGLE_UNUSED(flow_ctl);
}

bool muggle_fast_flow_ctl_check_and_update(
	muggle_fast_flow_controller_t *flow_ctl)
{
	MUGGLE_UNUSED(flow_ctl);
	return false;
}

bool muggle_fast_flow_ctl_check_and_force_update(
	muggle_fast_flow_controller_t *flow_ctl)
{
	MUGGLE_UNUSED(flow_ctl);
	return false;
}

int64_t
muggle_fast_flow_ctl_get_curr_elapsed(muggle_fast_flow_controller_t *flow_ctl)
{
	MUGGLE_UNUSED(flow_ctl);
	return 0;
}

bool muggle_fast_flow_ctl_check(muggle_fast_flow_controller_t *flow_ctl,
								int64_t elapsed_ticks)
{
	MUGGLE_UNUSED(flow_ctl);
	MUGGLE_UNUSED(elapsed_ticks);
	return false;
}

void muggle_fast_flow_ctl_update(muggle_fast_flow_controller_t *flow_ctl,
								 int64_t elapsed_ticks)
{
	MUGGLE_UNUSED(flow_ctl);
	MUGGLE_UNUSED(elapsed_ticks);
}

#endif
