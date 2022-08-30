#include "args.h"
#include "server_evloop.h"

int main(int argc, char *argv[])
{
	// initialize network
	muggle_socket_lib_init();

	// initialize log
	muggle_log_complicated_init(LOG_LEVEL_DEBUG, -1, NULL);

	// parse input arguments
	sys_args_t args;
	if (parse_sys_args(argc, argv, &args) != 0)
	{
		LOG_ERROR("failed parse input arguments");
		exit(EXIT_FAILURE);
	}

	// run
	run_echo_server_evloop(args.host, args.port);

	return 0;
}
