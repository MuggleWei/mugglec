#ifndef NET_FOO_MSG_H_
#define NET_FOO_MSG_H_

#include "foo/foo_macro.h"

EXTERN_C_BEGIN

#pragma pack(push)
#pragma pack(1)

enum
{
	FOO_MSG_TYPE_NULL = 0,
	FOO_MSG_TYPE_REQ_LOGIN,
	FOO_MSG_TYPE_RSP_LOGIN,
	FOO_MSG_TYPE_PING,
	FOO_MSG_TYPE_PONG,
	FOO_MSG_TYPE_REQ_SUM,
	FOO_MSG_TYPE_RSP_SUM,
	FOO_MSG_TYPE_MAX
};

#define FOO_VARIABLE_LEN_MSG (-1)  //!< variable length message

#define FOO_HEAD_FLAG_LITTLE_ENDIAN 0x01

/**
 * @brief message header
 */
typedef struct foo_msg_header
{
	uint32_t msg_type;
	uint32_t body_len;
}foo_msg_header_t;

/**
 * @brief message - request login
 */
typedef struct foo_msg_req_login
{
	foo_msg_header_t header;

	uint32_t user_id;
	char     password[32];
} foo_msg_req_login_t;

/**
 * @brief message - response login
 */
typedef struct foo_msg_rsp_login
{
	foo_msg_header_t header;

	uint8_t login_result;
} foo_msg_rsp_login_t;

/**
 * @brief message - heartbeat ping
 */
typedef struct foo_msg_ping
{
	foo_msg_header_t header;

	uint64_t sec;
} foo_msg_ping_t;

/**
 * @brief message - heartbeat ping
 */
typedef struct foo_msg_pong
{
	foo_msg_header_t header;

	uint64_t sec;
} foo_msg_pong_t;

// max message request sum array length
#define MAX_MSG_REQ_SUM_LEN 16

/**
 * @brief message - request sum
 */
typedef struct foo_msg_req_sum
{
	foo_msg_header_t header;

	uint32_t req_id;
	uint32_t arr_len;
}foo_msg_req_sum_t;

/**
 * @brief message - response sum
 */
typedef struct foo_msg_rsp_sum
{
	foo_msg_header_t header;

	uint32_t req_id;
	int32_t  sum;
}foo_msg_rsp_sum_t;

#pragma pack(pop)

EXTERN_C_END

#endif /* ifndef NET_FOO_MSG_H_ */
