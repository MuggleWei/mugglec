#ifndef EXAMPLE_ARGS_H_
#define EXAMPLE_ARGS_H_

#include "muggle/c/muggle_c.h"

enum
{
	ACTION_NULL = 0,
	ACTION_TCP_SERVER,
	ACTION_TCP_CLIENT,
	ACTION_UDP_RECV,
	ACTION_UDP_SEND,
};

typedef struct evloop_user_data
{
	muggle_event_loop_t *evloop; //!< event loop
	void                *data;   //!< data
} evloop_user_data_t;

typedef struct sys_args
{
	int action_type;
	char host[32];
	char port[16];
} sys_args_t;

int parse_sys_args(int argc, char **argv, sys_args_t *args);

#endif /* ifndef EXAMPLE_ARGS_H_ */
