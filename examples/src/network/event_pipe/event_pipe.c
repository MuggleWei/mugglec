#include "muggle/c/muggle_c.h"

void on_message(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	MUGGLE_UNUSED(evloop);

	MUGGLE_ASSERT(ctx->sock_type == MUGGLE_SOCKET_CTX_TYPE_PIPE);

	muggle_socket_evloop_pipe_t *ev_pipe =
		(muggle_socket_evloop_pipe_t *)muggle_ev_ctx_data(
			(muggle_event_context_t *)ctx);

	void *data = NULL;
	while ((data = muggle_socket_evloop_pipe_read(ev_pipe)) != NULL) {
		char *s = (char *)data;
		LOG_INFO("on evloop pipe message: %s", s);
		free(s);

		// simulate blocking
		// muggle_msleep(500);
	}
}

muggle_thread_ret_t run_pipe_write(void *args)
{
	muggle_socket_evloop_pipe_t *ev_pipe = (muggle_socket_evloop_pipe_t *)args;

	int idx = 0;
	while (true) {
		muggle_nsleep(100);

		char *s = malloc(16);
		snprintf(s, 16, "%08d", ++idx);

		do {
			if (!muggle_socket_evloop_pipe_write(ev_pipe, s)) {
				LOG_ERROR("failed pipe write: %s", s);

				muggle_msleep(1000);
				LOG_INFO("retry pipe write: %s", s);
			} else {
				break;
			}
		} while (true);
	}
}

void run_evloop(muggle_socket_evloop_pipe_t *ev_pipe)
{
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
	muggle_socket_evloop_handle_set_cb_msg(&handle, on_message);
	muggle_socket_evloop_handle_attach(&handle, evloop);
	LOG_INFO("socket handle attached event loop");

	// add reader into evloop
	muggle_socket_context_t *pipe_reader =
		muggle_socket_evloop_pipe_get_reader(ev_pipe);
	muggle_socket_evloop_add_ctx(evloop, pipe_reader);

	// run event loop
	LOG_INFO("run event loop");
	muggle_evloop_run(evloop);

	// destroy socket event loop handle
	muggle_socket_evloop_handle_destroy(&handle);

	// delete event loop
	muggle_evloop_delete(evloop);
}

int main()
{
	if (muggle_log_complicated_init(LOG_LEVEL_INFO, -1, NULL) != 0) {
		LOG_ERROR("failed initalize log");
		exit(EXIT_FAILURE);
	}

	if (muggle_socket_lib_init() != 0) {
		LOG_ERROR("failed initalize socket library");
		exit(EXIT_FAILURE);
	}

	muggle_socket_evloop_pipe_t ev_pipe;
	if (muggle_socket_evloop_pipe_init(&ev_pipe) != 0) {
		LOG_ERROR("failed init event pipe");
		exit(EXIT_FAILURE);
	}

	muggle_thread_t th;
	muggle_thread_create(&th, run_pipe_write, &ev_pipe);

	run_evloop(&ev_pipe);

	muggle_thread_join(&th);

	muggle_socket_evloop_pipe_destroy(&ev_pipe);

	return 0;
}
