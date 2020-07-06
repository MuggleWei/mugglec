#include "gen_report.h"
#include "trans_message.h"

void gen_benchmark_report(const char *name, muggle_benchmark_block_t *blocks, int cnt)
{
	muggle_benchmark_config_t config;
	strncpy(config.name, name, sizeof(config.name)-1);
	config.loop = TRANS_PKG_ROUND;
	config.cnt_per_loop = PKG_PER_ROUND;
	config.loop_interval_ms = ROUND_INTERVAL_MS;
	config.report_step = 10;

	char file_name[128];
	snprintf(file_name, sizeof(file_name)-1, "benchmark_%s.csv", config.name);
	FILE *fp = fopen(file_name, "wb");
	if (fp == NULL)
	{
		printf("failed open file: %s\n", file_name);
		exit(1);
	}

	muggle_benchmark_gen_reports_head(fp, &config);
	muggle_benchmark_gen_reports_body(fp, &config, blocks, "sort by idx", cnt, 0, 1, 0);
	muggle_benchmark_gen_reports_body(fp, &config, blocks, "sort by elapsed", cnt, 0, 1, 1);
}
