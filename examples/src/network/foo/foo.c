#include "foo/foo_handle.h"
#include "foo/foo_dispatcher.h"
#include "foo_server_handle.h"
#include "foo_client_handle.h"

enum
{
	APP_TYPE_NULL = 0,
	APP_TYPE_TCP_SERV,
	APP_TYPE_TCP_CLIENT,
	MAX_APP_TYPE,
};

static const char *s_str_app_type[MAX_APP_TYPE] = {
	"unknown", "TCP_SERVER", "TCP_CLIENT"
};

void parse_sys_args(int argc, char **argv, sys_args_t *args)
{
	char str_usage[1024];
	snprintf(str_usage, sizeof(str_usage),
		"Usage: %s -t <app_type> -H <host> -P <port>\n"
		"\t-h print help information\n"
		"\t-v print mugglec version\n"
		"\t-t application type: tcp_serv or tcp_client\n"
		"\t-H listen/connect host\n"
		"\t-P listen/connect port",
		argv[0]);

	memset(args, 0, sizeof(*args));

	int opt;
	while ((opt = getopt(argc, argv, "hvt:H:P:")) != -1)
	{
		switch (opt)
		{
		case 'h':
		{
			LOG_INFO("\n%s", str_usage);
			exit(EXIT_SUCCESS);
		}break;
		case 'v':
		{
			char compile_dt[128];
			mugglec_compile_time_iso8601(compile_dt, sizeof(compile_dt));
			fprintf(stdout, "%s\n%s\n", mugglec_version(), compile_dt);
			exit(EXIT_SUCCESS);
		}break;
		case 't':
		{
			if (strcmp(optarg, "tcp_serv") == 0)
			{
				args->app_type = APP_TYPE_TCP_SERV;
			}
			else if (strcmp(optarg, "tcp_client") == 0)
			{
				args->app_type = APP_TYPE_TCP_CLIENT;
			}
			else
			{
				LOG_ERROR("Failed parse app type: %s\n%s", optarg, str_usage);
				exit(EXIT_FAILURE);
			}
		}break;
		case 'H':
		{
			strncpy(args->host, optarg, sizeof(args->host)-1);
		}break;
		case 'P':
		{
			strncpy(args->port, optarg, sizeof(args->port)-1);
		}break;
		}
	}

	if (args->app_type <= 0 || args->app_type >= MAX_APP_TYPE ||
		args->host[0] == '\0' ||
		args->port[0] == '\0')
	{
		LOG_ERROR("\n%s", str_usage);
		exit(EXIT_FAILURE);
	}

	LOG_INFO(
		"\n"
		"---------------------------\n"
		"app_type=%s\n"
		"host=%s\n"
		"port=%s",
		s_str_app_type[args->app_type],
		args->host, args->port);
}

int main(int argc, char *argv[])
{
	// initialize log
	if (muggle_log_complicated_init(LOG_LEVEL_INFO, -1, NULL) != 0)
	{
		LOG_ERROR("failed initalize log");
		exit(EXIT_FAILURE);
	}

	// initialize socket library
	if (muggle_socket_lib_init() != 0)
	{
		LOG_ERROR("failed initalize socket library");
		exit(EXIT_FAILURE);
	}

	// parse input arguments
	sys_args_t args;
	parse_sys_args(argc, argv, &args);

	// init event loop
	muggle_event_loop_init_args_t ev_init_args;
	memset(&ev_init_args, 0, sizeof(ev_init_args));
	ev_init_args.evloop_type = MUGGLE_EVLOOP_TYPE_NULL;
	ev_init_args.hints_max_fd = CONN_NUM_LIMIT * 2;
	ev_init_args.use_mem_pool = 0;

	muggle_event_loop_t *evloop = muggle_evloop_new(&ev_init_args);
	if (evloop == NULL)
	{
		LOG_ERROR("failed new event loop");
		exit(EXIT_FAILURE);
	}
	LOG_INFO("success new event loop");

	// init socket event loop handle
	muggle_socket_evloop_handle_t handle;
	switch (args.app_type)
	{
	case APP_TYPE_TCP_SERV:
	{
		init_tcp_server_handle(evloop, &handle, &args);

		// register message callbacks
		foo_dispatcher_t *dispatcher = foo_handle_dipatcher(evloop);
		foo_dispatcher_register(dispatcher, FOO_MSG_TYPE_REQ_LOGIN, tcp_server_on_req_login);
		foo_dispatcher_register(dispatcher, FOO_MSG_TYPE_PONG, tcp_server_on_msg_pong);
		foo_dispatcher_register(dispatcher, FOO_MSG_TYPE_REQ_SUM, tcp_server_on_req_sum);
	}break;
	case APP_TYPE_TCP_CLIENT:
	{
		init_tcp_client_handle(evloop, &handle, &args);

		// register message callbacks
		foo_dispatcher_t *dispatcher = foo_handle_dipatcher(evloop);
		foo_dispatcher_register(dispatcher, FOO_MSG_TYPE_RSP_LOGIN, tcp_client_on_rsp_login);
		foo_dispatcher_register(dispatcher, FOO_MSG_TYPE_PING, tcp_client_on_msg_ping);
		foo_dispatcher_register(dispatcher, FOO_MSG_TYPE_RSP_SUM, tcp_server_on_rsp_sum);
	}break;
	default:
	{
		LOG_ERROR("invalid app type, exit");
		exit(EXIT_FAILURE);
	}break;
	}

	// run listen or connect socket
	muggle_evloop_run(evloop);

	// destroy
	foo_handle_destroy(evloop);
	muggle_socket_evloop_handle_destroy(&handle);
	muggle_evloop_delete(evloop);

	return 0;
}
