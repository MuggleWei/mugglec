#include "codec_endian_msg.h"

void* enc_endian_req_login(void *data, uint32_t data_len)
{
	foo_msg_req_login_t *msg = (foo_msg_req_login_t*)data;
	msg->user_id = htonl(msg->user_id);
	return msg;
}
void* dec_endian_req_login(void *data, uint32_t data_len)
{
	foo_msg_req_login_t *msg = (foo_msg_req_login_t*)data;
	msg->user_id = ntohl(msg->user_id);
	return msg;
}

void* enc_endian_rsp_login(void *data, uint32_t data_len)
{
	return data;
}
void* dec_endian_rsp_login(void *data, uint32_t data_len)
{
	return data;
}

void* enc_endian_ping(void *data, uint32_t data_len)
{
	foo_msg_ping_t *msg = (foo_msg_ping_t*)data;
	if (muggle_endianness() == MUGGLE_LITTLE_ENDIAN)
	{
		MUGGLE_ENDIAN_SWAP_64(msg->sec);
	}
	return msg;
}
void* dec_endian_ping(void *data, uint32_t data_len)
{
	foo_msg_ping_t *msg = (foo_msg_ping_t*)data;
	if (muggle_endianness() == MUGGLE_LITTLE_ENDIAN)
	{
		MUGGLE_ENDIAN_SWAP_64(msg->sec);
	}
	return msg;
}

void* enc_endian_pong(void *data, uint32_t data_len)
{
	foo_msg_pong_t *msg = (foo_msg_pong_t*)data;
	if (muggle_endianness() == MUGGLE_LITTLE_ENDIAN)
	{
		MUGGLE_ENDIAN_SWAP_64(msg->sec);
	}
	return msg;
}
void* dec_endian_pong(void *data, uint32_t data_len)
{
	foo_msg_pong_t *msg = (foo_msg_pong_t*)data;
	if (muggle_endianness() == MUGGLE_LITTLE_ENDIAN)
	{
		MUGGLE_ENDIAN_SWAP_64(msg->sec);
	}
	return msg;
}

void* enc_endian_req_sum(void *data, uint32_t data_len)
{
	foo_msg_req_sum_t *msg = (foo_msg_req_sum_t*)data;

	uint32_t *arr = (uint32_t*)(msg + 1);
	for (uint32_t i = 0; i < msg->arr_len; i++)
	{
		arr[i] = htonl(arr[i]);
	}

	msg->req_id = htonl(msg->req_id);
	msg->arr_len = htonl(msg->arr_len);

	return msg;
}
void* dec_endian_req_sum(void *data, uint32_t data_len)
{
	foo_msg_req_sum_t *msg = (foo_msg_req_sum_t*)data;

	msg->req_id = ntohl(msg->req_id);
	msg->arr_len = ntohl(msg->arr_len);

	if (msg->arr_len > MAX_MSG_REQ_SUM_LEN)
	{
		LOG_WARNING("recv request sum length beyond the limit");
		return NULL;
	}

	uint32_t *arr = (uint32_t*)(msg + 1);
	for (uint32_t i = 0; i < msg->arr_len; i++)
	{
		arr[i] = ntohl(arr[i]);
	}

	return msg;
}

void* enc_endian_rsp_sum(void *data, uint32_t data_len)
{
	foo_msg_rsp_sum_t *msg = (foo_msg_rsp_sum_t*)data;

	msg->req_id = htonl(msg->req_id);
	msg->sum = htonl(msg->sum);

	return msg;
}
void* dec_endian_rsp_sum(void *data, uint32_t data_len)
{
	foo_msg_rsp_sum_t *msg = (foo_msg_rsp_sum_t*)data;

	msg->req_id = ntohl(msg->req_id);
	msg->sum = ntohl(msg->sum);

	return msg;
}
