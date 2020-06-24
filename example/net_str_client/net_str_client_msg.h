#ifndef EXAMPLE_NET_STR_CLIENT_MSG_H_
#define EXAMPLE_NET_STR_CLIENT_MSG_H_

#include "muggle/c/muggle_c.h"

#define SND_RCV_BUF_SIZE 4096

enum
{
	MSG_TYPE_EXIT = 0,
	MSG_TYPE_CONSOLE_INPUT,
	MSG_TYPE_PEER_RECV,
	MSG_TYPE_PEER_CONNECT,
	MSG_TYPE_PEER_DISCONNECT,
};

struct message_block
{
	int msg_type;
};

struct message_text
{
	int msg_type;
	char buf[SND_RCV_BUF_SIZE + 1];
};

struct message_peer_event
{
	int msg_type;
	muggle_socket_peer_t *peer;
};


#endif