#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>

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
	bool is_listener;                //!< is listener peer
	bool is_alive;                   //!< is socket alive
}socket_peer_t;

void close_socket_peer(socket_peer_t *peer)
{
	if (peer->is_alive)
	{
		close(peer->fd);
		fprintf(stderr, "close socket: %s\n", peer->straddr);
		peer->is_alive = false;
	}
}

void free_socket_peer(void *pool, void *data)
{
	socket_peer_t *peer = (socket_peer_t*)data;
	close_socket_peer(peer);
	free(peer);
}

void on_connection(socket_peer_t *peer)
{
	fprintf(stdout, "on_connection: %s\n", peer->straddr);
}

void on_message(socket_peer_t *peer, char *buf, ssize_t num_bytes)
{
	while (1)
	{
		ssize_t num_write = send(peer->fd, buf, num_bytes, 0);
		if (num_write != num_bytes)
		{
			if (errno == EINTR)
			{
				fprintf(stdout, "send interupt by signal, retry...\n");
				continue;
			}
			else
			{
				fprintf(stderr, "send error: addr=%s, err=%s", 
					peer->straddr, strerror(errno));
				close_socket_peer(peer);
			}
		}
		else
		{
			fprintf(stdout, "send %ld bytes to %s\n", (long)num_write, peer->straddr);
		}

		break;
	}
}

void on_timer()
{
	fprintf(stdout, "timer...\n");
}

void on_error(socket_peer_t *peer)
{
	fprintf(stdout, "on_error: %s\n", peer->straddr);
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

bool tcp_listen(const char *host, const char *port, socket_peer_t *peer, int backlog)
{
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int n = 0;
	if ((n = getaddrinfo(host, port, &hints, &res)) != 0)
	{
		fprintf(stderr, "failed tcp listen for %s:%s - getaddrinfo return '%s'\n",
			host, port, gai_strerror(n));
		return false;
	}
	
	int lfd = -1;
	struct addrinfo *p_addr = NULL;
	for (p_addr = res; p_addr != NULL; p_addr = p_addr->ai_next)
	{
		lfd = socket(p_addr->ai_family, p_addr->ai_socktype, p_addr->ai_protocol);
		if (lfd == -1)
		{
			continue;
		}

		// always set SO_REUSEADDR for bind socket
		int on = 1;
		if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, (void*)&on, sizeof(on)) != 0)
		{
			fprintf(stderr, "failed setsockopt: err=%s\n", strerror(errno));
			close(lfd);
			lfd = -1;
			continue;
		}

		if (bind(lfd, p_addr->ai_addr, p_addr->ai_addrlen) == 0)
		{
			// success
			break;
		}

		close(lfd);
		lfd = -1;
	}

	if (p_addr == NULL)
	{
		fprintf(stderr, "failed to create and bind tcp socket %s:%s\n", host, port);
		goto tcp_listen_err;
	}

	if (listen(lfd, backlog) != 0)
	{
		fprintf(stderr, "failed listen %s:%s\n", host, port);
		goto tcp_listen_err;
	}

	peer->fd = lfd;
	memcpy(&peer->addr, p_addr->ai_addr, p_addr->ai_addrlen);
	peer->addrlen = p_addr->ai_addrlen;
	get_addr_str(
		(struct sockaddr*)&peer->addr, peer->addrlen,
		peer->straddr, sizeof(peer->straddr));

	peer->is_listener = true;
	peer->is_alive = true;

	set_socket_nonblock(peer->fd);

	freeaddrinfo(res);

	return true;

tcp_listen_err:
	if (lfd != -1)
	{
		close(lfd);
	}

	if (res)
	{
		freeaddrinfo(res);
	}

	return false;
}

void on_accept_connection(
	socket_peer_t *new_peer, muggle_linked_list_t *p_list, fd_set *read_fds, int *nfds)
{
	// fillup peer info
	get_addr_str(
		(struct sockaddr*)&new_peer->addr, new_peer->addrlen,
		new_peer->straddr, sizeof(new_peer->straddr));
	new_peer->is_listener = false;
	new_peer->is_alive = true;

	// set socket nonblocking
	set_socket_nonblock(new_peer->fd);

	// add new socket into read fds
	FD_SET(new_peer->fd, read_fds);
	if (new_peer->fd > *nfds)
	{
		*nfds = new_peer->fd;
	}

	// append to list
	muggle_linked_list_append(p_list, NULL, new_peer);

	// connection callback functions
	on_connection(new_peer);
}

void do_accept(socket_peer_t *listener, muggle_linked_list_t *p_list, fd_set *read_fds, int *nfds)
{
	socket_peer_t *new_peer = NULL;
	while (1)
	{
		if (new_peer == NULL)
		{
			new_peer = (socket_peer_t*)malloc(sizeof(socket_peer_t));
			if (new_peer == NULL)
			{
				break;
			}
		}

		memset(new_peer, 0, sizeof(*new_peer));
		new_peer->addrlen = sizeof(new_peer->addr);
		new_peer->fd = accept(listener->fd, (struct sockaddr*)&new_peer->addr, &new_peer->addrlen);
		if (new_peer->fd == -1)
		{
			if (errno == EINTR)
			{
				continue;
			}
			else if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				break;
			}
			else
			{
				fprintf(stderr, "failed accept: %s\n", strerror(errno));
				close_socket_peer(listener);
				on_error(listener);
				break;
			}
		}

		// on accept connection
		on_accept_connection(new_peer, p_list, read_fds, nfds);

		// set new_peer null
		new_peer = NULL;
	}

	if (new_peer)
	{
		free(new_peer);
	}
}

void do_read(socket_peer_t *peer)
{
	char buf[1024];
	ssize_t num_read;
	while (1)
	{
		if (!peer->is_alive)
		{
			break;
		}

		num_read = recv(peer->fd, buf, sizeof(buf), 0);
		if (num_read > 0)
		{
			on_message(peer, buf, num_read);
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
					break;
				}
				else
				{
					fprintf(stdout, "client disconnected: addr=%s, err=%s\n",
						peer->straddr, strerror(errno));
					close_socket_peer(peer);
					break;
				}
			}
			else if (num_read == 0)
			{
				fprintf(stdout, "remote client closed: addr=%s\n", peer->straddr);
				close_socket_peer(peer);
				break;
			}
		}
	}
}

void select_timer(struct timeval *timeout,  struct timespec *t1, struct timespec *t2)
{
	if (!timeout)
	{
		return;
	}

	timespec_get(t2, TIME_UTC);
	int interval_ms =
		(int)((t2->tv_sec - t1->tv_sec) * 1000 +
		(t2->tv_nsec - t1->tv_nsec) / 1000000);

	int timeout_ms = timeout->tv_sec * 1000 + timeout->tv_usec / 1000;
	if (interval_ms >= timeout_ms)
	{
		on_timer();
		t1->tv_sec = t2->tv_sec;
		t1->tv_nsec = t2->tv_nsec;
	}
}

void select_loop(socket_peer_t *listener, struct timeval *timeout)
{
	// set read fd set
	fd_set read_fds;
	FD_ZERO(&read_fds);
	FD_SET(listener->fd, &read_fds);

	// set max fd
	int nfds = listener->fd;

	// socket peer linked list
	muggle_linked_list_t list;
	muggle_linked_list_init(&list, 0);
	muggle_linked_list_append(&list, NULL, listener);

	// for timer
	struct timespec t1, t2;
	if (timeout)
	{
		timespec_get(&t1, TIME_UTC);
	}

	// select loop
	struct timeval cur_timeout;
	fd_set cur_read_fds;
	while (1)
	{
		// reset timeout
		if (timeout)
		{
			memcpy(&cur_timeout, timeout, sizeof(cur_timeout));
		}

		// reset read fd set
		cur_read_fds = read_fds;
		int n = select(nfds + 1, &cur_read_fds, NULL, NULL, &cur_timeout);
		if (n > 0)
		{
			muggle_linked_list_node_t *node = muggle_linked_list_first(&list);
			while (node)
			{
				socket_peer_t *peer = (socket_peer_t*)node->data;

				if (FD_ISSET(peer->fd, &cur_read_fds))
				{
					if (peer->is_listener)
					{
						do_accept(peer, &list, &read_fds, &nfds);
						if (!peer->is_alive)
						{
							fprintf(stderr, "listener socket accept error\n");
							break;
						}
					}
					else
					{
						do_read(peer);
					}

					if (!peer->is_alive)
					{
						on_error(peer);

						FD_CLR(peer->fd, &read_fds);
						node = muggle_linked_list_remove(&list, node, free_socket_peer, NULL);

						if (--n <= 0)
						{
							break;
						}

						continue;
					}
				}

				node = muggle_linked_list_next(&list, node);
			}

			// calculate timer
			select_timer(timeout, &t1, &t2);
		}
		else if (n == 0)
		{
			select_timer(timeout, &t1, &t2);
		}
		else
		{
			if (errno == EINTR)
			{
				continue;
			}

			fprintf(stderr, "failed select loop: %s\n", strerror(errno));
			break;
		}
	}

	fprintf(stderr, "exit loop\n");

	muggle_linked_list_destroy(&list, free_socket_peer, NULL);
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

	// tcp listen
	socket_peer_t *listener = (socket_peer_t*)malloc(sizeof(socket_peer_t));
	memset(listener, 0, sizeof(*listener));
	if (!tcp_listen(host, str_port, listener, 32))
	{
		fprintf(stderr, "failed tcp_listen %s:%s\n", host, str_port);
		exit(EXIT_FAILURE);
	}

	// set timeout
	struct timeval timeout;
	memset(&timeout, 0, sizeof(timeout));
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	// select loop
	select_loop(listener, &timeout);
	
	return 0;
}
