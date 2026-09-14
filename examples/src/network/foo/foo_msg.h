#ifndef FOO_MESSAGE_H_
#define FOO_MESSAGE_H_

#include "foo_config.h"
#include "assert.h"

EXTERN_C_BEGIN

#pragma pack(push)
#pragma pack(1)

enum {
	FOO_MSG_ID_NULL = 0,
	FOO_MSG_ID_PING,
	FOO_MSG_ID_PONG,
	FOO_MSG_ID_REQ_LOGIN,
	FOO_MSG_ID_RSP_LOGIN,
	MAX_FOO_MSG_ID,
};

#define FOO_MSG_HDR_MAGIC_WORD "FOOF"

#define FOO_MSG_HDR_FLAG_VERSION 0
#define FOO_MSG_HDR_FLAG_RESERVED1 1
#define FOO_MSG_HDR_FLAG_RESERVED2 2
#define FOO_MSG_HDR_FLAG_RESERVED3 3

#define FOO_PROTOCOL_VERSION 1

typedef struct {
	char magic[4]; //!< magic word
	char flags[4]; //!< flags
	uint32_t msg_id; //!< message id
	uint32_t payload_len; //!< payload length (not include head and tail)
} foo_msg_hdr_t;

typedef struct {
	uint32_t checksum; //!< message checksum
	uint32_t reserved; //!< reserved
} foo_msg_tail_t;

#define FOO_NEW_STACK_MSG(msgid, msgstruct, var)                           \
	static_assert(sizeof(foo_msg_hdr_t) + sizeof(msgstruct) +              \
						  sizeof(foo_msg_tail_t) <=                        \
					  (1500 - 60 - 20),                                    \
				  "message's packet size > MTU");                          \
	char msg_placeholder_##var[sizeof(foo_msg_hdr_t) + sizeof(msgstruct) + \
							   sizeof(foo_msg_tail_t)];                    \
	memset(msg_placeholder_##var, 0, sizeof(msg_placeholder_##var));       \
	foo_msg_hdr_t *hdr_##var = (foo_msg_hdr_t *)msg_placeholder_##var;     \
	hdr_##var->msg_id = msgid;                                             \
	hdr_##var->payload_len = (uint32_t)sizeof(msgstruct);                  \
	msgstruct *var = (msgstruct *)(hdr_##var + 1);

#define FOO_SEND_MSG(session, var)                 \
	foo_handle_msg_encode_send(session, hdr_##var, \
							   sizeof(msg_placeholder_##var))

typedef struct {
	int32_t err_id;
	int32_t reserved;
	char err_msg[128];
} err_info_t;

/**
 * @brief heartbeat ping
 * @msg   FOO_MSG_ID_PING
 */
typedef struct {
	int64_t sec;
	uint32_t nsec;
} foo_msg_ping_t;

/**
 * @brief request login
 * @msg   FOO_MSG_ID_REQ_LOGIN
 */
typedef struct {
	uint32_t user_id;
	char password[32];
} foo_msg_req_login_t;

/**
 * @brief response login
 * @msg   FOO_MSG_ID_RSP_LOGIN
 */
typedef struct {
	uint32_t user_id;
	err_info_t err;
} foo_msg_rsp_login_t;

#pragma pack(pop)

EXTERN_C_END

#endif // !FOO_MESSAGE_H_
