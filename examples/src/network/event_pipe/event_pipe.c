#include "muggle/c/muggle_c.h"

static int s_batch_read = 0;
static int s_write_n = 1;

int evpipe_batch_read(muggle_socket_evloop_pipe_t *ev_pipe)
{
	const int cap = 1024;
	int total_read = 0;
	void *arr[cap];
	int offset = 0;
	int remain = cap * sizeof(void *);
	int n = 0;

	while (true) {
		char *p = (char *)arr + offset;
		n = muggle_socket_evloop_pipe_read_n(ev_pipe, p, remain);
		if (n <= 0) {
			if (offset % sizeof(void *) != 0) {
				remain = sizeof(void *) - (offset % sizeof(void *));
				n = muggle_socket_evloop_pipe_block_read_n(ev_pipe, p, remain);
				if (n != remain) {
					LOG_FATAL("something wrong!!!");
				}
				offset += n;
			}

			break;
		}

		offset += n;
		remain -= n;
		if (remain == 0) {
			for (int i = 0; i < cap; ++i) {
				char *s = (char *)arr[i];
				LOG_TRACE("on evloop pipe message: %s", s);
				++total_read;
			}

			offset = 0;
			remain = cap * sizeof(void *);
		}
	}

	for (size_t i = 0; i < offset / sizeof(void *); ++i) {
		char *s = (char *)arr[i];
		LOG_TRACE("on evloop pipe message: %s", s);
		++total_read;
	}

	return total_read;
}

void on_message(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	MUGGLE_UNUSED(evloop);

	MUGGLE_ASSERT(ctx->sock_type == MUGGLE_SOCKET_CTX_TYPE_PIPE);

	muggle_socket_evloop_pipe_t *ev_pipe =
		(muggle_socket_evloop_pipe_t *)muggle_ev_ctx_data(
			(muggle_event_context_t *)ctx);

	int total_read = 0;
	muggle_time_counter_t tc;
	muggle_time_counter_init(&tc);
	muggle_time_counter_start(&tc);
	if (s_batch_read) {
		total_read = evpipe_batch_read(ev_pipe);
	} else {
		void *data = NULL;
		while ((data = muggle_socket_evloop_pipe_read(ev_pipe)) != NULL) {
			char *s = (char *)data;
			LOG_TRACE("on evloop pipe message: %s", s);
			++total_read;

			// simulate blocking
			// muggle_msleep(500);
		}
	}
	muggle_time_counter_end(&tc);

	long long elpased = (long long)muggle_time_counter_interval_ns(&tc);
	long long mean = elpased / total_read;
	LOG_INFO("read: %2d, elapsed: %6lld ns, mean: %3lld", total_read, elpased,
			 mean);
}

muggle_thread_ret_t run_pipe_write(void *args)
{
	muggle_socket_evloop_pipe_t *ev_pipe = (muggle_socket_evloop_pipe_t *)args;

	const int bufsize = 16;
	const int num = 1024;
	char *str_array = (char *)malloc(sizeof(char *) * num * bufsize);
	for (int i = 0; i < num; ++i) {
		char *s = str_array + bufsize * i;
		snprintf(s, bufsize, "%08d", i);
	}

	int idx = 0;
	while (true) {
		if (idx % s_write_n == 0) {
			muggle_msleep(100);
		}

		char *s = str_array + bufsize * idx;
		muggle_socket_evloop_pipe_write(ev_pipe, s);
		idx = (idx + 1) % num;
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
	muggle_socket_evloop_handle_set_timer_interval(&handle, 0);
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

int main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr,
				"Usage: %s <batch_read> <write_n>\n"
				"  - batch_read: read use batch or one by one\n"
				"  - write_n: write number of data once\n"
				"e.g.\n"
				"  %s 0 64\n"
				"  %s 1 64\n"
				"  %s 0 1\n"
				"  %s 1 1\n"
				"",
				argv[0], argv[0], argv[0], argv[0], argv[0]);
		exit(EXIT_FAILURE);
	}
	s_batch_read = atoi(argv[1]);
	s_write_n = atoi(argv[2]);

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

	const int require_pipe_size = 4 * 1024 * 1024;
	int r_bufsize = 0;
	int w_bufsize = 0;
	r_bufsize = muggle_socket_evloop_pipe_get_r_size(&ev_pipe);
	w_bufsize = muggle_socket_evloop_pipe_get_w_size(&ev_pipe);
	LOG_INFO("default pipe size: r=%d, w=%d", r_bufsize, w_bufsize);

	if (muggle_socket_evloop_pipe_set_r_size(&ev_pipe, require_pipe_size)) {
		LOG_INFO("success set pipe r size: %d", require_pipe_size);
	} else {
		int errnum = muggle_event_lasterror();
		char errmsg[256];
		muggle_event_strerror(errnum, errmsg, sizeof(errmsg));
		LOG_ERROR("failed set pipe r size: %d, errno: %d, errmsg: %s",
				  require_pipe_size, errnum, errmsg);
	}
	if (muggle_socket_evloop_pipe_set_w_size(&ev_pipe, require_pipe_size)) {
		LOG_INFO("success set pipe w size: %d", require_pipe_size);
	} else {
		int errnum = muggle_event_lasterror();
		char errmsg[256];
		muggle_event_strerror(errnum, errmsg, sizeof(errmsg));
		LOG_ERROR("failed set pipe w size: %d, errno: %d, errmsg: %s",
				  require_pipe_size, errnum, errmsg);
	}

	r_bufsize = muggle_socket_evloop_pipe_get_r_size(&ev_pipe);
	w_bufsize = muggle_socket_evloop_pipe_get_w_size(&ev_pipe);
	LOG_INFO("current pipe size: r=%d, w=%d", r_bufsize, w_bufsize);

	muggle_msleep(1000);

	muggle_thread_t th;
	muggle_thread_create(&th, run_pipe_write, &ev_pipe);

	run_evloop(&ev_pipe);

	muggle_thread_join(&th);

	muggle_socket_evloop_pipe_destroy(&ev_pipe);

	return 0;
}
