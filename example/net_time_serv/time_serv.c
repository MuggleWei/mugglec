#include "time_serv_handle.h"

muggle_event_loop_t* create_evloop(evloop_data_t *args)
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
	if (muggle_log_simple_init(LOG_LEVEL_INFO, LOG_LEVEL_INFO) != 0)
	{
		LOG_ERROR("failed initalize log");
		exit(EXIT_FAILURE);
	}

	if (argc < 3)
	{
		LOG_ERROR("usage: %s <IP> <Port> [udp-ip] [udp-port]", argv[0]);
		exit(EXIT_FAILURE);
	}

	// initialize socket library
	if (muggle_socket_lib_init() != 0)
	{
		LOG_ERROR("failed initialize socket library");
		exit(EXIT_FAILURE);
	}

	// prepare aruments
	evloop_data_t args;
	memset(&args, 0, sizeof(args));
	args.host = argv[1];
	args.serv = argv[2];
	if (argc >= 5)
	{
		args.multicast_host = argv[3];
		args.multicast_serv = argv[4];
	}
	muggle_linked_list_init(&args.conn_list, 0);

	// init event loop
	muggle_event_loop_t *evloop = create_evloop(&args);

	// bind socket event loop handle
	muggle_socket_evloop_handle_t handle;
	muggle_socket_evloop_handle_init(&handle);
	muggle_socket_evloop_handle_set_timer_interval(&handle, 1000);
	muggle_socket_evloop_handle_set_cb_add_ctx(&handle, on_add_ctx);
	muggle_socket_evloop_handle_set_cb_conn(&handle, on_connect);
	muggle_socket_evloop_handle_set_cb_close(&handle, on_close);
	muggle_socket_evloop_handle_set_cb_timer(&handle, on_timer);
	muggle_socket_evloop_handle_set_cb_release(&handle, on_release);
	muggle_socket_evloop_handle_attach(&handle, evloop);
	LOG_INFO("socket handle attached to event loop");

	// run listen and mcast thread
	run_tcp_listen(&args);
	run_mcast_conn(&args);

	// run
	muggle_evloop_run(evloop);

	// clear
	muggle_evloop_delete(evloop);

	// destroy conn list
	muggle_linked_list_destroy(&args.conn_list, NULL, NULL);

	return 0;
}
