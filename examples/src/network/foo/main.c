#include "foo_config.h"
#include "foo_handle.h"
#include "foo_server.h"
#include "foo_client.h"

static int customize_log_fmt(const muggle_log_msg_t *msg, char *buf,
							 size_t bufsize)
{
	const char *level = muggle_log_level_to_str(msg->level);

	char filename[MUGGLE_MAX_PATH];
	muggle_path_basename(msg->src_loc.file, filename, sizeof(filename));

	struct tm t;
	localtime_r(&msg->ts.tv_sec, &t);

	const char *payload = "";
	if (msg->payload) {
		payload = msg->payload;
	}

	return (int)snprintf(
		buf, bufsize,
		"%s|%d-%02d-%02dT%02d:%02d:%02d.%09d|%s:%u|%s|%llu - %s\n", level,
		(int)t.tm_year + 1900, (int)t.tm_mon + 1, (int)t.tm_mday,
		(int)t.tm_hour, (int)t.tm_min, (int)t.tm_sec, (int)msg->ts.tv_nsec,
		filename, (unsigned int)msg->src_loc.line, msg->src_loc.func,
		(unsigned long long)msg->tid, payload);
}

static void init_log(const char *app_type)
{
	static muggle_log_fmt_t formatter = {
		MUGGLE_LOG_FMT_LEVEL | MUGGLE_LOG_FMT_FILE | MUGGLE_LOG_FMT_FUNC |
			MUGGLE_LOG_FMT_TIME | MUGGLE_LOG_FMT_THREAD,
		customize_log_fmt
	};
	const int console_level = LOG_LEVEL_DEBUG;
	const int file_level = LOG_LEVEL_DEBUG;

	muggle_logger_t *logger = muggle_logger_default();

	// console log
	static muggle_log_console_handler_t console_handler;
	do {
		muggle_log_console_handler_init(&console_handler, 1);
		muggle_log_handler_set_level((muggle_log_handler_t *)&console_handler,
									 console_level);
		muggle_log_handler_set_fmt((muggle_log_handler_t *)&console_handler,
								   &formatter);
		logger->add_handler(logger, (muggle_log_handler_t *)&console_handler);
	} while (0);

	// file log
	static muggle_log_file_handler_t file_handler;
	do {
		char filepath[256];
		snprintf(filepath, sizeof(filepath), "logs/foo_%s.log", app_type);
		int ret = muggle_log_file_handler_init(&file_handler, filepath, "a");
		if (ret != 0) {
			fprintf(stderr, "failed init file handler with path: %s\n",
					filepath);
			break;
		}

		muggle_log_handler_set_level((muggle_log_handler_t *)&file_handler,
									 file_level);
		muggle_log_handler_set_fmt((muggle_log_handler_t *)&file_handler,
								   &formatter);
		logger->add_handler(logger, (muggle_log_handler_t *)&file_handler);
	} while (0);
}

void output_welcome_info(foo_config_t *cfg)
{
	char compile_time_buf[64];
	mugglec_compile_time_iso8601(compile_time_buf, sizeof(compile_time_buf));
	LOG_INFO("mugglec %s (%s)", MUGGLE_C_VERSION, compile_time_buf);
	LOG_INFO("----------------");
	LOG_INFO("launch foo");
	LOG_INFO("  host: %s", cfg->host);
	LOG_INFO("  port: %s", cfg->port);
	LOG_INFO("  bind.host: %s", cfg->bind_host);
	LOG_INFO("  bind.port: %s", cfg->bind_port);
	LOG_INFO("  app_type: %s", cfg->app_type);
}

void run(foo_config_t *cfg)
{
	output_welcome_info(cfg);

	// initialize event loop
	muggle_event_loop_init_args_t ev_init_args;
	memset(&ev_init_args, 0, sizeof(ev_init_args));
	ev_init_args.evloop_type = MUGGLE_EVLOOP_TYPE_NULL;
	ev_init_args.hints_max_fd = 512;
	ev_init_args.use_mem_pool = 1;

	muggle_event_loop_t *evloop = muggle_evloop_new(&ev_init_args);
	if (evloop == NULL) {
		LOG_ERROR("failed new event loop");
		return;
	}
	LOG_INFO("success new event loop");

	// initialize foo handle
	foo_handle_t handle;
	if (!foo_handle_init(&handle, evloop, cfg)) {
		LOG_ERROR("failed init handle");
		return;
	}

	// run
	if (strcmp(cfg->app_type, "server") == 0) {
		foo_server_run(evloop);
	} else if (strcmp(cfg->app_type, "client") == 0) {
		foo_client_run(evloop);
	} else {
		LOG_ERROR("invalid app type: %s", cfg->app_type);
	}

	// cleanup
	muggle_evloop_delete(evloop);
}

int main(int argc, char *argv[])
{
	// parse input arguments
	foo_config_t cfg;
	if (!parse_sys_args(argc, argv, &cfg)) {
		fprintf(stderr, "failed parse sys arguments\n");
		exit(EXIT_FAILURE);
	}

	// initialize log
	init_log(cfg.app_type);

	// initialize socket library
	if (muggle_socket_lib_init() != 0) {
		LOG_ERROR("failed init socket library");
		exit(EXIT_FAILURE);
	}

	run(&cfg);

	return 0;
}
