#include "muggle/c/muggle_c.h"
#include "muggle/c/sync/ma_ring.h"

#define TOTAL_CNT 16 * 1024

typedef struct {
	uint32_t u32;
} data_t;

static uint32_t s_u32 = 0;
void fn_msg(muggle_ma_ring_t *ring, void *data)
{
	MUGGLE_UNUSED(ring);
	MUGGLE_ASSERT(ring->tid != 0);

	data_t *p = (data_t *)data;
	if (p->u32 != s_u32) {
		LOG_ERROR("something wrong! p->u32=%u, s_u32=%u", p->u32, s_u32);
		MUGGLE_ASSERT(p->u32 == s_u32);
	}

	LOG_DEBUG("ma_ring backend recv: %u", p->u32);

	s_u32++;

	if (s_u32 == TOTAL_CNT) {
		LOG_INFO("reach the end");
	}
}

void fn_before_run()
{
	LOG_INFO("before ma_ring run");
}

int main()
{
	muggle_log_complicated_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_DEBUG,
								"logs/ma_ring.log");

	muggle_ma_ring_ctx_set_capacity(TOTAL_CNT / 4);
	muggle_ma_ring_ctx_set_data_size(1024 - 128);
	muggle_ma_ring_ctx_set_callback(fn_msg);
	muggle_ma_ring_ctx_set_before_run_callback(fn_before_run);
	muggle_ma_ring_backend_run();

	muggle_ma_ring_thread_ctx_init();

	muggle_ma_ring_t *ring = muggle_ma_ring_thread_ctx_get();
	for (int i = 0; i < TOTAL_CNT; i++) {
		data_t *data = (data_t *)muggle_ma_ring_alloc(ring);
		memset(data, 0, sizeof(*data));
		data->u32 = i;
		muggle_ma_ring_move(ring);
	}

	muggle_ma_ring_thread_ctx_cleanup();

	LOG_INFO("bye");

	return 0;
}
