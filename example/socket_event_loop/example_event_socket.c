#include "args.h"
#include "tcp_server.h"

muggle_thread_ret_t wakeup_evloop_routine(void *p)
{
	evloop_user_data_t *user_data = (evloop_user_data_t*)p;

	for (int i = 0; i < 5; i++)
	{
		muggle_msleep(user_data->wakeup_interval * 1000);
		LOG_INFO("try wakeup event loop #%d", i);
		if (user_data->evloop)
		{
			muggle_evloop_wakeup(user_data->evloop);
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	// initialize network
	muggle_socket_lib_init();

	// initialize log
	muggle_log_complicated_init(LOG_LEVEL_DEBUG, -1, NULL);

	// parse input arguments
	sys_args_t args;
	parse_sys_args(argc, argv, &args);

	// run wakeup thread
	evloop_user_data_t user_data;
	memset(&user_data, 0, sizeof(user_data));
	user_data.msg = "hello world";
	user_data.wakeup_interval = 1;

	muggle_thread_t th;
	muggle_thread_create(&th, wakeup_evloop_routine, &user_data);

	// run example
	switch (args.action_type)
	{
		case ACTION_TCP_SERVER:
		{
			tcp_server_run(&args, &user_data);
		}break;
		case ACTION_TCP_CLIENT:
		{
			// TODO:
		}break;
		case ACTION_UDP_RECV:
		{
			// TODO:
		}break;
		case ACTION_UDP_SEND:
		{
			// TODO:
		}break;
		default:
		{
			LOG_ERROR("invalid action type");
		};
	}

	// wait wakeup thread close
	muggle_thread_join(&th);

	LOG_INFO("bye");

	return 0;
}
