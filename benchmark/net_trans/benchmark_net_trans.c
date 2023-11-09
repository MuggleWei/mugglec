#include "trans_message.h"
#include "tcp_client.h"
#include "tcp_serv.h"
#include "udp_receiver.h"
#include "udp_sender.h"
#include "muggle/c/muggle_c.h"
#include "muggle_benchmark/muggle_benchmark.h"

typedef struct args {
	char role[16];
	char host[16];
	char port[16];
	int is_multiplexing;
	int is_busy;
	int round;
	int cnt_per_round;
	int round_interval; // nano seconds
} args_t;

#define OPT_ROUND 1001
#define OPT_CNT_PER_ROUND 1002
#define OPT_ROUND_INTERVAL 1003

bool parse_args(int argc, char **argv, args_t *args)
{
	int c;

	memset(args, 0, sizeof(*args));
	strncpy(args->role, "null", sizeof(args->role) - 1);
	strncpy(args->host, "127.0.0.1", sizeof(args->host) - 1);
	strncpy(args->port, "10102", sizeof(args->port) - 1);
	args->is_multiplexing = 0;
	args->is_busy = 1;
	args->round = 100;
	args->cnt_per_round = 10;
	args->round_interval = 1;

	while (1) {
		int option_index = 0;
		static struct option long_options[] = {
			{ "help", no_argument, NULL, 'h' },
			{ "role", required_argument, NULL, 'r' },
			{ "host", required_argument, NULL, 'H' },
			{ "port", required_argument, NULL, 'P' },
			{ "multiplexing", required_argument, NULL, 'm' },
			{ "busy", required_argument, NULL, 'b' },
			{ "round", required_argument, NULL, OPT_ROUND },
			{ "cnt_per_round", required_argument, NULL, OPT_CNT_PER_ROUND },
			{ "interval", required_argument, NULL, OPT_ROUND_INTERVAL },
			{ NULL, 0, NULL, 0 }
		};

		c = getopt_long(argc, argv, "hr:m:H:P:", long_options, &option_index);
		if (c == -1) {
			break;
		}

		switch (c) {
		case 'h': {
			fprintf(stdout,
					"Usage: %s <options>\n"
					"    -r, --role           "
					"udp-send|udp-recv|tcp-serv|tcp-client\n"
					"    -H, --host           server host\n"
					"    -P, --port           server port\n"
					"    -m, --multiplexing   on or off\n"
					"    -b, --busy           on or off\n"
					"      , --round          round of run\n"
					"      , --cnt_per_round  number of message of per round\n"
					"      , --interval       round interval (milli-seconds)\n"
					"",
					argv[0]);
			exit(EXIT_SUCCESS);
		} break;
		case 'r': {
			strncpy(args->role, optarg, sizeof(args->role) - 1);
		} break;
		case 'H': {
			strncpy(args->host, optarg, sizeof(args->host) - 1);
		} break;
		case 'P': {
			strncpy(args->port, optarg, sizeof(args->port) - 1);
		} break;
		case 'm': {
			if (strcmp(optarg, "on") == 0 || strcmp(optarg, "ON") == 0) {
				args->is_multiplexing = 1;
			} else if (strcmp(optarg, "off") == 0 ||
					   strcmp(optarg, "OFF") == 0) {
				args->is_multiplexing = 0;
			} else {
				LOG_ERROR("invalid 'model' value: %s", optarg);
				return false;
			}
		} break;
		case 'b': {
			if (strcmp(optarg, "on") == 0 || strcmp(optarg, "ON") == 0) {
				args->is_busy = 1;
			} else if (strcmp(optarg, "off") == 0 ||
					   strcmp(optarg, "OFF") == 0) {
				args->is_busy = 0;
			} else {
				LOG_ERROR("invalid 'busy' value: %s", optarg);
				return false;
			}
		} break;
		case OPT_ROUND: {
			uint32_t v = 0;
			if (muggle_str_tou(optarg, &v, 10)) {
				args->round = (int)v;
			} else {
				LOG_ERROR("invalid 'round' value: %s", optarg);
				return false;
			}
		} break;
		case OPT_CNT_PER_ROUND: {
			uint32_t v = 0;
			if (muggle_str_tou(optarg, &v, 10)) {
				args->cnt_per_round = (int)v;
			} else {
				LOG_ERROR("invalid 'cnt_per_round' value: %s", optarg);
				return false;
			}
		} break;
		case OPT_ROUND_INTERVAL: {
			uint32_t v = 0;
			if (muggle_str_tou(optarg, &v, 10)) {
				args->round_interval = (int)v;
			} else {
				LOG_ERROR("invalid 'interval' value: %s", optarg);
				return false;
			}
		} break;
		}
	}

	fprintf(stdout,
			"----------------\n"
			"role:          %s\n"
			"host:          %s\n"
			"port:          %s\n"
			"multiplexing:  %s\n"
			"busy:          %s\n"
			"round:         %d\n"
			"cnt_per_round: %d\n"
			"interval:      %d ms\n"
			"----------------\n"
			"",
			args->role, args->host, args->port,
			args->is_multiplexing ? "ON" : "OFF", args->is_busy ? "ON" : "OFF",
			args->round, args->cnt_per_round, args->round_interval);

	return true;
}

void gen_report(muggle_benchmark_handle_t *handle,
				muggle_benchmark_config_t *config, int action1, int action2,
				char *name)
{
	// generate records report
	char records_filepath[512];
	memset(records_filepath, 0, sizeof(records_filepath));
	snprintf(records_filepath, sizeof(records_filepath),
			 "benchmark_%s_records.csv", name);
	FILE *fp_records = fopen(records_filepath, "wb");
	muggle_benchmark_handle_gen_records_report(handle, fp_records,
											   config->elapsed_unit);
	fclose(fp_records);

	// generate latency report
	char latency_filepath[512];
	memset(latency_filepath, 0, sizeof(latency_filepath));
	snprintf(latency_filepath, sizeof(latency_filepath),
			 "benchmark_%s_latency.csv", name);
	FILE *fp_latency = fopen(latency_filepath, "wb");
	muggle_benchmark_gen_latency_report_head(fp_latency, config);
	muggle_benchmark_gen_latency_report_body(fp_latency, handle, config,
											 action1, action2, 0);
	muggle_benchmark_gen_latency_report_body(fp_latency, handle, config,
											 action1, action2, 1);
	fclose(fp_latency);
}

int main(int argc, char *argv[])
{
	// init log
	if (muggle_log_complicated_init(LOG_LEVEL_INFO, LOG_LEVEL_DEBUG, NULL) !=
		0) {
		fprintf(stderr, "failed init log\n");
		exit(EXIT_FAILURE);
	}

	// init socket library
	if (muggle_socket_lib_init() != 0) {
		LOG_ERROR("failed init socket library");
		exit(EXIT_FAILURE);
	}

	// parse arguments
	args_t args;
	if (!parse_args(argc, argv, &args)) {
		LOG_ERROR("failed parse arguments");
		exit(EXIT_FAILURE);
	}

	char name[64];
	snprintf(name, sizeof(name), "%s-%s-%s", args.role,
			 args.is_multiplexing ? "multiplexing" : "single",
			 args.is_busy ? "busy" : "block");

	// benchmark config
	muggle_benchmark_config_t config;
	memset(&config, 0, sizeof(config));
	config.rounds = args.round;
	config.record_per_round = args.cnt_per_round;
	config.round_interval_ms = args.round_interval;
	config.elapsed_unit = MUGGLE_BENCHMARK_ELAPSED_UNIT_NS;
	config.producer = 1;
	config.consumer = 1;
	config.report_step = 10;

	// benchmark handle
	uint64_t record_count =
		config.rounds * config.record_per_round * config.producer;
	muggle_benchmark_handle_t handle;
	muggle_benchmark_handle_init(&handle, record_count, MAX_NET_TRANS_ACTION);

	if (strcmp(args.role, "udp-send") == 0) {
		muggle_benchmark_handle_set_action(&handle, NET_TRANS_ACTION_WRITE_BEG,
										   "write_beg");
		muggle_benchmark_handle_set_action(&handle, NET_TRANS_ACTION_WRITE_END,
										   "write_end");
		run_udp_sender(args.host, args.port, args.is_busy, &handle, &config);
		gen_report(&handle, &config, NET_TRANS_ACTION_WRITE_BEG,
				   NET_TRANS_ACTION_WRITE_END, name);
	} else if (strcmp(args.role, "udp-recv") == 0) {
		muggle_benchmark_handle_set_action(&handle, NET_TRANS_ACTION_WRITE_BEG,
										   "write_beg");
		muggle_benchmark_handle_set_action(&handle, NET_TRANS_ACTION_READ,
										   "read");
		run_udp_receiver(args.host, args.port, args.is_multiplexing,
						 args.is_busy, &handle, &config);
		gen_report(&handle, &config, NET_TRANS_ACTION_WRITE_BEG,
				   NET_TRANS_ACTION_READ, name);
	} else if (strcmp(args.role, "tcp-client") == 0) {
		muggle_benchmark_handle_set_action(&handle, NET_TRANS_ACTION_WRITE_BEG,
										   "write_beg");
		muggle_benchmark_handle_set_action(&handle, NET_TRANS_ACTION_WRITE_END,
										   "write_end");
		run_tcp_client(args.host, args.port, args.is_busy, &handle, &config);
		gen_report(&handle, &config, NET_TRANS_ACTION_WRITE_BEG,
				   NET_TRANS_ACTION_WRITE_END, name);
	} else if (strcmp(args.role, "tcp-serv") == 0) {
		muggle_benchmark_handle_set_action(&handle, NET_TRANS_ACTION_WRITE_BEG,
										   "write_beg");
		muggle_benchmark_handle_set_action(&handle, NET_TRANS_ACTION_READ,
										   "read");
		run_tcp_serv(args.host, args.port, args.is_multiplexing, args.is_busy,
					 &handle, &config);
		gen_report(&handle, &config, NET_TRANS_ACTION_WRITE_BEG,
				   NET_TRANS_ACTION_READ, name);
	} else {
		LOG_ERROR("invalid role: %s", args.role);
		exit(EXIT_FAILURE);
	}

	return 0;
}
