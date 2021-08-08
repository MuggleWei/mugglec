#include "muggle/c/muggle_c.h"

int main(int argc, char *argv[])
{
	// init log and socket library
	if (muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO) != 0)
	{
		MUGGLE_LOG_ERROR("failed initalize log");
		exit(EXIT_FAILURE);
	}

	if (muggle_socket_lib_init() != 0)
	{
		MUGGLE_LOG_ERROR("failed initalize socket library");
		exit(EXIT_FAILURE);
	}

	if (argc != 3)
	{
		MUGGLE_LOG_ERROR("usage: %s <host> <port>", argv[0]);
		exit(EXIT_FAILURE);
	}

	// TODO:

	return 0;
}