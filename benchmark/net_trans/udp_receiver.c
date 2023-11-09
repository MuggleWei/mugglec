#include "udp_receiver.h"

struct udp_recv_user_data {
	muggle_benchmark_handle_t *handle;
	muggle_benchmark_config_t *config;
};

static void on_udp_message(muggle_event_loop_t *evloop,
						   muggle_socket_context_t *ctx)
{
	struct udp_recv_user_data *data =
		(struct udp_recv_user_data *)muggle_evloop_get_data(evloop);

	char buf[65536];
	while (1) {
		int n = muggle_socket_ctx_recv(ctx, buf, sizeof(buf), 0);
		if (n > 0) {
			if (onRecvPkg(ctx, (struct pkg *)buf, data->handle, data->config) !=
				0) {
				muggle_evloop_exit(evloop);
				break;
			}
		} else {
			int errnum = MUGGLE_SOCKET_LAST_ERRNO;
			if (errnum == MUGGLE_SYS_ERRNO_WOULDBLOCK) {
				continue;
			} else {
				muggle_event_strerror(errnum, buf, sizeof(buf));
				LOG_ERROR("failed socket read: %s", buf);
			}
			break;
		}
	}
}

static void run_udp_receiver_multiplexing(const char *host, const char *port,
										  int is_busy,
										  muggle_benchmark_handle_t *handle,
										  muggle_benchmark_config_t *config)
{
	// user data
	struct udp_recv_user_data user_data;
	memset(&user_data, 0, sizeof(user_data));
	user_data.handle = handle;
	user_data.config = config;

	// init event loop
	muggle_event_loop_init_args_t ev_init_args;
	memset(&ev_init_args, 0, sizeof(ev_init_args));
	ev_init_args.hints_max_fd = 32;
	ev_init_args.use_mem_pool = 0;

	muggle_event_loop_t *evloop = muggle_evloop_new(&ev_init_args);
	if (evloop == NULL) {
		LOG_ERROR("failed new event loop");
		exit(EXIT_FAILURE);
	}
	LOG_INFO("success new event loop");
	muggle_evloop_set_data(evloop, &user_data);

	// init socket event loop handle
	muggle_socket_evloop_handle_t evloop_handle;
	muggle_socket_evloop_handle_init(&evloop_handle);
	muggle_socket_evloop_handle_set_cb_msg(&evloop_handle, on_udp_message);
	if (is_busy) {
		muggle_socket_evloop_handle_set_timer_interval(&evloop_handle, 0);
	}
	muggle_socket_evloop_handle_attach(&evloop_handle, evloop);
	LOG_INFO("socket handle attached event loop");

	// init udp socket context
	muggle_socket_t fd = muggle_udp_bind(host, port);
	if (fd == MUGGLE_INVALID_SOCKET) {
		MUGGLE_LOG_ERROR("failed create udp bind for %s:%s", host, port);
		exit(EXIT_FAILURE);
	}
	muggle_socket_context_t ctx;
	muggle_socket_ctx_init(&ctx, fd, NULL, MUGGLE_SOCKET_CTX_TYPE_UDP);

	// add socket context into evloop
	muggle_socket_evloop_add_ctx(evloop, &ctx);

	// run event loop
	muggle_evloop_run(evloop);

	// destroy socket event loop handle
	muggle_socket_evloop_handle_destroy(&evloop_handle);

	// delete event loop
	muggle_evloop_delete(evloop);
}

static void run_udp_receiver_single(const char *host, const char *port,
									int is_busy,
									muggle_benchmark_handle_t *handle,
									muggle_benchmark_config_t *config)
{
	muggle_socket_t fd = muggle_udp_bind(host, port);
	if (fd == MUGGLE_INVALID_SOCKET) {
		MUGGLE_LOG_ERROR("failed create udp bind for %s:%s", host, port);
		exit(EXIT_FAILURE);
	}

	if (is_busy) {
		muggle_socket_set_nonblock(fd, 1);
	}

	muggle_socket_context_t ctx;
	muggle_socket_ctx_init(&ctx, fd, NULL, MUGGLE_SOCKET_CTX_TYPE_UDP);

	char buf[65536];
	while (1) {
		int n = muggle_socket_read(fd, buf, sizeof(buf));
		if (n > 0) {
			if (onRecvPkg(&ctx, (struct pkg *)buf, handle, config) != 0) {
				break;
			}
		} else {
			int errnum = MUGGLE_SOCKET_LAST_ERRNO;
			if (is_busy && errnum == MUGGLE_SYS_ERRNO_WOULDBLOCK) {
				continue;
			} else {
				muggle_event_strerror(errnum, buf, sizeof(buf));
				LOG_ERROR("failed socket read: %s", buf);
			}
			break;
		}
	}

	muggle_socket_ctx_close(&ctx);
}

void run_udp_receiver(const char *host, const char *port, int is_multiplexing,
					  int is_busy, muggle_benchmark_handle_t *handle,
					  muggle_benchmark_config_t *config)
{
	if (is_multiplexing) {
		run_udp_receiver_multiplexing(host, port, is_busy, handle, config);
	} else {
		run_udp_receiver_single(host, port, is_busy, handle, config);
	}
}
