#ifndef FOO_CODEC_ENDIAN_MSG_H_
#define FOO_CODEC_ENDIAN_MSG_H_

#include "foo/foo_macro.h"
#include "foo/foo_msg.h"

EXTERN_C_BEGIN

NET_FOO_EXPORT
void* enc_endian_req_login(void *data, uint32_t data_len);
NET_FOO_EXPORT
void* dec_endian_req_login(void *data, uint32_t data_len);

NET_FOO_EXPORT
void* enc_endian_rsp_login(void *data, uint32_t data_len);
NET_FOO_EXPORT
void* dec_endian_rsp_login(void *data, uint32_t data_len);

NET_FOO_EXPORT
void* enc_endian_ping(void *data, uint32_t data_len);
NET_FOO_EXPORT
void* dec_endian_ping(void *data, uint32_t data_len);

NET_FOO_EXPORT
void* enc_endian_pong(void *data, uint32_t data_len);
NET_FOO_EXPORT
void* dec_endian_pong(void *data, uint32_t data_len);

NET_FOO_EXPORT
void* enc_endian_req_sum(void *data, uint32_t data_len);
NET_FOO_EXPORT
void* dec_endian_req_sum(void *data, uint32_t data_len);

NET_FOO_EXPORT
void* enc_endian_rsp_sum(void *data, uint32_t data_len);
NET_FOO_EXPORT
void* dec_endian_rsp_sum(void *data, uint32_t data_len);

EXTERN_C_END

#endif /* ifndef FOO_CODEC_ENDIAN_MSG_H_ */
