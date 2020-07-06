#include "udp_receiver.h"
#include "gen_report.h"

typedef int (*fn_on_pkg)(void *buf);

static fn_on_pkg s_callbacks[MAX_MSG_TYPE] = {NULL};

static int s_cnt = 0;

int on_pkg(void *buf)
{
	struct pkg *msg = (struct pkg*)buf;

	if (msg->data.idx >= TRANS_PKG_ROUND * PKG_PER_ROUND)
	{
		MUGGLE_LOG_ERROR("message idx[%u] beyond range", (unsigned int)msg->data.idx);
		return 0;
	}

	muggle_benchmark_block_t *block = &g_blocks[s_cnt++];
	block->ts[0].tv_sec = msg->data.sec;
	block->ts[0].tv_nsec = msg->data.nsec;

	timespec_get(&block->ts[1], TIME_UTC);

	return 0;
}

int on_end(void *buf)
{
	return 1;
}

void run_udp_receiver(const char *host, const char *port)
{
	muggle_socket_peer_t udp_peer;
	udp_peer.fd = muggle_udp_bind(host, port, &udp_peer);
	if (udp_peer.fd == MUGGLE_INVALID_SOCKET)
	{
		MUGGLE_LOG_ERROR("failed create udp bind for %s:%s", host, port);
		exit(EXIT_FAILURE);
	}

	g_blocks = malloc(sizeof(muggle_benchmark_block_t) * TRANS_PKG_ROUND * PKG_PER_ROUND);
	for (int i = 0; i < TRANS_PKG_ROUND * PKG_PER_ROUND; i++)
	{
		memset(&g_blocks[i], 0, sizeof(muggle_benchmark_block_t));
	}

	s_callbacks[MSG_TYPE_NULL] = NULL;
	s_callbacks[MSG_TYPE_PKG] = on_pkg;
	s_callbacks[MSG_TYPE_END] = on_end;

	char buf[65536];
	while (1)
	{
		int n = recv(udp_peer.fd, buf, sizeof(buf), 0);
		if (n > 0)
		{
			struct pkg *msg = (struct pkg*)buf;
			if (msg->header.msg_type >= MAX_MSG_TYPE)
			{
				MUGGLE_LOG_ERROR("invalid msg_type: %d", (int)msg->header.msg_type);
				continue;
			}

			if (s_callbacks[msg->header.msg_type])
			{
				if (s_callbacks[msg->header.msg_type](buf) != 0)
				{
					break;
				}
			}
		}
	}

	MUGGLE_LOG_INFO("recv %d pkgs", s_cnt);
	gen_benchmark_report("udp_latency", g_blocks, s_cnt);

	free(g_blocks);
}
