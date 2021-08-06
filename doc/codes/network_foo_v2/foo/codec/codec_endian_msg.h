#ifndef NET_FOO_CODEC_ENDIAN_MSG_H_
#define NET_FOO_CODEC_ENDIAN_MSG_H_

#include "foo/foo_macro.h"

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
void* enc_endian_req_sum(void *data, uint32_t data_len);
NET_FOO_EXPORT
void* dec_endian_req_sum(void *data, uint32_t data_len);

NET_FOO_EXPORT
void* enc_endian_rsp_sum(void *data, uint32_t data_len);
NET_FOO_EXPORT
void* dec_endian_rsp_sum(void *data, uint32_t data_len);

EXTERN_C_END

#endif /* ifndef NET_FOO_CODEC_ENDIAN_MSG_H_ */
