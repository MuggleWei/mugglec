#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

TEST(event_context, init)
{
	muggle_event_context_t ctx;
	muggle_ev_ctx_init(&ctx, MUGGLE_INVALID_EVENT_FD, NULL);

	ASSERT_EQ(ctx.fd, MUGGLE_INVALID_EVENT_FD);
	ASSERT_EQ(ctx.flags & MUGGLE_EV_CTX_FLAG_CLOSED, 0);
	ASSERT_EQ(ctx.ref_cnt, 1);
	ASSERT_TRUE(ctx.data == NULL);
}

TEST(event_context, ref_cnt)
{
	muggle_event_context_t ctx;
	muggle_ev_ctx_init(&ctx, MUGGLE_INVALID_EVENT_FD, NULL);

	int n = 0;

	n = muggle_ev_ctx_ref_retain(&ctx);
	ASSERT_EQ(n, 2);

	n = muggle_ev_ctx_ref_release(&ctx);
	ASSERT_EQ(n, 1);

	n = muggle_ev_ctx_ref_release(&ctx);
	ASSERT_EQ(n, 0);

	n = muggle_ev_ctx_ref_release(&ctx);
	ASSERT_EQ(n, -1);
	ASSERT_EQ(ctx.ref_cnt, 0);

	n = muggle_ev_ctx_ref_release(&ctx);
	ASSERT_EQ(n, -1);
	ASSERT_EQ(ctx.ref_cnt, 0);

	n = muggle_ev_ctx_ref_retain(&ctx);
	ASSERT_EQ(n, -1);
	ASSERT_EQ(ctx.ref_cnt, 0);
}
