#include "async_serv_handle.h"

static muggle_atomic_int s_task_id = 0;

struct async_args
{
	struct muggle_socket_peer *peer;
	int task_id;
};

muggle_thread_ret_t async_worker(void *arg)
{
	struct async_args *p = (struct async_args*)arg;
	struct muggle_socket_peer *peer = p->peer;
	int task_id = (int)p->task_id;
	free(arg);

	char str_addr[128];
	muggle_socket_ntop((struct sockaddr*)&peer->addr, str_addr, sizeof(str_addr), 0);
	MUGGLE_LOG_INFO("peer[%s] task[%d] start", str_addr, task_id);

	if (task_id % 3 == 0)
	{
		muggle_socket_peer_close(peer);
		MUGGLE_LOG_INFO("close peer[%s]", str_addr);
	}
	else
	{
		// do somthing
		muggle_msleep(3 * 1000);
		MUGGLE_LOG_INFO("peer[%s] task[%d] completed", str_addr, task_id);

		char buf[128];
		snprintf(buf, sizeof(buf), "task[%d] completed", task_id);
		muggle_socket_peer_send(peer, buf, strlen(buf), 0);
	}

	// release socket peer
	int ref_cnt = muggle_socket_peer_release(peer);
	MUGGLE_LOG_INFO("peer[%s] release, reference count=%d", str_addr, ref_cnt);

	return 0;
}

void on_error(struct muggle_socket_event *ev, struct muggle_socket_peer *peer)
{
	char str_addr[128];
	muggle_socket_ntop((struct sockaddr*)&peer->addr, str_addr, sizeof(str_addr), 0);
	MUGGLE_LOG_INFO("peer[%s] closed", str_addr);
}

void on_message(struct muggle_socket_event *ev, struct muggle_socket_peer *peer)
{
	char buf[4096];
	if (muggle_socket_peer_recv(peer, buf, sizeof(buf), 0) > 0)
	{
		// retain socket peer
		int ref_cnt = muggle_socket_peer_retain(peer);
		if (ref_cnt == 0)
		{
			MUGGLE_LOG_WARNING("failed retain peer");
			return;
		}

		char buf[128];
		muggle_socket_ntop((struct sockaddr*)&peer->addr, buf, sizeof(buf), 0);
		MUGGLE_LOG_INFO("peer[%s] retain, reference count=%d", buf, ref_cnt);

		struct async_args *args = (struct async_args*)malloc(sizeof(struct async_args));
		args->peer = peer;
		args->task_id = muggle_atomic_fetch_add(&s_task_id, 1, muggle_memory_order_relaxed);

		MUGGLE_LOG_INFO("peer[%s] task[%d] launch", buf, (int)args->task_id);

		muggle_thread_t thread;
		muggle_thread_create(&thread, async_worker, (void*)args);
		muggle_thread_detach(&thread);
	}
}