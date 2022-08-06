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

int muggle_ev_ctx_init(muggle_event_context_t *ctx, muggle_event_fd fd, void *data)
{
	memset(ctx, 0, sizeof(*ctx));
	ctx->fd = fd;
	ctx->ref_cnt = 1;
	ctx->data = data;

	return 0;
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
	ctx->flags |= MUGGLE_EV_CTX_FLAG_CLOSED;
	return muggle_event_shutdown(ctx->fd, MUGGLE_EVENT_FD_SHUT_RDWR);
}

int muggle_ev_ctx_close(muggle_event_context_t *ctx)
{
	ctx->flags |= MUGGLE_EV_CTX_FLAG_CLOSED;

	int ret = muggle_event_close(ctx->fd);
	ctx->fd = MUGGLE_INVALID_EVENT_FD;
	return ret;
}
