#include "net_str_client_msg.h"
#include "net_str_client_console.h"
#include "net_str_client_handle.h"

void str_client(struct client_thread_arg *th_arg)
{
	// create ring buffer as message queue
	muggle_ring_buffer_t ring;
	int capacity = 1024;
	int flag =
		MUGGLE_RING_BUFFER_FLAG_WRITE_LOCK |
		MUGGLE_RING_BUFFER_FLAG_SINGLE_READER;
	muggle_ring_buffer_init(&ring, capacity, flag);

	// create console input thread
	muggle_thread_t console_input_thread;
	muggle_thread_create(&console_input_thread, thread_get_console_input, (void*)&ring);
	muggle_thread_detach(&console_input_thread);

	// create socket client thread
	th_arg->ring = &ring;
	muggle_thread_t socket_client_thread;
	muggle_thread_create(&socket_client_thread, thread_socket_event, (void*)th_arg);
	muggle_thread_detach(&socket_client_thread);

	// run message queue loop
	muggle_socket_context_t *cur_ctx = NULL;
	muggle_atomic_int pos = 0;
	while (1)
	{
		struct message_block *block = (struct message_block*)muggle_ring_buffer_read(&ring, pos++);
		if (block == NULL)
		{
			LOG_INFO("bye bye");
			exit(0);
		}
		
		switch (block->msg_type)
		{
		case MSG_TYPE_CONSOLE_INPUT:
		{
			struct message_text *msg_text = (struct message_text*)block;
			if (cur_ctx)
			{
				muggle_socket_ctx_write(cur_ctx, msg_text->buf, strlen(msg_text->buf));
			}
		}break;
		case MSG_TYPE_SOCKET_RECV:
		{
			struct message_text *msg_text = (struct message_text*)block;
			LOG_INFO("socket recv: %s", msg_text->buf);
		}break;
		case MSG_TYPE_SOCKET_CONNECT:
		{
			struct message_socket_event *msg = (struct message_socket_event*)block;
			MUGGLE_ASSERT(cur_ctx == NULL);
			MUGGLE_ASSERT(msg->ctx != NULL);
			cur_ctx = msg ->ctx;

			char straddr[MUGGLE_SOCKET_ADDR_STRLEN];
			muggle_socket_remote_addr(cur_ctx->base.fd, straddr, sizeof(straddr), 0);
			LOG_INFO("set current socket context %s[%s]", th_arg->socket_type, straddr);
		}break;
		case MSG_TYPE_SOCKET_DISCONNECT:
		{
			struct message_socket_event *msg_peer_event = (struct message_socket_event*)block;
			MUGGLE_ASSERT(cur_ctx == msg_peer_event->ctx);
			if (cur_ctx && cur_ctx == msg_peer_event->ctx)
			{
				if (muggle_socket_ctx_ref_release(cur_ctx) == 0)
				{
					char straddr[MUGGLE_SOCKET_ADDR_STRLEN];
					muggle_socket_remote_addr(cur_ctx->base.fd, straddr, sizeof(straddr), 0);
					LOG_INFO("release socket context[%s] in work thread", straddr);

					muggle_socket_ctx_close(cur_ctx);
					free(cur_ctx);
				}
				cur_ctx = NULL;
			}
			else
			{
				LOG_ERROR("cur_peer not equal input peer");
			}
		}break;
		}

		muggle_sowr_memory_pool_free(block);
	}
}

int main(int argc, char *argv[])
{
	// initialize log
	if (muggle_log_simple_init(LOG_LEVEL_INFO, LOG_LEVEL_INFO) != 0)
	{
		LOG_ERROR("failed init log");
		exit(EXIT_FAILURE);
	}

	// initialize socket library
	muggle_socket_lib_init();

	if (argc != 4)
	{
		LOG_ERROR("usage: %s <IP> <Port> <tcp|udp>", argv[0]);
		exit(EXIT_FAILURE);
	}

	// run string client
	struct client_thread_arg th_arg;
	th_arg.host = argv[1];
	th_arg.serv = argv[2];
	th_arg.socket_type = argv[3];
	str_client(&th_arg);

	return 0;
}
