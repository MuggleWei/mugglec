#include "time_client_handle.h"

muggle_event_loop_t* create_evloop(conn_thread_args_t *args)
{
	// new event loop
	muggle_event_loop_init_args_t ev_init_args;
	memset(&ev_init_args, 0, sizeof(ev_init_args));
	ev_init_args.evloop_type = MUGGLE_EVLOOP_TYPE_NULL;
	ev_init_args.hints_max_fd = 32;
	ev_init_args.use_mem_pool = 0;

	muggle_event_loop_t *evloop = muggle_evloop_new(&ev_init_args);
	if (evloop == NULL)
	{
		LOG_ERROR("failed new event loop");
		exit(EXIT_FAILURE);
	}
	LOG_INFO("success new event loop");

	muggle_evloop_set_data(evloop, args);
	args->evloop = evloop;

	return evloop;
}

int main(int argc, char *argv[])
{
	// initialize log
	if (muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO) != 0)
	{
		MUGGLE_LOG_ERROR("failed init log");
		exit(EXIT_FAILURE);
	}

	if (argc != 4)
	{
		MUGGLE_LOG_ERROR("usage: %s <host> <port> <tcp|udp|mcast>", argv[0]);
		exit(EXIT_FAILURE);
	}

	// initialize socket library
	muggle_socket_lib_init();

	// prepare connection info
	conn_thread_args_t args;
	args.host = argv[1];
	args.serv = argv[2];
	args.sock_type = argv[3];

	// init event loop
	muggle_event_loop_t *evloop = create_evloop(&args);

	// bind socket event loop handle
	muggle_socket_evloop_handle_t handle;
	muggle_socket_evloop_handle_init(&handle);
	muggle_socket_evloop_handle_set_cb_add_ctx(&handle, on_add_ctx);
	muggle_socket_evloop_handle_set_cb_msg(&handle, on_message);
	muggle_socket_evloop_handle_set_cb_close(&handle, on_close);
	muggle_socket_evloop_handle_set_cb_release(&handle, on_release);
	muggle_socket_evloop_handle_attach(&handle, evloop);
	LOG_INFO("socket handle attached to event loop");

	// run connect thread
	run_conn_thread(&args);

	// run
	muggle_evloop_run(evloop);

	// clear
	muggle_evloop_delete(evloop);

	return 0;
}
