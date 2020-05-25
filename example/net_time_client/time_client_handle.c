#include "time_client_handle.h"

#define UDP_MESSAGE_MAX_SIZE 65536

static inline void on_timestr(struct muggle_socket_event *ev, struct muggle_socket_peer *peer, char *msg)
{
	MUGGLE_LOG_INFO("recv timestamp string: %s", msg);
}

int on_error(struct muggle_socket_event *ev, struct muggle_socket_peer *peer)
{
	// output disconnection address string
	char straddr[MUGGLE_SOCKET_ADDR_STRLEN];
	if (muggle_socket_ntop((struct sockaddr*)&peer->addr, straddr, MUGGLE_SOCKET_ADDR_STRLEN, 0) == NULL)
	{
		snprintf(straddr, MUGGLE_SOCKET_ADDR_STRLEN, "unknown:unknown");
	}
	MUGGLE_LOG_WARNING("disconnect - %s", straddr);

	muggle_bytes_buffer_t *bytes_buf = (muggle_bytes_buffer_t*)peer->data;
	muggle_bytes_buffer_destroy(bytes_buf);

	// exit event loop
	muggle_socket_event_loop_exit(ev);

	return 0;
}

int on_message(struct muggle_socket_event *ev, struct muggle_socket_peer *peer)
{
	int bufsize = UDP_MESSAGE_MAX_SIZE;
	char buf[UDP_MESSAGE_MAX_SIZE];
	if (peer->peer_type == MUGGLE_SOCKET_PEER_TYPE_UDP_PEER)
	{
		while (1)
		{
			int n = recv(peer->fd, buf, sizeof(buf), 0);
			if (n > 0)
			{
				if (n < 4)
				{
					MUGGLE_LOG_ERROR("invalid message with number of bytes < 4");
					return 0;
				}

				uint32_t len = ntohl(*(uint32_t*)buf);
				if (len > UDP_MESSAGE_MAX_SIZE - 4)
				{
					MUGGLE_LOG_ERROR("length field in message too long");
					return 0;
				}

				on_timestr(ev, peer, buf + 4);
			}
			else if (n < 0)
			{
				if (MUGGLE_SOCKET_LAST_ERRNO == MUGGLE_SYS_ERRNO_INTR)
				{
					continue;
				}
				else if (MUGGLE_SOCKET_LAST_ERRNO == MUGGLE_SYS_ERRNO_WOULDBLOCK)
				{
					break;
				}

				return MUGGLE_SOCKET_EV_CLOSE_SOCKET;
			}
			else
			{
				return MUGGLE_SOCKET_EV_CLOSE_SOCKET;
			}
		}
	}
	else if (peer->peer_type == MUGGLE_SOCKET_PEER_TYPE_TCP_PEER)
	{
		muggle_bytes_buffer_t *bytes_buf = (muggle_bytes_buffer_t*)peer->data;
		while (1)
		{
			// find contiguous memory
			char *ptr = (char*)muggle_bytes_buffer_writer_fc(bytes_buf, bufsize);
			if (ptr == NULL)
			{
				MUGGLE_LOG_ERROR("bytes buffer full!");
				return -1;
			}

			int n = recv(peer->fd, ptr, bufsize, 0);
			if (n > 0)
			{
				if (!muggle_bytes_buffer_writer_move(bytes_buf, n))
				{
					MUGGLE_LOG_ERROR("bytes buffer inner error!");
					return MUGGLE_SOCKET_EV_CLOSE_SOCKET;
				}

				if (n < bufsize)
				{
					break;
				}
			}
			else if (n < 0)
			{
				if (MUGGLE_SOCKET_LAST_ERRNO == MUGGLE_SYS_ERRNO_INTR)
				{
					continue;
				}
				else if (MUGGLE_SOCKET_LAST_ERRNO == MUGGLE_SYS_ERRNO_WOULDBLOCK)
				{
					break;
				}

				return MUGGLE_SOCKET_EV_CLOSE_SOCKET;
			}
			else
			{
				return MUGGLE_SOCKET_EV_CLOSE_SOCKET;
			}
		}

		while (1)
		{
			uint32_t len;
			if (!muggle_bytes_buffer_fetch(bytes_buf, 4, &len))
			{
				// failed get length information
				break;
			}

			// in this example, time string is yyyy-mm-dd hh:MM:ss, 65536 is enough
			len = ntohl(len);
			if (len > UDP_MESSAGE_MAX_SIZE - 4)
			{
				MUGGLE_LOG_ERROR("time string too long, must something wrong");
				// shutdown and return 0, or only return -1 both is ok
				muggle_socket_shutdown(peer->fd, MUGGLE_SOCKET_SHUT_RDWR);
				return 0;
			}

			// get readable number of bytes
			int pkg_size = len + 4;
			int readable = muggle_bytes_buffer_readable(bytes_buf);
			if (readable < pkg_size)
			{
				// wait for more bytes from network
				break;
			}

			// parse package
			char *ptr = muggle_bytes_buffer_reader_fc(bytes_buf, pkg_size);
			if (ptr)
			{
				on_timestr(ev, peer, ptr + 4);
				if (!muggle_bytes_buffer_reader_move(bytes_buf, pkg_size))
				{
					MUGGLE_LOG_ERROR("failed buffer reader move");
					return MUGGLE_SOCKET_EV_CLOSE_SOCKET;
				}
			}
			else
			{
				// there have no contiguous for this package
				if (!muggle_bytes_buffer_read(bytes_buf, pkg_size, buf))
				{
					MUGGLE_LOG_ERROR("failed buffer read");
					return MUGGLE_SOCKET_EV_CLOSE_SOCKET;
				}
				on_timestr(ev, peer, buf + 4);
			}
		}
	}

	return 0;
}