/******************************************************************************
 *  @file         event_context.c
 *  @author       Muggle Wei
 *  @email        mugglewei@gmail.com
 *  @date         2022-08-06
 *  @copyright    Copyright 2022 Muggle Wei
 *  @license      MIT License
 *  @brief        mugglec event context
 *****************************************************************************/

#include "event_context.h"
#include <string.h>
#include "muggle/c/log/log.h"
#include "muggle/c/os/sys.h"

int muggle_ev_ctx_init(muggle_event_context_t *ctx, muggle_event_fd fd, void *data)
{
	memset(ctx, 0, sizeof(*ctx));
	ctx->fd = fd;
	ctx->ref_cnt = 1;
	ctx->data = data;

	return 0;
}

muggle_event_fd muggle_ev_ctx_fd(muggle_event_context_t *ctx)
{
	return ctx->fd;
}

void* muggle_ev_ctx_get_data(muggle_event_context_t *ctx)
{
	return ctx->data;
}

void muggle_ev_ctx_set_flag(muggle_event_context_t *ctx, int flag)
{
	ctx->flags |= flag;
}

int muggle_ev_ctx_ref_retain(muggle_event_context_t *ctx)
{
	muggle_atomic_int ref_cnt = 0, desired = 0;
	do {
		ref_cnt = ctx->ref_cnt;
		if (ref_cnt == 0)
		{
			// try to retain released ctx
			return -1;
		}
		desired = ref_cnt + 1;
	} while (!muggle_atomic_cmp_exch_weak(&ctx->ref_cnt, &ref_cnt, desired, muggle_memory_order_relaxed));

	return desired;
}

int muggle_ev_ctx_ref_release(muggle_event_context_t *ctx)
{
	muggle_atomic_int ref_cnt = 0, desired = 0;
	do {
		ref_cnt = ctx->ref_cnt;
		if (ref_cnt == 0)
		{
			// repeated release error
			return -1;
		}
		desired = ref_cnt - 1;
	} while (!muggle_atomic_cmp_exch_weak(&ctx->ref_cnt, &ref_cnt, desired, muggle_memory_order_relaxed));

	return desired;
}

int muggle_ev_ctx_shutdown(muggle_event_context_t *ctx)
{
	muggle_ev_ctx_set_flag(ctx, MUGGLE_EV_CTX_FLAG_CLOSED);

	return muggle_ev_fd_shutdown(ctx->fd, MUGGLE_EVENT_FD_SHUT_RDWR);
}

int muggle_ev_ctx_close(muggle_event_context_t *ctx)
{
	muggle_ev_ctx_set_flag(ctx, MUGGLE_EV_CTX_FLAG_CLOSED);

	int ret = muggle_ev_fd_close(ctx->fd);
	ctx->fd = MUGGLE_INVALID_EVENT_FD;
	return ret;
}

int muggle_ev_ctx_read(muggle_event_context_t *ctx, void *buf, size_t len)
{
	int n = 0;
	while (1)
	{
		n = muggle_ev_fd_read(ctx->fd, buf, len);
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
					MUGGLE_LOG_SYS_ERR(MUGGLE_LOG_LEVEL_TRACE, "failed ev_fd_recv");
				}
#endif
			}

			// event fd closed(n == 0) or
			// error(n == -1 && errno != MUGGLE_SYS_ERRNO_WOULDBLOCK or MUGGLE_SYS_ERRNO_INTR)
			muggle_ev_ctx_set_flag(ctx, MUGGLE_EV_CTX_FLAG_CLOSED);
			break;
		}
	}

	return n;
}

int muggle_ev_ctx_write(muggle_event_context_t *ctx, void *buf, size_t len)
{
	int n = muggle_ev_fd_write(ctx->fd, buf, len);
	if (n != (int)len)
	{
#if MUGGLE_ENABLE_TRACE
		if (n == MUGGLE_EVENT_ERROR)
		{
			MUGGLE_LOG_SYS_ERR(MUGGLE_LOG_LEVEL_TRACE, "failed ev_fd_send");
		}
		else
		{
			MUGGLE_LOG_TRACE("send buffer full");
		}
#endif
	}

	return n;
}

