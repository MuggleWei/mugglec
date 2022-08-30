#ifndef FOO_ARGS_H_
#define FOO_ARGS_H_

#include "foo/foo_macro.h"

typedef struct sys_args
{
	int  app_type;
	char host[64];
	char port[16];
} sys_args_t;

#endif /* ifndef FOO_ARGS_H_ */
