#include "socket_event_utils.h"

int muggle_socket_event_peer_on_message(muggle_socket_event_t *ev, muggle_socket_peer_t *peer)
{
	if (ev->on_message)
	{
		int ret = ev->on_message(ev, peer);
		if (ret == 0)
		{
			return 0;
		}
		else
		{
			if (ret == -1)
			{
				muggle_socket_close(peer->fd);
			}
			return 1;
		}
	}
	else
	{
		char buf[4096];
		int n;
		while (1)
		{
			n = recv(peer->fd, buf, sizeof(buf), 0);
			if (n > 0)
			{
				continue;
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

				muggle_socket_close(peer->fd);
				return 1;
			}
			else
			{
				muggle_socket_close(peer->fd);
				return 1;
			}
		}
	}

	return 0;
}
