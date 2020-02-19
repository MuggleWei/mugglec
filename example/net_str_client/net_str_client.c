#include "muggle/c/muggle_c.h"

#define SND_RCV_BUF_SIZE 4096

#if MUGGLE_PLATFORM_WINDOWS

struct thread_args
{
	FILE *fp;
	muggle_socket_t client;
};

muggle_thread_ret_t thread_get_console_input(void *arg)
{
	struct thread_args *parg = (struct thread_args*)arg;
	FILE *fp = parg->fp;
	muggle_socket_t client = parg->client;

	char sendbuf[SND_RCV_BUF_SIZE + 1];
	while (fgets(sendbuf, SND_RCV_BUF_SIZE, fp) != NULL)
	{
		int n = (int)strlen(sendbuf);
		int num_bytes = send(client, sendbuf, n, 0);

		if (num_bytes != n)
		{
			if (num_bytes == MUGGLE_SOCKET_ERROR)
			{
				char err_msg[1024] = { 0 };
				muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
				MUGGLE_WARNING("failed send msg - %s", err_msg);
			}
			else
			{
				MUGGLE_WARNING("send buffer full");
			}
			return 0;
		}
	}

	muggle_socket_close(client);
	return 0;
}

void str_client_windows(FILE *fp, muggle_socket_t client)
{
	struct thread_args arg = { fp, client };

	muggle_thread_t thread;
	muggle_thread_create(&thread, thread_get_console_input, &arg);
	muggle_thread_detach(&thread);

	char recvbuf[SND_RCV_BUF_SIZE + 1];
	while (1)
	{
		int num_bytes = recv(client, recvbuf, SND_RCV_BUF_SIZE, 0);
		if (num_bytes == 0)
		{
			MUGGLE_INFO("disconnect");
			return;
		}
		else if (num_bytes == MUGGLE_SOCKET_ERROR)
		{
			if (MUGGLE_SOCKET_LAST_ERRNO == WSAEINTR)
			{
				continue;
			}

			char err_msg[1024] = { 0 };
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_ERROR("failed recv - %s", err_msg);
			break;
		}

		recvbuf[num_bytes] = '\0';
		MUGGLE_INFO(recvbuf);
	}
}

#else
void str_client_posix(FILE *fp, muggle_socket_t client)
{
	char sendbuf[SND_RCV_BUF_SIZE + 1], recvbuf[SND_RCV_BUF_SIZE + 1];

	fd_set rset;
	FD_ZERO(&rset);
	while (1)
	{
		int nfds = 0;

		nfds = MUGGLE_FILENO(fp) > client ? MUGGLE_FILENO(fp) : client;
		nfds += 1;

		FD_SET(MUGGLE_FILENO(fp), &rset);
		FD_SET(client, &rset);
		int n = select(nfds, &rset, NULL, NULL, NULL);
		if (n == MUGGLE_SOCKET_ERROR)
		{
			char err_msg[1024] = { 0 };
			muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
			MUGGLE_ERROR("failed select - %s", err_msg);
			break;
		}
		else if (n == 0)
		{
			// timeout
		}

		if (FD_ISSET(client, &rset)) // socket is readable
		{
			int num_bytes = recv(client, recvbuf, SND_RCV_BUF_SIZE, 0);
			if (num_bytes == 0)
			{
				MUGGLE_INFO("disconnect");
				break;
			}
			else if (num_bytes == MUGGLE_SOCKET_ERROR)
			{
				if (MUGGLE_SOCKET_LAST_ERRNO == EINTR)
				{
					continue;
				}

				char err_msg[1024] = {0};
				muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
				MUGGLE_ERROR("failed recv - %s", err_msg);
				break;
			}

			recvbuf[num_bytes] = '\0';
			MUGGLE_INFO(recvbuf);
		}

		if (FD_ISSET(MUGGLE_FILENO(fp), &rset))
		{
			if (fgets(sendbuf, SND_RCV_BUF_SIZE, fp) == NULL)
			{
				break;
			}

			int n = (int)strlen(sendbuf);
			int num_bytes = send(client, sendbuf, n, 0);

			if (num_bytes != n)
			{
				if (num_bytes == MUGGLE_SOCKET_ERROR)
				{
					char err_msg[1024] = { 0 };
					muggle_socket_strerror(MUGGLE_SOCKET_LAST_ERRNO, err_msg, sizeof(err_msg));
					MUGGLE_WARNING("failed send msg - %s", err_msg);
				}
				else
				{
					MUGGLE_WARNING("send buffer full");
				}
				break;
			}
		}
	}
}
#endif

int main(int argc, char *argv[])
{
	// initialize log
	if (muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO) != 0)
	{
		MUGGLE_ERROR("failed init log");
		exit(EXIT_FAILURE);
	}

	if (argc != 4)
	{
		MUGGLE_ERROR("usage: %s <IP> <Port> <tcp|udp>", argv[0]);
		exit(EXIT_FAILURE);
	}

	// initialize socket library
	muggle_socket_lib_init();

#if !MUGGLE_PLATFORM_WINDOWS
	// ignore PIPE
	signal(SIGPIPE, SIG_IGN);
#endif

	muggle_socket_peer_t peer;
	if (strcmp(argv[3], "tcp") == 0)
	{
		peer.fd = muggle_tcp_connect(argv[1], argv[2], 3, &peer);
	}
	else if (strcmp(argv[3], "udp") == 0)
	{
		peer.fd = muggle_udp_connect(argv[1], argv[2], &peer);
	}
	else
	{
		MUGGLE_ERROR("invalid socket peer type: %s", argv[3]);
		exit(EXIT_FAILURE);
	}

	if (peer.fd == MUGGLE_INVALID_SOCKET)
	{
		MUGGLE_ERROR("%s failed connect: %s:%s", argv[3], argv[1], argv[2]);
		exit(EXIT_FAILURE);
	}

	char straddr[MUGGLE_SOCKET_ADDR_STRLEN];
	if (muggle_socket_ntop((struct sockaddr*)&peer.addr, straddr, MUGGLE_SOCKET_ADDR_STRLEN, 0) == NULL)
	{
		snprintf(straddr, MUGGLE_SOCKET_ADDR_STRLEN, "unknown:unknown");
	}

	MUGGLE_INFO("%s connect %s", argv[3], straddr);

#if MUGGLE_PLATFORM_WINDOWS
	str_client_windows(stdin, peer.fd);
#else
	str_client_posix(stdin, peer.fd); // use select
#endif

	muggle_socket_close(peer.fd);

	return 0;
}
