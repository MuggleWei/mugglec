#ifndef EXAMPLE_ARGS_H_
#define EXAMPLE_ARGS_H_

#include "muggle/c/muggle_c.h"

typedef struct sys_args
{
	int action_type;
	char host[32];
	char port[16];
} sys_args_t;

int parse_sys_args(int argc, char **argv, sys_args_t *args);

#endif /* ifndef EXAMPLE_ARGS_H_ */
