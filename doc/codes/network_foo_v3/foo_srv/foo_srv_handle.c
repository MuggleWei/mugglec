#include "foo_srv_handle.h"

void on_connect(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *listen_peer,
	muggle_socket_peer_t *peer)
{
	foo_on_connect(ev, listen_peer, peer);
}

void on_error(muggle_socket_event_t *ev, muggle_socket_peer_t *peer)
{
	foo_on_error(ev, peer);
}

void on_close(muggle_socket_event_t *ev, muggle_socket_peer_t *peer)
{
	foo_on_close(ev, peer);
}

void on_message(muggle_socket_event_t *ev, muggle_socket_peer_t *peer)
{
	foo_on_message(ev, peer);
}

void on_timer(muggle_socket_event_t *ev)
{
}

void on_msg_req_login(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer,
	void *msg)
{
	foo_msg_req_login_t *req = (foo_msg_req_login_t*)msg;
	MUGGLE_LOG_INFO(
		"recv request login message:"
		"user_id=%lu, passwd=%s",
		(unsigned long)req->user_id, req->password);

	// do something check use password
	
	foo_msg_rsp_login_t rsp;
	memset(&rsp, 0, sizeof(rsp));
	rsp.header.msg_type = FOO_MSG_TYPE_RSP_LOGIN;
	rsp.login_result = 1;

	foo_send(ev, peer, (void*)&rsp, sizeof(rsp));
}

void on_msg_req_sum(
	muggle_socket_event_t *ev,
	muggle_socket_peer_t *peer,
	void *msg)
{
	foo_socket_peer_data_t *peer_data = (foo_socket_peer_data_t*)peer->data;

	foo_msg_req_sum_t *req = (foo_msg_req_sum_t*)msg;
	MUGGLE_LOG_INFO(
		"recv request sum message: addr=%s, req_id=%lu",
		peer_data->straddr, (unsigned long)req->req_id);

	// put message to worker
	static int chan_idx = 0;
	chan_idx = (chan_idx+1) % NUM_WORKER;
	foo_ev_data_t *ev_data = ev->datas;
	muggle_channel_t *chan_arr = (muggle_channel_t*)ev_data->user_data;
	muggle_channel_t *chan = &chan_arr[chan_idx];

	worker_thread_msg_t *worker_msg = malloc(sizeof(worker_thread_msg_t));
	worker_msg->ev = ev;
	worker_msg->peer = peer;
	memcpy(&worker_msg->req, req, sizeof(foo_msg_req_sum_t));
	worker_msg->req.arr = malloc(req->arr_len * sizeof(int32_t));
	for (int i = 0; i < req->arr_len; i++)
	{
		worker_msg->req.arr[i] = req->arr[i];
	}

	// retain socket peer
	int ref_cnt = muggle_socket_peer_retain(peer);
	if (ref_cnt == 0)
	{
		MUGGLE_LOG_WARNING("failed retain peer");
		return;
	}
	MUGGLE_LOG_INFO("peer[%s] retain, reference count=%d", peer_data->straddr, ref_cnt);

	int ret = muggle_channel_write(chan, worker_msg);
	if (ret != 0)
	{
		MUGGLE_LOG_ERROR("failed send message to channel: idx=%d, ret=%d",
			chan_idx, ret);

		// free memory
		free(worker_msg->req.arr);
		free(worker_msg);

		// release socket peer
		int ref_cnt = muggle_socket_peer_release(worker_msg->peer);
		MUGGLE_LOG_INFO("callback thread, peer[%s] release, reference count=%d",
			peer_data->straddr, ref_cnt);
	}
}

muggle_thread_t worker_thread(void *args)
{
	worker_thread_args_t *p_args = (worker_thread_args_t*)args;
	muggle_channel_t *chan = p_args->chan;

	while (1)
	{
		worker_thread_msg_t *msg = (worker_thread_msg_t*)muggle_channel_read(chan);
		foo_msg_req_sum_t *req = &msg->req;
		foo_socket_peer_data_t *peer_data = msg->peer->data;

		int32_t sum = 0;
		for (uint32_t i = 0; i < req->arr_len; i++)
		{
			sum += req->arr[i];
		}

		foo_msg_rsp_sum_t rsp;
		memset(&rsp, 0, sizeof(rsp));
		rsp.header.msg_type = FOO_MSG_TYPE_RSP_SUM;
		rsp.sum = sum;
		rsp.req_id = req->req_id;

		foo_send(msg->ev, msg->peer, (void*)&rsp, sizeof(rsp));

		// release socket peer
		int ref_cnt = muggle_socket_peer_release(msg->peer);
		MUGGLE_LOG_INFO("worker thread[%d], peer[%s] release, reference count=%d",
			p_args->idx, peer_data->straddr, ref_cnt);

		// free memory
		free(msg->req.arr);
		free(msg);
	}
}