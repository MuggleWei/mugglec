#include "muggle/c/muggle_c.h"

muggle_thread_ret_t async_evloop_exit(void *args)
{
	muggle_event_loop_t *evloop = (muggle_event_loop_t *)args;

	muggle_msleep(100);
	LOG_INFO("async call evloop exit");
	muggle_evloop_exit(evloop);
	LOG_INFO("async call evloop exit completed");

	return 0;
}

void on_timer(muggle_event_loop_t *evloop)
{
	MUGGLE_UNUSED(evloop);
	LOG_INFO("sync call evloop exit");
	muggle_evloop_exit(evloop);
	LOG_INFO("sync call evloop exit completed");
}

int main(int argc, char *argv[])
{
	if (muggle_log_complicated_init(LOG_LEVEL_INFO, -1, NULL) != 0) {
		LOG_ERROR("failed initalize log");
		exit(EXIT_FAILURE);
	}

	if (argc != 2) {
		LOG_ERROR("Usage: %s <sync|async>", argv[0]);
		exit(EXIT_FAILURE);
	}

	int sync_exit = 1;
	if (strcmp(argv[1], "sync") == 0) {
		sync_exit = 1;
	} else if (strcmp(argv[1], "async") == 0) {
		sync_exit = 0;
	} else {
		LOG_ERROR("invalid input arguments");
		LOG_ERROR("Usage: %s <sync|async>", argv[0]);
		exit(EXIT_FAILURE);
	}

	if (muggle_socket_lib_init() != 0) {
		LOG_ERROR("failed initalize socket library");
		exit(EXIT_FAILURE);
	}

	// init event loop
	muggle_event_loop_init_args_t ev_init_args;
	memset(&ev_init_args, 0, sizeof(ev_init_args));
	ev_init_args.evloop_type = MUGGLE_EVLOOP_TYPE_NULL;
	ev_init_args.hints_max_fd = 32;
	ev_init_args.use_mem_pool = 0;

	muggle_event_loop_t *evloop = muggle_evloop_new(&ev_init_args);
	if (evloop == NULL) {
		LOG_ERROR("failed new event loop");
		exit(EXIT_FAILURE);
	}
	LOG_INFO("success new event loop");

	// init socket event loop handle
	muggle_socket_evloop_handle_t handle;
	muggle_socket_evloop_handle_init(&handle);
	muggle_socket_evloop_handle_set_timer_interval(&handle, 0);
	if (sync_exit) {
		muggle_socket_evloop_handle_set_cb_timer(&handle, on_timer);
	}
	muggle_socket_evloop_handle_attach(&handle, evloop);
	LOG_INFO("socket handle attached event loop");

	// run async notify evloop exit
	if (!sync_exit) {
		muggle_thread_t th;
		muggle_thread_create(&th, async_evloop_exit, evloop);
	}

	// run event loop
	LOG_INFO("run event loop");
	muggle_evloop_run(evloop);

	// destroy socket event loop handle
	muggle_socket_evloop_handle_destroy(&handle);

	// delete event loop
	muggle_evloop_delete(evloop);

	LOG_INFO("evloop already destroy");

	muggle_msleep(1000);
	LOG_INFO("bye");

	return 0;
}
