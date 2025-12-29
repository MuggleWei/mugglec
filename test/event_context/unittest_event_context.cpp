#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

TEST(event_context, init)
{
	int val = 5;
	int *p = &val;

	muggle_event_context_t ctx;
	muggle_ev_ctx_init(&ctx, MUGGLE_INVALID_EVENT_FD, p);

	ASSERT_EQ(muggle_ev_ctx_fd(&ctx), MUGGLE_INVALID_EVENT_FD);
	ASSERT_EQ(muggle_ev_ctx_data(&ctx), p);
	ASSERT_EQ(muggle_ev_ctx_ref_num(&ctx, muggle_memory_order_relaxed), 1);

	ASSERT_EQ(ctx.fd, MUGGLE_INVALID_EVENT_FD);
	ASSERT_EQ(ctx.flags & MUGGLE_EV_CTX_FLAG_CLOSED, 0);
	ASSERT_EQ(ctx.ref_cnt, 1);
	ASSERT_TRUE(ctx.data == p);
}

TEST(event_context, ref_cnt)
{
	muggle_event_context_t ctx;
	muggle_ev_ctx_init(&ctx, MUGGLE_INVALID_EVENT_FD, NULL);

	int n = 0;

	n = muggle_ev_ctx_ref_retain(&ctx);
	ASSERT_EQ(n, 2);
	ASSERT_EQ(muggle_ev_ctx_ref_num(&ctx, muggle_memory_order_relaxed), n);

	n = muggle_ev_ctx_ref_release(&ctx);
	ASSERT_EQ(n, 1);
	ASSERT_EQ(muggle_ev_ctx_ref_num(&ctx, muggle_memory_order_relaxed), n);

	n = muggle_ev_ctx_ref_release(&ctx);
	ASSERT_EQ(n, 0);
	ASSERT_EQ(muggle_ev_ctx_ref_num(&ctx, muggle_memory_order_relaxed), n);

	n = muggle_ev_ctx_ref_release(&ctx);
	ASSERT_EQ(n, -1);
	ASSERT_EQ(ctx.ref_cnt, 0);
	ASSERT_EQ(muggle_ev_ctx_ref_num(&ctx, muggle_memory_order_relaxed), 0);

	n = muggle_ev_ctx_ref_release(&ctx);
	ASSERT_EQ(n, -1);
	ASSERT_EQ(ctx.ref_cnt, 0);
	ASSERT_EQ(muggle_ev_ctx_ref_num(&ctx, muggle_memory_order_relaxed), 0);

	n = muggle_ev_ctx_ref_retain(&ctx);
	ASSERT_EQ(n, -1);
	ASSERT_EQ(ctx.ref_cnt, 0);
	ASSERT_EQ(muggle_ev_ctx_ref_num(&ctx, muggle_memory_order_relaxed), 0);
}

TEST(event_context, flags)
{
	muggle_event_context_t ctx;
	muggle_ev_ctx_init(&ctx, MUGGLE_INVALID_EVENT_FD, NULL);

	muggle_ev_ctx_init(&ctx, MUGGLE_INVALID_EVENT_FD, NULL);
	ASSERT_EQ(ctx.flags & MUGGLE_EV_CTX_FLAG_CLOSED, 0);
	muggle_ev_ctx_set_flag(&ctx, MUGGLE_EV_CTX_FLAG_CLOSED);
	ASSERT_TRUE(ctx.flags & MUGGLE_EV_CTX_FLAG_CLOSED);
}
