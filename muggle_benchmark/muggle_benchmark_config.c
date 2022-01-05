#include "muggle_benchmark_config.h"

void muggle_benchmark_config_parse_cli(
	muggle_benchmark_config_t *config,
	int argc, char *argv[])
{
	memset(config, 0, sizeof(*config));
	config->rounds = 100;
	config->record_per_round = 10;
	config->round_interval_ms = 1;
	config->elapsed_unit = MUGGLE_BENCHMARK_ELAPSED_UNIT_NS;
	config->capacity = 1024;
	config->block_size = 64;
	config->producer = 0;
	config->consumer = 1;
	config->report_step = 10;

	int opt;
	unsigned long long val = 0;
	while ((opt = getopt(argc, argv, "r:m:i:e:p:c:s:f:hv")) != -1)
	{
		switch (opt)
		{
		case 'r':
		{
			muggle_str_toull(optarg, &val, 0);
			config->rounds = atoi(optarg);
		} break;
		case 'm':
		{
			muggle_str_toull(optarg, &val, 0);
			config->record_per_round = atoi(optarg);
		} break;
		case 'i':
		{
			muggle_str_toi(optarg, &config->round_interval_ms, 0);
		} break;
		case 'e':
		{
			if (strcmp(optarg, "cpu") == 0)
			{
				config->elapsed_unit = MUGGLE_BENCHMARK_ELAPSED_UNIT_CPU_CYCLE;
			}
			else
			{
				config->elapsed_unit = MUGGLE_BENCHMARK_ELAPSED_UNIT_NS;
			}
		} break;
		case 's':
		{
			muggle_str_toi(optarg, &config->capacity, 0);
		} break;
		case 'b':
		{
			muggle_str_toi(optarg, &config->block_size, 0);
		} break;
		case 'p':
		{
			muggle_str_toi(optarg, &config->producer, 0);
		} break;
		case 'c':
		{
			muggle_str_toi(optarg, &config->consumer, 0);
		} break;
		case 'f':
		{
			muggle_str_toi(optarg, &config->report_step, 0);
		} break;
		case 'v':
		{
			printf("mugglec version: %s\n", mugglec_version());
			exit(EXIT_SUCCESS);
		} break;
		case 'h':
		{
			printf(
				"Usage of %s:\n"
				"  -r uint\n"
				"    rounds\n"
				"  -m uint\n"
				"    record per round\n"
				"  -i int\n"
				"    round interval milliseconds\n"
				"  -e [cpu|ts]\n"
				"    elapsed unit\n"
				"  -s int\n"
				"    capacity\n"
				"  -b int\n"
				"    block_size\n"
				"  -p int\n"
				"    number of producer\n"
				"  -c int\n"
				"    number of consumer\n"
				"  -f int\n"
				"    report step\n"
				, argv[0]);
			exit(EXIT_SUCCESS);
		} break;
		}
	}

	if (config->producer == 0)
	{
		int hc = (int)muggle_thread_hardware_concurrency();
		hc /= 2;
		if (hc <= 1)
		{
			hc = 2;
		}
		config->producer = hc;
	}
}

void muggle_benchmark_config_output(muggle_benchmark_config_t *config)
{
	MUGGLE_LOG_INFO("rounds: %llu", (unsigned long long)config->rounds);
	MUGGLE_LOG_INFO("record per round: %llu", (unsigned long long)config->record_per_round);
	MUGGLE_LOG_INFO("interval ms between round: %d", config->round_interval_ms);
	MUGGLE_LOG_INFO("elapsed unit: %s", config->elapsed_unit == MUGGLE_BENCHMARK_ELAPSED_UNIT_NS ? "ns" : "cpu_cycle");
	MUGGLE_LOG_INFO("capacity: %d", config->capacity);
	MUGGLE_LOG_INFO("block size: %d", config->block_size);
	MUGGLE_LOG_INFO("number of producer: %d", config->producer);
	MUGGLE_LOG_INFO("number of consumer: %d", config->consumer);
	MUGGLE_LOG_INFO("report step: %d", config->report_step);
}
