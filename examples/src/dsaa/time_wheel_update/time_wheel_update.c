#include "muggle/c/muggle_c.h"

#define UNIT_TIME_MS 500
#define N_SLOT 10

typedef struct {
	uint32_t session_id;
	muggle_time_wheel_node_t tw_node;
} session_t;

void wheel_update_cb(muggle_time_wheel_node_t *node, void *user_data)
{
	char *mode = user_data;
	session_t *session = node->data;
	LOG_INFO("[%s] %u", mode, session->session_id);
}

void run(muggle_time_wheel_t *wheel, const char *mode)
{
	session_t sessions[60];
	for (uint32_t i = 0; i < sizeof(sessions) / sizeof(sessions[0]); ++i) {
		memset(&sessions[i], 0, sizeof(session_t));
		sessions[i].session_id = i;
		sessions[i].tw_node.data = &sessions[i];
		muggle_time_wheel_insert(wheel, &sessions[i].tw_node);
	}

	for (int i = 0; i < 20; ++i) {
		LOG_INFO("----------------");
		muggle_time_wheel_update(wheel, wheel_update_cb, (void *)mode);
		muggle_msleep(UNIT_TIME_MS);
	}
}

int main()
{
	muggle_log_complicated_init(LOG_LEVEL_TRACE, -1, NULL);

	// insert mode: loop
	{
		LOG_INFO("--------------------------------");
		LOG_INFO("run with insert mode: current");
		muggle_time_wheel_t wheel;
		muggle_time_wheel_init(&wheel, N_SLOT, UNIT_TIME_MS,
							   MUGGLE_TIME_WHEEL_INSERT_MODE_CURRENT);
		run(&wheel, "current mode");
		muggle_time_wheel_destroy(&wheel);
	}

	// insert mode: loop
	{
		LOG_INFO("--------------------------------");
		LOG_INFO("run with insert mode: loop");
		muggle_time_wheel_t wheel;
		muggle_time_wheel_init(&wheel, N_SLOT, UNIT_TIME_MS,
							   MUGGLE_TIME_WHEEL_INSERT_MODE_LOOP);
		run(&wheel, "loop mode");
		muggle_time_wheel_destroy(&wheel);
	}

	return 0;
}
