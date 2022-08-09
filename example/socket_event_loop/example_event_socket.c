#include "args.h"
#include "tcp_server.h"

int main(int argc, char *argv[])
{
	// initialize network
	muggle_socket_lib_init();

	// initialize log
	muggle_log_complicated_init(LOG_LEVEL_DEBUG, -1, NULL);

	// parse input arguments
	sys_args_t args;
	parse_sys_args(argc, argv, &args);

	// run example
	switch (args.action_type)
	{
		case ACTION_TCP_SERVER:
		{
			tcp_server_run(&args);
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

	LOG_INFO("bye");

	return 0;
}
