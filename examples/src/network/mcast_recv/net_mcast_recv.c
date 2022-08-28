#include "muggle/c/muggle_c.h"

int main(int argc, char *argv[])
{
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

	if (argc < 3)
	{
		MUGGLE_LOG_ERROR("usage: %s <mcast-IP> <mcast-Port> [net-iface] [mcast-source-group]", argv[0]);
		exit(EXIT_FAILURE);
	}

	const char *grp_host = argv[1];
	const char *grp_serv = argv[2];
	const char *iface = NULL;
	const char *src_grp = NULL;
	if (argc > 3)
	{
		iface = argv[3];
	}
	if (argc > 4)
	{
		src_grp = argv[4];
	}

	MUGGLE_LOG_INFO("join multicast group");
	muggle_socket_t fd = muggle_mcast_join(grp_host, grp_serv, iface, src_grp);
	if (fd == MUGGLE_INVALID_SOCKET)
	{
		MUGGLE_LOG_ERROR("failed multicast join %s:%s", grp_host, grp_serv);
		exit(EXIT_FAILURE);
	}

	int n;
	char straddr[MUGGLE_SOCKET_ADDR_STRLEN];
	char buf[65536];
	struct sockaddr_storage addr;
	muggle_socklen_t addrlen;
	int count = 0;
	while (1)
	{
		addrlen = sizeof(addr);
		n = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &addrlen);

		if (muggle_socket_ntop((struct sockaddr*)&addr, straddr, MUGGLE_SOCKET_ADDR_STRLEN, 0) == NULL)
		{
			snprintf(straddr, MUGGLE_SOCKET_ADDR_STRLEN, "unknown:unknown");
		}
		MUGGLE_LOG_INFO("recv %d bytes from %s", n, straddr);

		if (++count > 10)
		{
			MUGGLE_LOG_INFO("leave multicast group");
			if (muggle_mcast_leave(fd, grp_host, grp_serv, iface, src_grp) != 0)
			{
				MUGGLE_LOG_ERROR("failed leave multicast group");
			}
			muggle_socket_close(fd);
			break;
		}
	}

	return 0;
}
