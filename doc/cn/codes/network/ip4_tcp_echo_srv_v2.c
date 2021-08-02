#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "muggle/c/muggle_c.h"

#define INET_ADDR_STR_LEN (NI_MAXHOST + NI_MAXSERV + 5)

typedef struct socket_peer
{
	int                     fd;      //!< socket file description
	struct sockaddr_storage addr;    //!< socket address
	socklen_t               addrlen; //!< len of socket address struct

	char straddr[INET_ADDR_STR_LEN]; //!< socket address presentation string
}socket_peer_t;

bool echo_message(socket_peer_t *peer, char *buf, ssize_t num_bytes)
{
	bool ret = false;
	while (1)
	{
		int flags = MSG_DONTWAIT;
		ssize_t num_write = send(peer->fd, buf, num_bytes, flags);
		if (num_write != num_bytes)
		{
			if (errno == EINTR)
			{
				fprintf(stdout, "send interupt by signal, retry...\n");
				continue;
			}
			else
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK)
				{
					fprintf(stdout, "send would block: addr=%s\n", peer->straddr);
					ret = false;
				}
				else
				{
					fprintf(stderr, "failed send: addr=%s, err=%s",
						peer->straddr, strerror(errno));
					ret = false;
				}
			}
		}
		else
		{
			fprintf(stdout, "send %ld bytes to %s\n", (long)num_write, peer->straddr);
			ret = true;
		}

		break;
	}

	return true;
}

void* handle_client(socket_peer_t *peer)
{
	fprintf(stdout, "handle client: addr=%s\n", peer->straddr);

	char buf[1024];
	ssize_t num_read;
	while (1)
	{
		num_read = recv(peer->fd, buf, sizeof(buf), 0);
		if (num_read > 0)
		{
			if (!echo_message(peer, buf, num_read))
			{
				fprintf(stdout, "failed echo message: addr=%s\n", peer->straddr);
				break;
			}
			continue;
		}
		else
		{
			if (num_read == -1)
			{
				if (errno == EINTR)
				{
					continue;
				}
				else if (errno == EAGAIN || errno == EWOULDBLOCK)
				{
					fprintf(stdout, "client would block: addr=%s\n", peer->straddr);
					return peer;
				}
				else
				{
					fprintf(stdout, "client disconnected: addr=%s, err=%s\n",
						peer->straddr, strerror(errno));
					break;
				}
			}
			else if (num_read == 0)
			{
				fprintf(stdout, "remote client closed: addr=%s\n", peer->straddr);
				break;
			}
		}
	}

	close(peer->fd);

	free(peer);

	return NULL;
}

char* get_addr_str(const struct sockaddr *addr, socklen_t addrlen,
	char *addr_str, int addr_str_len)
{
	char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
	int ret = getnameinfo(addr, addrlen, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICSERV);
	if (ret == 0)
	{
		snprintf(addr_str, addr_str_len, "(%s, %s)", hbuf, sbuf);
	}
	else
	{
		snprintf(addr_str, addr_str_len, "(?UNKNOWN?)");
	}

	addr_str[addr_str_len - 1] = '\0';

	return addr_str;
}

int set_socket_nonblock(int socket)
{
	int flags = 0;
	flags = fcntl(socket, F_GETFL, 0);
	flags |= O_NONBLOCK;
	return fcntl(socket, F_SETFL, flags);
}

void free_socket_peer(void *pool, void *data)
{
	socket_peer_t *peer = (socket_peer_t*)data;

	close(peer->fd);
	free(peer);
}

int main(int argc, char *argv[])
{
	// check input arguments
	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s host port\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	const char *host = argv[1];
	const char *str_port = argv[2];

	// ignore signal SIGPIPE
	signal(SIGPIPE, SIG_IGN);

	// create ipv4 stream socket
	int lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd == -1)
	{
		fprintf(stderr, "failed create socket: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	// get listen address
	unsigned char buf[sizeof(struct in_addr)];
	int ret = inet_pton(AF_INET, host, buf);
	if (ret <= 0)
	{
		if (ret == 0)
		{
			fprintf(stderr, "host not in presentation format: %s\n", host);
		}
		else
		{
			fprintf(stderr, "failed inet_pton: %s\n", strerror(errno));
		}
		exit(EXIT_FAILURE);
	}

	uint16_t port = (uint16_t)atoi(str_port);
	if (port == 0)
	{
		fprintf(stderr, "invalid port: %s\n", str_port);
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	memcpy(&sin.sin_addr.s_addr, buf, sizeof(sin.sin_addr.s_addr));
	sin.sin_port = htons(port);

	// set socket option
	int optval = 1;
	if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
	{
		fprintf(stderr, "failed setsockopt: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	// bind and listen
	if (bind(lfd, (struct sockaddr*)&sin, sizeof(sin)) < 0)
	if (bind(lfd, (struct sockaddr*)&sin, sizeof(sin)) < 0)
	{
		fprintf(stderr, "failed bind: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (listen(lfd, 16) < 0)
	{
		fprintf(stderr, "failed listen: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	// set nonblocking
	if (set_socket_nonblock(lfd) == -1)
	{
		fprintf(stderr, "failed set socket nonblocking: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	// socket fd list
	muggle_linked_list_t list;
	muggle_linked_list_init(&list, 16);

	// handle client connection and message
	socket_peer_t *peer = NULL;
	while (1)
	{
		if (peer == NULL)
		{
			peer = (socket_peer_t*)malloc(sizeof(socket_peer_t));
			if (peer == NULL)
			{
				fprintf(stderr, "failed allocate memory");
				exit(EXIT_FAILURE);
			}
			memset(peer, 0, sizeof(*peer));
			peer->addrlen = sizeof(peer->addr);
		}

		peer->fd = accept(lfd, (struct sockaddr*)&peer->addr, &peer->addrlen);
		if (peer->fd == -1)
		{
			if (errno == EINTR)
			{
				continue;
			}
			else if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				// do nothing
				fprintf(stdout, "accept would block\n");
				muggle_msleep(500);
			}
			else
			{
				fprintf(stderr, "failed accept: %s\n", strerror(errno));

				free(peer);
				peer = NULL;
				break;
			}
		}
		else
		{
			// get address presentation string
			get_addr_str(
				(struct sockaddr*)&peer->addr, peer->addrlen,
				peer->straddr, sizeof(peer->straddr));

			// set nonblocking
			set_socket_nonblock(peer->fd);

			// append into list
			muggle_linked_list_append(&list, NULL, (void*)peer);

			peer = NULL;
		}

		// traverse and handle client
		muggle_linked_list_node_t *node = muggle_linked_list_first(&list);
		while (node)
		{
			socket_peer_t *data = (socket_peer_t*)node->data;

			if (handle_client(data) == NULL)
			{
				// already disconnected and close fd, remove node from list
				node = muggle_linked_list_remove(&list, node, NULL, NULL);
			}
			else
			{
				node = muggle_linked_list_next(&list, node);
			}
		}
	}

	muggle_linked_list_destroy(&list, free_socket_peer, NULL);
	
	return 0;
}

