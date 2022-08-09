#include "socket_context.h"
#include <string.h>

int muggle_socket_ctx_init(
	muggle_socket_context_t *ctx,
	muggle_socket_t fd,
	void *data, int sock_type)
{
	memset(ctx, 0, sizeof(*ctx));
	int ret = muggle_ev_ctx_init((muggle_event_context_t*)ctx, fd, data);
	if (ret != 0)
	{
		return ret;
	}

	ctx->sock_type = sock_type;
	return ret;
}

int muggle_socket_ctx_type(muggle_socket_context_t *ctx)
{
	muggle_socket_context_t *sock_ev_ctx = (muggle_socket_context_t*)ctx;
	return sock_ev_ctx->sock_type;
}

int muggle_socket_ctx_recv(muggle_socket_context_t *ctx, void *buf, size_t len, int flags)
{
	return muggle_socket_ctx_recvfrom(ctx, buf, len, flags, NULL, NULL);
}

int muggle_socket_ctx_send(muggle_socket_context_t *ctx, void *buf, size_t len, int flags)
{
	return muggle_socket_ctx_sendto(ctx, buf, len, flags, NULL, 0);
}

int muggle_socket_ctx_recvfrom(
	muggle_socket_context_t *ctx, void *buf, size_t len, int flags,
	struct sockaddr *addr, muggle_socklen_t *addrlen)
{
	int n = 0;
	while (1)
	{
		n = muggle_socket_recvfrom(ctx->base.fd, buf, len, flags, addr, addrlen);
		if (n > 0)
		{
			break;
		}
		else
		{
			if (n < 0)
			{
				if (MUGGLE_EVENT_LAST_ERRNO == MUGGLE_SYS_ERRNO_WOULDBLOCK)
				{
					break;
				}
				else if (MUGGLE_EVENT_LAST_ERRNO == MUGGLE_SYS_ERRNO_INTR)
				{
					continue;
				}
#if MUGGLE_ENABLE_TRACE
				else
				{
					MUGGLE_LOG_SYS_ERR(MUGGLE_LOG_LEVEL_TRACE, "failed socket recv");
				}
#endif
			}

			// event fd closed(n == 0) or
			// error(n == -1 && errno != MUGGLE_SYS_ERRNO_WOULDBLOCK or MUGGLE_SYS_ERRNO_INTR)
			muggle_socket_ctx_set_flag(ctx, MUGGLE_EV_CTX_FLAG_CLOSED);
			break;
		}
	}

	return n;
}

int muggle_socket_ctx_sendto(
	muggle_socket_context_t *ctx, void *buf, size_t len, int flags,
	const struct sockaddr *dest_addr, socklen_t addrlen)
{
	int n = muggle_socket_sendto(ctx->base.fd, buf, len, flags, dest_addr, addrlen);
	if (n != (int)len)
	{
#if MUGGLE_ENABLE_TRACE
		if (n == MUGGLE_EVENT_ERROR)
		{
			MUGGLE_LOG_SYS_ERR(MUGGLE_LOG_LEVEL_TRACE, "failed socket send");
		}
		else
		{
			MUGGLE_LOG_TRACE("send buffer full");
		}
#endif
	}

	return n;
}
