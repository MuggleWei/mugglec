#include "net_str_client_handle.h"

struct thread_connect_args
{
	const char *host;
	const char *serv;
	const char *socket_type;
	muggle_event_loop_t *evloop;
};

struct client_event_data
{
	muggle_ring_buffer_t       *ring;                   //!< message ring buffer
	muggle_sowr_memory_pool_t  *socket_event_sowr_pool; //!< socket context memory pool
	muggle_sowr_memory_pool_t  *text_sowr_pool;         //!< string message memory pool
	struct thread_connect_args *conn_args;              //!< connect arguments
};

muggle_thread_ret_t thread_connect(void *arg)
{
	struct thread_connect_args *th_arg = (struct thread_connect_args*)arg;

	muggle_socket_t fd = MUGGLE_INVALID_SOCKET;
	muggle_socket_context_t *ctx = NULL;
	while (1)
	{
		if (strcmp(th_arg->socket_type, "tcp") == 0)
		{
			fd = muggle_tcp_connect(th_arg->host, th_arg->serv, 3);
			if (fd != MUGGLE_INVALID_SOCKET)
			{
				ctx = (muggle_socket_context_t*)malloc(sizeof(muggle_socket_context_t));
				muggle_socket_ctx_init(ctx, fd, NULL, MUGGLE_SOCKET_CTX_TYPE_TCP_CLIENT);
			}
		}
		else if (strcmp(th_arg->socket_type, "udp") == 0)
		{
			fd = muggle_udp_connect(th_arg->host, th_arg->serv);
			if (fd != MUGGLE_INVALID_SOCKET)
			{
				ctx = (muggle_socket_context_t*)malloc(sizeof(muggle_socket_context_t));
				muggle_socket_ctx_init(ctx, fd, NULL, MUGGLE_SOCKET_CTX_TYPE_UDP);
			}
		}
		else
		{
			LOG_ERROR("invalid socket context type: %s", th_arg->socket_type);
			exit(EXIT_FAILURE);
		}

		if (ctx == NULL)
		{
			LOG_ERROR("%s failed connect: %s:%s", th_arg->socket_type, th_arg->host, th_arg->serv);
			muggle_msleep(3000);
			LOG_INFO("reconnect...");
			continue;
		}

		muggle_socket_evloop_add_ctx(th_arg->evloop, ctx);
		break;
	}

	return 0;
}

void on_add_ctx(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	struct client_event_data *ev_data = (struct client_event_data*)muggle_evloop_get_data(evloop);
	muggle_ring_buffer_t *ring = (muggle_ring_buffer_t*)ev_data->ring;
	muggle_sowr_memory_pool_t *socket_event_sowr_pool = ev_data->socket_event_sowr_pool;

	// retain peer
	muggle_socket_ctx_ref_retain(ctx);
	struct message_socket_event *msg = (struct message_socket_event*)muggle_sowr_memory_pool_alloc(socket_event_sowr_pool);
	msg->msg_type = MSG_TYPE_SOCKET_CONNECT;
	msg->ctx = ctx;
	muggle_ring_buffer_write(ring, msg);
}

void on_message(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	struct client_event_data *ev_data = (struct client_event_data*)muggle_evloop_get_data(evloop);
	muggle_ring_buffer_t *ring = (muggle_ring_buffer_t*)ev_data->ring;
	while (1)
	{
		struct message_text *msg = (struct message_text*)muggle_sowr_memory_pool_alloc(ev_data->text_sowr_pool);
		msg->msg_type = MSG_TYPE_SOCKET_RECV;
		int n = muggle_socket_ctx_read(ctx, msg->buf, sizeof(msg->buf) - 1);
		if (n > 0)
		{
			msg->buf[n] = '\0';
			muggle_ring_buffer_write(ring, msg);
		}
		else
		{
			// don't free message in this thread, let message override automatically
			// muggle_sowr_memory_pool_free(msg);
			break;
		}
	}
}

void on_close(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	struct client_event_data *ev_data = (struct client_event_data*)muggle_evloop_get_data(evloop);
	muggle_ring_buffer_t *ring = (muggle_ring_buffer_t*)ev_data->ring;
	struct thread_connect_args *conn_args = ev_data->conn_args;

	// write disconnect message
	muggle_sowr_memory_pool_t *socket_event_sowr_pool = ev_data->socket_event_sowr_pool;
	struct message_socket_event *msg = (struct message_socket_event*)muggle_sowr_memory_pool_alloc(socket_event_sowr_pool);
	msg->msg_type = MSG_TYPE_SOCKET_DISCONNECT;
	msg->ctx = ctx;
	muggle_ring_buffer_write(ring, msg);

	// run reconnect thread
	muggle_thread_t th;
	muggle_thread_create(&th, thread_connect, conn_args);
	muggle_thread_detach(&th);
}

muggle_thread_ret_t thread_socket_event(void *arg)
{
	struct client_thread_arg *th_arg = (struct client_thread_arg*)arg;

	// get message queue(ring buffer)
	muggle_ring_buffer_t *ring = (muggle_ring_buffer_t*)th_arg->ring;

	// get message pool
	muggle_sowr_memory_pool_t socket_event_sowr_pool;
	muggle_sowr_memory_pool_init(&socket_event_sowr_pool, 16, sizeof(struct message_socket_event));

	muggle_sowr_memory_pool_t text_sowr_pool;
	muggle_sowr_memory_pool_init(&text_sowr_pool, 16, sizeof(struct message_text));

	// init event loop
	muggle_event_loop_init_args_t ev_init_args;
	memset(&ev_init_args, 0, sizeof(ev_init_args));
	ev_init_args.evloop_type = MUGGLE_EVLOOP_TYPE_NULL;
	ev_init_args.hints_max_fd = 32;
	ev_init_args.use_mem_pool = 0;

	muggle_event_loop_t *evloop = muggle_evloop_new(&ev_init_args);
	if (evloop == NULL)
	{
		LOG_ERROR("failed new event loop");
		exit(EXIT_FAILURE);
	}
	LOG_INFO("success new event loop");

	// init socket event loop handle
	muggle_socket_evloop_handle_t handle;
	muggle_socket_evloop_handle_init(&handle);
	muggle_socket_evloop_handle_set_cb_add_ctx(&handle, on_add_ctx);
	muggle_socket_evloop_handle_set_cb_msg(&handle, on_message);
	muggle_socket_evloop_handle_set_cb_close(&handle, on_close);
	muggle_socket_evloop_handle_attach(&handle, evloop);
	LOG_INFO("socket handle attached event loop");

	// event data
	struct thread_connect_args conn_args;
	conn_args.host = th_arg->host;
	conn_args.serv = th_arg->serv;
	conn_args.socket_type = th_arg->socket_type;
	conn_args.evloop = evloop;

	struct client_event_data ev_data;
	ev_data.ring = ring;
	ev_data.socket_event_sowr_pool = &socket_event_sowr_pool;
	ev_data.text_sowr_pool = &text_sowr_pool;
	ev_data.conn_args = &conn_args;

	muggle_evloop_set_data(evloop, &ev_data);

	// run connect thread
	muggle_thread_t th;
	muggle_thread_create(&th, thread_connect, &conn_args);
	muggle_thread_detach(&th);

	// run event loop
	muggle_evloop_run(evloop);

	// destroy socket event handle
	muggle_socket_evloop_handle_destroy(&handle);

	// delete event loop
	muggle_evloop_delete(evloop);

	// wait for all message consumed, destroy sowr pool
	while (!muggle_sowr_memory_pool_is_all_free(&socket_event_sowr_pool))
	{
		muggle_msleep(100);
	}
	muggle_sowr_memory_pool_destroy(&socket_event_sowr_pool);

	while (!muggle_sowr_memory_pool_is_all_free(&text_sowr_pool))
	{
		muggle_msleep(100);
	}
	muggle_sowr_memory_pool_destroy(&text_sowr_pool);
}
