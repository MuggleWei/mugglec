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
	muggle_socket_peer_t *cur_peer = NULL;
	muggle_atomic_int pos = 0;
	while (1)
	{
		struct message_block *block = (struct message_block*)muggle_ring_buffer_read(&ring, pos++);
		if (block == NULL)
		{
			MUGGLE_LOG_INFO("bye bye");
			exit(0);
		}
		
		switch (block->msg_type)
		{
		case MSG_TYPE_CONSOLE_INPUT:
		{
			struct message_text *msg_text = (struct message_text*)block;
			if (cur_peer)
			{
				muggle_socket_peer_send(cur_peer, msg_text->buf, strlen(msg_text->buf), 0);
			}
		}break;
		case MSG_TYPE_PEER_RECV:
		{
			struct message_text *msg_text = (struct message_text*)block;
			MUGGLE_LOG_INFO("recv: %s", msg_text->buf);
		}break;
		case MSG_TYPE_PEER_CONNECT:
		{
			struct message_peer_event *msg_peer_event = (struct message_peer_event*)block;
			MUGGLE_ASSERT(cur_peer == NULL);
			MUGGLE_ASSERT(msg_peer_event->peer != NULL);
			if (cur_peer != NULL)
			{
				MUGGLE_LOG_ERROR("cur_peer is not null!");
				if (msg_peer_event->peer)
				{
					muggle_socket_peer_release(msg_peer_event->peer);
				}
			}
			cur_peer = msg_peer_event->peer;

			char straddr[MUGGLE_SOCKET_ADDR_STRLEN];
			if (muggle_socket_ntop((struct sockaddr*)&cur_peer->addr, straddr, MUGGLE_SOCKET_ADDR_STRLEN, 0) == NULL)
			{
				snprintf(straddr, MUGGLE_SOCKET_ADDR_STRLEN, "unknown:unknown");
			}
			MUGGLE_LOG_INFO("%s connect %s", th_arg->socket_type, straddr);
		}break;
		case MSG_TYPE_PEER_DISCONNECT:
		{
			struct message_peer_event *msg_peer_event = (struct message_peer_event*)block;
			MUGGLE_ASSERT(cur_peer == msg_peer_event->peer);
			if (cur_peer && cur_peer == msg_peer_event->peer)
			{
				muggle_socket_peer_release(cur_peer);
				cur_peer = NULL;
			}
			else
			{
				MUGGLE_LOG_ERROR("cur_peer not equal input peer");
			}
		}break;
		}

		muggle_sowr_memory_pool_free(block);
	}
}

int main(int argc, char *argv[])
{
	// initialize log
	if (muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO) != 0)
	{
		MUGGLE_LOG_ERROR("failed init log");
		exit(EXIT_FAILURE);
	}

	if (argc != 4)
	{
		MUGGLE_LOG_ERROR("usage: %s <IP> <Port> <tcp|udp>", argv[0]);
		exit(EXIT_FAILURE);
	}

	// initialize socket library
	muggle_socket_lib_init();

	// connect to server
	struct client_thread_arg th_arg;
	th_arg.host = argv[1];
	th_arg.serv = argv[2];
	th_arg.socket_type = argv[3];

	// str client
	str_client(&th_arg);

	return 0;
}
