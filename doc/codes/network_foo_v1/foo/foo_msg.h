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
	FOO_MSG_TYPE_REQ_SUM,
	FOO_MSG_TYPE_RSP_SUM,
	FOO_MSG_TYPE_MAX
};

#define FOO_VARIABLE_LEN_MSG (-1)  //!< variable length message

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
}foo_msg_req_login_t;

/**
 * @brief message - response login
 */
typedef struct foo_msg_rsp_login
{
	foo_msg_header_t header;

	uint8_t login_result;
}foo_msg_rsp_login_t;

/**
 * @brief message - request sum
 */
typedef struct foo_msg_req_sum
{
	foo_msg_header_t header;

	uint32_t         arr_len;
	int32_t          *arr;
}foo_msg_req_sum_t;

/**
 * @brief message - response sum
 */
typedef struct foo_msg_rsp_sum
{
	foo_msg_header_t header;

	int32_t          sum;
}foo_msg_rsp_sum_t;

#pragma pack(pop)

EXTERN_C_END

#endif /* ifndef NET_FOO_MSG_H_ */