#include "muggle/c/muggle_c.h"

muggle_thread_ret_t thread_routine(void *args)
{
	// simulate do something
	LOG_INFO("start do something");

	muggle_msleep(1000);

	LOG_INFO("end do something");

	return 0;
}

void run_thread_detach()
{
	muggle_thread_t th;

	LOG_INFO("create thread and detach");
	muggle_thread_create(&th, thread_routine, NULL);
	muggle_thread_detach(&th);
	LOG_INFO("detach thread completed");
}

void run_thread_join()
{
	muggle_thread_t th;

	LOG_INFO("create thread and join");
	muggle_thread_create(&th, thread_routine, NULL);
	muggle_thread_join(&th);
	LOG_INFO("join thread completed");
}

int main()
{
	muggle_log_complicated_init(LOG_LEVEL_TRACE, -1, NULL);

	LOG_INFO("-----------------");
	run_thread_detach();
	muggle_msleep(2000);

	LOG_INFO("-----------------");
	run_thread_join();

	return 0;
}