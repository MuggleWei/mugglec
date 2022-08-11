#include "trans_message.h"
#include "udp_sender.h"
#include "udp_receiver.h"
#include "tcp_serv.h"
#include "tcp_client.h"

void gen_report(
	muggle_benchmark_handle_t *handle,
	muggle_benchmark_config_t *config,
	int action1, int action2, char *name)
{
	// generate records report
	char records_filepath[512];
	memset(records_filepath, 0, sizeof(records_filepath));
	snprintf(records_filepath, sizeof(records_filepath), "benchmark_%s_records.csv", name);
	FILE *fp_records = fopen(records_filepath, "wb");
	muggle_benchmark_handle_gen_records_report(handle, fp_records, config->elapsed_unit);
	fclose(fp_records);

	// generate latency report
	char latency_filepath[512];
	memset(latency_filepath, 0, sizeof(latency_filepath));
	snprintf(latency_filepath, sizeof(latency_filepath), "benchmark_%s_latency.csv", name);
	FILE *fp_latency = fopen(latency_filepath, "wb");
	muggle_benchmark_gen_latency_report_head(fp_latency, config);
	muggle_benchmark_gen_latency_report_body(
		fp_latency, handle, config,
		action1,
		action2,
		0);
	muggle_benchmark_gen_latency_report_body(
		fp_latency, handle, config,
		action1,
		action2,
		1);
	fclose(fp_latency);
}

int main(int argc, char *argv[])
{
	// init log
	if (muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO) != 0)
	{
		MUGGLE_LOG_ERROR("failed initalize log");
		exit(EXIT_FAILURE);
	}

	// init socket library
	if (muggle_socket_lib_init() != 0)
	{
		MUGGLE_LOG_ERROR("failed initalize socket library");
		exit(EXIT_FAILURE);
	}

	if (argc < 4)
	{
		MUGGLE_LOG_ERROR("usage: %s <udp-send|udp-recv|tcp-serv|tcp-client> <host> <port> [busy_read|wait_read]", argv[0]);
		exit(EXIT_FAILURE);
	}

	const char *app_type = argv[1];
	const char *host = argv[2];
	const char *port = argv[3];

	int read_flags = 0;
	if (argc >= 5)
	{
		if (strcmp(argv[4], "busy_read") == 0)
		{
#if ! defined(MUGGLE_PLATFORM_WINDOWS)
			read_flags |= MSG_DONTWAIT;
#endif
		}
	}

	// benchmark config
	muggle_benchmark_config_t config;
	memset(&config, 0, sizeof(config));
	config.rounds = 1000;
	config.record_per_round = 1;
	config.round_interval_ms = 5;
	config.elapsed_unit = MUGGLE_BENCHMARK_ELAPSED_UNIT_NS;
	config.producer = 1;
	config.consumer = 1;
	config.report_step = 10;

	// benchmark handle
	uint64_t record_count =
		config.rounds * config.record_per_round * config.producer;

	muggle_benchmark_handle_t handle;
	muggle_benchmark_handle_init(&handle, record_count, MAX_NET_TRANS_ACTION);

	int flags = 0;
	if (strcmp(app_type, "udp-send") == 0)
	{
		muggle_benchmark_handle_set_action(&handle, NET_TRANS_ACTION_WRITE_BEG, "write_beg");
		muggle_benchmark_handle_set_action(&handle, NET_TRANS_ACTION_WRITE_END, "write_end");

		run_udp_sender(host, port, flags, &handle, &config);
		gen_report(
			&handle, &config,
			NET_TRANS_ACTION_WRITE_BEG, NET_TRANS_ACTION_WRITE_END, "udp_send");
	}
	else if (strcmp(app_type, "udp-recv") == 0)
	{
		flags = read_flags;

		muggle_benchmark_handle_set_action(&handle, NET_TRANS_ACTION_WRITE_BEG, "write_beg");
		muggle_benchmark_handle_set_action(&handle, NET_TRANS_ACTION_READ, "read");

		run_udp_receiver(host, port, flags, &handle, &config);
		gen_report(
			&handle, &config,
			NET_TRANS_ACTION_WRITE_BEG, NET_TRANS_ACTION_READ, "udp_recv");
	}
	else if (strcmp(app_type, "tcp-serv") == 0)
	{
		muggle_benchmark_handle_set_action(&handle, NET_TRANS_ACTION_WRITE_BEG, "write_beg");
		muggle_benchmark_handle_set_action(&handle, NET_TRANS_ACTION_WRITE_END, "write_end");

		run_tcp_serv(host, port, flags, &handle, &config);
		gen_report(
			&handle, &config,
			NET_TRANS_ACTION_WRITE_BEG, NET_TRANS_ACTION_WRITE_END, "tcp_send");
	}
	else if (strcmp(app_type, "tcp-client") == 0)
	{
		flags = read_flags;

		muggle_benchmark_handle_set_action(&handle, NET_TRANS_ACTION_WRITE_BEG, "write_beg");
		muggle_benchmark_handle_set_action(&handle, NET_TRANS_ACTION_READ, "read");

		run_tcp_client(host, port, flags, &handle, &config);
		gen_report(
			&handle, &config,
			NET_TRANS_ACTION_WRITE_BEG, NET_TRANS_ACTION_READ, "tcp_recv");
	}
	else
	{
		MUGGLE_LOG_WARNING("invalid app type: %s", app_type);
	}

	// destroy benchmark handle
	muggle_benchmark_handle_destroy(&handle);
	
	return 0;
}
