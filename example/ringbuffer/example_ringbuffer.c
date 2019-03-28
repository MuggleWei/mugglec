#include "muggle/c/muggle_c.h"

#define WRITE_USE_SINGLE_THREAD 0x01
#define WRITE_USE_NEXT_COMMIT   0x02

typedef struct ThreadWithTask_tag
{
	MuggleThread tid;
	MuggleRingBuffer *ring_buf;
	uint64_t loop;
	uint64_t cnt_per_loop;
	int write_flag;
}ThreadWithTask;

typedef struct MissingPkg_tag
{
	int64_t idx;
	int64_t cursor;
}MissingPkg;

THREAD_ROUTINE_RETURN ProducerRoutine(void *args)
{
	ThreadWithTask *task = (ThreadWithTask*)args;

	uint64_t i = 0, j = 0, k = 0;
	for (i = 0; i < task->loop; ++i)
	{
		for (j = 0; j < task->cnt_per_loop; ++j)
		{
			switch (task->write_flag)
			{
			case WRITE_USE_SINGLE_THREAD | WRITE_USE_NEXT_COMMIT:
			{
				int64_t idx = MuggleRingBufferNextSingleThread(task->ring_buf);
				uint64_t *p = (uint64_t*)MuggleRingBufferGet(task->ring_buf, idx);
				*p = i * task->cnt_per_loop + j;
				MuggleRingBufferCommitSingleThread(task->ring_buf, idx);
			}break;
			case WRITE_USE_SINGLE_THREAD:
			{
				k = i * task->cnt_per_loop + j;
				MuggleRingBufferWriteSingleThread(task->ring_buf, &k, sizeof(uint64_t));
			}break;
			case WRITE_USE_NEXT_COMMIT:
			{
				int64_t idx = MuggleRingBufferNext(task->ring_buf);
				uint64_t *p = (uint64_t*)MuggleRingBufferGet(task->ring_buf, idx);
				*p = i * task->cnt_per_loop + j;
				MuggleRingBufferCommit(task->ring_buf, idx);
			}break;
			default:
			{
				k = i * task->cnt_per_loop + j;
				MuggleRingBufferWrite(task->ring_buf, &k, sizeof(uint64_t));
			}break;
			}
		}
		MSleep(1);
	}

	printf("producer exit...\n");

	return 0;
}

THREAD_ROUTINE_RETURN ConsumerRoutine(void *args)
{
	ThreadWithTask *task = (ThreadWithTask*)args;
	MuggleRingBuffer *ring_buf = task->ring_buf;
	uint64_t idx = 0, cnt = task->cnt_per_loop * task->loop;
	MissingPkg *missing_pkg_array = (MissingPkg*)malloc(sizeof(MissingPkg) * (1 + cnt / ring_buf->capacity));
	int64_t missing_idx = 0;
	while (true)
	{
		uint64_t *p = (uint64_t*)MuggleRingBufferSpinRead(ring_buf, idx);
		if (*p != idx)
		{
			// printf("oh!!! something wrong - %lld != %lld\n", (long long)idx, (long long)*p);
			missing_pkg_array[missing_idx].idx = idx;
			missing_pkg_array[missing_idx].cursor = *p;
			idx = *p;
			++missing_idx;
		}
		if (++idx == cnt)
		{
			break;
		}
	}

	printf("consumer exit...\n");

	for (int64_t i = 0; i < missing_idx; ++i)
	{
		printf("missing pkg: [%lld - %lld]\n", missing_pkg_array[i].idx, missing_pkg_array[i].cursor);
	}
	free(missing_pkg_array);

	return 0;
}

int main(int argc, char *argv[])
{
	ThreadWithTask *args = (ThreadWithTask*)malloc(sizeof(ThreadWithTask) * 2);
	MuggleRingBuffer *ring_buf = (MuggleRingBuffer*)malloc(sizeof(MuggleRingBuffer));
	MuggleRingBufferInit(ring_buf, 10000, sizeof(uint64_t));

	args[0].ring_buf = args[1].ring_buf = ring_buf;
	args[0].loop = args[1].loop = 200;
	args[0].cnt_per_loop = args[1].cnt_per_loop = 50000;
	args[0].write_flag = args[1].write_flag = WRITE_USE_SINGLE_THREAD | WRITE_USE_NEXT_COMMIT;

	MuggleThreadCreate(&args[0].tid, NULL, ConsumerRoutine, &args[0]);
	MuggleThreadCreate(&args[1].tid, NULL, ProducerRoutine, &args[1]);

	MuggleThreadWaitExit(&args[1].tid);
	MuggleThreadWaitExit(&args[0].tid);

	MuggleRingBufferDestory(ring_buf);

	return 0;
}