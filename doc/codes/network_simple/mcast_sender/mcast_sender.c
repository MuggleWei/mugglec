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

	// create udp socket peer
	if (argc != 3)
	{
		MUGGLE_LOG_ERROR("usage: %s <host> <port>", argv[0]);
		exit(EXIT_FAILURE);
	}

	const char *host = argv[1];
	const char *serv = argv[2];

	muggle_socket_peer_t peer;
	if (muggle_udp_connect(host, serv, &peer) == MUGGLE_INVALID_SOCKET)
	{
		MUGGLE_LOG_ERROR("failed connect multicast target");
		exit(EXIT_FAILURE);
	}

	// send timestamp message
	uint32_t msg_id = 0;
	while (1)
	{
		char buf[128];
		memset(buf, 0, sizeof(buf));
		time_t ts = time(NULL);
		struct tm t;
		gmtime_r(&ts, &t);
		uint32_t n = (uint32_t)snprintf(buf, sizeof(buf),
			"[%u] %d-%02d-%02d %02d:%02d:%02d",
			(unsigned int)msg_id++,
			t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
			t.tm_hour, t.tm_min, t.tm_sec);

		muggle_socket_peer_send(&peer, buf, (size_t)n, 0);

		muggle_msleep(1000);
	}

	return 0;
}
