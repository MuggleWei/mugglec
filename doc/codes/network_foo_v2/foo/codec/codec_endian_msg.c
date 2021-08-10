#include "codec_endian_msg.h"
#include "foo/foo_msg.h"

void* enc_endian_req_login(void *data, uint32_t data_len)
{
	foo_msg_req_login_t *req = (foo_msg_req_login_t*)data;
	req->user_id = htonl(req->user_id);

	return req;
}
void* dec_endian_req_login(void *data, uint32_t data_len)
{
	foo_msg_req_login_t *req = (foo_msg_req_login_t*)data;
	req->user_id = ntohl(req->user_id);

	return req;
}

void* enc_endian_rsp_login(void *data, uint32_t data_len)
{
	return data;
}
void* dec_endian_rsp_login(void *data, uint32_t data_len)
{
	return data;
}

void* enc_endian_req_sum(void *data, uint32_t data_len)
{
	foo_msg_req_sum_t *req = (foo_msg_req_sum_t*)data;
	req->arr_len = htonl(req->arr_len);
	req->req_id = htonl(req->req_id);
	req->arr = (int32_t*)htonl((int32_t)(sizeof(foo_msg_req_sum_t)));

	return req;
}
void* dec_endian_req_sum(void *data, uint32_t data_len)
{
	foo_msg_req_sum_t *req = (foo_msg_req_sum_t*)data;
	req->arr_len = ntohl(req->arr_len);
	req->req_id = ntohl(req->req_id);
	ptrdiff_t diff = (ptrdiff_t)ntohl((uint32_t)req->arr);
	req->arr = (int32_t*)((intptr_t)data + diff);

	return req;
}

void* enc_endian_rsp_sum(void *data, uint32_t data_len)
{
	foo_msg_rsp_sum_t *rsp = (foo_msg_rsp_sum_t*)data;
	rsp->sum = htonl(rsp->sum);

	return rsp;
}
void* dec_endian_rsp_sum(void *data, uint32_t data_len)
{
	foo_msg_rsp_sum_t *rsp = (foo_msg_rsp_sum_t*)data;
	rsp->sum = ntohl(rsp->sum);

	return rsp;
}