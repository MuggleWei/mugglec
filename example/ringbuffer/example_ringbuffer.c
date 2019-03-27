#include "muggle/c/muggle_c.h"

typedef struct ThreadWithTask_tag
{
	MuggleThread tid;
	MuggleRingBuffer *ring_buf;
	uint64_t loop;
	uint64_t cnt_per_loop;
}ThreadWithTask;

THREAD_ROUTINE_RETURN ProducerRoutine(void *args)
{
	ThreadWithTask *task = (ThreadWithTask*)args;

	uint64_t i = 0, j = 0, k = 0;
	for (i = 0; i < task->loop; ++i)
	{
		for (j = 0; j < task->cnt_per_loop; ++j)
		{
			k = i * task->cnt_per_loop + j;
			MuggleRingBufferWrite(task->ring_buf, &k, sizeof(uint64_t));
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
	while (true)
	{
		uint64_t *p = (uint64_t*)MuggleRingBufferSpinRead(ring_buf, idx);
		if (*p != idx)
		{
			printf("oh!!! something wrong - %lld != %lld\n", (long long)idx, (long long)*p);
		}
		if (++idx == cnt)
		{
			break;
		}
	}

	printf("consumer exit...\n");

	return 0;
}

int main(int argc, char *argv[])
{
	ThreadWithTask *args = (ThreadWithTask*)malloc(sizeof(ThreadWithTask) * 2);
	MuggleRingBuffer *ring_buf = (MuggleRingBuffer*)malloc(sizeof(MuggleRingBuffer));
	MuggleRingBufferInit(ring_buf, 10000, sizeof(uint64_t));

	args[0].ring_buf = args[1].ring_buf = ring_buf;
	args[0].loop = args[1].loop = 100;
	args[0].cnt_per_loop = args[1].cnt_per_loop = 100000;

	MuggleThreadCreate(&args[0].tid, NULL, ConsumerRoutine, &args[0]);
	MuggleThreadCreate(&args[1].tid, NULL, ProducerRoutine, &args[1]);

	MuggleThreadWaitExit(&args[1].tid);
	MuggleThreadWaitExit(&args[0].tid);

	MuggleRingBufferDestory(ring_buf);

	return 0;
}