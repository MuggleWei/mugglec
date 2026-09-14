#ifndef FOO_CONFIG_H_
#define FOO_CONFIG_H_

#include "muggle/c/muggle_c.h"

EXTERN_C_BEGIN

typedef struct {
	char host[64];
	char port[16];
	char bind_host[64];
	char bind_port[16];
	char app_type[16];

	int64_t evloop_timer_interval_ms; //!< event loop timer interval
	int64_t logic_timer_interval_ms; //!< logic timer interval
	uint32_t n_time_wheel_slot; //!< time wheel slot
	int64_t read_idle_ms; //!< read idle milli-seconds
	int64_t write_idle_ms; //!< write idle milli-seconds
} foo_config_t;

bool parse_sys_args(int argc, char **argv, foo_config_t *cfg);

EXTERN_C_END

#endif // !FOO_CONFIG_H_
