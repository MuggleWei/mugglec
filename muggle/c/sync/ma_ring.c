#include "ma_ring.h"
#include "muggle/c/base/err.h"
#include "muggle/c/base/sleep.h"
#include "muggle/c/base/utils.h"
#include <stdlib.h>
#include <string.h>

#define MUGGLE_MA_RING_CACHE_LINE 64
#define MUGGLE_MA_RING_CACHE_INTERVAL (2 * MUGGLE_MA_RING_CACHE_LINE)

#define MUGGLE_MA_RING_DEFAULT_CAPACITY 4 * 1024
#define MUGGLE_MA_RING_DEFAULT_BLOCK_SIZE 1024
#define MUGGLE_MA_RING_DEFAULT_DATA_SIZE \
	(MUGGLE_MA_RING_DEFAULT_BLOCK_SIZE - MUGGLE_MA_RING_CACHE_INTERVAL)

static void s_default_ma_ring_cb(muggle_ma_ring_t *ring, void *data)
{
	MUGGLE_UNUSED(ring);
	MUGGLE_UNUSED(data);
}

static void muggle_ma_ring_handle_new(muggle_ma_ring_context_t *ctx)
{
	muggle_spinlock_lock(&ctx->spinlock);

	while (ctx->add_list.next) {
		// remove from add list
		muggle_ma_ring_list_node_t *node = ctx->add_list.next;
		ctx->add_list.next = node->next;
		node->next = NULL;

		// add into ring list
		node->next = ctx->ring_list.next;
		ctx->ring_list.next = node;
	}

	muggle_spinlock_unlock(&ctx->spinlock);
}

static void muggle_ma_ring_handle_remove(muggle_ma_ring_context_t *ctx)
{
	muggle_ma_ring_list_node_t *prev = &ctx->ring_list;
	muggle_ma_ring_list_node_t *node = prev->next;
	while (node) {
		muggle_sync_t status = muggle_atomic_load(&node->ring->status,
												  muggle_memory_order_relaxed);
		if (status == MUGGLE_MA_RING_STATUS_WAIT_REMOVE) {
			muggle_atomic_store(&node->ring->status, MUGGLE_MA_RING_STATUS_DONE,
								muggle_memory_order_relaxed);
			prev->next = node->next;
			node->next = NULL;

			free(node);

			node = prev->next;
		} else {
			prev = node;
			node = node->next;
		}
	}
}

static int muggle_ma_ring_consume(muggle_ma_ring_context_t *ctx,
								  muggle_ma_ring_t *ring)
{
	int num_consume = 0;
	char *block = (char *)ring->buffer;

	muggle_sync_t w =
		muggle_atomic_load(&ring->wpos, muggle_memory_order_acquire);
	muggle_sync_t r = ring->rpos;
	while (r != w) {
		char *data = block + ctx->block_size * r;
		ctx->cb(ring, data);

		++r;
		if (r >= ctx->capacity) {
			r = 0;
		}

		++num_consume;
	}

	if (num_consume > 0) {
		muggle_atomic_store(&ring->rpos, r, muggle_memory_order_relaxed);
	}

	return num_consume;
}

#if MUGGLE_PLATFORM_WINDOWS
static muggle_thread_ret_t __stdcall s_muggle_ma_ring_backend_func(void *args)
#else
static muggle_thread_ret_t s_muggle_ma_ring_backend_func(void *args)
#endif
{
	MUGGLE_UNUSED(args);

	muggle_ma_ring_context_t *ctx = muggle_ma_ring_ctx_get();

	if (ctx->before_run_cb) {
		ctx->before_run_cb();
	}

	while (1) {
		int num_consume = 0;

		muggle_ma_ring_handle_new(ctx);

		muggle_ma_ring_list_node_t *node = ctx->ring_list.next;
		while (node) {
			num_consume += muggle_ma_ring_consume(ctx, node->ring);
			node = node->next;
		}

		muggle_ma_ring_handle_remove(ctx);

		// if no message readed, sleep this thread for a while
		if (num_consume == 0) {
			// muggle_ma_ring_thread_yield();
			muggle_nsleep(100);
		}
	}

	return 0;
}

void muggle_ma_ring_backend_run()
{
	muggle_thread_t th_backend;
	muggle_thread_create(&th_backend, s_muggle_ma_ring_backend_func, NULL);
	muggle_thread_detach(&th_backend);
}

muggle_ma_ring_context_t *muggle_ma_ring_ctx_get()
{
	static muggle_ma_ring_context_t s_ctx = {
		.spinlock = 0,
		.capacity = MUGGLE_MA_RING_DEFAULT_CAPACITY,
		.block_size = MUGGLE_MA_RING_DEFAULT_BLOCK_SIZE,
		.add_list = { .next = NULL, .ring = NULL },
		.ring_list = { .next = NULL, .ring = NULL },
		.cb = s_default_ma_ring_cb,
		.before_run_cb = NULL,
	};
	return &s_ctx;
}

void muggle_ma_ring_ctx_set_capacity(muggle_sync_t capacity)
{
	muggle_ma_ring_context_t *ctx = muggle_ma_ring_ctx_get();
	ctx->capacity = capacity;
}

void muggle_ma_ring_ctx_set_data_size(muggle_sync_t data_size)
{
	muggle_ma_ring_context_t *ctx = muggle_ma_ring_ctx_get();

	// align block_size to cacheline and add another 2 cacheline
	muggle_sync_t align_size =
		ROUND_UP_POW_OF_2_MUL(data_size, MUGGLE_CACHE_LINE_SIZE);
	ctx->block_size = align_size + MUGGLE_CACHE_LINE_X2_SIZE;
}

void muggle_ma_ring_ctx_set_callback(muggle_ma_ring_callback fn)
{
	muggle_ma_ring_context_t *ctx = muggle_ma_ring_ctx_get();
	ctx->cb = fn;
}

void muggle_ma_ring_ctx_set_before_run_callback(
	muggle_ma_ring_before_run_callback fn)
{
	muggle_ma_ring_context_t *ctx = muggle_ma_ring_ctx_get();
	ctx->before_run_cb = fn;
}

static muggle_thread_local muggle_ma_ring_t *s_muggle_ma_ring_thread_ctx = NULL;
muggle_ma_ring_t *muggle_ma_ring_thread_ctx_get()
{
	if (s_muggle_ma_ring_thread_ctx == NULL) {
		return muggle_ma_ring_thread_ctx_init();
	}
	return s_muggle_ma_ring_thread_ctx;
}

static int muggle_ma_ring_insert_thread_ctx(muggle_ma_ring_t *ring)
{
	muggle_ma_ring_context_t *ctx = muggle_ma_ring_ctx_get();

	muggle_ma_ring_list_node_t *node = (muggle_ma_ring_list_node_t *)malloc(
		sizeof(muggle_ma_ring_list_node_t));
	if (node == NULL) {
		return MUGGLE_ERR_MEM_ALLOC;
	}
	node->ring = ring;

	muggle_spinlock_lock(&ctx->spinlock);
	node->next = ctx->add_list.next;
	ctx->add_list.next = node;
	muggle_spinlock_unlock(&ctx->spinlock);

	return 0;
}

static void muggle_ma_ring_remove_thread_ctx(muggle_ma_ring_t *ring)
{
	muggle_atomic_store(&ring->status, MUGGLE_MA_RING_STATUS_WAIT_REMOVE,
						muggle_memory_order_relaxed);

	do {
		muggle_atomic_int status =
			muggle_atomic_load(&ring->status, muggle_memory_order_relaxed);
		if (status == MUGGLE_MA_RING_STATUS_DONE) {
			break;
		}
		muggle_msleep(1);
	} while (1);
}

static void muggle_ma_ring_join(muggle_ma_ring_t *ring)
{
	do {
		muggle_sync_t r =
			muggle_atomic_load(&ring->rpos, muggle_memory_order_relaxed);
		if (r == ring->wpos) {
			break;
		}
		muggle_msleep(1);
	} while (1);
}

muggle_ma_ring_t *muggle_ma_ring_thread_ctx_init()
{
	if (s_muggle_ma_ring_thread_ctx) {
		return s_muggle_ma_ring_thread_ctx;
	}

	s_muggle_ma_ring_thread_ctx =
		(muggle_ma_ring_t *)malloc(sizeof(muggle_ma_ring_t));
	if (s_muggle_ma_ring_thread_ctx == NULL) {
		return NULL;
	}
	memset(s_muggle_ma_ring_thread_ctx, 0,
		   sizeof(*s_muggle_ma_ring_thread_ctx));

	muggle_ma_ring_context_t *ctx = muggle_ma_ring_ctx_get();
#if MUGGLE_C_HAVE_ALIGNED_ALLOC
	s_muggle_ma_ring_thread_ctx->buffer =
		aligned_alloc(MUGGLE_CACHE_LINE_SIZE, ctx->capacity * ctx->block_size);
#else
	s_muggle_ma_ring_thread_ctx->buffer =
		malloc(ctx->capacity * ctx->block_size);
#endif
	if (s_muggle_ma_ring_thread_ctx->buffer == NULL) {
		muggle_ma_ring_thread_ctx_cleanup();
		return NULL;
	}

	s_muggle_ma_ring_thread_ctx->tid = muggle_thread_current_readable_id();
	s_muggle_ma_ring_thread_ctx->status = MUGGLE_MA_RING_STATUS_NORMAL;

	int ret = muggle_ma_ring_insert_thread_ctx(s_muggle_ma_ring_thread_ctx);
	if (ret != 0) {
		muggle_ma_ring_thread_ctx_cleanup();
		return NULL;
	}

	return s_muggle_ma_ring_thread_ctx;
}

void muggle_ma_ring_thread_ctx_cleanup()
{
	if (s_muggle_ma_ring_thread_ctx) {
		muggle_ma_ring_join(s_muggle_ma_ring_thread_ctx);

		muggle_ma_ring_remove_thread_ctx(s_muggle_ma_ring_thread_ctx);

		free(s_muggle_ma_ring_thread_ctx);
		s_muggle_ma_ring_thread_ctx = NULL;
	}
}

void *muggle_ma_ring_alloc(muggle_ma_ring_t *ring)
{
	muggle_ma_ring_context_t *ctx = muggle_ma_ring_ctx_get();
	return (char *)ring->buffer + ctx->block_size * ring->wpos;
}

void muggle_ma_ring_move(muggle_ma_ring_t *ring)
{
	static muggle_thread_local muggle_sync_t r = 0;
	muggle_ma_ring_context_t *ctx = muggle_ma_ring_ctx_get();
	muggle_sync_t next_w = ring->wpos + 1;
	if (next_w >= ctx->capacity) {
		next_w = 0;
	}

	while (next_w == r) {
		muggle_thread_yield();
		r = muggle_atomic_load(&ring->rpos, muggle_memory_order_relaxed);
	}

	muggle_atomic_store(&ring->wpos, next_w, muggle_memory_order_release);
}
