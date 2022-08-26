#include "net_str_client_console.h"

muggle_thread_ret_t thread_get_console_input(void *arg)
{
	// get message queue(ring buffer)
	muggle_ring_buffer_t *ring = (muggle_ring_buffer_t*)arg;

	// get message pool
	muggle_sowr_memory_pool_t sowr_pool;
	muggle_sowr_memory_pool_init(&sowr_pool, 16, sizeof(struct message_text));

	// get user input
	struct message_text *msg = (struct message_text*)muggle_sowr_memory_pool_alloc(&sowr_pool);
	while (fgets(msg->buf, SND_RCV_BUF_SIZE, stdin) != NULL)
	{
		msg->msg_type = MSG_TYPE_CONSOLE_INPUT;
		muggle_ring_buffer_write(ring, msg);
		msg = (struct message_text*)muggle_sowr_memory_pool_alloc(&sowr_pool);
		if (msg == NULL)
		{
			LOG_WARNING("failed allocate memory for console input message");
			break;
		}
	}

	// without this, the last message will not free
	if (msg != NULL)
	{
		msg->msg_type = MSG_TYPE_CONSOLE_INPUT;
		msg->buf[0] = '\0';
		muggle_ring_buffer_write(ring, msg);
	}

	// wait for all message consumed, destroy sowr pool
	while (!muggle_sowr_memory_pool_is_all_free(&sowr_pool))
	{
		muggle_msleep(100);
	}
	muggle_sowr_memory_pool_destroy(&sowr_pool);

	// notify worker thread to exit
	muggle_ring_buffer_write(ring, NULL);

	return 0;
}
