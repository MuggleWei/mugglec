#include "time_client_handle.h"

static void on_read(struct muggle_socket_event *ev, struct muggle_socket_peer *peer, muggle_bytes_buffer_t *bytes_buf)
{
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
		int pkg_size = 4 + (int)len;
		char tmp_buf[65536];
		if (pkg_size >= sizeof(tmp_buf))
		{
			MUGGLE_ERROR("time string too long, must something wrong");
			muggle_socket_shutdown(peer->fd, MUGGLE_SOCKET_SHUT_RDWR);
			return;
		}

		// get readable number of bytes
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
			MUGGLE_INFO("recv timestamp string: %s", (char*)(ptr + 4));
			muggle_bytes_buffer_reader_move(bytes_buf, pkg_size);
		}
		else
		{
			// there have no contiguous for this package
			muggle_bytes_buffer_read(bytes_buf, pkg_size, tmp_buf);
			MUGGLE_INFO("recv timestamp string: %s", (char*)(tmp_buf + 4));
		}
	}
}

int on_error(struct muggle_socket_event *ev, struct muggle_socket_peer *peer)
{
	// output disconnection address string
	char straddr[MUGGLE_SOCKET_ADDR_STRLEN];
	if (muggle_socket_ntop((struct sockaddr*)&peer->addr, straddr, MUGGLE_SOCKET_ADDR_STRLEN, 0) == NULL)
	{
		snprintf(straddr, MUGGLE_SOCKET_ADDR_STRLEN, "unknown:unknown");
	}
	MUGGLE_WARNING("disconnect - %s", straddr);

	muggle_bytes_buffer_t *bytes_buf = (muggle_bytes_buffer_t*)peer->data;
	muggle_bytes_buffer_destroy(bytes_buf);

	// exit event loop
	muggle_socket_event_loop_exit(ev);

	return 0;
}

int on_message(struct muggle_socket_event *ev, struct muggle_socket_peer *peer)
{
	int bufsize = 65536;
	muggle_bytes_buffer_t *bytes_buf = (muggle_bytes_buffer_t*)peer->data;
	char *buf = (char*)muggle_bytes_buffer_writer_fc(bytes_buf, bufsize);
	if (buf == NULL)
	{
		MUGGLE_ERROR("bytes buffer full!");
		return -1;
	}
	while (1)
	{
		int n = recv(peer->fd, buf, bufsize, 0);
		if (n > 0)
		{
			if (!muggle_bytes_buffer_writer_move(bytes_buf, n))
			{
				MUGGLE_ERROR("bytes buffer inner error!");
				return -1;
			}

			if (peer->peer_type == MUGGLE_SOCKET_PEER_TYPE_UDP_PEER)
			{
				continue;
			}
			else if (n < bufsize)
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

			return -1;
		}
		else
		{
			return -1;
		}
	}

	on_read(ev, peer, bytes_buf);

	return 0;
}