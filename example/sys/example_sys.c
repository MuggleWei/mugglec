#include "muggle/c/muggle_c.h"

int main()
{
	muggle_log_complicated_init(MUGGLE_LOG_LEVEL_DEBUG, -1, NULL);

	muggle_socket_t fd = MUGGLE_INVALID_SOCKET;
	muggle_socket_close(fd);

	MUGGLE_LOG_SYS_ERR(MUGGLE_LOG_LEVEL_INFO, "failed close socket fd");

	return 0;
}
