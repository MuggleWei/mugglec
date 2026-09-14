#include "foo_config.h"

enum {
	OPT_START = 1000,
	OPT_CLI_BIND_HOST,
	OPT_CLI_BIND_PORT,
};

bool parse_sys_args(int argc, char **argv, foo_config_t *cfg)
{
	const char *str_usage =
		"Usage: %s <options>\n"
		"    -h, --help         show man info\n"
		"    -v, --version      show version\n"
		"    -H, --host         server listen or client connect host\n"
		"    -P, --port         server listen or client connect port\n"
		"      , --bind.host    client bind host\n"
		"      , --bind.port    client bind port\n"
		"    -t, --type         app type [server|client]\n"
		"\ne.g.\n"
		"  %s -H 127.0.0.1 -P 10102 -t server\n"
		"  %s -H 127.0.0.1 -P 10102 --bind.host=127.0.0.1 --bind.port=10101 "
		"-t client\n";

	memset(cfg, 0, sizeof(foo_config_t));

	// set default value
	strncpy(cfg->host, "127.0.0.1", sizeof(cfg->host) - 1);
	strncpy(cfg->port, "10102", sizeof(cfg->port) - 1);
	strncpy(cfg->app_type, "unknown", sizeof(cfg->app_type) - 1);

	cfg->evloop_timer_interval_ms = 50;
	cfg->logic_timer_interval_ms = 100;
	cfg->n_time_wheel_slot = 50;
	cfg->read_idle_ms = 30 * 1000;
	cfg->write_idle_ms = 10 * 1000;

	int c = 0;
	while (true) {
		int option_index = 0;
		static struct option long_options[] = {
			{ "help", no_argument, NULL, 'h' },
			{ "version", no_argument, NULL, 'v' },
			{ "host", required_argument, NULL, 'H' },
			{ "port", required_argument, NULL, 'P' },
			{ "bind.host", required_argument, NULL, OPT_CLI_BIND_HOST },
			{ "bind.port", required_argument, NULL, OPT_CLI_BIND_PORT },
			{ "type", required_argument, NULL, 't' },
			{ NULL, 0, NULL, 0 }
		};

		c = getopt_long(argc, argv, "hvH:P:t:", long_options, &option_index);
		if (c == -1) {
			break;
		}

		switch (c) {
		case 'h': {
			fprintf(stdout, str_usage, argv[0], argv[0], argv[0]);
			exit(EXIT_SUCCESS);
		} break;
		case 'v': {
			fprintf(stdout, "mugglec: %s\n", MUGGLE_C_VERSION);
			exit(EXIT_SUCCESS);
		} break;
		case 'H': {
			strncpy(cfg->host, optarg, sizeof(cfg->host) - 1);
		} break;
		case 'P': {
			strncpy(cfg->port, optarg, sizeof(cfg->port) - 1);
		} break;
		case 't': {
			strncpy(cfg->app_type, optarg, sizeof(cfg->app_type) - 1);
		} break;
		case OPT_CLI_BIND_HOST: {
			strncpy(cfg->bind_host, optarg, sizeof(cfg->bind_host) - 1);
		} break;
		case OPT_CLI_BIND_PORT: {
			strncpy(cfg->bind_port, optarg, sizeof(cfg->bind_port) - 1);
		} break;
		default: {
			fprintf(stderr, "unrecognized opt: %s", optarg);
		} break;
		}
	}

	return true;
}
