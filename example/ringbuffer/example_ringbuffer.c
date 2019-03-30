#include "muggle/c/muggle_c.h"

#define WRITE_USE_SINGLE_THREAD 0x01
#define WRITE_USE_NEXT_COMMIT   0x02

typedef struct ThreadWithTask_tag
{
	MuggleThread tid;
	MuggleRingBuffer *ring_buf;
	int loop;
	int cnt_per_loop;
	int write_flag;
}ThreadWithTask;

typedef struct MissingPkg_tag
{
	int idx;
	int cursor;
}MissingPkg;

THREAD_ROUTINE_RETURN ProducerRoutine(void *args)
{
	ThreadWithTask *task = (ThreadWithTask*)args;

	int i = 0, j = 0, k = 0;
	for (i = 0; i < task->loop; ++i)
	{
		for (j = 0; j < task->cnt_per_loop; ++j)
		{
			switch (task->write_flag)
			{
			case WRITE_USE_SINGLE_THREAD | WRITE_USE_NEXT_COMMIT:
			{
				int idx = MuggleRingBufferNextSingleThread(task->ring_buf);
				int *p = (int*)MuggleRingBufferGet(task->ring_buf, idx);
				*p = i * task->cnt_per_loop + j;
				MuggleRingBufferCommitSingleThread(task->ring_buf, idx);
			}break;
			case WRITE_USE_SINGLE_THREAD:
			{
				k = i * task->cnt_per_loop + j;
				MuggleRingBufferWriteSingleThread(task->ring_buf, &k, sizeof(int));
			}break;
			case WRITE_USE_NEXT_COMMIT:
			{
				int idx = MuggleRingBufferNext(task->ring_buf);
				int *p = (int*)MuggleRingBufferGet(task->ring_buf, idx);
				*p = i * task->cnt_per_loop + j;
				MuggleRingBufferCommit(task->ring_buf, idx);
			}break;
			default:
			{
				k = i * task->cnt_per_loop + j;
				MuggleRingBufferWrite(task->ring_buf, &k, sizeof(int));
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
	int idx = 0, cnt = task->cnt_per_loop * task->loop;
	MissingPkg *missing_pkg_array = (MissingPkg*)malloc(sizeof(MissingPkg) * (1 + cnt / ring_buf->capacity));
	int missing_idx = 0;
	while (true)
	{
		int *p = (int*)MuggleRingBufferRead(ring_buf, idx);
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

	for (int i = 0; i < missing_idx; ++i)
	{
		printf("missing pkg: [%d - %d]\n", missing_pkg_array[i].idx, missing_pkg_array[i].cursor);
	}
	free(missing_pkg_array);

	return 0;
}

int main(int argc, char *argv[])
{
	ThreadWithTask *args = (ThreadWithTask*)malloc(sizeof(ThreadWithTask) * 2);
	MuggleRingBuffer *ring_buf = (MuggleRingBuffer*)malloc(sizeof(MuggleRingBuffer));
	MuggleRingBufferInit(ring_buf, 500000, sizeof(int));

	args[0].ring_buf = args[1].ring_buf = ring_buf;
	args[0].loop = args[1].loop = 20;
	args[0].cnt_per_loop = args[1].cnt_per_loop = 500000;
	args[0].write_flag = args[1].write_flag = WRITE_USE_SINGLE_THREAD;

	MuggleThreadCreate(&args[0].tid, NULL, ConsumerRoutine, &args[0]);
	MuggleThreadCreate(&args[1].tid, NULL, ProducerRoutine, &args[1]);

	MuggleThreadWaitExit(&args[1].tid);
	MuggleThreadWaitExit(&args[0].tid);

	MuggleRingBufferDestory(ring_buf);

	return 0;
}
